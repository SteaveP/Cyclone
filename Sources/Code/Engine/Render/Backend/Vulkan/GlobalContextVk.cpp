#include "GlobalContextVk.h"

#include "Engine/Framework/IApplication.h"
#include "Engine/Framework/IWindow.h"

#include "RenderBackendVk.h"

namespace Cyclone::Render
{

#ifdef _DEBUG
    const bool g_enableValidationLayers = true;
#else
    const bool g_enableValidationLayers = false;
#endif

std::vector<const char*> g_validationLayers = { "VK_LAYER_KHRONOS_validation" };
std::vector<const char*> g_deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

C_STATUS GlobalContextVk::Init(RenderBackendVulkan* RenderBackend)
{
    m_renderBackend = RenderBackend;

    C_STATUS result = createInstance();
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(result), result);

    return C_STATUS::C_STATUS_OK;
}

C_STATUS GlobalContextVk::Shutdown()
{
    destroyInstance();

    return C_STATUS::C_STATUS_OK;
}

C_STATUS GlobalContextVk::createInstance()
{
    if (g_enableValidationLayers && !checkValidationlayerSupport())
    {
        CASSERT(false);
        return C_STATUS::C_STATUS_INVALID_ARG;
    }

    // #todo_vk
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "CycloneApp";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Cyclone";
    appInfo.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

#if PLATFORM_WIN64
    uint32_t glfwExtensionsCount = 2;
    const char* glfwExtenstions[] = { 
        VK_KHR_SURFACE_EXTENSION_NAME, 
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME
    };
#else
#error unsupported platform
#endif

    createInfo.enabledExtensionCount = glfwExtensionsCount;
    createInfo.ppEnabledExtensionNames = glfwExtenstions;

    if (g_enableValidationLayers)
    {
        createInfo.ppEnabledLayerNames = g_validationLayers.data();
        createInfo.enabledLayerCount = static_cast<uint32_t>(g_validationLayers.size());
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    VkResult result = vkCreateInstance(&createInfo, nullptr, &m_instance);
    C_ASSERT_VK_SUCCEEDED_RET(result, C_STATUS::C_STATUS_ERROR);

    if (1)
    {
        uint32_t extensionsCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, nullptr);

        std::vector<VkExtensionProperties> extensions(extensionsCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, extensions.data());

        std::cout << "available extensions:\n";
        for (const auto& extension : extensions)
        {
            std::cout << '\t' << extension.extensionName << '\n';
        }
    }

    return C_STATUS::C_STATUS_OK;
}

void GlobalContextVk::destroyInstance()
{
    vkDestroyInstance(m_instance, nullptr);
}

bool GlobalContextVk::checkValidationlayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const auto& layerName : g_validationLayers)
    {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
            return false;
    }

    return true;
}

} // namespace Cyclone::Render
