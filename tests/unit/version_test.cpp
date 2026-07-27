#include <gtest/gtest.h>

#include "inferlite/core/version.h"

TEST(VersionTest, ReturnsProjectName) { EXPECT_STREQ(inferlite::project_name(), "InferLite"); }