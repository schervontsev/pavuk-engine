#include "Renderer.h"

#include "../ResourceManagers/MaterialManager.h"
#include "../ResourceManagers/MeshManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "../ECS/ECSManager.h"
#include "../ECS/Systems/RenderSystem.h"
#include "../ECS/Systems/UpdateLightSystem.h"
#include "UniformBufferObject.h"

#include <iostream>
#include <fstream>
#include <chrono>
#include <stdexcept>
#include <algorithm>
#include <cstring>
#include <limits>
#include <array>
#include <set>
#include "../ECS/Components/TransformComponent.h"
#include "../ECS/Components/CameraComponent.h"
#include "../ECS/Components/RenderComponent.h"
#include "../ECS/Systems/UpdateLightSystem.h"
#include "../ECS/Systems/ShadowSystem.h"
#include <vector>
#include "../Input/InputManager.h"

namespace {
constexpr uint32_t kShadowLightViewProjPushBytes = 64u; // glm::mat4
constexpr uint32_t kShadowMeshPushConstantOffset = kShadowLightViewProjPushBytes;
constexpr uint32_t kShadowPushConstantsTotalSize = kShadowMeshPushConstantOffset + static_cast<uint32_t>(sizeof(MeshPushConstants));
} // namespace

GLFWwindow* Renderer::initWindow() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = std::shared_ptr<GLFWwindow>(glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr), [](GLFWwindow* window) {
        glfwDestroyWindow(window);
    });
    glfwSetWindowUserPointer(window.get(), this);
    glfwSetFramebufferSizeCallback(window.get(), framebufferResizeCallback);
    return window.get();
}

void Renderer::SetScene(const std::shared_ptr<Scene>& newScene)
{
    scene = newScene;
}

void Renderer::SetRenderSystem(const std::shared_ptr<RenderSystem>& newRenderSystem)
{
    renderSystem = newRenderSystem;
}

void Renderer::SetUpdateLightSystem(const std::shared_ptr<UpdateLightSystem>& newUpdateLightSystem)
{
    updateLightSystem = newUpdateLightSystem;
}

void Renderer::SetShadowSystem(const std::shared_ptr<ShadowSystem>& newShadowSystem)
{
    shadowSystem = newShadowSystem;
}

void Renderer::InitVulkan() {
    CreateInstance();
    SetupDebugMessenger();
    CreateSurface();

    PickPhysicalDevice();
    CreateLogicalDevice();

    CreateSwapChain();
    CreateImageViews();
    CreateRenderPass();
    CreateShadowPass();
    CreateDescriptorSetLayout();
    CreateShadowDescriptorSetLayout();
    CreateShadowPipelineLayout();
    CreateGraphicsPipeline();
    CreateDebugPipeline();
    CreateDebugAxesPipeline();
    CreateShadowPipeline();
    CreateCommandPool();
    CreateDepthResources();
    CreateFramebuffers();
    CreateShadowmapImage();
    CreateShadowFramebuffers();

    CreateTextureImages();
    CreateTextureImageView();
    CreateTextureSampler();
    CreateShadowDepthSampler();

    CreateUniformBuffers();
    CreateShadowUniformBuffer();
    CreateDescriptorPool();
    CreateDescriptorSets();
    CreateShadowDescriptorSet();
    CreateCommandBuffers();
    CreateSyncObjects();
    CreateDebugLightBuffers();
    CreateDebugAxesBuffers();
}

void Renderer::UpdateBuffers()
{
    if (!scene || !scene->IsDirty()) {
        return;
    }
    if (!renderSystem) {
        assert(renderSystem);
        return;
    }
    CreateVertexBuffer(renderSystem->GetVertices());
    CreateIndexBuffer(renderSystem->GetIndices());
    if (scene) {
        scene->SetDirty(false);
    }
}

void Renderer::CleanupSwapChain() {

    for (auto& framebuffer : swapChainFramebuffers) {
        device->destroyFramebuffer(framebuffer, nullptr);
    }

    for (auto& imageView : swapChainImageViews) {
        device->destroyImageView(imageView, nullptr);
    }

    device->destroySwapchainKHR(swapChain, nullptr);
}

void Renderer::Cleanup() {
    // NOTE: instance destruction is handled by UniqueInstance, same for device

    CleanupSwapChain();

    MaterialManager::Instance()->DestroyMaterials(device);

    device->destroyImageView(depthAttach.view, nullptr);
    device->destroyImage(depthAttach.image, nullptr);
    device->freeMemory(depthAttach.mem, nullptr);

    device->destroyImage(shadowAttach.image, nullptr);
    device->freeMemory(shadowAttach.mem, nullptr);

    for (size_t i = 0; i < shadowBuffers.size(); i++) {
        device->destroyFramebuffer(shadowBuffers[i], nullptr);
    }
    for (size_t i = 0; i < shadowViews.size(); i++) {
        device->destroyImageView(shadowViews[i], nullptr);
    }
    device->destroyImageView(shadowCubeMapView, nullptr);
    device->destroyPipeline(shadowPipeline);
    device->destroyPipelineLayout(shadowPipelineLayout);
    device->destroyDescriptorSetLayout(shadowDescriptorSetLayout);
    for (size_t i = 0; i < shadowUniformBuffers.size(); i++) {
        device->destroyBuffer(shadowUniformBuffers[i]);
        device->freeMemory(shadowUniformBuffersMemory[i]);
    }
    device->destroyRenderPass(shadowPass);

    device->destroyBuffer(vertexBuffer);
    device->destroyBuffer(indexBuffer);
    device->freeMemory(vertexBufferMemory);
    device->freeMemory(indexBufferMemory);

    device->destroyBuffer(debugLightVertexBuffer);
    device->freeMemory(debugLightVertexBufferMemory);
    device->destroyBuffer(debugLightIndexBuffer);
    device->freeMemory(debugLightIndexBufferMemory);

    device->destroyBuffer(debugAxesVertexBuffer);
    device->freeMemory(debugAxesVertexBufferMemory);

    for (auto& buffer : vertexUniformBuffers) {
        device->destroyBuffer(buffer);
    }
    for (auto& buffer : fragmentUniformBuffers) {
        device->destroyBuffer(buffer);
    }
    for (auto& memory : vertexUniformBuffersMemory) {
        device->freeMemory(memory);
    }
    for (auto& memory : fragmentUniformBuffersMemory) {
        device->freeMemory(memory);
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        device->destroySemaphore(renderFinishedSemaphores[i]);
        device->destroySemaphore(imageAvailableSemaphores[i]);
        device->destroyFence(inFlightFences[i]);
    }

    device->destroyCommandPool(commandPool);

    // surface is created by glfw, therefore not using a Unique handle
    instance->destroySurfaceKHR(surface);

    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(*instance, callback, nullptr);
    }

    device->destroyRenderPass(renderPass);
    device->destroyDescriptorSetLayout(descriptorSetLayout);
    device->destroySampler(textureSampler);
    device->destroySampler(shadowDepthSampler);
    device->destroyPipelineLayout(pipelineLayout);
    device->destroyPipeline(graphicsPipeline);
    device->destroyPipeline(debugPipeline);
    device->destroyPipeline(debugAxesPipeline);
    device->destroyPipelineCache(graphicsPipelineCache);
    device->destroyDescriptorPool(descriptorPool);

    glfwDestroyWindow(window.get());

    glfwTerminate();
}

void Renderer::RecreateSwapChain() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(window.get(), &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window.get(), &width, &height);
        glfwWaitEvents();
    }

    device->waitIdle();

    CleanupSwapChain();

    CreateSwapChain();
    CreateImageViews();
    CreateDepthResources();
    CreateFramebuffers();
}

void Renderer::CreateInstance() {
    if (enableValidationLayers && !CheckValidationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    auto appInfo = vk::ApplicationInfo(
        "Pavukan Engine",
        VK_MAKE_VERSION(1, 0, 0),
        "No Engine",
        VK_MAKE_VERSION(1, 0, 0),
        VK_API_VERSION_1_1
    );

    auto extensions = GetRequiredExtensions();


    auto createInfo = vk::InstanceCreateInfo(
        vk::InstanceCreateFlags(),
        &appInfo,
        0, nullptr, // enabled layers
        static_cast<uint32_t>(extensions.size()), extensions.data() // enabled extensions
    );

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }

    try {
        instance = vk::createInstanceUnique(createInfo, nullptr);
    } catch (vk::SystemError err) {
        throw std::runtime_error("failed to create instance!");
    }
}

void Renderer::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

void Renderer::SetupDebugMessenger() {
    if (!enableValidationLayers) return;

    auto createInfo = vk::DebugUtilsMessengerCreateInfoEXT(
        vk::DebugUtilsMessengerCreateFlagsEXT(),
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
        debugCallback,
        nullptr
    );

    // NOTE: reinterpret_cast is also used by vulkan.hpp internally for all these structs
    if (CreateDebugUtilsMessengerEXT(*instance, reinterpret_cast<const VkDebugUtilsMessengerCreateInfoEXT*>(&createInfo), nullptr, &callback) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug callback!");
    }
}

void Renderer::CreateSurface() {
    VkSurfaceKHR rawSurface;
    if (glfwCreateWindowSurface(*instance, window.get(), nullptr, &rawSurface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }

    surface = vk::SurfaceKHR(rawSurface);
}

void Renderer::PickPhysicalDevice() {
    auto devices = instance->enumeratePhysicalDevices();
    if (devices.size() == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    for (const auto& device : devices) {
        if (IsDeviceSuitable(device)) {
            physicalDevice = device;
            break;
        }
    }

    if (!physicalDevice) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

void Renderer::CreateLogicalDevice() {
    QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    float queuePriority = 1.0f;

    for (uint32_t queueFamily : uniqueQueueFamilies) {
        queueCreateInfos.push_back({
            vk::DeviceQueueCreateFlags(),
            queueFamily,
            1, // queueCount
            &queuePriority
            });
    }

    auto deviceFeatures = vk::PhysicalDeviceFeatures();
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    deviceFeatures.imageCubeArray = VK_TRUE;

    auto indexingFeatures = vk::PhysicalDeviceDescriptorIndexingFeatures();
    indexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;

    auto createInfo = vk::DeviceCreateInfo(
        vk::DeviceCreateFlags(),
        static_cast<uint32_t>(queueCreateInfos.size()),
        queueCreateInfos.data()
    );
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    createInfo.setPNext(&indexingFeatures);

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }

    try {
        device = physicalDevice.createDeviceUnique(createInfo);
    } catch (vk::SystemError err) {
        throw std::runtime_error("failed to create logical device!");
    }

    graphicsQueue = device->getQueue(indices.graphicsFamily.value(), 0);
    presentQueue = device->getQueue(indices.presentFamily.value(), 0);
}

void Renderer::CreateSwapChain() {
    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(physicalDevice);

    vk::SurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
    vk::PresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
    vk::Extent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR createInfo(
        vk::SwapchainCreateFlagsKHR(),
        surface,
        imageCount,
        surfaceFormat.format,
        surfaceFormat.colorSpace,
        extent,
        1, // imageArrayLayers
        vk::ImageUsageFlagBits::eColorAttachment
    );

    QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = vk::SwapchainKHR(nullptr);

    try {
        swapChain = device->createSwapchainKHR(createInfo);
    } catch (vk::SystemError err) {
        throw std::runtime_error("failed to create swap chain!");
    }

    swapChainImages = device->getSwapchainImagesKHR(swapChain);

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

void Renderer::CreateImageViews() {
    swapChainImageViews.resize(swapChainImages.size());

    for (uint32_t i = 0; i < swapChainImages.size(); i++) {
        swapChainImageViews[i] = CreateImageView(swapChainImages[i], swapChainImageFormat, vk::ImageAspectFlagBits::eColor);
    }
}

void Renderer::CreateRenderPass() {
    vk::AttachmentDescription colorAttachment = {};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = vk::SampleCountFlagBits::e1;
    colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
    colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

    vk::AttachmentDescription depthAttachment {};
    depthAttachment.format = FindDepthFormat();
    depthAttachment.samples = vk::SampleCountFlagBits::e1;
    depthAttachment.loadOp =  vk::AttachmentLoadOp::eClear;
    depthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
    depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
    depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    vk::AttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::AttachmentReference depthAttachmentRef {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    vk::SubpassDescription subpass = {};
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    vk::SubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;

    dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;

    std::array< vk::AttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    vk::RenderPassCreateInfo renderPassInfo {};
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    try {
        renderPass = device->createRenderPass(renderPassInfo);
    } catch (vk::SystemError err) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void Renderer::CreateShadowPass() {
    vk::AttachmentDescription depthAttachment {};
    shadowCubeDepthFormat = FindShadowCubeDepthFormat();
    depthAttachment.format = shadowCubeDepthFormat;
    depthAttachment.samples = vk::SampleCountFlagBits::e1;

    depthAttachment.loadOp = vk::AttachmentLoadOp::eLoad;
    depthAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    depthAttachment.initialLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    vk::AttachmentReference depthAttachmentRef {};
    depthAttachmentRef.attachment = 0;
    depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    vk::SubpassDescription subpass = {};
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpass.colorAttachmentCount = 0;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    std::array< vk::AttachmentDescription, 1> attachments = {depthAttachment};
    vk::RenderPassCreateInfo renderPassInfo {};
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 0;

    try {
        shadowPass = device->createRenderPass(renderPassInfo);
    } catch (vk::SystemError err) {
        throw std::runtime_error("failed to create shadow render pass!");
    }
}

void Renderer::CreateDescriptorSetLayout() {

    std::vector<vk::DescriptorSetLayoutBinding> bindings;

    vk::DescriptorSetLayoutBinding uboLayoutBinding {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
    uboLayoutBinding.pImmutableSamplers = nullptr;
    uboLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;
    bindings.push_back(uboLayoutBinding);

    size_t materialSize = MaterialManager::Instance()->GetMaterialCount();

    vk::DescriptorSetLayoutBinding samplerLayoutBinding {};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = (uint32_t)materialSize;
    samplerLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;
    bindings.push_back(samplerLayoutBinding);

    vk::DescriptorSetLayoutBinding fragUboLayoutBinding{};
    fragUboLayoutBinding.binding = 2;
    fragUboLayoutBinding.descriptorCount = 1;
    fragUboLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
    fragUboLayoutBinding.pImmutableSamplers = nullptr;
    fragUboLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;
    bindings.push_back(fragUboLayoutBinding);

    vk::DescriptorSetLayoutBinding shadowSamplerBinding{};
    shadowSamplerBinding.binding = 3;
    shadowSamplerBinding.descriptorCount = 1;
    shadowSamplerBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    shadowSamplerBinding.pImmutableSamplers = nullptr;
    shadowSamplerBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;
    bindings.push_back(shadowSamplerBinding);

    vk::DescriptorSetLayoutCreateInfo layoutInfo {};
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    vk::Result result;
    try {
        result = device->createDescriptorSetLayout(&layoutInfo, nullptr, &descriptorSetLayout);
    } catch (vk::SystemError err) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void Renderer::CreateGraphicsPipeline() {
    auto vertShaderCode = ReadFile("shaders/vert.spv");
    auto fragShaderCode = ReadFile("shaders/frag.spv");

    auto vertShaderModule = CreateShaderModule(vertShaderCode);
    auto fragShaderModule = CreateShaderModule(fragShaderCode);

    vk::PipelineShaderStageCreateInfo shaderStages[] = {
            {
                vk::PipelineShaderStageCreateFlags(),
                vk::ShaderStageFlagBits::eVertex,
                *vertShaderModule,
                "main"
            },
            {
                vk::PipelineShaderStageCreateFlags(),
                vk::ShaderStageFlagBits::eFragment,
                *fragShaderModule,
                "main"
            }
    };

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    vk::PipelineViewportStateCreateInfo viewportState = {};
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    vk::PipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = vk::PolygonMode::eFill;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = vk::CullModeFlagBits::eBack;
    rasterizer.frontFace = vk::FrontFace::eClockwise;
    rasterizer.depthBiasEnable = VK_FALSE;

    vk::PipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

    vk::PipelineDepthStencilStateCreateInfo depthStencil {};
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = vk::CompareOp::eLess;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    colorBlendAttachment.blendEnable = VK_FALSE;

    vk::PipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = vk::LogicOp::eCopy;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    std::vector<vk::DynamicState> dynamicStates = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };
    vk::PipelineDynamicStateCreateInfo dynamicState {};
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    vk::PushConstantRange push_constant;
    push_constant.offset = 0;
    push_constant.size = sizeof(MeshPushConstants);
    push_constant.stageFlags = vk::ShaderStageFlagBits::eVertex;

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    pipelineLayoutInfo.pPushConstantRanges = &push_constant;
    pipelineLayoutInfo.pushConstantRangeCount = 1;

    try {
        pipelineLayout = device->createPipelineLayout(pipelineLayoutInfo);
    } catch (vk::SystemError err) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    vk::GraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = nullptr;

    try {
        auto result = device->createGraphicsPipeline(nullptr, pipelineInfo);
        graphicsPipeline = result.value;

    } catch (vk::SystemError err) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }
}

void Renderer::CreateDebugPipeline() {
    auto vertShaderCode = ReadFile("shaders/debug_vert.spv");
    auto fragShaderCode = ReadFile("shaders/debug_frag.spv");
    auto vertShaderModule = CreateShaderModule(vertShaderCode);
    auto fragShaderModule = CreateShaderModule(fragShaderCode);

    vk::PipelineShaderStageCreateInfo shaderStages[] = {
        { vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex, *vertShaderModule, "main" },
        { vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eFragment, *fragShaderModule, "main" }
    };

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    vk::PipelineViewportStateCreateInfo viewportState{};
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    vk::PipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = vk::PolygonMode::eFill;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = vk::CullModeFlagBits::eBack;
    rasterizer.frontFace = vk::FrontFace::eClockwise;
    rasterizer.depthBiasEnable = VK_FALSE;

    vk::PipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

    vk::PipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.depthTestEnable = VK_FALSE;
    depthStencil.depthWriteEnable = VK_FALSE;
    depthStencil.depthCompareOp = vk::CompareOp::eLess;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    colorBlendAttachment.blendEnable = VK_FALSE;

    vk::PipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    std::vector<vk::DynamicState> dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
    vk::PipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    vk::GraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;

    try {
        auto result = device->createGraphicsPipeline(nullptr, pipelineInfo);
        debugPipeline = result.value;
    } catch (vk::SystemError err) {
        throw std::runtime_error("failed to create debug pipeline!");
    }
}

void Renderer::CreateDebugAxesPipeline() {
    auto vertShaderCode = ReadFile("shaders/debug_vert.spv");
    auto fragShaderCode = ReadFile("shaders/debug_frag.spv");
    auto vertShaderModule = CreateShaderModule(vertShaderCode);
    auto fragShaderModule = CreateShaderModule(fragShaderCode);

    vk::PipelineShaderStageCreateInfo shaderStages[] = {
        { vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex, *vertShaderModule, "main" },
        { vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eFragment, *fragShaderModule, "main" }
    };

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.topology = vk::PrimitiveTopology::eLineList;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    vk::PipelineViewportStateCreateInfo viewportState{};
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    vk::PipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = vk::PolygonMode::eFill;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = vk::CullModeFlagBits::eNone;
    rasterizer.frontFace = vk::FrontFace::eClockwise;
    rasterizer.depthBiasEnable = VK_FALSE;

    vk::PipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

    vk::PipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.depthTestEnable = VK_FALSE;
    depthStencil.depthWriteEnable = VK_FALSE;
    depthStencil.depthCompareOp = vk::CompareOp::eLess;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    colorBlendAttachment.blendEnable = VK_FALSE;

    vk::PipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    std::vector<vk::DynamicState> dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
    vk::PipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    vk::GraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;

    try {
        auto result = device->createGraphicsPipeline(nullptr, pipelineInfo);
        debugAxesPipeline = result.value;
    } catch (vk::SystemError err) {
        throw std::runtime_error("failed to create debug axes pipeline!");
    }
}

void Renderer::CreateFramebuffers() {
    swapChainFramebuffers.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        std::array<vk::ImageView, 2> attachments = {
                swapChainImageViews[i],
                depthAttach.view
        };

        vk::FramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        try {
            swapChainFramebuffers[i] = device->createFramebuffer(framebufferInfo);
        } catch (vk::SystemError err) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void Renderer::CreateShadowFramebuffers() {
    vk::FramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.renderPass = shadowPass;
    framebufferInfo.width = SHADOW_WIDTH;
    framebufferInfo.height = SHADOW_HEIGHT;
    framebufferInfo.layers = 1;

    shadowBuffers.resize(shadowViews.size());

    try {
        for (uint32_t i = 0; i < static_cast<uint32_t>(shadowViews.size()); i++) {
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = &shadowViews[i];
            shadowBuffers[i] = device->createFramebuffer(framebufferInfo);
        }
    } catch (vk::SystemError err) {
        throw std::runtime_error("failed to create shadow framebuffers!");
    }
}

void Renderer::CreateShadowDescriptorSetLayout() {
    vk::DescriptorSetLayoutBinding uboBinding{};
    uboBinding.binding = 0;
    uboBinding.descriptorCount = 1;
    uboBinding.descriptorType = vk::DescriptorType::eUniformBuffer;

    uboBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

    vk::DescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboBinding;

    try {
        shadowDescriptorSetLayout = device->createDescriptorSetLayout(layoutInfo);
    } catch (vk::SystemError err) {
        throw std::runtime_error("failed to create shadow descriptor set layout!");
    }
}

void Renderer::CreateShadowPipelineLayout() {
    vk::PushConstantRange pushConstant{};
    pushConstant.offset = 0;
    pushConstant.size = kShadowPushConstantsTotalSize;
    pushConstant.stageFlags = vk::ShaderStageFlagBits::eVertex;

    vk::PipelineLayoutCreateInfo layoutInfo{};
    layoutInfo.setLayoutCount = 1;
    layoutInfo.pSetLayouts = &shadowDescriptorSetLayout;
    layoutInfo.pushConstantRangeCount = 1;
    layoutInfo.pPushConstantRanges = &pushConstant;

    try {
        shadowPipelineLayout = device->createPipelineLayout(layoutInfo);
    } catch (vk::SystemError err) {
        throw std::runtime_error("failed to create shadow pipeline layout!");
    }
}

void Renderer::CreateShadowUniformBuffer() {
    shadowUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    shadowUniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        CreateBuffer(sizeof(ShadowUBOData), vk::BufferUsageFlagBits::eUniformBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
            shadowUniformBuffers[i], shadowUniformBuffersMemory[i]);
    }
}

void Renderer::CreateShadowPipeline() {
    auto vertShaderCode = ReadFile("shaders/shadow_vert.spv");
    auto fragShaderCode = ReadFile("shaders/shadow_frag.spv");

    auto vertShaderModule = CreateShaderModule(vertShaderCode);
    auto fragShaderModule = CreateShaderModule(fragShaderCode);

    vk::PipelineShaderStageCreateInfo shaderStages[] = {
        { vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex, *vertShaderModule, "main" },
        { vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eFragment, *fragShaderModule, "main" }
    };

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    vk::PipelineViewportStateCreateInfo viewportState{};
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    vk::PipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = vk::PolygonMode::eFill;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = vk::CullModeFlagBits::eNone;
    rasterizer.frontFace = vk::FrontFace::eClockwise;
    rasterizer.depthBiasEnable = VK_TRUE;
    rasterizer.depthBiasConstantFactor = 2.0f;
    rasterizer.depthBiasSlopeFactor = 2.0f;

    vk::PipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

    vk::PipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = vk::CompareOp::eLess;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    vk::PipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 0;

    std::vector<vk::DynamicState> dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
    vk::PipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    vk::GraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = shadowPipelineLayout;
    pipelineInfo.renderPass = shadowPass;
    pipelineInfo.subpass = 0;

    try {
        auto result = device->createGraphicsPipeline(nullptr, pipelineInfo);
        shadowPipeline = result.value;
    } catch (vk::SystemError err) {
        throw std::runtime_error("failed to create shadow pipeline!");
    }
}

void Renderer::CreateShadowDescriptorSet() {
    std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, shadowDescriptorSetLayout);
    vk::DescriptorSetAllocateInfo allocInfo{};
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    shadowDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    try {
        shadowDescriptorSets = device->allocateDescriptorSets(allocInfo);
    } catch (vk::SystemError err) {
        throw std::runtime_error("failed to allocate shadow descriptor sets!");
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vk::DescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = shadowUniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(ShadowUBOData);

        vk::WriteDescriptorSet write{};
        write.dstSet = shadowDescriptorSets[i];
        write.dstBinding = 0;
        write.dstArrayElement = 0;
        write.descriptorType = vk::DescriptorType::eUniformBuffer;
        write.descriptorCount = 1;
        write.pBufferInfo = &bufferInfo;

        device->updateDescriptorSets(1, &write, 0, nullptr);
    }
}

void Renderer::CreateCommandPool() {
    QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(physicalDevice);

    vk::CommandPoolCreateInfo poolInfo = {};
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

    try {
        commandPool = device->createCommandPool(poolInfo);
    } catch (vk::SystemError err) {
        throw std::runtime_error("failed to create command pool!");
    }
}

void Renderer::CreateDepthResources() {
    vk::Format depthFormat = FindDepthFormat();

    vk::ImageCreateInfo imageInfo {};
    imageInfo.imageType = vk::ImageType::e2D;
    imageInfo.extent.width = swapChainExtent.width;
    imageInfo.extent.height = swapChainExtent.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = FindDepthFormat();
    imageInfo.tiling = vk::ImageTiling::eOptimal;
    imageInfo.initialLayout = vk::ImageLayout::eUndefined;
    imageInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
    imageInfo.samples = vk::SampleCountFlagBits::e1;
    imageInfo.sharingMode = vk::SharingMode::eExclusive;

    CreateImage(imageInfo, vk::MemoryPropertyFlagBits::eDeviceLocal, depthAttach.image, depthAttach.mem);
    depthAttach.view = CreateImageView(depthAttach.image, depthFormat, vk::ImageAspectFlagBits::eDepth);
}

void Renderer::UpdateShadowCubeFace(uint32_t faceIndex, uint32_t cubeIndex, vk::CommandBuffer commandBuffer)
{
    glm::vec3 lightPos(0.0f, 0.0f, 0.0f);
    if (shadowSystem) {
        lightPos = shadowSystem->GetShadowLightPosition(static_cast<size_t>(cubeIndex));
    }

    glm::vec3 direction;
    glm::vec3 up;
    switch (faceIndex) {
    case 0: direction = glm::vec3(1.0f, 0.0f, 0.0f);  up = glm::vec3(0.0f, -1.0f, 0.0f);  break;   // +X
    case 1: direction = glm::vec3(-1.0f, 0.0f, 0.0f); up = glm::vec3(0.0f, -1.0f, 0.0f);  break;   // -X
    case 2: direction = glm::vec3(0.0f, -1.0f, 0.0f);  up = glm::vec3(0.0f, 0.0f, -1.0f);  break;   // +Y
    case 3: direction = glm::vec3(0.0f, 1.0f, 0.0f); up = glm::vec3(0.0f, 0.0f, 1.0f); break;   // -Y
    case 4: direction = glm::vec3(0.0f, 0.0f, 1.0f);  up = glm::vec3(0.0f, -1.0f, 0.0f);  break;   // +Z
    case 5: direction = glm::vec3(0.0f, 0.0f, -1.0f); up = glm::vec3(0.0f, -1.0f, 0.0f);  break;   // -Z
    default: direction = glm::vec3(0.0f, 0.0f, 1.0f); up = glm::vec3(0.0f, 1.0f, 0.0f); break;
    }

    glm::mat4 view = glm::lookAt(lightPos, lightPos + direction, up);
    const float shadow_near = 0.0001f;
    const float shadow_far = 50.0f;
    glm::mat4 proj = glm::perspective(glm::radians(90.0f), 1.0f, shadow_near, shadow_far);
    proj[1][1] *= -1.0f;
    glm::mat4 lightViewProj = proj * view;

    ShadowUBOData shadowUbo{};
    shadowUbo.light_pos = glm::vec4(lightPos, 1.0f);
    shadowUbo.near_far = glm::vec2(shadow_near, shadow_far);
    shadowUbo._pad_to_32 = glm::vec2(0.f);

    void* data = device->mapMemory(shadowUniformBuffersMemory[currentFrame], 0, sizeof(ShadowUBOData));
    memcpy(data, &shadowUbo, sizeof(ShadowUBOData));
    device->unmapMemory(shadowUniformBuffersMemory[currentFrame]);

    const uint32_t fbIndex = cubeIndex * 6u + faceIndex;
    vk::RenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.renderPass = shadowPass;
    renderPassBeginInfo.framebuffer = shadowBuffers[fbIndex];
    renderPassBeginInfo.renderArea.extent.width = SHADOW_WIDTH;
    renderPassBeginInfo.renderArea.extent.height = SHADOW_HEIGHT;
    renderPassBeginInfo.clearValueCount = 0;
    renderPassBeginInfo.pClearValues = nullptr;

    commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, shadowPipeline);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, shadowPipelineLayout, 0, 1, &shadowDescriptorSets[currentFrame], 0, nullptr);
    commandBuffer.pushConstants(
        shadowPipelineLayout,
        vk::ShaderStageFlagBits::eVertex,
        0,
        sizeof(glm::mat4),
        &lightViewProj);

    vk::Buffer vertexBuffers[] = { vertexBuffer };
    vk::DeviceSize offsets[] = { 0 };
    commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
    commandBuffer.bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint32);

    if (renderSystem) {
        renderSystem->UpdateCommandBuffer(commandBuffer, shadowPipelineLayout, kShadowMeshPushConstantOffset);
    }

    commandBuffer.endRenderPass();
}

void Renderer::CreateShadowmapImage() {
    const uint32_t shadowArrayLayers = 6u * static_cast<uint32_t>(MAX_SHADOW_LIGHTS);

    vk::ImageCreateInfo imageInfo {};
    imageInfo.flags = vk::ImageCreateFlagBits::eCubeCompatible;
    imageInfo.imageType = vk::ImageType::e2D;
    imageInfo.extent.width = SHADOW_WIDTH;
    imageInfo.extent.height = SHADOW_HEIGHT;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = shadowArrayLayers;
    imageInfo.format = shadowCubeDepthFormat;
    imageInfo.tiling = vk::ImageTiling::eOptimal;
    imageInfo.initialLayout = vk::ImageLayout::eUndefined;
    imageInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled
        | vk::ImageUsageFlagBits::eTransferDst;
    imageInfo.samples = vk::SampleCountFlagBits::e1;
    imageInfo.sharingMode = vk::SharingMode::eExclusive;

    CreateImage(imageInfo, vk::MemoryPropertyFlagBits::eDeviceLocal, shadowAttach.image, shadowAttach.mem);

    shadowViews.resize(shadowArrayLayers);

    vk::ImageViewCreateInfo viewInfo = {};
    viewInfo.image = shadowAttach.image;
    viewInfo.viewType = vk::ImageViewType::e2D;
    viewInfo.format = shadowCubeDepthFormat;
    viewInfo.components = vk::ComponentMapping();
    viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.layerCount = 1;

    for (uint32_t i = 0; i < shadowArrayLayers; i++) {
        viewInfo.subresourceRange.baseArrayLayer = i;
        try {
            shadowViews[i] = device->createImageView(viewInfo);
        } catch (vk::SystemError err) {
            throw std::runtime_error("failed to create shadow image views!");
        }
    }

    vk::ImageViewCreateInfo cubeViewInfo{};
    cubeViewInfo.image = shadowAttach.image;
    cubeViewInfo.viewType = vk::ImageViewType::eCubeArray;
    cubeViewInfo.format = shadowCubeDepthFormat;
    cubeViewInfo.components = vk::ComponentMapping();
    cubeViewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
    cubeViewInfo.subresourceRange.baseMipLevel = 0;
    cubeViewInfo.subresourceRange.levelCount = 1;
    cubeViewInfo.subresourceRange.baseArrayLayer = 0;
    cubeViewInfo.subresourceRange.layerCount = shadowArrayLayers;
    try {
        shadowCubeMapView = device->createImageView(cubeViewInfo);
    } catch (vk::SystemError err) {
        throw std::runtime_error("failed to create shadow cube array map view!");
    }
}

void Renderer::CreateShadowDepthSampler() {
    vk::SamplerCreateInfo samplerInfo{};

    samplerInfo.magFilter = vk::Filter::eNearest;
    samplerInfo.minFilter = vk::Filter::eNearest;
    samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
    samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
    samplerInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_TRUE;
    samplerInfo.compareOp = vk::CompareOp::eLessOrEqual;
    samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;
    try {
        shadowDepthSampler = device->createSampler(samplerInfo);
    } catch (vk::SystemError err) {
        throw std::runtime_error("failed to create shadow depth sampler!");
    }
}

vk::Format Renderer::FindSupportedFormat(const std::vector< vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) {
    for (vk::Format format : candidates) {
        vk::FormatProperties props = physicalDevice.getFormatProperties(format);

        if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

vk::Format Renderer::FindDepthFormat() {
    return FindSupportedFormat(
        {vk::Format::eD32Sfloat ,vk::Format::eD32SfloatS8Uint , vk::Format::eD24UnormS8Uint},
        vk::ImageTiling::eOptimal,
        vk::FormatFeatureFlagBits::eDepthStencilAttachment
    );
}

vk::Format Renderer::FindShadowCubeDepthFormat() {
    const vk::FormatFeatureFlags required =
        vk::FormatFeatureFlagBits::eDepthStencilAttachment
        | vk::FormatFeatureFlagBits::eSampledImage
        | vk::FormatFeatureFlagBits::eSampledImageFilterLinear;
    return FindSupportedFormat(
        {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint},
        vk::ImageTiling::eOptimal,
        required);
}

bool Renderer::HasStencilComponent(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void Renderer::LoadTextureImage(Material& material, const std::string& fileName) {
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(fileName.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    vk::DeviceSize imageSize = vk::DeviceSize(texWidth) * vk::DeviceSize(texHeight) * 4;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    vk::Buffer stagingBuffer;
    vk::DeviceMemory stagingBufferMemory;
    CreateBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);

    void* data = device->mapMemory(stagingBufferMemory, vk::DeviceSize(0), imageSize);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    device->unmapMemory(stagingBufferMemory);

    stbi_image_free(pixels);
    vk::ImageCreateInfo imageInfo {};
    imageInfo.imageType = vk::ImageType::e2D;
    imageInfo.extent.width = texWidth;
    imageInfo.extent.height = texHeight;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = vk::Format::eR8G8B8A8Srgb;
    imageInfo.tiling = vk::ImageTiling::eOptimal;
    imageInfo.initialLayout = vk::ImageLayout::eUndefined;
    imageInfo.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
    imageInfo.samples = vk::SampleCountFlagBits::e1;
    imageInfo.sharingMode = vk::SharingMode::eExclusive;
    CreateImage(imageInfo, vk::MemoryPropertyFlagBits::eDeviceLocal, material.textureImage, material.textureImageMemory);

    TransitionImageLayout(material.textureImage, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
    CopyBufferToImage(stagingBuffer, material.textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    TransitionImageLayout(material.textureImage, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

    device->destroyBuffer(stagingBuffer);
    device->freeMemory(stagingBufferMemory);
}

void Renderer::CreateTextureImageView() {

    for (auto& material : MaterialManager::Instance()->GetMaterials()) {
        material.textureImageView = CreateImageView(material.textureImage, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor);
    }
}

void Renderer::CreateTextureSampler() {
    vk::PhysicalDeviceProperties properties {};
    physicalDevice.getProperties(&properties);

    vk::SamplerCreateInfo samplerInfo {};
    samplerInfo.magFilter = vk::Filter::eLinear;
    samplerInfo.minFilter = vk::Filter::eLinear;
    samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = vk::CompareOp::eAlways;
    samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;

    try {
        auto result = device->createSampler(&samplerInfo, nullptr, &textureSampler);
    } catch (vk::SystemError err) {
        throw std::runtime_error("failed to create command pool!");
    }
}

vk::ImageView Renderer::CreateImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags) {
    vk::ImageViewCreateInfo createInfo = {};
    createInfo.image = image;
    createInfo.viewType = vk::ImageViewType::e2D;
    createInfo.format = format;
    createInfo.components.r = vk::ComponentSwizzle::eR;
    createInfo.components.g = vk::ComponentSwizzle::eG;
    createInfo.components.b = vk::ComponentSwizzle::eB;
    createInfo.components.a = vk::ComponentSwizzle::eA;
    createInfo.subresourceRange.aspectMask = aspectFlags;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    try {
        return device->createImageView(createInfo);
    } catch (vk::SystemError err) {
        throw std::runtime_error("failed to create image views!");
    }
}

void Renderer::CreateImage(vk::ImageCreateInfo imageInfo, vk::MemoryPropertyFlags properties, vk::Image& image, vk::DeviceMemory& imageMemory) {
    try {
        auto result = device->createImage(&imageInfo, nullptr, &image);
    } catch (vk::SystemError err) {
        throw std::runtime_error("failed to create image!");
    }

    vk::MemoryRequirements memRequirements;

    device->getImageMemoryRequirements(image, &memRequirements);

    vk::MemoryAllocateInfo allocInfo {};
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

    try {
        auto result = device->allocateMemory(&allocInfo, nullptr, &imageMemory);
    } catch (vk::SystemError err) {
        throw std::runtime_error("failed to allocate image memory");
    }
    device->bindImageMemory(image, imageMemory, 0);
}

void Renderer::TransitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) {
    vk::CommandBuffer commandBuffer = BeginSingleTimeCommands();

    vk::ImageMemoryBarrier barrier {};
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    vk::PipelineStageFlags sourceStage;
    vk::PipelineStageFlags destinationStage;

    if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {

        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe ;
        destinationStage = vk::PipelineStageFlagBits::eTransfer;
    } else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        sourceStage = vk::PipelineStageFlagBits::eTransfer;
        destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
    } else {
        throw std::invalid_argument("unsupported layout transition!");
    }
    commandBuffer.pipelineBarrier(
        sourceStage, destinationStage,
        vk::DependencyFlags(),
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    EndSingleTimeCommands(commandBuffer);
}

void Renderer::CopyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height) {
    vk::CommandBuffer commandBuffer = BeginSingleTimeCommands();

    vk::BufferImageCopy region {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = vk::Offset3D(0, 0, 0);
    region.imageExtent = vk::Extent3D(
        width,
        height,
        1
    );

    commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, 1, &region);

    EndSingleTimeCommands(commandBuffer);
}

void Renderer::CreateTextureImages() {
    for (auto& material : MaterialManager::Instance()->GetMaterials()) {
        LoadTextureImage(material, material.texturePath);
    }
}

void Renderer::CreateVertexBuffer(const std::vector<Vertex>& vertices) {
    vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    vk::Buffer stagingBuffer;
    vk::DeviceMemory stagingBufferMemory;
    CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);

    void* data = device->mapMemory(stagingBufferMemory, 0, bufferSize);
    memcpy(data, vertices.data(), (size_t) bufferSize);
    device->unmapMemory(stagingBufferMemory);

    CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, vertexBuffer, vertexBufferMemory);

    CopyBuffer(stagingBuffer, vertexBuffer, bufferSize);

    device->destroyBuffer(stagingBuffer);
    device->freeMemory(stagingBufferMemory);
}

void Renderer::CreateIndexBuffer(const std::vector<uint32_t>& indices) {
    vk::DeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    vk::Buffer stagingBuffer;
    vk::DeviceMemory stagingBufferMemory;
    CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);

    void* data = device->mapMemory(stagingBufferMemory, 0, bufferSize);
    memcpy(data, indices.data(), (size_t) bufferSize);
    device->unmapMemory(stagingBufferMemory);

    CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, indexBuffer, indexBufferMemory);

    CopyBuffer(stagingBuffer, indexBuffer, bufferSize);

    device->destroyBuffer(stagingBuffer);
    device->freeMemory(stagingBufferMemory);
}

void Renderer::CreateDebugLightBuffers() {
    const glm::vec3 color(1.0f, 1.0f, 0.0f);
    const glm::vec2 uv(0.0f, 0.0f);
    const int texIndex = 0;
    std::vector<Vertex> vertices = {
        { {-0.5f,-0.5f, 0.5f}, color, uv, { 0.0f, 0.0f, 1.0f}, texIndex },
        { { 0.5f,-0.5f, 0.5f}, color, uv, { 0.0f, 0.0f, 1.0f}, texIndex },
        { { 0.5f, 0.5f, 0.5f}, color, uv, { 0.0f, 0.0f, 1.0f}, texIndex },
        { {-0.5f, 0.5f, 0.5f}, color, uv, { 0.0f, 0.0f, 1.0f}, texIndex },
        { { 0.5f,-0.5f,-0.5f}, color, uv, { 0.0f, 0.0f,-1.0f}, texIndex },
        { {-0.5f,-0.5f,-0.5f}, color, uv, { 0.0f, 0.0f,-1.0f}, texIndex },
        { {-0.5f, 0.5f,-0.5f}, color, uv, { 0.0f, 0.0f,-1.0f}, texIndex },
        { { 0.5f, 0.5f,-0.5f}, color, uv, { 0.0f, 0.0f,-1.0f}, texIndex },
        { { 0.5f,-0.5f, 0.5f}, color, uv, { 1.0f, 0.0f, 0.0f}, texIndex },
        { { 0.5f,-0.5f,-0.5f}, color, uv, { 1.0f, 0.0f, 0.0f}, texIndex },
        { { 0.5f, 0.5f,-0.5f}, color, uv, { 1.0f, 0.0f, 0.0f}, texIndex },
        { { 0.5f, 0.5f, 0.5f}, color, uv, { 1.0f, 0.0f, 0.0f}, texIndex },
        { {-0.5f,-0.5f,-0.5f}, color, uv, {-1.0f, 0.0f, 0.0f}, texIndex },
        { {-0.5f,-0.5f, 0.5f}, color, uv, {-1.0f, 0.0f, 0.0f}, texIndex },
        { {-0.5f, 0.5f, 0.5f}, color, uv, {-1.0f, 0.0f, 0.0f}, texIndex },
        { {-0.5f, 0.5f,-0.5f}, color, uv, {-1.0f, 0.0f, 0.0f}, texIndex },
        { {-0.5f, 0.5f, 0.5f}, color, uv, { 0.0f, 1.0f, 0.0f}, texIndex },
        { { 0.5f, 0.5f, 0.5f}, color, uv, { 0.0f, 1.0f, 0.0f}, texIndex },
        { { 0.5f, 0.5f,-0.5f}, color, uv, { 0.0f, 1.0f, 0.0f}, texIndex },
        { {-0.5f, 0.5f,-0.5f}, color, uv, { 0.0f, 1.0f, 0.0f}, texIndex },
        { {-0.5f,-0.5f,-0.5f}, color, uv, { 0.0f,-1.0f, 0.0f}, texIndex },
        { { 0.5f,-0.5f,-0.5f}, color, uv, { 0.0f,-1.0f, 0.0f}, texIndex },
        { { 0.5f,-0.5f, 0.5f}, color, uv, { 0.0f,-1.0f, 0.0f}, texIndex },
        { {-0.5f,-0.5f, 0.5f}, color, uv, { 0.0f,-1.0f, 0.0f}, texIndex },
    };
    std::vector<uint32_t> indices = {
        0,1,2, 0,2,3, 4,5,6, 4,6,7, 8,9,10, 8,10,11,
        12,13,14, 12,14,15, 16,17,18, 16,18,19, 20,21,22, 20,22,23
    };
    debugLightIndexCount = static_cast<uint32_t>(indices.size());

    vk::DeviceSize vertexBufferSize = sizeof(vertices[0]) * vertices.size();
    vk::Buffer stagingVertexBuffer;
    vk::DeviceMemory stagingVertexBufferMemory;
    CreateBuffer(vertexBufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingVertexBuffer, stagingVertexBufferMemory);
    void* vertexData = device->mapMemory(stagingVertexBufferMemory, 0, vertexBufferSize);
    memcpy(vertexData, vertices.data(), static_cast<size_t>(vertexBufferSize));
    device->unmapMemory(stagingVertexBufferMemory);
    CreateBuffer(vertexBufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, debugLightVertexBuffer, debugLightVertexBufferMemory);
    CopyBuffer(stagingVertexBuffer, debugLightVertexBuffer, vertexBufferSize);
    device->destroyBuffer(stagingVertexBuffer);
    device->freeMemory(stagingVertexBufferMemory);

    vk::DeviceSize indexBufferSize = sizeof(indices[0]) * indices.size();
    vk::Buffer stagingIndexBuffer;
    vk::DeviceMemory stagingIndexBufferMemory;
    CreateBuffer(indexBufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingIndexBuffer, stagingIndexBufferMemory);
    void* indexData = device->mapMemory(stagingIndexBufferMemory, 0, indexBufferSize);
    memcpy(indexData, indices.data(), static_cast<size_t>(indexBufferSize));
    device->unmapMemory(stagingIndexBufferMemory);
    CreateBuffer(indexBufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, debugLightIndexBuffer, debugLightIndexBufferMemory);
    CopyBuffer(stagingIndexBuffer, debugLightIndexBuffer, indexBufferSize);
    device->destroyBuffer(stagingIndexBuffer);
    device->freeMemory(stagingIndexBufferMemory);
}

void Renderer::CreateDebugAxesBuffers() {
    const float L = 2.0f;
    const glm::vec2 uv(0.0f, 0.0f);
    const int texIndex = 0;
    const glm::vec3 nx(1.0f, 0.0f, 0.0f);
    const glm::vec3 ny(0.0f, 1.0f, 0.0f);
    const glm::vec3 nz(0.0f, 0.0f, 1.0f);
    std::vector<Vertex> vertices = {
        { { 0.f, 0.f, 0.f }, { 1.f, 0.f, 0.f }, uv, nx, texIndex },
        { {   L, 0.f, 0.f }, { 1.f, 0.f, 0.f }, uv, nx, texIndex },
        { { 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, uv, ny, texIndex },
        { { 0.f,   L, 0.f }, { 0.f, 1.f, 0.f }, uv, ny, texIndex },
        { { 0.f, 0.f, 0.f }, { 0.f, 0.f, 1.f }, uv, nz, texIndex },
        { { 0.f, 0.f,   L }, { 0.f, 0.f, 1.f }, uv, nz, texIndex },
    };
    debugAxesVertexCount = static_cast<uint32_t>(vertices.size());

    vk::DeviceSize vertexBufferSize = sizeof(vertices[0]) * vertices.size();
    vk::Buffer stagingVertexBuffer;
    vk::DeviceMemory stagingVertexBufferMemory;
    CreateBuffer(vertexBufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
        stagingVertexBuffer, stagingVertexBufferMemory);
    void* vertexData = device->mapMemory(stagingVertexBufferMemory, 0, vertexBufferSize);
    memcpy(vertexData, vertices.data(), static_cast<size_t>(vertexBufferSize));
    device->unmapMemory(stagingVertexBufferMemory);
    CreateBuffer(vertexBufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal,
        debugAxesVertexBuffer, debugAxesVertexBufferMemory);
    CopyBuffer(stagingVertexBuffer, debugAxesVertexBuffer, vertexBufferSize);
    device->destroyBuffer(stagingVertexBuffer);
    device->freeMemory(stagingVertexBufferMemory);
}

void Renderer::CreateUniformBuffers() {
    vk::DeviceSize bufferSize = sizeof(VertexUniformBufferObject);

    vertexUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    vertexUniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, vertexUniformBuffers[i], vertexUniformBuffersMemory[i]);
    }

    vk::DeviceSize fragBufferSize = sizeof(FragmentUniformBufferObject);

    fragmentUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    fragmentUniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        CreateBuffer(fragBufferSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, fragmentUniformBuffers[i], fragmentUniformBuffersMemory[i]);
    }
}

void Renderer::CreateDescriptorPool() {
    size_t materialSize = MaterialManager::Instance()->GetMaterialCount();
    std::array<vk::DescriptorPoolSize, 5> poolSizes {};
    poolSizes[0].type = vk::DescriptorType::eUniformBuffer;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    poolSizes[1].type = vk::DescriptorType::eCombinedImageSampler;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * static_cast<uint32_t>(materialSize);
    poolSizes[2].type = vk::DescriptorType::eUniformBuffer;
    poolSizes[2].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    poolSizes[3].type = vk::DescriptorType::eUniformBuffer;
    poolSizes[3].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    poolSizes[4].type = vk::DescriptorType::eCombinedImageSampler;
    poolSizes[4].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    vk::DescriptorPoolCreateInfo poolInfo {};
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * 2;

    try {
        auto result = device->createDescriptorPool(&poolInfo, nullptr, &descriptorPool);
    } catch (vk::SystemError err) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void Renderer::CreateDescriptorSets() {
    std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
    vk::DescriptorSetAllocateInfo allocInfo {};
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    try {
        auto result = device->allocateDescriptorSets(&allocInfo, descriptorSets.data());
    } catch (vk::SystemError err) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vk::DescriptorBufferInfo bufferInfo {};
        bufferInfo.buffer = vertexUniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(VertexUniformBufferObject);

        std::vector<vk::WriteDescriptorSet> descriptorWrites {};
        auto uniformSet = vk::WriteDescriptorSet {};
        uniformSet.dstSet = descriptorSets[i];
        uniformSet.dstBinding = 0;
        uniformSet.dstArrayElement = 0;
        uniformSet.descriptorType = vk::DescriptorType::eUniformBuffer;
        uniformSet.descriptorCount = 1;
        uniformSet.pBufferInfo = &bufferInfo;
        descriptorWrites.push_back(uniformSet);

        std::vector<vk::DescriptorImageInfo> imageInfos;
        size_t materialSize = MaterialManager::Instance()->GetMaterialCount();
        for (auto& material : MaterialManager::Instance()->GetMaterials()) {
            vk::DescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
            imageInfo.imageView = material.textureImageView;
            imageInfo.sampler = textureSampler;
            imageInfos.push_back(imageInfo);
        }
        auto descSet = vk::WriteDescriptorSet {};
        descSet.dstSet = descriptorSets[i];
        descSet.dstBinding = 1;
        descSet.dstArrayElement = 0;
        descSet.descriptorType = vk::DescriptorType::eCombinedImageSampler;
        descSet.descriptorCount = (static_cast<uint32_t>(materialSize));
        descSet.pImageInfo = imageInfos.data();
        descriptorWrites.push_back(descSet);

        vk::DescriptorBufferInfo fragbufferInfo{};
        fragbufferInfo.buffer = fragmentUniformBuffers[i];
        fragbufferInfo.offset = 0;
        fragbufferInfo.range = sizeof(FragmentUniformBufferObject);

        auto fragUniformSet = vk::WriteDescriptorSet{};
        fragUniformSet.dstSet = descriptorSets[i];
        fragUniformSet.dstBinding = 2;
        fragUniformSet.dstArrayElement = 0;
        fragUniformSet.descriptorType = vk::DescriptorType::eUniformBuffer;
        fragUniformSet.descriptorCount = 1;
        fragUniformSet.pBufferInfo = &fragbufferInfo;
        descriptorWrites.push_back(fragUniformSet);

        vk::DescriptorImageInfo shadowImageInfo{};
        shadowImageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        shadowImageInfo.imageView = shadowCubeMapView;
        shadowImageInfo.sampler = shadowDepthSampler;
        auto shadowSamplerSet = vk::WriteDescriptorSet{};
        shadowSamplerSet.dstSet = descriptorSets[i];
        shadowSamplerSet.dstBinding = 3;
        shadowSamplerSet.dstArrayElement = 0;
        shadowSamplerSet.descriptorType = vk::DescriptorType::eCombinedImageSampler;
        shadowSamplerSet.descriptorCount = 1;
        shadowSamplerSet.pImageInfo = &shadowImageInfo;
        descriptorWrites.push_back(shadowSamplerSet);

        try {
            device->updateDescriptorSets(static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        } catch (vk::SystemError err) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

    }
}

void Renderer::CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer& buffer, vk::DeviceMemory& bufferMemory) {
    vk::BufferCreateInfo bufferInfo = {};
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = vk::SharingMode::eExclusive;

    try {
        buffer = device->createBuffer(bufferInfo);
    } catch (vk::SystemError err) {
        throw std::runtime_error("failed to create buffer!");
    }

    vk::MemoryRequirements memRequirements = device->getBufferMemoryRequirements(buffer);

    vk::MemoryAllocateInfo allocInfo = {};
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

    try {
        bufferMemory = device->allocateMemory(allocInfo);
    } catch (vk::SystemError err) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }
    device->bindBufferMemory(buffer, bufferMemory, 0);
}

vk::CommandBuffer Renderer::BeginSingleTimeCommands() {
    vk::CommandBufferAllocateInfo allocInfo {};
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    vk::CommandBuffer commandBuffer;
    {
        auto result = device->allocateCommandBuffers(&allocInfo, &commandBuffer);
    }
    vk::CommandBufferBeginInfo beginInfo {};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    {
        auto result = commandBuffer.begin(&beginInfo);
    }

    return commandBuffer;
}

void Renderer::EndSingleTimeCommands(vk::CommandBuffer commandBuffer) {
    commandBuffer.end();

    vk::SubmitInfo submitInfo {};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    auto result = graphicsQueue.submit(1, &submitInfo, VK_NULL_HANDLE);
    graphicsQueue.waitIdle();

    device->freeCommandBuffers(commandPool, 1, &commandBuffer);
}

void Renderer::CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size) {
    vk::CommandBuffer commandBuffer = BeginSingleTimeCommands();

    vk::BufferCopy copyRegion {};
    copyRegion.size = size;
    commandBuffer.copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);

    EndSingleTimeCommands(commandBuffer);
}

uint32_t Renderer::FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
    vk::PhysicalDeviceMemoryProperties memProperties;
    physicalDevice.getMemoryProperties(&memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void Renderer::CreateCommandBuffers() {
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    vk::CommandBufferAllocateInfo allocInfo {};
    allocInfo.commandPool = commandPool;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();
    try {
        auto result = device->allocateCommandBuffers(&allocInfo, commandBuffers.data());
    } catch (vk::SystemError) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void Renderer::RecordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex) {
    vk::CommandBufferBeginInfo beginInfo {};

    try {
        auto result = commandBuffer.begin(&beginInfo);
    } catch (vk::SystemError) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    if constexpr (kEnableShadowPass) {

        static bool shadowFirstFrame = true;
        const uint32_t shadowArrayLayers = 6u * static_cast<uint32_t>(MAX_SHADOW_LIGHTS);
        vk::ImageMemoryBarrier shadowToWriteBarrier{};
        shadowToWriteBarrier.oldLayout = shadowFirstFrame ? vk::ImageLayout::eUndefined : vk::ImageLayout::eShaderReadOnlyOptimal;
        shadowToWriteBarrier.newLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
        shadowToWriteBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        shadowToWriteBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        shadowToWriteBarrier.image = shadowAttach.image;
        shadowToWriteBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
        shadowToWriteBarrier.subresourceRange.baseMipLevel = 0;
        shadowToWriteBarrier.subresourceRange.levelCount = 1;
        shadowToWriteBarrier.subresourceRange.baseArrayLayer = 0;
        shadowToWriteBarrier.subresourceRange.layerCount = shadowArrayLayers;
        shadowToWriteBarrier.srcAccessMask = shadowFirstFrame ? vk::AccessFlags() : vk::AccessFlagBits::eShaderRead;
        shadowToWriteBarrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        commandBuffer.pipelineBarrier(
            shadowFirstFrame ? vk::PipelineStageFlagBits::eTopOfPipe : vk::PipelineStageFlagBits::eFragmentShader,
            vk::PipelineStageFlagBits::eEarlyFragmentTests,
            vk::DependencyFlags(),
            0, nullptr, 0, nullptr, 1, &shadowToWriteBarrier);
        shadowFirstFrame = false;

        vk::Viewport shadowViewport{};
        shadowViewport.x = 0.f;
        shadowViewport.y = 0.f;
        shadowViewport.width = (float)SHADOW_WIDTH;
        shadowViewport.height = (float)SHADOW_HEIGHT;
        shadowViewport.minDepth = 0.f;
        shadowViewport.maxDepth = 1.f;
        commandBuffer.setViewport(0, 1, &shadowViewport);

        vk::Rect2D scissor{};
        scissor.offset = vk::Offset2D(0, 0);
        scissor.extent.width = SHADOW_WIDTH;
        scissor.extent.height = SHADOW_HEIGHT;
        commandBuffer.setScissor(0, 1, &scissor);

        {
            vk::ImageMemoryBarrier clearPrep{};
            clearPrep.oldLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
            clearPrep.newLayout = vk::ImageLayout::eTransferDstOptimal;
            clearPrep.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            clearPrep.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            clearPrep.image = shadowAttach.image;
            clearPrep.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
            clearPrep.subresourceRange.baseMipLevel = 0;
            clearPrep.subresourceRange.levelCount = 1;
            clearPrep.subresourceRange.baseArrayLayer = 0;
            clearPrep.subresourceRange.layerCount = shadowArrayLayers;
            clearPrep.srcAccessMask = vk::AccessFlags();
            clearPrep.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
            commandBuffer.pipelineBarrier(
                vk::PipelineStageFlagBits::eTopOfPipe,
                vk::PipelineStageFlagBits::eTransfer,
                vk::DependencyFlags(),
                0, nullptr, 0, nullptr, 1, &clearPrep);

            vk::ClearDepthStencilValue clearFar{1.0f, 0};
            vk::ImageSubresourceRange clearRange{};
            clearRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
            clearRange.baseMipLevel = 0;
            clearRange.levelCount = 1;
            clearRange.baseArrayLayer = 0;
            clearRange.layerCount = shadowArrayLayers;
            commandBuffer.clearDepthStencilImage(shadowAttach.image, vk::ImageLayout::eTransferDstOptimal, clearFar, clearRange);

            vk::ImageMemoryBarrier clearDone{};
            clearDone.oldLayout = vk::ImageLayout::eTransferDstOptimal;
            clearDone.newLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
            clearDone.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            clearDone.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            clearDone.image = shadowAttach.image;
            clearDone.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
            clearDone.subresourceRange.baseMipLevel = 0;
            clearDone.subresourceRange.levelCount = 1;
            clearDone.subresourceRange.baseArrayLayer = 0;
            clearDone.subresourceRange.layerCount = shadowArrayLayers;
            clearDone.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            clearDone.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
            commandBuffer.pipelineBarrier(
                vk::PipelineStageFlagBits::eTransfer,
                vk::PipelineStageFlagBits::eEarlyFragmentTests,
                vk::DependencyFlags(),
                0, nullptr, 0, nullptr, 1, &clearDone);
        }

        uint32_t shadowLightCount = 0;
        if (shadowSystem) {
            shadowLightCount = static_cast<uint32_t>(std::min(shadowSystem->entities.size(), static_cast<size_t>(MAX_SHADOW_LIGHTS)));
        }
        for (uint32_t cube = 0; cube < shadowLightCount; cube++) {
            for (uint32_t face = 0; face < 6; face++) {
                if ((shadowCubeFaceMask & (1u << face)) != 0u) {
                    UpdateShadowCubeFace(face, cube, commandBuffer);
                }
            }
        }

        vk::ImageMemoryBarrier shadowBarrier{};
        shadowBarrier.oldLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
        shadowBarrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        shadowBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        shadowBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        shadowBarrier.image = shadowAttach.image;
        shadowBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
        shadowBarrier.subresourceRange.baseMipLevel = 0;
        shadowBarrier.subresourceRange.levelCount = 1;
        shadowBarrier.subresourceRange.baseArrayLayer = 0;
        shadowBarrier.subresourceRange.layerCount = shadowArrayLayers;
        shadowBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite
            | vk::AccessFlagBits::eDepthStencilAttachmentRead
            | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        shadowBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
        commandBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer | vk::PipelineStageFlagBits::eLateFragmentTests,
            vk::PipelineStageFlagBits::eFragmentShader,
            vk::DependencyFlags(),
            0, nullptr, 0, nullptr, 1, &shadowBarrier);
    }

    if constexpr (!kEnableShadowPass) {
        static bool shadowImgShaderReadReady = false;
        vk::ImageMemoryBarrier shadowDisabledBarrier{};
        shadowDisabledBarrier.oldLayout = shadowImgShaderReadReady ? vk::ImageLayout::eShaderReadOnlyOptimal : vk::ImageLayout::eUndefined;
        shadowDisabledBarrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        shadowDisabledBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        shadowDisabledBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        shadowDisabledBarrier.image = shadowAttach.image;
        shadowDisabledBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
        shadowDisabledBarrier.subresourceRange.baseMipLevel = 0;
        shadowDisabledBarrier.subresourceRange.levelCount = 1;
        shadowDisabledBarrier.subresourceRange.baseArrayLayer = 0;
        shadowDisabledBarrier.subresourceRange.layerCount = 6u * static_cast<uint32_t>(MAX_SHADOW_LIGHTS);
        shadowDisabledBarrier.srcAccessMask = shadowImgShaderReadReady ? vk::AccessFlagBits::eShaderRead : vk::AccessFlags();
        shadowDisabledBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
        commandBuffer.pipelineBarrier(
            shadowImgShaderReadReady ? vk::PipelineStageFlagBits::eFragmentShader : vk::PipelineStageFlagBits::eTopOfPipe,
            vk::PipelineStageFlagBits::eFragmentShader,
            vk::DependencyFlags(),
            0, nullptr, 0, nullptr, 1, &shadowDisabledBarrier);
        shadowImgShaderReadReady = true;
    }

    vk::RenderPassBeginInfo renderPassInfo {};
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = vk::Offset2D(0, 0);
    renderPassInfo.renderArea.extent = swapChainExtent;

    vk::ClearColorValue clearColorValue = {std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 1.0f }};

    std::array<vk::ClearValue, 2> clearValues {};
    clearValues[0].color = clearColorValue;
    clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);

    vk::Viewport viewport{};
    viewport.x = 0.f;
    viewport.y = 0.f;
    viewport.width = (float)swapChainExtent.width;
    viewport.height = (float)swapChainExtent.height;
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;
    commandBuffer.setViewport(0, 1, &viewport);

    vk::Rect2D scissor {};
    scissor.offset = vk::Offset2D(0, 0);
    scissor.extent = swapChainExtent;
    commandBuffer.setScissor(0, 1, &scissor);

    vk::Buffer vertexBuffers[] = {vertexBuffer};
    vk::DeviceSize offsets[] = {0};
    commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);

    commandBuffer.bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint32);

    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);

    renderSystem->UpdateCommandBuffer(commandBuffer, pipelineLayout);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, debugAxesPipeline);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);
    vk::Buffer axesVertexBuffers[] = { debugAxesVertexBuffer };
    vk::DeviceSize axesOffsets[] = { 0 };
    commandBuffer.bindVertexBuffers(0, 1, axesVertexBuffers, axesOffsets);
    {
        MeshPushConstants axesPush{};
        axesPush.transform = glm::mat4(1.0f);
        axesPush.normal_matrix = glm::mat4(1.0f);
        commandBuffer.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(MeshPushConstants), &axesPush);
    }
    commandBuffer.draw(debugAxesVertexCount, 1, 0, 0);

    if (shadowSystem) {
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, debugPipeline);
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);
        vk::Buffer debugVertexBuffers[] = { debugLightVertexBuffer };
        vk::DeviceSize debugOffsets[] = { 0 };
        commandBuffer.bindVertexBuffers(0, 1, debugVertexBuffers, debugOffsets);
        commandBuffer.bindIndexBuffer(debugLightIndexBuffer, 0, vk::IndexType::eUint32);
        const float debugMarkerScale = 0.05f;
        const size_t nShadow = shadowSystem->entities.size();
        for (size_t i = 0; i < nShadow; ++i) {
            glm::vec3 lightPos = shadowSystem->GetShadowLightPosition(i);
            glm::mat4 model = glm::translate(glm::mat4(1.0f), lightPos) * glm::scale(glm::mat4(1.0f), glm::vec3(debugMarkerScale));
            MeshPushConstants debugPushConstants{};
            debugPushConstants.transform = model;
            debugPushConstants.normal_matrix = glm::transpose(glm::inverse(model));
            commandBuffer.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(MeshPushConstants), &debugPushConstants);
            commandBuffer.drawIndexed(debugLightIndexCount, 1, 0, 0, 0);
        }
    }

    commandBuffer.endRenderPass();

    try {
        commandBuffer.end();
    } catch (vk::SystemError) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void Renderer::CreateSyncObjects() {
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    try {
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            imageAvailableSemaphores[i] = device->createSemaphore({});
            renderFinishedSemaphores[i] = device->createSemaphore({});
            inFlightFences[i] = device->createFence({vk::FenceCreateFlagBits::eSignaled});
        }
    } catch (vk::SystemError err) {
        throw std::runtime_error("failed to create synchronization objects for a frame!");
    }
}

void Renderer::UpdateUniformBuffer(uint32_t currentImage) {
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    std::vector<glm::mat4> modelMatrices;
    auto& transformComponent = ecsManager.GetComponent<TransformComponent>(scene->GetMainCamera());
    auto& cameraComponent = ecsManager.GetComponent<CameraComponent>(scene->GetMainCamera());
    VertexUniformBufferObject ubo_v {};
    FragmentUniformBufferObject ubo_f {};
    const glm::vec3 eye = transformComponent.translation;
    const glm::vec3 forward = transformComponent.GetForwardVector();
    const glm::vec3 upVulkan(0.f, -1.f, 0.f);
    auto view = glm::lookAt(eye, eye + forward, upVulkan);
    auto proj = glm::perspective(cameraComponent.fov, swapChainExtent.width / (float) swapChainExtent.height, 0.01f, 100.0f);
    ubo_v.view_proj = proj * view;
    if (updateLightSystem) {
        updateLightSystem->UpdateLightInUBO(ubo_f);
        if (shadowSystem) {
            shadowSystem->AssignShadowCubeIndices(ubo_f, updateLightSystem->GetOrderedLightEntities());
        }
    }
    ubo_f.shadow_near = 0.0001f;
    ubo_f.shadow_far = 50.0f;
    ubo_f.render_mode = normalViewMode ? 1.f : 0.f;
    ubo_f._pad_render_mode = 0.f;

    void* data = device->mapMemory(vertexUniformBuffersMemory[currentImage], vk::DeviceSize(0), sizeof(ubo_v));
    memcpy(data, &ubo_v, sizeof(ubo_v));
    device->unmapMemory(vertexUniformBuffersMemory[currentImage]);

    void* fragData = device->mapMemory(fragmentUniformBuffersMemory[currentImage], vk::DeviceSize(0), sizeof(ubo_f));
    memcpy(fragData, &ubo_f, sizeof(ubo_f));
    device->unmapMemory(fragmentUniformBuffersMemory[currentImage]);

}

void Renderer::DrawFrame() {
    {
        auto result = device->waitForFences(1, &inFlightFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
    }

    uint32_t imageIndex;
    try {
        vk::ResultValue result = device->acquireNextImageKHR(swapChain, std::numeric_limits<uint64_t>::max(),
            imageAvailableSemaphores[currentFrame], nullptr);
        imageIndex = result.value;
    } catch (vk::OutOfDateKHRError err) {
        RecreateSwapChain();
        return;
    } catch (vk::SystemError err) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    UpdateUniformBuffer(uint32_t(currentFrame));

    {
        auto result = device->resetFences(1, &inFlightFences[currentFrame]);
    }

    commandBuffers[currentFrame].reset();
    RecordCommandBuffer(commandBuffers[currentFrame], imageIndex);

    vk::SubmitInfo submitInfo {};

    vk::Semaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

    vk::Semaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    try {
        graphicsQueue.submit(submitInfo, inFlightFences[currentFrame]);
    } catch (vk::SystemError err) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    vk::PresentInfoKHR presentInfo = {};
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    vk::SwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    vk::Result resultPresent;
    try {
        resultPresent = presentQueue.presentKHR(presentInfo);
    } catch (vk::OutOfDateKHRError err) {
        resultPresent = vk::Result::eErrorOutOfDateKHR;
    } catch (vk::SystemError err) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    if (resultPresent == vk::Result::eErrorOutOfDateKHR || resultPresent == vk::Result::eSuboptimalKHR || framebufferResized) {
        framebufferResized = false;
        RecreateSwapChain();
        return;
    }
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

bool Renderer::WindowShouldClose()
{
    return glfwWindowShouldClose(window.get());
}

void Renderer::WaitDevice()
{
    device->waitIdle();
}

vk::UniqueShaderModule Renderer::CreateShaderModule(const std::vector<char>& code) {
    try {
        return device->createShaderModuleUnique({
            vk::ShaderModuleCreateFlags(),
            code.size(),
            reinterpret_cast<const uint32_t*>(code.data())
        });
    } catch (vk::SystemError err) {
        throw std::runtime_error("failed to create shader module!");
    }
}

vk::SurfaceFormatKHR Renderer::ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
    if (availableFormats.size() == 1 && availableFormats[0].format == vk::Format::eUndefined) {
        return {vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear};
    }

    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

vk::PresentModeKHR Renderer::ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) {
    vk::PresentModeKHR bestMode = vk::PresentModeKHR::eFifo;

    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
            return availablePresentMode;
        } else if (availablePresentMode == vk::PresentModeKHR::eImmediate) {
            bestMode = availablePresentMode;
        }
    }

    return bestMode;
}

vk::Extent2D Renderer::ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window.get(), &width, &height);

        vk::Extent2D actualExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}

SwapChainSupportDetails Renderer::QuerySwapChainSupport(const vk::PhysicalDevice& device) {
    SwapChainSupportDetails details;
    details.capabilities = device.getSurfaceCapabilitiesKHR(surface);
    details.formats = device.getSurfaceFormatsKHR(surface);
    details.presentModes = device.getSurfacePresentModesKHR(surface);

    return details;
}

bool Renderer::IsDeviceSuitable(vk::PhysicalDevice physicalDevice) {
    QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);

    bool extensionsSupported = CheckDeviceExtensionSupport(physicalDevice);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(physicalDevice);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    vk::PhysicalDeviceFeatures supportedFeatures;
    physicalDevice.getFeatures(&supportedFeatures);

    return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy
        && supportedFeatures.imageCubeArray;
}

bool Renderer::CheckDeviceExtensionSupport(vk::PhysicalDevice device) {
    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : device.enumerateDeviceExtensionProperties()) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

QueueFamilyIndices Renderer::FindQueueFamilies(vk::PhysicalDevice device) {
    QueueFamilyIndices indices;

    auto queueFamilies = device.getQueueFamilyProperties();

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
            indices.graphicsFamily = i;
        }

        if (queueFamily.queueCount > 0 && device.getSurfaceSupportKHR(i, surface)) {
            indices.presentFamily = i;
        }

        if (indices.isComplete()) {
            break;
        }

        i++;
    }

    return indices;
}

std::vector<const char*> Renderer::GetRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

bool Renderer::CheckValidationLayerSupport() {
    auto availableLayers = vk::enumerateInstanceLayerProperties();
    for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

std::vector<char> Renderer::ReadFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

VKAPI_ATTR VkBool32 VKAPI_CALL Renderer::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

VkResult Renderer::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pCallback) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pCallback);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void Renderer::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT callback, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, callback, pAllocator);
    }
}

void Renderer::Update(float dt)
{
    InputManager* input = InputManager::Instance();
    const bool pressed = input->InputPressed(Input::Action::ToggleNormalView);
    if (pressed && !toggleNormalViewPrev) {
        normalViewMode = !normalViewMode;
    }
    toggleNormalViewPrev = pressed;

    GLFWwindow* w = window.get();
    for (int i = 0; i < 6; i++) {
        const bool down = glfwGetKey(w, GLFW_KEY_1 + i) == GLFW_PRESS;
        if (down && !shadowFaceTogglePrev[static_cast<size_t>(i)]) {
            shadowCubeFaceMask ^= (1u << static_cast<uint32_t>(i));
        }
        shadowFaceTogglePrev[static_cast<size_t>(i)] = down;
    }
}
