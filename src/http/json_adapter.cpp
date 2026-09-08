#include "inferlite/http/json_adapter.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

namespace inferlite {
namespace {

using Json = nlohmann::json;

Status InvalidJson(std::string message) {
    return Status::InvalidArgument("invalid inference JSON: " + std::move(message));
}

StatusOr<std::string> ReadRequiredString(const Json& object, const char* field,
                                         std::string_view path) {
    const auto iterator = object.find(field);
    if (iterator == object.end()) {//检查是否存在指定的字段，如果不存在则返回一个Status对象，表示JSON无效，并附带错误信息
        return StatusOr<std::string>(InvalidJson(std::string(path) + "." + field + " is required"));
    }
    if (!iterator->is_string()) {
        return StatusOr<std::string>(
            InvalidJson(std::string(path) + "." + field + " must be a string"));//将string_view类型的path转换为std::string类型，并拼接上字段名，形成完整的路径信息，然后返回一个Status对象，表示JSON无效，并附带错误信息
    }
    return StatusOr<std::string>(iterator->get<std::string>());
}

StatusOr<Shape> ParseShape(const Json& value, std::string_view path) {//判断
    if (!value.is_array()) {
        return StatusOr<Shape>(InvalidJson(std::string(path) + " must be an array"));
    }

    Shape shape;
    shape.reserve(value.size());
    for (std::size_t index = 0; index < value.size(); ++index) {
        const Json& dimension = value[index];
        if (!dimension.is_number_integer() && !dimension.is_number_unsigned()) {
            return StatusOr<Shape>(InvalidJson(std::string(path) + "[" + std::to_string(index) +
                                               "] must be an integer"));
        }

        if (dimension.is_number_unsigned()) {
            const auto unsigned_value = dimension.get<std::uint64_t>();
            if (unsigned_value >
                static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
                return StatusOr<Shape>(InvalidJson(std::string(path) + "[" + std::to_string(index) +
                                                   "] exceeds int64 range"));
            }
            shape.push_back(static_cast<std::int64_t>(unsigned_value));
            continue;
        }

        shape.push_back(dimension.get<std::int64_t>());
    }

    return StatusOr<Shape>(std::move(shape));
}

StatusOr<TensorData> ParseData(const Json& value, std::string_view path) {
    if (!value.is_array()) {
        return StatusOr<TensorData>(InvalidJson(std::string(path) + " must be an array"));
    }

    TensorData data;
    data.reserve(value.size());
    for (std::size_t index = 0; index < value.size(); ++index) {
        const Json& element = value[index];
        if (!element.is_number()) {
            return StatusOr<TensorData>(InvalidJson(std::string(path) + "[" +
                                                    std::to_string(index) + "] must be a number"));
        }

        const double value_as_double = element.get<double>();
        if (!std::isfinite(value_as_double) ||
            value_as_double > static_cast<double>(std::numeric_limits<float>::max()) ||
            value_as_double < static_cast<double>(std::numeric_limits<float>::lowest())) {
            return StatusOr<TensorData>(InvalidJson(std::string(path) + "[" +
                                                    std::to_string(index) +
                                                    "] is outside the finite float range"));
        }
        data.push_back(static_cast<float>(value_as_double));
    }

    return StatusOr<TensorData>(std::move(data));
}

StatusOr<NamedTensor> ParseNamedTensor(const Json& value, std::size_t index) {
    const std::string path = "inputs[" + std::to_string(index) + "]";
    if (!value.is_object()) {
        return StatusOr<NamedTensor>(InvalidJson(path + " must be an object"));
    }

    StatusOr<std::string> name = ReadRequiredString(value, "name", path);
    if (!name.ok()) {
        return StatusOr<NamedTensor>(name.status());
    }

    const auto shape_iterator = value.find("shape");
    if (shape_iterator == value.end()) {
        return StatusOr<NamedTensor>(InvalidJson(path + ".shape is required"));
    }
    StatusOr<Shape> shape = ParseShape(*shape_iterator, path + ".shape");
    if (!shape.ok()) {
        return StatusOr<NamedTensor>(shape.status());
    }

    const auto data_iterator = value.find("data");
    if (data_iterator == value.end()) {
        return StatusOr<NamedTensor>(InvalidJson(path + ".data is required"));
    }
    StatusOr<TensorData> data = ParseData(*data_iterator, path + ".data");
    if (!data.ok()) {
        return StatusOr<NamedTensor>(data.status());
    }

    StatusOr<Tensor> tensor = Tensor::Create(std::move(shape).value(), std::move(data).value());
    if (!tensor.ok()) {
        return StatusOr<NamedTensor>(tensor.status());
    }

    return StatusOr<NamedTensor>(NamedTensor{std::move(name).value(), std::move(tensor).value()});
}

} // namespace

StatusOr<InferenceRequest> ParseInferenceRequestJson(std::string_view json_text) {
    Json root;
    try {
        root = Json::parse(json_text.begin(), json_text.end());
    } catch (const Json::exception& error) {
        return StatusOr<InferenceRequest>(InvalidJson(std::string("parse error: ") + error.what()));
    }

    if (!root.is_object()) {
        return StatusOr<InferenceRequest>(InvalidJson("root must be an object"));
    }

    StatusOr<std::string> model_name = ReadRequiredString(root, "model", "root");
    if (!model_name.ok()) {
        return StatusOr<InferenceRequest>(model_name.status());
    }

    const auto inputs_iterator = root.find("inputs");
    if (inputs_iterator == root.end()) {
        return StatusOr<InferenceRequest>(InvalidJson("root.inputs is required"));
    }
    if (!inputs_iterator->is_array()) {
        return StatusOr<InferenceRequest>(InvalidJson("root.inputs must be an array"));
    }

    std::vector<NamedTensor> inputs;
    inputs.reserve(inputs_iterator->size());
    for (std::size_t index = 0; index < inputs_iterator->size(); ++index) {
        StatusOr<NamedTensor> input = ParseNamedTensor((*inputs_iterator)[index], index);
        if (!input.ok()) {
            return StatusOr<InferenceRequest>(input.status());
        }
        inputs.push_back(std::move(input).value());
    }

    return InferenceRequest::Create(std::move(model_name).value(), std::move(inputs));
}

StatusOr<std::string> SerializeInferenceResultJson(const InferenceResult& result) {
    Json outputs = Json::array();
    for (const NamedTensor& output : result.outputs()) {
        Json data = Json::array();
        for (const float value : output.tensor.data()) {
            if (!std::isfinite(value)) {
                return StatusOr<std::string>(
                    Status::Internal("cannot serialize non-finite tensor data"));
            }
            data.push_back(value);
        }

        outputs.push_back(Json{
            {"name", output.name}, {"shape", output.tensor.shape()}, {"data", std::move(data)}});
    }

    Json root{{"outputs", std::move(outputs)}};

    try {
        return StatusOr<std::string>(root.dump());
    } catch (const Json::exception& error) {
        return StatusOr<std::string>(
            Status::Internal(std::string("JSON result serialization failed: ") + error.what()));
    }
}

} // namespace inferlite
