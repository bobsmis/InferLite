#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "inferlite/backend/fake_backend.h"
#include "inferlite/core/inference_request.h"

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

InferenceRequest MakeRequest(std::vector<NamedTensor> inputs) {
    StatusOr<InferenceRequest> result = InferenceRequest::Create("test_model", std::move(inputs));
    if (!result.ok()) {
        throw std::logic_error("test request creation failed");
    }
    return std::move(result).value();
}

TEST(IBackendTest, HasVirtualDestructor) { EXPECT_TRUE(std::has_virtual_destructor_v<IBackend>); }

TEST(FakeBackendTest, RunsThroughBaseUniquePtrAndEchoesInput) {
    const InferenceRequest request = MakeRequest({MakeNamedTensor("x", {2}, {1.0F, 2.0F})});
    std::unique_ptr<IBackend> backend = std::make_unique<FakeBackend>(FakeBackendMode::kEcho);

    StatusOr<InferenceResult> result = backend->Infer(request);

    ASSERT_TRUE(result.ok());
    EXPECT_EQ(result.value().output_count(), 1U);
    const Tensor* output = result.value().FindOutput("x");
    ASSERT_NE(output, nullptr);
    EXPECT_EQ(output->shape(), Shape({2}));
    EXPECT_EQ(output->data(), TensorData({1.0F, 2.0F}));
}

TEST(FakeBackendTest, PreservesMultipleInputOrderAndValues) {
    const InferenceRequest request = MakeRequest(
        {MakeNamedTensor("x", {2}, {1.0F, 2.0F}), MakeNamedTensor("y", {1, 2}, {3.0F, 4.0F})});
    FakeBackend backend(FakeBackendMode::kEcho);

    StatusOr<InferenceResult> result = backend.Infer(request);

    ASSERT_TRUE(result.ok());
    ASSERT_EQ(result.value().outputs().size(), 2U);
    EXPECT_EQ(result.value().outputs()[0].name, "x");
    EXPECT_EQ(result.value().outputs()[0].tensor.shape(), Shape({2}));
    EXPECT_EQ(result.value().outputs()[0].tensor.data(), TensorData({1.0F, 2.0F}));
    EXPECT_EQ(result.value().outputs()[1].name, "y");
    EXPECT_EQ(result.value().outputs()[1].tensor.shape(), Shape({1, 2}));
    EXPECT_EQ(result.value().outputs()[1].tensor.data(), TensorData({3.0F, 4.0F}));
}

TEST(FakeBackendTest, EchoedOutputOwnsIndependentTensorStorage) {
    const InferenceRequest request = MakeRequest({MakeNamedTensor("x", {2}, {1.0F, 2.0F})});
    FakeBackend backend(FakeBackendMode::kEcho);

    StatusOr<InferenceResult> result = backend.Infer(request);

    ASSERT_TRUE(result.ok());
    const Tensor* input = request.FindInput("x");
    const Tensor* output = result.value().FindOutput("x");
    ASSERT_NE(input, nullptr);
    ASSERT_NE(output, nullptr);
    EXPECT_EQ(input->data(), output->data());
    EXPECT_NE(input->data().data(), output->data().data());
}

TEST(FakeBackendTest, ForcedFailureReturnsInternalStatus) {
    const InferenceRequest request = MakeRequest({MakeNamedTensor("x", {1}, {1.0F})});
    FakeBackend backend(FakeBackendMode::kForcedFailure);

    const StatusOr<InferenceResult> result = backend.Infer(request);

    ASSERT_FALSE(result.ok());
    EXPECT_EQ(result.status().code(), ErrorCode::kInternal);
    EXPECT_NE(result.status().message().find("fake backend forced failure"), std::string::npos);
}

} // namespace
} // namespace inferlite
