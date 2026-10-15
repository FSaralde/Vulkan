#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>
#include <stdexcept>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <fstream>
#include <array>

std::vector<char> readFile(const std::string& filename) {

    std::ifstream file(
        filename,
        std::ios::ate | std::ios::binary
    );

    if (!file.is_open()) {
        throw std::runtime_error(
            "Failed to open file: " + filename
        );
    }

    size_t fileSize =
        static_cast<size_t>(file.tellg());

    std::vector<char> buffer(fileSize);

    file.seekg(0);

    file.read(
        buffer.data(),
        fileSize
    );

    file.close();

    return buffer;
}
VkShaderModule createShaderModule(
    VkDevice device,
    const std::vector<char>& code
) {

    VkShaderModuleCreateInfo createInfo{};

    createInfo.sType =
        VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    createInfo.codeSize =
        code.size();

    createInfo.pCode =
        reinterpret_cast<const uint32_t*>(
            code.data()
        );

    VkShaderModule shaderModule;

    if (vkCreateShaderModule(
            device,
            &createInfo,
            nullptr,
            &shaderModule
        ) != VK_SUCCESS) {

        throw std::runtime_error(
            "Failed to create shader module"
        );
    }

    return shaderModule;
}

int main() {

    // =========================================================
    // Initialize GLFW
    // =========================================================

    if (!glfwInit()) {
        throw std::runtime_error(
            "Failed to initialize GLFW"
        );
    }

    std::cout
        << "GLFW: "
        << glfwGetVersionString()
        << "\n";

    glfwWindowHint(
        GLFW_CLIENT_API,
        GLFW_NO_API
    );

    GLFWwindow* window =
        glfwCreateWindow(
            800,
            600,
            "My Vulkan Engine",
            nullptr,
            nullptr
        );

    if (!window) {
        glfwTerminate();

        throw std::runtime_error(
            "Failed to create GLFW window"
        );
    }


    // =========================================================
    // Check Vulkan Instance Extensions
    // =========================================================

    uint32_t extensionCount = 0;

    if (vkEnumerateInstanceExtensionProperties(
            nullptr,
            &extensionCount,
            nullptr
        ) != VK_SUCCESS) {

        glfwDestroyWindow(window);
        glfwTerminate();

        throw std::runtime_error(
            "Failed to enumerate Vulkan extensions"
        );
    }

    std::cout
        << "Vulkan reports "
        << extensionCount
        << " instance extensions\n";


    // =========================================================
    // Vulkan Application Info
    // =========================================================

    VkApplicationInfo appInfo{};

    appInfo.sType =
        VK_STRUCTURE_TYPE_APPLICATION_INFO;

    appInfo.pApplicationName =
        "My Vulkan Engine";

    appInfo.applicationVersion =
        VK_MAKE_VERSION(1, 0, 0);

    appInfo.pEngineName =
        "My Engine";

    appInfo.engineVersion =
        VK_MAKE_VERSION(1, 0, 0);

    appInfo.apiVersion =
        VK_API_VERSION_1_3;


    // =========================================================
    // Get GLFW Vulkan Extensions
    // =========================================================

    uint32_t glfwExtensionCount = 0;

    const char** glfwExtensions =
        glfwGetRequiredInstanceExtensions(
            &glfwExtensionCount
        );

    if (!glfwExtensions ||
        glfwExtensionCount == 0) {

        glfwDestroyWindow(window);
        glfwTerminate();

        throw std::runtime_error(
            "Failed to get GLFW Vulkan extensions"
        );
    }

    std::vector<const char*> extensions(
        glfwExtensions,
        glfwExtensions + glfwExtensionCount
    );

    extensions.push_back(
        VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
    );


    // =========================================================
    // Create Vulkan Instance
    // =========================================================

    VkInstanceCreateInfo createInfo{};

    createInfo.sType =
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    createInfo.flags |=
        VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

    createInfo.pApplicationInfo =
        &appInfo;

    createInfo.enabledExtensionCount =
        static_cast<uint32_t>(
            extensions.size()
        );

    createInfo.ppEnabledExtensionNames =
        extensions.data();

    VkInstance instance;

    if (vkCreateInstance(
            &createInfo,
            nullptr,
            &instance
        ) != VK_SUCCESS) {

        glfwDestroyWindow(window);
        glfwTerminate();

        throw std::runtime_error(
            "Failed to create Vulkan instance"
        );
    }

    std::cout
        << "Vulkan instance created successfully!\n";


    // =========================================================
    // Find Physical Devices
    // =========================================================

    uint32_t deviceCount = 0;

    if (vkEnumeratePhysicalDevices(
            instance,
            &deviceCount,
            nullptr
        ) != VK_SUCCESS) {

        vkDestroyInstance(
            instance,
            nullptr
        );

        glfwDestroyWindow(window);
        glfwTerminate();

        throw std::runtime_error(
            "Failed to enumerate physical devices"
        );
    }

    if (deviceCount == 0) {

        vkDestroyInstance(
            instance,
            nullptr
        );

        glfwDestroyWindow(window);
        glfwTerminate();

        throw std::runtime_error(
            "Failed to find GPUs with Vulkan support"
        );
    }

    std::vector<VkPhysicalDevice> devices(
        deviceCount
    );

    vkEnumeratePhysicalDevices(
        instance,
        &deviceCount,
        devices.data()
    );

    std::cout
        << "Found "
        << deviceCount
        << " Vulkan GPU(s)\n";


    // =========================================================
    // Select Physical Device
    // =========================================================

    VkPhysicalDevice physicalDevice =
        devices[0];

    VkPhysicalDeviceProperties deviceProperties{};

    vkGetPhysicalDeviceProperties(
        physicalDevice,
        &deviceProperties
    );

    std::cout
        << "Using GPU: "
        << deviceProperties.deviceName
        << "\n";


    // =========================================================
    // Find Queue Families
    // =========================================================

    uint32_t queueFamilyCount = 0;

    vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice,
        &queueFamilyCount,
        nullptr
    );

    std::vector<VkQueueFamilyProperties> queueFamilies(
        queueFamilyCount
    );

    vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice,
        &queueFamilyCount,
        queueFamilies.data()
    );

    int graphicsQueueFamily = -1;

    for (uint32_t i = 0;
         i < queueFamilyCount;
         i++) {

        if (queueFamilies[i].queueFlags &
            VK_QUEUE_GRAPHICS_BIT) {

            graphicsQueueFamily =
                static_cast<int>(i);

            std::cout
                << "Graphics queue family found: "
                << i
                << "\n";

            break;
        }
    }

    if (graphicsQueueFamily == -1) {

        vkDestroyInstance(
            instance,
            nullptr
        );

        glfwDestroyWindow(window);
        glfwTerminate();

        throw std::runtime_error(
            "Failed to find graphics queue family"
        );
    }


    // =========================================================
    // Create Logical Device
    // =========================================================

    float queuePriority = 1.0f;

    VkDeviceQueueCreateInfo queueCreateInfo{};

    queueCreateInfo.sType =
        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;

    queueCreateInfo.queueFamilyIndex =
        static_cast<uint32_t>(
            graphicsQueueFamily
        );

    queueCreateInfo.queueCount = 1;

    queueCreateInfo.pQueuePriorities =
        &queuePriority;


    // =========================================================
    // Device Features
    // =========================================================

    VkPhysicalDeviceFeatures deviceFeatures{};


    // =========================================================
    // Device Extensions
    // =========================================================

    const char* deviceExtensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        "VK_KHR_portability_subset"
    };


    // =========================================================
    // Create Logical Device
    // =========================================================

    VkDeviceCreateInfo deviceCreateInfo{};

    deviceCreateInfo.sType =
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    deviceCreateInfo.pQueueCreateInfos =
        &queueCreateInfo;

    deviceCreateInfo.queueCreateInfoCount = 1;

    deviceCreateInfo.pEnabledFeatures =
        &deviceFeatures;

    deviceCreateInfo.enabledExtensionCount =
        sizeof(deviceExtensions) /
        sizeof(deviceExtensions[0]);

    deviceCreateInfo.ppEnabledExtensionNames =
        deviceExtensions;

    VkDevice device;

    if (vkCreateDevice(
            physicalDevice,
            &deviceCreateInfo,
            nullptr,
            &device
        ) != VK_SUCCESS) {

        vkDestroyInstance(
            instance,
            nullptr
        );

        glfwDestroyWindow(window);
        glfwTerminate();

        throw std::runtime_error(
            "Failed to create logical device"
        );
    }

    // =========================================================
// Command Pool
// =========================================================

VkCommandPoolCreateInfo poolInfo{};

poolInfo.sType =
    VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

poolInfo.flags =
    VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

poolInfo.queueFamilyIndex =
    static_cast<uint32_t>(graphicsQueueFamily);

VkCommandPool commandPool;

if (vkCreateCommandPool(
        device,
        &poolInfo,
        nullptr,
        &commandPool
    ) != VK_SUCCESS) {

    throw std::runtime_error(
        "Failed to create command pool"
    );
}

std::cout << "Command pool created successfully!\n";

    std::cout
        << "Logical device created successfully!\n";


    // =========================================================
    // Get Graphics Queue
    // =========================================================

    VkQueue graphicsQueue;

    vkGetDeviceQueue(
        device,
        static_cast<uint32_t>(
            graphicsQueueFamily
        ),
        0,
        &graphicsQueue
    );

    std::cout
        << "Graphics queue obtained successfully!\n";


    // =========================================================
    // Create Window Surface
    // =========================================================

    VkSurfaceKHR surface;

    if (glfwCreateWindowSurface(
            instance,
            window,
            nullptr,
            &surface
        ) != VK_SUCCESS) {

        vkDestroyDevice(
            device,
            nullptr
        );

        vkDestroyInstance(
            instance,
            nullptr
        );

        glfwDestroyWindow(window);
        glfwTerminate();

        throw std::runtime_error(
            "Failed to create window surface"
        );
    }

    std::cout
        << "Window surface created successfully!\n";


    // =========================================================
    // Query Surface Capabilities
    // =========================================================

    VkSurfaceCapabilitiesKHR surfaceCapabilities{};

    if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
            physicalDevice,
            surface,
            &surfaceCapabilities
        ) != VK_SUCCESS) {

        throw std::runtime_error(
            "Failed to get surface capabilities"
        );
    }


    // =========================================================
    // Query Surface Formats
    // =========================================================

    uint32_t formatCount = 0;

    vkGetPhysicalDeviceSurfaceFormatsKHR(
        physicalDevice,
        surface,
        &formatCount,
        nullptr
    );

    if (formatCount == 0) {
        throw std::runtime_error(
            "No surface formats available"
        );
    }

    std::vector<VkSurfaceFormatKHR> surfaceFormats(
        formatCount
    );

    vkGetPhysicalDeviceSurfaceFormatsKHR(
        physicalDevice,
        surface,
        &formatCount,
        surfaceFormats.data()
    );


    // =========================================================
    // Query Present Modes
    // =========================================================

    uint32_t presentModeCount = 0;

    vkGetPhysicalDeviceSurfacePresentModesKHR(
        physicalDevice,
        surface,
        &presentModeCount,
        nullptr
    );

    if (presentModeCount == 0) {
        throw std::runtime_error(
            "No presentation modes available"
        );
    }

    std::vector<VkPresentModeKHR> presentModes(
        presentModeCount
    );

    vkGetPhysicalDeviceSurfacePresentModesKHR(
        physicalDevice,
        surface,
        &presentModeCount,
        presentModes.data()
    );

    std::cout
        << "Surface formats: "
        << formatCount
        << "\n";

    std::cout
        << "Present modes: "
        << presentModeCount
        << "\n";


    // =========================================================
    // Choose Surface Format
    // =========================================================

    VkSurfaceFormatKHR surfaceFormat =
        surfaceFormats[0];

    for (const auto& availableFormat :
         surfaceFormats) {

        if (availableFormat.format ==
                VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace ==
                VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {

            surfaceFormat =
                availableFormat;

            break;
        }
    }


    // =========================================================
    // Choose Present Mode
    // =========================================================

    VkPresentModeKHR presentMode =
        VK_PRESENT_MODE_FIFO_KHR;


    // =========================================================
    // Choose Swapchain Extent
    // =========================================================

    VkExtent2D swapchainExtent;

    if (surfaceCapabilities.currentExtent.width !=
            UINT32_MAX) {

        swapchainExtent =
            surfaceCapabilities.currentExtent;

    } else {

        int width;
        int height;

        glfwGetFramebufferSize(
            window,
            &width,
            &height
        );

        swapchainExtent.width =
            static_cast<uint32_t>(width);

        swapchainExtent.height =
            static_cast<uint32_t>(height);

        swapchainExtent.width =
            std::clamp(
                swapchainExtent.width,
                surfaceCapabilities.minImageExtent.width,
                surfaceCapabilities.maxImageExtent.width
            );

        swapchainExtent.height =
            std::clamp(
                swapchainExtent.height,
                surfaceCapabilities.minImageExtent.height,
                surfaceCapabilities.maxImageExtent.height
            );
    }


    // =========================================================
    // Choose Swapchain Image Count
    // =========================================================

    uint32_t imageCount =
        surfaceCapabilities.minImageCount + 1;

    if (surfaceCapabilities.maxImageCount > 0 &&
        imageCount >
            surfaceCapabilities.maxImageCount) {

        imageCount =
            surfaceCapabilities.maxImageCount;
    }


    // =========================================================
    // Create Swapchain
    // =========================================================

    VkSwapchainCreateInfoKHR swapchainCreateInfo{};

    swapchainCreateInfo.sType =
        VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;

    swapchainCreateInfo.surface =
        surface;

    swapchainCreateInfo.minImageCount =
        imageCount;

    swapchainCreateInfo.imageFormat =
        surfaceFormat.format;

    swapchainCreateInfo.imageColorSpace =
        surfaceFormat.colorSpace;

    swapchainCreateInfo.imageExtent =
        swapchainExtent;

    swapchainCreateInfo.imageArrayLayers =
        1;

    swapchainCreateInfo.imageUsage =
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    swapchainCreateInfo.imageSharingMode =
        VK_SHARING_MODE_EXCLUSIVE;

    swapchainCreateInfo.preTransform =
        surfaceCapabilities.currentTransform;

    swapchainCreateInfo.compositeAlpha =
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    swapchainCreateInfo.presentMode =
        presentMode;

    swapchainCreateInfo.clipped =
        VK_TRUE;

    swapchainCreateInfo.oldSwapchain =
        VK_NULL_HANDLE;

    VkSwapchainKHR swapchain;

    if (vkCreateSwapchainKHR(
            device,
            &swapchainCreateInfo,
            nullptr,
            &swapchain
        ) != VK_SUCCESS) {

        throw std::runtime_error(
            "Failed to create swapchain"
        );
    }

    std::cout
        << "Swapchain created successfully!\n";


    // =========================================================
    // Get Swapchain Images
    // =========================================================

    uint32_t swapchainImageCount = 0;

    vkGetSwapchainImagesKHR(
        device,
        swapchain,
        &swapchainImageCount,
        nullptr
    );

    std::vector<VkImage> swapchainImages(
        swapchainImageCount
    );

    vkGetSwapchainImagesKHR(
        device,
        swapchain,
        &swapchainImageCount,
        swapchainImages.data()
    );

    std::cout
        << "Swapchain images: "
        << swapchainImageCount
        << "\n";
    // =========================================================
// Create Swapchain Image Views
// =========================================================

std::vector<VkImageView> swapchainImageViews(
    swapchainImages.size()
);

for (size_t i = 0; i < swapchainImages.size(); i++) {

    VkImageViewCreateInfo viewInfo{};

    viewInfo.sType =
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

    viewInfo.image =
        swapchainImages[i];

    viewInfo.viewType =
        VK_IMAGE_VIEW_TYPE_2D;

    viewInfo.format =
        surfaceFormat.format;

    viewInfo.components.r =
        VK_COMPONENT_SWIZZLE_IDENTITY;

    viewInfo.components.g =
        VK_COMPONENT_SWIZZLE_IDENTITY;

    viewInfo.components.b =
        VK_COMPONENT_SWIZZLE_IDENTITY;

    viewInfo.components.a =
        VK_COMPONENT_SWIZZLE_IDENTITY;

    viewInfo.subresourceRange.aspectMask =
        VK_IMAGE_ASPECT_COLOR_BIT;

    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;

    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(
            device,
            &viewInfo,
            nullptr,
            &swapchainImageViews[i]
        ) != VK_SUCCESS) {

        throw std::runtime_error(
            "Fuck Yourself"
        );
    }
}

std::cout
    << "Created "
    << swapchainImageViews.size()
    << " swapchain image views\n";


    // =========================================================
    // Create Render Pass
    // =========================================================

    VkAttachmentDescription colorAttachment{};

    colorAttachment.format =
        surfaceFormat.format;

    colorAttachment.samples =
        VK_SAMPLE_COUNT_1_BIT;

    colorAttachment.loadOp =
        VK_ATTACHMENT_LOAD_OP_CLEAR;

    colorAttachment.storeOp =
        VK_ATTACHMENT_STORE_OP_STORE;

    colorAttachment.stencilLoadOp =
        VK_ATTACHMENT_LOAD_OP_DONT_CARE;

    colorAttachment.stencilStoreOp =
        VK_ATTACHMENT_STORE_OP_DONT_CARE;

    colorAttachment.initialLayout =
        VK_IMAGE_LAYOUT_UNDEFINED;

    colorAttachment.finalLayout =
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};

    colorAttachmentRef.attachment = 0;

    colorAttachmentRef.layout =
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};

    subpass.pipelineBindPoint =
        VK_PIPELINE_BIND_POINT_GRAPHICS;

    subpass.colorAttachmentCount = 1;

    subpass.pColorAttachments =
        &colorAttachmentRef;

    VkSubpassDependency dependency{};

    dependency.srcSubpass =
        VK_SUBPASS_EXTERNAL;

    dependency.dstSubpass = 0;

    dependency.srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    dependency.srcAccessMask = 0;

    dependency.dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    dependency.dstAccessMask =
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};

    renderPassInfo.sType =
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

    renderPassInfo.attachmentCount = 1;

    renderPassInfo.pAttachments =
        &colorAttachment;

    renderPassInfo.subpassCount = 1;

    renderPassInfo.pSubpasses =
        &subpass;

    renderPassInfo.dependencyCount = 1;

    renderPassInfo.pDependencies =
        &dependency;

    VkRenderPass renderPass;

    if (vkCreateRenderPass(
            device,
            &renderPassInfo,
            nullptr,
            &renderPass
        ) != VK_SUCCESS) {

        throw std::runtime_error(
            "Failed to create render pass"
        );
    }

    std::cout
        << "My Render Pass works\n";
    // =========================================================
// Create Framebuffers
// =========================================================

std::vector<VkFramebuffer> swapchainFramebuffers(
    swapchainImageViews.size()
);

for (size_t i = 0;
     i < swapchainImageViews.size();
     i++) {

    VkImageView attachments[] = {
        swapchainImageViews[i]
    };

    VkFramebufferCreateInfo framebufferInfo{};

    framebufferInfo.sType =
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

    framebufferInfo.renderPass =
        renderPass;

    framebufferInfo.attachmentCount = 1;

    framebufferInfo.pAttachments =
        attachments;

    framebufferInfo.width =
        swapchainExtent.width;

    framebufferInfo.height =
        swapchainExtent.height;

    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(
            device,
            &framebufferInfo,
            nullptr,
            &swapchainFramebuffers[i]
        ) != VK_SUCCESS) {

        throw std::runtime_error(
            "My framebuffer sucks ass"
        );
    }
}

std::cout
    << "Created "
    << swapchainFramebuffers.size()
    << " framebuffers\n";

    // =========================================================
// Load Shaders
// =========================================================

auto vertShaderCode =
    readFile("shaders/triangle.vert.spv");

auto fragShaderCode =
    readFile("shaders/triangle.frag.spv");

VkShaderModule vertShaderModule =
    createShaderModule(
        device,
        vertShaderCode
    );

VkShaderModule fragShaderModule =
    createShaderModule(
        device,
        fragShaderCode
    );

std::cout
    << "Shader modules created successfully!\n";

// =========================================================
// Graphics Pipeline - Shader Stages
// =========================================================

VkPipelineShaderStageCreateInfo vertShaderStageInfo{};

vertShaderStageInfo.sType =
    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

vertShaderStageInfo.stage =
    VK_SHADER_STAGE_VERTEX_BIT;

vertShaderStageInfo.module =
    vertShaderModule;

vertShaderStageInfo.pName =
    "main";


VkPipelineShaderStageCreateInfo fragShaderStageInfo{};

fragShaderStageInfo.sType =
    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

fragShaderStageInfo.stage =
    VK_SHADER_STAGE_FRAGMENT_BIT;

fragShaderStageInfo.module =
    fragShaderModule;

fragShaderStageInfo.pName =
    "main";


VkPipelineShaderStageCreateInfo shaderStages[] = {
    vertShaderStageInfo,
    fragShaderStageInfo
};
// =========================================================
// Vertex Input
// =========================================================

VkPipelineVertexInputStateCreateInfo vertexInputInfo{};

vertexInputInfo.sType =
    VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

vertexInputInfo.vertexBindingDescriptionCount = 0;

vertexInputInfo.pVertexBindingDescriptions = nullptr;

vertexInputInfo.vertexAttributeDescriptionCount = 0;

vertexInputInfo.pVertexAttributeDescriptions = nullptr;

// =========================================================
// Input Assembly
// =========================================================

VkPipelineInputAssemblyStateCreateInfo inputAssembly{};

inputAssembly.sType =
    VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;

inputAssembly.topology =
    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

inputAssembly.primitiveRestartEnable =
    VK_FALSE;

// =========================================================
// Viewport
// =========================================================

VkViewport viewport{};

viewport.x = 0.0f;
viewport.y = 0.0f;

viewport.width =
    static_cast<float>(swapchainExtent.width);

viewport.height =
    static_cast<float>(swapchainExtent.height);

viewport.minDepth = 0.0f;
viewport.maxDepth = 1.0f;

// =========================================================
// Scissor
// =========================================================

VkRect2D scissor{};

scissor.offset = {
    0,
    0
};

scissor.extent =
    swapchainExtent;
// =========================================================
// Viewport State
// =========================================================

VkPipelineViewportStateCreateInfo viewportState{};

viewportState.sType =
    VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

viewportState.viewportCount = 1;

viewportState.pViewports =
    &viewport;

viewportState.scissorCount = 1;

viewportState.pScissors =
    &scissor;
// =========================================================
// Rasterizer
// =========================================================

VkPipelineRasterizationStateCreateInfo rasterizer{};

rasterizer.sType =
    VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;

rasterizer.depthClampEnable = VK_FALSE;

rasterizer.rasterizerDiscardEnable = VK_FALSE;

rasterizer.polygonMode =
    VK_POLYGON_MODE_FILL;

rasterizer.lineWidth = 1.0f;

rasterizer.cullMode =
    VK_CULL_MODE_BACK_BIT;

rasterizer.frontFace =
    VK_FRONT_FACE_CLOCKWISE;

rasterizer.depthBiasEnable = VK_FALSE;

VkPipelineMultisampleStateCreateInfo multisampling{};

multisampling.sType =
    VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;

multisampling.sampleShadingEnable =
    VK_FALSE;

multisampling.sampleShadingEnable =
    VK_FALSE;
multisampling.rasterizationSamples =
    VK_SAMPLE_COUNT_1_BIT;

VkPipelineColorBlendAttachmentState colorBlendAttachment{};

colorBlendAttachment.colorWriteMask =
    VK_COLOR_COMPONENT_R_BIT |
    VK_COLOR_COMPONENT_G_BIT |
    VK_COLOR_COMPONENT_B_BIT |
    VK_COLOR_COMPONENT_A_BIT;

VkPipelineColorBlendStateCreateInfo colorBlending{};

colorBlending.sType =
    VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

colorBlending.logicOpEnable =
    VK_FALSE;

colorBlending.logicOp =
    VK_LOGIC_OP_COPY;

colorBlending.attachmentCount =
    1;

colorBlending.pAttachments =
    &colorBlendAttachment;

colorBlending.blendConstants[0] = 0.0f;
colorBlending.blendConstants[1] = 0.0f;
colorBlending.blendConstants[2] = 0.0f;
colorBlending.blendConstants[3] = 0.0f;

// =========================================================
// Create Graphics Pipeline
// =========================================================

VkPipelineLayoutCreateInfo pipelineLayoutInfo{};

pipelineLayoutInfo.sType =
    VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

pipelineLayoutInfo.setLayoutCount = 0;

pipelineLayoutInfo.pushConstantRangeCount = 0;

VkPipelineLayout pipelineLayout;

if (vkCreatePipelineLayout(
        device,
        &pipelineLayoutInfo,
        nullptr,
        &pipelineLayout
    ) != VK_SUCCESS) {

    throw std::runtime_error(
        "Failed to create pipeline layout"
    );
}

std::cout << "Pipeline layout created successfully!\n";

VkGraphicsPipelineCreateInfo pipelineInfo{};

pipelineInfo.sType =
    VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

pipelineInfo.stageCount = 2;

pipelineInfo.pStages =
    shaderStages;

pipelineInfo.pVertexInputState =
    &vertexInputInfo;

pipelineInfo.pInputAssemblyState =
    &inputAssembly;

pipelineInfo.pViewportState =
    &viewportState;

pipelineInfo.pRasterizationState =
    &rasterizer;

pipelineInfo.pMultisampleState =
    &multisampling;

pipelineInfo.pColorBlendState =
    &colorBlending;

pipelineInfo.layout =
    pipelineLayout;

pipelineInfo.renderPass =
    renderPass;

pipelineInfo.subpass = 0;

pipelineInfo.basePipelineHandle =
    VK_NULL_HANDLE;

VkPipeline graphicsPipeline;

if (vkCreateGraphicsPipelines(
        device,
        VK_NULL_HANDLE,
        1,
        &pipelineInfo,
        nullptr,
        &graphicsPipeline
    ) != VK_SUCCESS) {

    throw std::runtime_error(
        "Failed to create graphics pipeline"
    );
}

std::cout
    << "Graphics pipeline created successfully!\n";

    // =========================================================
    // Create Command Buffers
    // =========================================================

std::vector<VkCommandBuffer> commandBuffers(
    swapchainFramebuffers.size()
);

VkCommandBufferAllocateInfo commandBufferAllocInfo{};

commandBufferAllocInfo.sType =
    VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;

commandBufferAllocInfo.commandPool =
    commandPool;

commandBufferAllocInfo.level =
    VK_COMMAND_BUFFER_LEVEL_PRIMARY;

commandBufferAllocInfo.commandBufferCount =
    static_cast<uint32_t>(commandBuffers.size());

if (vkAllocateCommandBuffers(
        device,
        &commandBufferAllocInfo,
        commandBuffers.data()
    ) != VK_SUCCESS) {

    throw std::runtime_error(
        "Failed to allocate command buffers"
    );
}

std::cout
    << "Command buffer created successfully!\n";

// =========================================================
// Create Synchronization Objects
// =========================================================

VkSemaphoreCreateInfo semaphoreInfo{};

semaphoreInfo.sType =
    VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

VkFenceCreateInfo fenceInfo{};

fenceInfo.sType =
    VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

fenceInfo.flags =
    VK_FENCE_CREATE_SIGNALED_BIT;

VkSemaphore imageAvailableSemaphore;
VkSemaphore renderFinishedSemaphore;
VkFence inFlightFence;

if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
    vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS ||
    vkCreateFence(device, &fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS) {

    throw std::runtime_error(
        "Failed to create synchronization objects"
    );
}

std::cout
    << "Synchronization objects created successfully!\n";

    // =========================================================
    // Main Loop
    // =========================================================

    while (!glfwWindowShouldClose(window)) {

        glfwPollEvents();

        vkWaitForFences(
            device,
            1,
            &inFlightFence,
            VK_TRUE,
            UINT64_MAX
        );

        vkResetFences(
            device,
            1,
            &inFlightFence
        );

        uint32_t imageIndex;

        vkAcquireNextImageKHR(
            device,
            swapchain,
            UINT64_MAX,
            imageAvailableSemaphore,
            VK_NULL_HANDLE,
            &imageIndex
        );

        vkResetCommandBuffer(commandBuffers[0], 0);

        // =========================================================
        // Begin Recording Command Buffer
        // =========================================================

        VkCommandBufferBeginInfo beginInfo{};

        beginInfo.sType =
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(
                commandBuffers[0],
                &beginInfo
            ) != VK_SUCCESS) {

            throw std::runtime_error(
                "Failed to begin recording command buffer"
            );
        }

        // =========================================================
        // Begin Render Pass
        // =========================================================

        VkRenderPassBeginInfo renderPassBeginInfo{};

        renderPassBeginInfo.sType =
            VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

        renderPassBeginInfo.renderPass =
            renderPass;

        renderPassBeginInfo.framebuffer =
            swapchainFramebuffers[imageIndex];

        renderPassBeginInfo.renderArea.offset =
            {0, 0};

        renderPassBeginInfo.renderArea.extent =
            swapchainExtent;

        // Clear the screen color
        VkClearValue clearColor{};

        clearColor.color =
            {{0.0f, 0.0f, 0.0f, 1.0f}};

        renderPassBeginInfo.clearValueCount =
            1;

        renderPassBeginInfo.pClearValues =
            &clearColor;

        // Begin rendering
        vkCmdBeginRenderPass(
            commandBuffers[0],
            &renderPassBeginInfo,
            VK_SUBPASS_CONTENTS_INLINE
        );

        // =========================================================
        // Bind Graphics Pipeline
        // =========================================================

        vkCmdBindPipeline(
            commandBuffers[0],
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            graphicsPipeline
        );

        // =========================================================
        // Draw
        // =========================================================

        vkCmdDraw(
            commandBuffers[0],
            3,
            1,
            0,
            0
        );

        vkCmdEndRenderPass(commandBuffers[0]);

        if (vkEndCommandBuffer(commandBuffers[0]) != VK_SUCCESS) {

            throw std::runtime_error(
                "Failed to record command buffer"
            );
        }

        // =========================================================
        // Submit Command Buffer
        // =========================================================

        VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };

        VkSubmitInfo submitInfo{};

        submitInfo.sType =
            VK_STRUCTURE_TYPE_SUBMIT_INFO;

        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[0];

        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(
                graphicsQueue,
                1,
                &submitInfo,
                inFlightFence
            ) != VK_SUCCESS) {

            throw std::runtime_error(
                "Failed to submit draw command buffer"
            );
        }

        // =========================================================
        // Present
        // =========================================================

        VkPresentInfoKHR presentInfo{};

        presentInfo.sType =
            VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapchains[] = { swapchain };

        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapchains;
        presentInfo.pImageIndices = &imageIndex;

        vkQueuePresentKHR(
            graphicsQueue,
            &presentInfo
        );
    }


    // =========================================================
    // Cleanup
    // =========================================================



    vkDeviceWaitIdle(device);

    vkDestroySemaphore(
        device,
        renderFinishedSemaphore,
        nullptr
    );

    vkDestroySemaphore(
        device,
        imageAvailableSemaphore,
        nullptr
    );

    vkDestroyFence(
        device,
        inFlightFence,
        nullptr
    );

    vkDestroyPipeline(
        device,
        graphicsPipeline,
        nullptr
    );

    vkDestroyPipelineLayout(
        device,
        pipelineLayout,
        nullptr
    );

    for (auto framebuffer : swapchainFramebuffers) {

    

    vkDestroyFramebuffer(
        device,
        framebuffer,
        nullptr
    );
}
    vkDestroyRenderPass(
        device,
        renderPass,
        nullptr
    );

for (auto imageView : swapchainImageViews) {

    vkDestroyImageView(
        device,
        imageView,
        nullptr
    );
}

    vkDestroySwapchainKHR(
        device,
        swapchain,
        nullptr
    );

    vkDestroyShaderModule(
        device,
        fragShaderModule,
        nullptr
    );

    vkDestroyShaderModule(
        device,
        vertShaderModule,
        nullptr
    );
    vkDestroyCommandPool(
    device,
    commandPool,
    nullptr
);

    vkDestroyDevice(
        device,
        nullptr
    );

    vkDestroySurfaceKHR(
        instance,
        surface,
        nullptr
    );

    vkDestroyInstance(
        instance,
        nullptr
    );

    glfwDestroyWindow(window);

    glfwTerminate();

    return 0;
}