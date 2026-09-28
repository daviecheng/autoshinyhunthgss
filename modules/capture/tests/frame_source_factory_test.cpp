// #include <filesystem>

#include <gtest/gtest.h>

#include <capture/frame_source_factory.hpp>

#include "test_helpers.hpp"

namespace autoshinyhunthgss::capture {
namespace tests {

TEST(FrameSourceFactoryTest, CreateImageFrameSource)
{
    const auto test_dir = make_test_directory("frame_source_factory", "create_image_source");

    ImageSourceConfig config;
    config.image_paths = write_locatable_images(test_dir, 2);

    const auto source = make_image_frame_source(config);
    ASSERT_NE(source, nullptr);
}

TEST(FrameSourceFactoryTest, NoImageFrameSourceWithoutImages)
{
    EXPECT_EQ(make_image_frame_source(ImageSourceConfig{}), nullptr);
}

TEST(FrameSourceFactoryTest, CreateCamerFrameSource)
{
    // Unimplemented
    EXPECT_EQ(make_camera_frame_source(CameraSourceConfig{}), nullptr);
}

} // namespace tests
} // namespace autoshinyhunthgss::capture