#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkan/vulkan.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <chrono>
#include <vector>
#include <cstdint>
#include <optional>

#include "Vertex.h"
#include "UniformBufferObject.h"
#include "../../Scene/Scene.h"

class UpdateLightSystem;
class RenderSystem;

//TODO: move to config
const int WIDTH = 1280;
const int HEIGHT = 720;

const int SHADOW_WIDTH = 1024;
const int SHADOW_HEIGHT = 1024;

const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"//,"VK_LAYER_LUNARG_api_dump"
};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

class Renderer {
public:
    GLFWwindow* initWindow();
    void InitVulkan();

    void Update(float dt);

    void Cleanup();

    void DrawFrame();

    bool WindowShouldClose();
    void WaitDevice();

    void SetScene(const std::shared_ptr<Scene>& scene);
    void SetRenderSystem(const std::shared_ptr<RenderSystem>& renderSystem);
    void SetUpdateLightSystem(const std::shared_ptr<UpdateLightSystem>& newUpdateLightSystem);

    void UpdateBuffers();

private:
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<Renderer*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }

    void CreateInstance();
    void CreateSurface();
    void CreateRenderPass();
    void CreateGraphicsPipeline();
    void CreateFramebuffers();
    void CreateCommandPool();
    vk::UniqueShaderModule CreateShaderModule(const std::vector<char>& code);

    void CreateDepthResources();
    void CreateShadowmapImage();
    void CreateTextureImages();

    void CreateDescriptorSetLayout();
    void CreateDescriptorPool();
    void CreateDescriptorSets();

    void CreateSyncObjects();

    void CreateImageViews();
    void CreateTextureImageView();
    void CreateTextureSampler();
    vk::ImageView CreateImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags);
    void CreateImage(vk::ImageCreateInfo imageInfo, vk::MemoryPropertyFlags properties, vk::Image& image, vk::DeviceMemory& imageMemory);
    
    void LoadTextureImage(Material& material, const std::string& fileName);
    void TransitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);

    void CreateVertexBuffer(const std::vector<Vertex>& vertices);
    void CreateIndexBuffer(const std::vector<uint32_t>& indices);
    void CreateUniformBuffers();
    void UpdateUniformBuffer(uint32_t currentImage);
    void CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer& buffer, vk::DeviceMemory& bufferMemory);
    void CopyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height);
    void CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);

    void CreateCommandBuffers();
    void RecordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex);

    vk::CommandBuffer BeginSingleTimeCommands();
    void EndSingleTimeCommands(vk::CommandBuffer commandBuffer);

    void PickPhysicalDevice();
    void CreateLogicalDevice();

    void CreateSwapChain();
    void CleanupSwapChain();
    void RecreateSwapChain();

    void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    void SetupDebugMessenger();

    vk::Format FindSupportedFormat(const std::vector< vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);
    vk::Format FindDepthFormat();

    bool HasStencilComponent(VkFormat format);
    uint32_t FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
    bool IsDeviceSuitable(vk::PhysicalDevice device);
    bool CheckDeviceExtensionSupport(vk::PhysicalDevice device);
    QueueFamilyIndices FindQueueFamilies(vk::PhysicalDevice device);
    std::vector<const char*> GetRequiredExtensions();
    bool CheckValidationLayerSupport();

    vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    vk::PresentModeKHR ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
    vk::Extent2D ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);
    SwapChainSupportDetails QuerySwapChainSupport(const vk::PhysicalDevice& device);

    static std::vector<char> ReadFile(const std::string& filename); //TODO: move to utils

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);


private:
    std::shared_ptr<RenderSystem> renderSystem;
    std::shared_ptr<UpdateLightSystem> updateLightSystem;
    std::shared_ptr<GLFWwindow> window;
    std::shared_ptr<Scene> scene;

    vk::UniqueInstance instance;

    VkDebugUtilsMessengerEXT callback;
    vk::SurfaceKHR surface;

    vk::PhysicalDevice physicalDevice;
    vk::UniqueDevice device;

    vk::Queue graphicsQueue;
    vk::Queue presentQueue;

    vk::SwapchainKHR swapChain;
    std::vector<vk::Image> swapChainImages;
    vk::Format swapChainImageFormat;
    vk::Extent2D swapChainExtent;
    std::vector<vk::ImageView> swapChainImageViews;
    std::vector<vk::Framebuffer> swapChainFramebuffers;

    vk::RenderPass renderPass;
    vk::PipelineLayout pipelineLayout;
    vk::Pipeline graphicsPipeline;
    vk::PipelineCache graphicsPipelineCache;
    vk::DescriptorSetLayout descriptorSetLayout;

    vk::CommandPool commandPool;

    vk::Image depthImage;
    vk::DeviceMemory depthImageMemory;
    vk::ImageView depthImageView;

    vk::Image shadowImage;
    vk::DeviceMemory shadowImageMemory;
    vk::ImageView shadowImageView;

    vk::Sampler textureSampler;

    vk::Buffer vertexBuffer;
    vk::DeviceMemory vertexBufferMemory;
    vk::Buffer indexBuffer;
    vk::DeviceMemory indexBufferMemory;

    std::vector<vk::Buffer> vertexUniformBuffers;
    std::vector<vk::DeviceMemory> vertexUniformBuffersMemory;

    std::vector<vk::Buffer> fragmentUniformBuffers;
    std::vector<vk::DeviceMemory> fragmentUniformBuffersMemory;

    vk::DescriptorPool descriptorPool;
    std::vector<vk::DescriptorSet> descriptorSets;

    std::vector<vk::CommandBuffer, std::allocator<vk::CommandBuffer>> commandBuffers;

    std::vector<vk::Semaphore> imageAvailableSemaphores;
    std::vector<vk::Semaphore> renderFinishedSemaphores;
    std::vector<vk::Fence> inFlightFences;
    size_t currentFrame = 0;

    bool framebufferResized = false;    
};