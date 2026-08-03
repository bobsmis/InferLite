#include <gtest/gtest.h>

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "inferlite/core/inference_request.h"
#include "inferlite/core/inference_result.h"

namespace inferlite {
namespace {

Tensor MakeTensor(float value) {
    StatusOr<Tensor> result = Tensor::Create({1}, {value});
    if (!result.ok()) {
        throw std::logic_error("test Tensor creation failed");
    }
    return std::move(result).value();
}

NamedTensor MakeNamedTensor(std::string name, float value) {
    return NamedTensor{std::move(name), MakeTensor(value)};
}

TEST(InferenceRequestTest, CreatesSingleInputRequest) {
    StatusOr<InferenceRequest> result =
        InferenceRequest::Create("add_model", {MakeNamedTensor("x", 1.0F)});

    ASSERT_TRUE(result.ok());
    const InferenceRequest& request = result.value();
    EXPECT_EQ(request.model_name(), "add_model");
    EXPECT_EQ(request.input_count(), 1U);
    ASSERT_EQ(request.inputs().size(), 1U);
    EXPECT_EQ(request.inputs()[0].name, "x");
    EXPECT_EQ(request.inputs()[0].tensor.data(), TensorData({1.0F}));
}

TEST(InferenceRequestTest, PreservesMultipleInputOrder) {
    StatusOr<InferenceRequest> result = InferenceRequest::Create(
        "add_model", {MakeNamedTensor("x", 1.0F), MakeNamedTensor("y", 2.0F)});

    ASSERT_TRUE(result.ok());
    ASSERT_EQ(result.value().inputs().size(), 2U);
    EXPECT_EQ(result.value().inputs()[0].name, "x");
    EXPECT_EQ(result.value().inputs()[1].name, "y");
}

TEST(InferenceRequestTest, RejectsEmptyModelName) {
    const StatusOr<InferenceRequest> result =
        InferenceRequest::Create("", {MakeNamedTensor("x", 1.0F)});

    ASSERT_FALSE(result.ok());
    EXPECT_EQ(result.status().code(), ErrorCode::kInvalidArgument);
    EXPECT_NE(result.status().message().find("model name"), std::string::npos);
}

TEST(InferenceRequestTest, RejectsEmptyInputs) {
    const StatusOr<InferenceRequest> result = InferenceRequest::Create("add_model", {});

    ASSERT_FALSE(result.ok());
    EXPECT_EQ(result.status().code(), ErrorCode::kInvalidArgument);
    EXPECT_NE(result.status().message().find("inputs"), std::string::npos);
}

TEST(InferenceRequestTest, RejectsEmptyInputNameWithIndex) {
    const StatusOr<InferenceRequest> result = InferenceRequest::Create(
        "add_model", {MakeNamedTensor("x", 1.0F), MakeNamedTensor("", 2.0F)});

    ASSERT_FALSE(result.ok());
    EXPECT_NE(result.status().message().find("input name is empty"), std::string::npos);
    EXPECT_NE(result.status().message().find("index=1"), std::string::npos);
}

TEST(InferenceRequestTest, RejectsDuplicateInputNameWithDetails) {
    const StatusOr<InferenceRequest> result = InferenceRequest::Create(
        "add_model", {MakeNamedTensor("x", 1.0F), MakeNamedTensor("x", 2.0F)});

    ASSERT_FALSE(result.ok());
    EXPECT_NE(result.status().message().find("duplicate input name"), std::string::npos);
    EXPECT_NE(result.status().message().find("name=x"), std::string::npos);
    EXPECT_NE(result.status().message().find("index=1"), std::string::npos);
}

TEST(InferenceRequestTest, FindsInputAndReturnsNullForMissingName) {
    StatusOr<InferenceRequest> result = InferenceRequest::Create(
        "add_model", {MakeNamedTensor("x", 1.0F), MakeNamedTensor("y", 2.0F)});
    ASSERT_TRUE(result.ok());

    const InferenceRequest& request = result.value();
    const Tensor* input = request.FindInput("y");

    ASSERT_NE(input, nullptr);
    EXPECT_EQ(input->data(), TensorData({2.0F}));
    EXPECT_EQ(request.FindInput("missing"), nullptr);
}

TEST(InferenceRequestTest, CopyOwnsIndependentTensorStorage) {
    StatusOr<InferenceRequest> result =
        InferenceRequest::Create("add_model", {MakeNamedTensor("x", 1.0F)});
    ASSERT_TRUE(result.ok());

    const InferenceRequest copied = result.value();

    const Tensor* original_tensor = result.value().FindInput("x");
    const Tensor* copied_tensor = copied.FindInput("x");
    ASSERT_NE(original_tensor, nullptr);
    ASSERT_NE(copied_tensor, nullptr);
    EXPECT_EQ(original_tensor->data(), copied_tensor->data());
    EXPECT_NE(original_tensor->data().data(), copied_tensor->data().data());
}

TEST(InferenceRequestTest, MovesRequestOutOfStatusOr) {
    StatusOr<InferenceRequest> result =
        InferenceRequest::Create("add_model", {MakeNamedTensor("x", 1.0F)});
    ASSERT_TRUE(result.ok());

    const InferenceRequest moved = std::move(result).value();

    EXPECT_EQ(moved.model_name(), "add_model");
    EXPECT_EQ(moved.input_count(), 1U);
    EXPECT_NE(moved.FindInput("x"), nullptr);
}

TEST(InferenceResultTest, CreatesSingleOutputResult) {
    StatusOr<InferenceResult> result = InferenceResult::Create({MakeNamedTensor("sum", 3.0F)});

    ASSERT_TRUE(result.ok());
    EXPECT_EQ(result.value().output_count(), 1U);
    ASSERT_EQ(result.value().outputs().size(), 1U);
    EXPECT_EQ(result.value().outputs()[0].name, "sum");
    EXPECT_EQ(result.value().outputs()[0].tensor.data(), TensorData({3.0F}));
}

TEST(InferenceResultTest, PreservesMultipleOutputOrder) {
    StatusOr<InferenceResult> result = InferenceResult::Create(
        {MakeNamedTensor("sum", 3.0F), MakeNamedTensor("difference", -1.0F)});

    ASSERT_TRUE(result.ok());
    ASSERT_EQ(result.value().outputs().size(), 2U);
    EXPECT_EQ(result.value().outputs()[0].name, "sum");
    EXPECT_EQ(result.value().outputs()[1].name, "difference");
}

TEST(InferenceResultTest, RejectsEmptyOutputs) {
    const StatusOr<InferenceResult> result = InferenceResult::Create({});

    ASSERT_FALSE(result.ok());
    EXPECT_EQ(result.status().code(), ErrorCode::kInvalidArgument);
    EXPECT_NE(result.status().message().find("outputs"), std::string::npos);
}

TEST(InferenceResultTest, RejectsEmptyOutputNameWithIndex) {
    const StatusOr<InferenceResult> result =
        InferenceResult::Create({MakeNamedTensor("sum", 3.0F), MakeNamedTensor("", 4.0F)});

    ASSERT_FALSE(result.ok());
    EXPECT_NE(result.status().message().find("output name is empty"), std::string::npos);
    EXPECT_NE(result.status().message().find("index=1"), std::string::npos);
}

TEST(InferenceResultTest, RejectsDuplicateOutputNameWithDetails) {
    const StatusOr<InferenceResult> result =
        InferenceResult::Create({MakeNamedTensor("sum", 3.0F), MakeNamedTensor("sum", 4.0F)});

    ASSERT_FALSE(result.ok());
    EXPECT_NE(result.status().message().find("duplicate output name"), std::string::npos);
    EXPECT_NE(result.status().message().find("name=sum"), std::string::npos);
    EXPECT_NE(result.status().message().find("index=1"), std::string::npos);
}

TEST(InferenceResultTest, FindsOutputAndReturnsNullForMissingName) {
    StatusOr<InferenceResult> result = InferenceResult::Create(
        {MakeNamedTensor("sum", 3.0F), MakeNamedTensor("difference", -1.0F)});
    ASSERT_TRUE(result.ok());

    const InferenceResult& inference_result = result.value();
    const Tensor* output = inference_result.FindOutput("difference");

    ASSERT_NE(output, nullptr);
    EXPECT_EQ(output->data(), TensorData({-1.0F}));
    EXPECT_EQ(inference_result.FindOutput("missing"), nullptr);
}

TEST(InferenceResultTest, CopyOwnsIndependentTensorStorage) {
    StatusOr<InferenceResult> result = InferenceResult::Create({MakeNamedTensor("sum", 3.0F)});
    ASSERT_TRUE(result.ok());

    const InferenceResult copied = result.value();

    const Tensor* original_tensor = result.value().FindOutput("sum");
    const Tensor* copied_tensor = copied.FindOutput("sum");
    ASSERT_NE(original_tensor, nullptr);
    ASSERT_NE(copied_tensor, nullptr);
    EXPECT_EQ(original_tensor->data(), copied_tensor->data());
    EXPECT_NE(original_tensor->data().data(), copied_tensor->data().data());
}

TEST(InferenceResultTest, MovesResultOutOfStatusOr) {
    StatusOr<InferenceResult> result = InferenceResult::Create({MakeNamedTensor("sum", 3.0F)});
    ASSERT_TRUE(result.ok());

    const InferenceResult moved = std::move(result).value();

    EXPECT_EQ(moved.output_count(), 1U);
    EXPECT_NE(moved.FindOutput("sum"), nullptr);
}

} // namespace
} // namespace inferlite
