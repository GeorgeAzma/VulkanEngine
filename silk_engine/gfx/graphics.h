#pragma once

#include "buffers/command_buffer.h"

enum class APIVersion
{
	VULKAN_1_0 = VK_API_VERSION_1_0,
	VULKAN_1_1 = VK_API_VERSION_1_1,
	VULKAN_1_2 = VK_API_VERSION_1_2,
	VULKAN_1_3 = VK_API_VERSION_1_3,
};

class CommandQueue;
class WindowResizeEvent;
class DebugMessenger;
class Instance;
class PhysicalDevice;
class LogicalDevice;
class Allocator;
class PipelineCache;
class DescriptorPool;
class DescriptorSet;
class Application;

class Graphics
{
public:
	static constexpr APIVersion API_VERSION = APIVersion::VULKAN_1_3;

public:
	static void init(const Application& app);
	static void destroy();
	static void update();

	static void submit(std::function<void(CommandBuffer&)>&& command);
	static void execute(); 
	static void execute(const CommandBuffer::SubmitInfo& submit_info);

	static void screenshot(const path& file);
	static void vulkanAssert(VkResult result);
	static std::string stringifyResult(VkResult result);

public:
#ifdef SK_ENABLE_DEBUG_MESSENGER
	static inline DebugMessenger* debug_messenger = nullptr;
#endif
	static inline Instance* instance = nullptr;
	static inline PhysicalDevice* physical_device = nullptr;
	static inline LogicalDevice* logical_device = nullptr;
	static inline Allocator* allocator = nullptr;
	static inline CommandQueue* command_queue = nullptr;
	static inline PipelineCache* pipeline_cache = nullptr;
};