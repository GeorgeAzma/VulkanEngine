#ifdef SK_ENABLE_DEBUG_MESSENGER

#include "debug_messenger.h"
#include "silk_engine/gfx/render_context.h"
#include "silk_engine/gfx/instance.h"

VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessenger::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    std::string message = pCallbackData->pMessage;
    std::string delimiter = std::format("MessageID = 0x{:x} | ", *(const uint32_t*)&pCallbackData->messageIdNumber);
    size_t offset = message.find(delimiter);
    if (offset != std::string::npos)
        message = message.substr(offset + delimiter.size());
    
    if (size_t off = message.find("(https://", offset != std::string::npos ? offset : 0); off != std::string::npos)
        message = message.substr(0, off);

    switch (messageSeverity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        SK_TRACE(message);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        SK_INFO(message);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        SK_WARN(message);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        SK_ERROR(message);
        break;
    }

    return VK_FALSE;
}

VkResult DebugMessenger::create(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

    if (func == nullptr)
        return VK_ERROR_EXTENSION_NOT_PRESENT;

    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
}

void DebugMessenger::destroy(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
    if (auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT")) 
        func(instance, debugMessenger, pAllocator);
}

DebugMessenger::DebugMessenger()
{
    uint32_t available_layer_count = 0;
    vkEnumerateInstanceLayerProperties(&available_layer_count, nullptr);
    std::vector<VkLayerProperties> available_layers(available_layer_count);
    vkEnumerateInstanceLayerProperties(&available_layer_count, available_layers.data());
    for (const auto& layer : available_layers)
        available_validation_layers.emplace(layer.layerName, layer);

    required_validation_layers = { "VK_LAYER_KHRONOS_validation" };
    for (const auto& required_validation_layer : required_validation_layers)
        if (!available_validation_layers.contains(required_validation_layer))
            SK_ERROR("Vulkan: Required validation layer({}) not found", required_validation_layer);

    ci.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    ci.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    ci.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;
    ci.pfnUserCallback = debugCallback;
}

void DebugMessenger::init(VkInstance instance)
{
    this->instance = instance;
    create(instance, &ci, nullptr, &debug_messenger);
}

DebugMessenger::~DebugMessenger()
{
    destroy(instance, debug_messenger, nullptr);
}
#endif