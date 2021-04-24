#pragma once

#include "Engine/Framework/IRenderer.h"
#include "Engine/Render/IRendererBackend.h"
#include "RenderBackendVkModule.h"

#include "Common/CommonVulkan.h"

// #todo_vk
#include <array>
#include <vector>
#include <optional>
#include <unordered_map>

namespace Cyclone::Render
{

//////////////////////////////////////////////////////////////////////////
struct vec2
{
    float x;
    float y;

    bool operator == (const vec2& other) const noexcept
    {
        return x == other.x && y == other.y;
    }
};
struct vec3
{
    float x;
    float y;
    float z;

    bool operator == (const vec3& other) const noexcept
    {
        return x == other.x && y == other.y && z == other.z;
    }
};

struct vec4
{
    float x;
    float y;
    float z;
    float w;

    bool operator == (const vec4& other) const noexcept
    {
        return x == other.x && y == other.y && z == other.z && w == other.w;
    }
};

struct Vertex
{
    vec3 pos;
    vec3 color;
    vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();

    bool operator == (const Vertex& other) const noexcept { return pos == other.pos && color == other.color && texCoord == other.texCoord; }
};

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete();
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};
//////////////////////////////////////////////////////////////////////////

class RenderBackendVulkan : public IRendererBackend
{
public:

    virtual C_STATUS Init(IRenderer* Renderer) override;
    virtual C_STATUS Shutdown() override;
    virtual C_STATUS Render() override;

protected:
    void createInstance();
    void destroyInstance();

    bool checkValidationlayerSupport();

    void createSurface(IWindow* window);
    void destroySurface();

    void pickPhysicalDevice();
    bool isDeviceSuitable(VkPhysicalDevice device);

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    void createSwapchain();
    void createImageViews();

    void createLogicalDevice();
    void destroyLogicalDevice();

    void createRenderPass();
    void createDescriptorSetLayout();
    void createGraphicsPipeline();
    void createFrameBuffers();
    void createCommandPool();
    void createCommandBuffers();
    void createSyncObjects();

    VkShaderModule createShaderModule(const std::vector<char>& code);

    void cleanupSwapchain();
    void recreateSwapchain();

    void createVertexBuffer();
    void createIndexBuffer();
    void createUniformBuffers();
    void updateUniformBuffer(uint32_t imageIndex);

    void createDescriptorPool();
    void createDescriptorSets();

    void createColorResources();
    void createDepthResources();

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryProperties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memoryProperties, VkImage& image, VkDeviceMemory& imageMemory);
    VkImageView createImageView(VkImage image, uint32_t mipLevels, VkFormat format, VkImageAspectFlags aspectMask);

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);


    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat findDepthFormat();
    bool hasStencilComponent(VkFormat format);
    VkSampleCountFlagBits getMaxUsableMSAASampleCount();

    void transitionImageLayout(VkImage image, uint32_t mipLevels, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t width, int32_t height, uint32_t mipLevels);


protected:
    IRenderer* m_renderer = nullptr;

protected:
    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;

    VkSurfaceKHR m_surface = VK_NULL_HANDLE;

    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;

    // #todo_vk async compute queue

    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainImageViews;
    std::vector<VkFramebuffer> m_swapchainFrameBuffers;
    VkFormat m_swapchainImageFormat;
    VkExtent2D m_swapchainExtent;

    VkRenderPass m_renderPass;
    VkDescriptorSetLayout m_descriptorSetLayout;
    VkDescriptorPool m_descriptorPool;
    std::vector<VkDescriptorSet> m_descriptorSets;
    VkPipelineLayout m_pipelineLayout;
    VkPipeline m_pipeline;

    VkCommandPool m_commandPool;
    std::vector<VkCommandBuffer> m_commandBuffers;

    std::vector<VkSemaphore> m_imageAvailabeSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inflightFences;
    std::vector<VkFence> m_imagesInFlight;

    VkBuffer m_vertexBuffer;
    VkDeviceMemory m_vertexBufferMemory;
    VkBuffer m_indexBuffer;
    VkDeviceMemory m_indexBufferMemory;

    VkImage m_colorMSAAImage;
    VkImageView m_colorMSAAImageView;
    VkDeviceMemory m_colorMSAAImageMemory;

    VkImage m_depthImage;
    VkImageView m_depthImageView;
    VkDeviceMemory m_depthImageMemory;

    std::vector<VkBuffer> m_uniformBuffers;
    std::vector<VkDeviceMemory> m_uniformBuffersMemory;

    size_t m_currentFrame = 0;
    bool m_framebufferResized = false;
    VkSampleCountFlagBits m_msaaSamples = VK_SAMPLE_COUNT_1_BIT;

    std::vector<const char*> m_validationLayers = { "VK_LAYER_KHRONOS_validation" };
    std::vector<const char*> m_deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
};

} // namespace Cyclone::Render
