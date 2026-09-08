#include <gtest/gtest.h>

#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include <nlohmann/json.hpp>

#include "inferlite/core/inference_result.h"
#include "inferlite/http/json_adapter.h"

namespace inferlite {
namespace {

Tensor MakeTensor(Shape shape, TensorData data) {
    StatusOr<Tensor> result = Tensor::Create(std::move(shape), std::move(data));
    if (!result.ok()) {
        throw std::logic_error("test Tensor creation failed");
    }
    return std::move(result).value();
}

NamedTensor MakeNamedTensor(std::string name, Shape shape, TensorData data) {
    return NamedTensor{std::move(name), MakeTensor(std::move(shape), std::move(data))};
}

void ExpectInvalidArgumentContaining(const StatusOr<InferenceRequest>& result,
                                     std::string_view text) {
    ASSERT_FALSE(result.ok());
    EXPECT_EQ(result.status().code(), ErrorCode::kInvalidArgument);
    EXPECT_NE(result.status().message().find(text), std::string::npos) << result.status().message();
}

TEST(JsonAdapterTest, ParsesSingleInputRequest) {
    const std::string json = R"({
        "model": "demo_model",
        "inputs": [
            {"name": "x", "shape": [2], "data": [1.5, -2.0]}
        ]
    })";

    StatusOr<InferenceRequest> result = ParseInferenceRequestJson(json);

    ASSERT_TRUE(result.ok()) << result.status().message();
    EXPECT_EQ(result.value().model_name(), "demo_model");
    ASSERT_EQ(result.value().input_count(), 1U);
    EXPECT_EQ(result.value().inputs()[0].name, "x");
    EXPECT_EQ(result.value().inputs()[0].tensor.shape(), Shape({2}));
    EXPECT_EQ(result.value().inputs()[0].tensor.data(), TensorData({1.5F, -2.0F}));
}

TEST(JsonAdapterTest, PreservesInputOrderAndSupportsScalarAndZeroElementTensor) {
    const std::string json = R"({
        "model": "mixed_model",
        "inputs": [
            {"name": "scalar", "shape": [], "data": [3.0]},
            {"name": "empty", "shape": [2, 0, 4], "data": []}
        ]
    })";

    StatusOr<InferenceRequest> result = ParseInferenceRequestJson(json);

    ASSERT_TRUE(result.ok()) << result.status().message();
    ASSERT_EQ(result.value().inputs().size(), 2U);
    EXPECT_EQ(result.value().inputs()[0].name, "scalar");
    EXPECT_TRUE(result.value().inputs()[0].tensor.shape().empty());
    EXPECT_EQ(result.value().inputs()[0].tensor.data(), TensorData({3.0F}));
    EXPECT_EQ(result.value().inputs()[1].name, "empty");
    EXPECT_EQ(result.value().inputs()[1].tensor.shape(), Shape({2, 0, 4}));
    EXPECT_TRUE(result.value().inputs()[1].tensor.data().empty());
}

TEST(JsonAdapterTest, RejectsMalformedJson) {
    const StatusOr<InferenceRequest> result =
        ParseInferenceRequestJson(R"({"model":"demo","inputs":[})");

    ExpectInvalidArgumentContaining(result, "parse error");
}

TEST(JsonAdapterTest, RejectsNonObjectRoot) {
    const StatusOr<InferenceRequest> result = ParseInferenceRequestJson("[]");

    ExpectInvalidArgumentContaining(result, "root must be an object");
}

TEST(JsonAdapterTest, RejectsMissingModel) {
    const StatusOr<InferenceRequest> result = ParseInferenceRequestJson(R"({"inputs":[]})");

    ExpectInvalidArgumentContaining(result, "root.model is required");
}

TEST(JsonAdapterTest, RejectsNonStringModel) {
    const StatusOr<InferenceRequest> result =
        ParseInferenceRequestJson(R"({"model":42,"inputs":[]})");

    ExpectInvalidArgumentContaining(result, "root.model must be a string");
}

TEST(JsonAdapterTest, RejectsEmptyModelThroughCoreValidation) {
    const StatusOr<InferenceRequest> result =
        ParseInferenceRequestJson(R"({"model":"","inputs":[{"name":"x","shape":[1],"data":[1]}]})");

    ExpectInvalidArgumentContaining(result, "model name cannot be empty");
}

TEST(JsonAdapterTest, RejectsMissingInputs) {
    const StatusOr<InferenceRequest> result = ParseInferenceRequestJson(R"({"model":"demo"})");

    ExpectInvalidArgumentContaining(result, "root.inputs is required");
}

TEST(JsonAdapterTest, RejectsNonArrayInputs) {
    const StatusOr<InferenceRequest> result =
        ParseInferenceRequestJson(R"({"model":"demo","inputs":{}})");

    ExpectInvalidArgumentContaining(result, "root.inputs must be an array");
}

TEST(JsonAdapterTest, RejectsEmptyInputsThroughCoreValidation) {
    const StatusOr<InferenceRequest> result =
        ParseInferenceRequestJson(R"({"model":"demo","inputs":[]})");

    ExpectInvalidArgumentContaining(result, "request inputs cannot be empty");
}

TEST(JsonAdapterTest, RejectsNonObjectInput) {
    const StatusOr<InferenceRequest> result =
        ParseInferenceRequestJson(R"({"model":"demo","inputs":[1]})");

    ExpectInvalidArgumentContaining(result, "inputs[0] must be an object");
}

TEST(JsonAdapterTest, RejectsMissingAndNonStringInputNames) {
    const StatusOr<InferenceRequest> missing =
        ParseInferenceRequestJson(R"({"model":"demo","inputs":[{"shape":[1],"data":[1]}]})");
    const StatusOr<InferenceRequest> wrong_type = ParseInferenceRequestJson(
        R"({"model":"demo","inputs":[{"name":1,"shape":[1],"data":[1]}]})");

    ExpectInvalidArgumentContaining(missing, "inputs[0].name is required");
    ExpectInvalidArgumentContaining(wrong_type, "inputs[0].name must be a string");
}

TEST(JsonAdapterTest, RejectsEmptyInputNameThroughCoreValidation) {
    const StatusOr<InferenceRequest> result = ParseInferenceRequestJson(
        R"({"model":"demo","inputs":[{"name":"","shape":[1],"data":[1]}]})");

    ExpectInvalidArgumentContaining(result, "input name is empty");
}

TEST(JsonAdapterTest, RejectsMissingOrNonArrayShape) {
    const StatusOr<InferenceRequest> missing =
        ParseInferenceRequestJson(R"({"model":"demo","inputs":[{"name":"x","data":[1]}]})");
    const StatusOr<InferenceRequest> wrong_type = ParseInferenceRequestJson(
        R"({"model":"demo","inputs":[{"name":"x","shape":1,"data":[1]}]})");

    ExpectInvalidArgumentContaining(missing, "inputs[0].shape is required");
    ExpectInvalidArgumentContaining(wrong_type, "inputs[0].shape must be an array");
}

TEST(JsonAdapterTest, RejectsNonIntegerAndOutOfRangeShapeDimensions) {
    const StatusOr<InferenceRequest> fractional = ParseInferenceRequestJson(
        R"({"model":"demo","inputs":[{"name":"x","shape":[1.5],"data":[1]}]})");
    const StatusOr<InferenceRequest> out_of_range = ParseInferenceRequestJson(
        R"({"model":"demo","inputs":[{"name":"x","shape":[18446744073709551615],"data":[]}]})");

    ExpectInvalidArgumentContaining(fractional, "inputs[0].shape[0] must be an integer");
    ExpectInvalidArgumentContaining(out_of_range, "inputs[0].shape[0] exceeds int64 range");
}

TEST(JsonAdapterTest, RejectsNegativeDimensionThroughTensorValidation) {
    const StatusOr<InferenceRequest> result = ParseInferenceRequestJson(
        R"({"model":"demo","inputs":[{"name":"x","shape":[2,-1],"data":[]}]})");

    ExpectInvalidArgumentContaining(result, "Tensor dimension is negative");
}

TEST(JsonAdapterTest, RejectsMissingOrNonArrayData) {
    const StatusOr<InferenceRequest> missing =
        ParseInferenceRequestJson(R"({"model":"demo","inputs":[{"name":"x","shape":[1]}]})");
    const StatusOr<InferenceRequest> wrong_type = ParseInferenceRequestJson(
        R"({"model":"demo","inputs":[{"name":"x","shape":[1],"data":1}]})");

    ExpectInvalidArgumentContaining(missing, "inputs[0].data is required");
    ExpectInvalidArgumentContaining(wrong_type, "inputs[0].data must be an array");
}

TEST(JsonAdapterTest, RejectsNonNumericAndOutOfFloatRangeData) {
    const StatusOr<InferenceRequest> non_numeric = ParseInferenceRequestJson(
        R"({"model":"demo","inputs":[{"name":"x","shape":[1],"data":[true]}]})");
    const StatusOr<InferenceRequest> out_of_range = ParseInferenceRequestJson(
        R"({"model":"demo","inputs":[{"name":"x","shape":[1],"data":[3.5e39]}]})");

    ExpectInvalidArgumentContaining(non_numeric, "inputs[0].data[0] must be a number");
    ExpectInvalidArgumentContaining(out_of_range,
                                    "inputs[0].data[0] is outside the finite float range");
}

TEST(JsonAdapterTest, RejectsShapeAndDataSizeMismatchThroughTensorValidation) {
    const StatusOr<InferenceRequest> result = ParseInferenceRequestJson(
        R"({"model":"demo","inputs":[{"name":"x","shape":[2],"data":[1]}]})");

    ExpectInvalidArgumentContaining(result, "Tensor data size mismatch");
}

TEST(JsonAdapterTest, RejectsDuplicateInputNamesThroughRequestValidation) {
    const StatusOr<InferenceRequest> result = ParseInferenceRequestJson(R"({
        "model":"demo",
        "inputs":[
            {"name":"x","shape":[1],"data":[1]},
            {"name":"x","shape":[1],"data":[2]}
        ]
    })");

    ExpectInvalidArgumentContaining(result, "duplicate input name");
}

TEST(JsonAdapterTest, SerializesResultWithOrderedOutputs) {
    StatusOr<InferenceResult> result = InferenceResult::Create(
        {MakeNamedTensor("sum", {2}, {3.0F, 7.5F}), MakeNamedTensor("scalar", {}, {-1.0F})});
    ASSERT_TRUE(result.ok());

    StatusOr<std::string> serialized = SerializeInferenceResultJson(result.value());

    ASSERT_TRUE(serialized.ok()) << serialized.status().message();
    const nlohmann::json root = nlohmann::json::parse(serialized.value());
    ASSERT_TRUE(root.is_object());
    ASSERT_TRUE(root.contains("outputs"));
    ASSERT_EQ(root["outputs"].size(), 2U);
    EXPECT_EQ(root["outputs"][0]["name"], "sum");
    EXPECT_EQ(root["outputs"][0]["shape"], nlohmann::json::array({2}));
    EXPECT_EQ(root["outputs"][0]["data"], nlohmann::json::array({3.0F, 7.5F}));
    EXPECT_EQ(root["outputs"][1]["name"], "scalar");
    EXPECT_EQ(root["outputs"][1]["shape"], nlohmann::json::array());
    EXPECT_EQ(root["outputs"][1]["data"], nlohmann::json::array({-1.0F}));
}

TEST(JsonAdapterTest, RejectsNonFiniteResultData) {
    StatusOr<InferenceResult> result = InferenceResult::Create(
        {MakeNamedTensor("bad", {1}, {std::numeric_limits<float>::infinity()})});
    ASSERT_TRUE(result.ok());

    const StatusOr<std::string> serialized = SerializeInferenceResultJson(result.value());

    ASSERT_FALSE(serialized.ok());
    EXPECT_EQ(serialized.status().code(), ErrorCode::kInternal);
    EXPECT_NE(serialized.status().message().find("non-finite"), std::string::npos);
}

} // namespace
} // namespace inferlite
