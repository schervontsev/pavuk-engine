#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkan/vulkan.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <array>
#include <chrono>
#include <vector>
#include <cstdint>
#include <optional>

#include "Vertex.h"
#include "UniformBufferObject.h"
#include "../../Scene/Scene.h"

class UpdateLightSystem;
class ShadowSystem;
class RenderSystem;

//TODO: move to config
const int WIDTH = 1280;
const int HEIGHT = 720;

const int SHADOW_WIDTH = 1024;
const int SHADOW_HEIGHT = 1024;

inline constexpr bool kEnableShadowPass = true;

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

struct FrameBufferAttachment {
    vk::Image image;
    vk::DeviceMemory mem;
    vk::ImageView view;
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
    void SetShadowSystem(const std::shared_ptr<ShadowSystem>& newShadowSystem);

    void UpdateBuffers();

private:
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<Renderer*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }

    void CreateInstance();
    void CreateSurface();
    void CreateRenderPass();
    void CreateShadowPass();
    void CreateShadowDescriptorSetLayout();
    void CreateShadowPipelineLayout();
    void CreateShadowUniformBuffer();
    void CreateShadowPipeline();
    void CreateShadowDescriptorSet();
    void CreateGraphicsPipeline();
    void CreateDebugPipeline();
    void CreateFramebuffers();
    void CreateShadowFramebuffers();
    void CreateCommandPool();
    vk::UniqueShaderModule CreateShaderModule(const std::vector<char>& code);

    void CreateDepthResources();
    void UpdateShadowCubeFace(uint32_t faceIndex, uint32_t cubeIndex, vk::CommandBuffer commandBuffer);
    void CreateShadowmapImage();
    void CreateTextureImages();

    void CreateDescriptorSetLayout();
    void CreateDescriptorPool();
    void CreateDescriptorSets();

    void CreateSyncObjects();

    void CreateImageViews();
    void CreateTextureImageView();
    void CreateTextureSampler();
    void CreateShadowDepthSampler();
    vk::ImageView CreateImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags);
    void CreateImage(vk::ImageCreateInfo imageInfo, vk::MemoryPropertyFlags properties, vk::Image& image, vk::DeviceMemory& imageMemory);

    void LoadTextureImage(Material& material, const std::string& fileName);
    void TransitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);

    void CreateVertexBuffer(const std::vector<Vertex>& vertices);
    void CreateIndexBuffer(const std::vector<uint32_t>& indices);
    void CreateDebugLightBuffers();
    void CreateDebugAxesBuffers();
    void CreateDebugAxesPipeline();
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

    vk::Format FindShadowCubeDepthFormat();

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
    std::shared_ptr<ShadowSystem> shadowSystem;
    std::shared_ptr<GLFWwindow> window;
    std::shared_ptr<Scene> scene;

    /** F3: toggle textured vs normals-only debug view */
    bool normalViewMode = false;
    bool toggleNormalViewPrev = false;

    /** Main-row keys 1-6 (glfwGetKey): toggle bits for shadow faces (+X,-X,+Y,-Y,+Z,-Z). Default all on. */
    uint32_t shadowCubeFaceMask = 0x3Fu;
    std::array<bool, 6> shadowFaceTogglePrev{};

    vk::UniqueInstance instance;

    VkDebugUtilsMessengerEXT callback;
    vk::SurfaceKHR surface;

    vk::PhysicalDevice physicalDevice;
    vk::UniqueDevice device;

    vk::Queue graphicsQueue;
    vk::Queue presentQueue;

    vk::SwapchainKHR swapChain;
    std::vector<vk::Image> swapChainImages;
    std::vector<vk::ImageView> swapChainImageViews;
    std::vector<vk::Framebuffer> swapChainFramebuffers;

    vk::Format swapChainImageFormat;
    vk::Extent2D swapChainExtent;

    vk::RenderPass renderPass;

    vk::PipelineLayout pipelineLayout;
    vk::Pipeline graphicsPipeline;
    vk::Pipeline debugPipeline;
    vk::Pipeline debugAxesPipeline;
    vk::PipelineCache graphicsPipelineCache;
    vk::DescriptorSetLayout descriptorSetLayout;

    vk::CommandPool commandPool;

    FrameBufferAttachment depthAttach;

    vk::RenderPass shadowPass;

    vk::Format shadowCubeDepthFormat = vk::Format::eUndefined;
    FrameBufferAttachment shadowAttach;
    std::vector<vk::ImageView> shadowViews;
    vk::ImageView shadowCubeMapView;
    std::vector<vk::Framebuffer> shadowBuffers;
    vk::Sampler shadowDepthSampler;
    vk::DescriptorSetLayout shadowDescriptorSetLayout;
    vk::PipelineLayout shadowPipelineLayout;
    vk::Pipeline shadowPipeline;
    std::vector<vk::DescriptorSet> shadowDescriptorSets;
    std::vector<vk::Buffer> shadowUniformBuffers;
    std::vector<vk::DeviceMemory> shadowUniformBuffersMemory;

    vk::Sampler textureSampler;

    vk::Buffer vertexBuffer;
    vk::DeviceMemory vertexBufferMemory;
    vk::Buffer indexBuffer;
    vk::DeviceMemory indexBufferMemory;

    vk::Buffer debugLightVertexBuffer;
    vk::DeviceMemory debugLightVertexBufferMemory;
    vk::Buffer debugLightIndexBuffer;
    vk::DeviceMemory debugLightIndexBufferMemory;
    uint32_t debugLightIndexCount;

    vk::Buffer debugAxesVertexBuffer;
    vk::DeviceMemory debugAxesVertexBufferMemory;
    uint32_t debugAxesVertexCount = 0;

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