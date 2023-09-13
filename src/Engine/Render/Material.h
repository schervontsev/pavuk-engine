#pragma once

#include <vulkan/vulkan.hpp>

struct Material
{
	std::string texturePath;

	vk::Image textureImage;
	vk::DeviceMemory textureImageMemory;
	vk::ImageView textureImageView;

public:
};

