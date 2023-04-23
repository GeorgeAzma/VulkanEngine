#pragma once

class Instance;

class PhysicalDevice : NonCopyable
{
	friend class LogicalDevice;
public:
	PhysicalDevice(const Instance& instance, VkPhysicalDevice physical_device);

	bool supportsExtension(const char* extension_name) const;
	uint32_t alignSize(uint32_t original_size) const
	{
		size_t min_ubo_alignment = properties.limits.minUniformBufferOffsetAlignment;
		return (min_ubo_alignment > 0) ? ((original_size + min_ubo_alignment - 1) & ~(min_ubo_alignment - 1)) : original_size;
	}

	VkDevice createLogicalDevice(const VkDeviceCreateInfo& create_info) const;
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;
	std::vector<VkSurfaceFormatKHR> getSurfaceFormats(VkSurfaceKHR surface) const;
	std::vector<VkPresentModeKHR> getSurfacePresentModes(VkSurfaceKHR surface) const;
	VkSurfaceCapabilitiesKHR getSurfaceCapabilities(VkSurfaceKHR surface) const;
	bool getSurfaceSupport(uint32_t queue_family_index, VkSurfaceKHR surface) const;

	const std::vector<VkQueueFamilyProperties>& getQueueFamilyProperties() const { return queue_family_properties; }
	const std::vector<uint32_t>& getQueueFamilyIndices() const { return queue_family_indices; }
	const VkPhysicalDeviceProperties& getProperties() const { return properties; }
	const VkPhysicalDeviceMemoryProperties& getMemoryProperties() const { return memory_properties; }
	const VkPhysicalDeviceFeatures& getFeatures() const { return features; }
	VkSampleCountFlagBits getMaxSampleCount() const { return max_usable_sample_count; }
	VkFormat getDepthFormat() const { return depth_format; }
	VkFormatProperties getFormatProperties(VkFormat format) const;
	VkImageFormatProperties getImageFormatProperties(VkFormat format, VkImageType type, VkImageTiling tilling, VkImageUsageFlags usage, VkImageCreateFlags flags) const;
	int getGraphicsQueue() const { return graphics_queue; }
	int getComputeQueue() const { return compute_queue; }
	int getTransferQueue() const { return transfer_queue; }
	operator const VkPhysicalDevice& () const { return physical_device; }
	const Instance& getInstance() const { return instance; }

private:
	VkPhysicalDevice physical_device = nullptr;
	const Instance& instance;
	std::vector<VkQueueFamilyProperties> queue_family_properties;
	std::vector<uint32_t> queue_family_indices; // Sorted
	std::unordered_map<std::string, uint32_t> supported_extensions; // extension name | extension spec version
	int32_t graphics_queue = -1;
	int32_t compute_queue = -1;
	int32_t transfer_queue = -1;
	VkPhysicalDeviceProperties properties;
	VkPhysicalDeviceMemoryProperties memory_properties;
	VkPhysicalDeviceFeatures features;
	VkSampleCountFlagBits max_usable_sample_count;
	VkFormat depth_format = VkFormat(0);
};