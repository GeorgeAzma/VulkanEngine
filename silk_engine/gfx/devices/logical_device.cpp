#include "logical_device.h"
#include "physical_device.h"
#include "gfx/graphics.h"
#include "gfx/queues/queue_family.h"

LogicalDevice::LogicalDevice()
{
	const auto& queue_family_indices = Graphics::physical_device->getQueueFamilyIndices();

	std::vector<VkDeviceQueueCreateInfo> queue_cis;

	std::vector<uint32_t> queue_families = queue_family_indices.getIndices();
	std::set<uint32_t> unique_queue_families(queue_families.begin(), queue_families.end());

	float queue_priority = 1.0f;
	for (uint32_t queue_family_index : unique_queue_families)
	{
		shared<QueueFamily> queue_family = makeShared<QueueFamily>(queue_family_index, std::vector<float>{ queue_priority });
		this->queue_families.emplace(queue_family_index, queue_family);
		queue_cis.emplace_back(*queue_family);
	}

	// Specifies which device features we want by enabling them
	VkPhysicalDeviceFeatures device_features{};
	device_features.samplerAnisotropy = VK_TRUE;
	device_features.occlusionQueryPrecise = VK_TRUE;
	device_features.multiDrawIndirect = VK_TRUE;
	device_features.fragmentStoresAndAtomics = VK_TRUE;
	device_features.fillModeNonSolid = VK_TRUE;
	device_features.geometryShader = VK_TRUE;
	device_features.wideLines = VK_TRUE;

	VkPhysicalDeviceVulkan13Features vulkan_13_device_features{};
	vulkan_13_device_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
	vulkan_13_device_features.maintenance4 = VK_TRUE;

	VkPhysicalDeviceVulkan12Features vulkan_12_device_features{};
	vulkan_12_device_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	vulkan_12_device_features.hostQueryReset = VK_TRUE;
	vulkan_12_device_features.drawIndirectCount = VK_TRUE;
	vulkan_12_device_features.pNext = &vulkan_13_device_features;

	VkDeviceCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	ci.queueCreateInfoCount = queue_cis.size();
	ci.pQueueCreateInfos = queue_cis.data();
	ci.pEnabledFeatures = &device_features;
	auto required_extensions = getRequiredExtensions();
	ci.enabledExtensionCount = required_extensions.size();
	ci.ppEnabledExtensionNames = required_extensions.data();
	ci.pNext = &vulkan_12_device_features;

	logical_device = Graphics::physical_device->createLogicalDevice(ci);

	//Get handles of the requried queues
	if (queue_family_indices.graphics)
		graphics_queue = this->queue_families.at(*queue_family_indices.graphics)->getQueue(logical_device, 0);
	if (queue_family_indices.transfer)
		transfer_queue = this->queue_families.at(*queue_family_indices.transfer)->getQueue(logical_device, 0);
	if (queue_family_indices.present)
		present_queue = this->queue_families.at(*queue_family_indices.present)->getQueue(logical_device, 0);
	if (queue_family_indices.compute)
		compute_queue = this->queue_families.at(*queue_family_indices.compute)->getQueue(logical_device, 0);
}

LogicalDevice::~LogicalDevice()
{
	vkDestroyDevice(logical_device, nullptr);
}

VkCommandPool LogicalDevice::createCommandPool(const VkCommandPoolCreateInfo& ci) const
{
	VkCommandPool command_pool = nullptr;
	Graphics::vulkanAssert(vkCreateCommandPool(logical_device, &ci, nullptr, &command_pool));
	return command_pool;
}

void LogicalDevice::resetCommandPool(VkCommandPool command_pool, VkCommandPoolResetFlags reset_flags) const
{
	vkResetCommandPool(logical_device, command_pool, reset_flags);
}

void LogicalDevice::destroyCommandPool(VkCommandPool command_pool) const
{
	vkDestroyCommandPool(logical_device, command_pool, nullptr);
}

VkQueryPool LogicalDevice::createQueryPool(const VkQueryPoolCreateInfo& ci) const
{
	VkQueryPool query_pool = nullptr;
	Graphics::vulkanAssert(vkCreateQueryPool(logical_device, &ci, nullptr, &query_pool));
	return query_pool;
}

void LogicalDevice::destroyQueryPool(VkQueryPool query_pool) const
{
	vkDestroyQueryPool(logical_device, query_pool, nullptr);
}

void LogicalDevice::resetQueryPool(VkQueryPool query_pool, uint32_t first_query, uint32_t query_count) const
{
	vkResetQueryPool(logical_device, query_pool, first_query, query_count);
}

std::vector<VkCommandBuffer> LogicalDevice::allocateCommandBuffers(const VkCommandBufferAllocateInfo& allocate_info) const
{
	std::vector<VkCommandBuffer> command_buffers(allocate_info.commandBufferCount, nullptr);
	Graphics::vulkanAssert(vkAllocateCommandBuffers(logical_device, &allocate_info, command_buffers.data()));
	return command_buffers;
}

void LogicalDevice::freeCommandBuffers(VkCommandPool command_pool, const std::vector<VkCommandBuffer>& command_buffers) const
{
	vkFreeCommandBuffers(logical_device, command_pool, command_buffers.size(), command_buffers.data());
}

void LogicalDevice::resetFences(const std::vector<VkFence>& fences) const
{
	Graphics::vulkanAssert(vkResetFences(logical_device, fences.size(), fences.data()));
}

VkFence LogicalDevice::createFence(bool signaled) const
{
	VkFenceCreateInfo fence_info{};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.flags = signaled * VK_FENCE_CREATE_SIGNALED_BIT;
	VkFence fence = nullptr;
	Graphics::vulkanAssert(vkCreateFence(logical_device, &fence_info, nullptr, &fence));
	return fence;
}

void LogicalDevice::destroyFence(VkFence fence) const
{
	vkDestroyFence(logical_device, fence, nullptr);
}

VkResult LogicalDevice::getFenceStatus(VkFence fence) const
{
	return vkGetFenceStatus(logical_device, fence);
}

VkSemaphore LogicalDevice::createSemaphore(const VkSemaphoreCreateFlags& flags) const
{
	VkSemaphoreCreateInfo semaphore_info{};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphore_info.flags = flags;
	VkSemaphore semaphore = nullptr;
	Graphics::vulkanAssert(vkCreateSemaphore(logical_device, &semaphore_info, nullptr, &semaphore));
	return semaphore;
}

void LogicalDevice::destroySemaphore(VkSemaphore semaphore) const
{
	vkDestroySemaphore(logical_device, semaphore, nullptr);
}

VkResult LogicalDevice::waitForFences(const std::vector<VkFence>& fences, VkBool32 wait_all, uint64_t timeout) const
{
	return vkWaitForFences(logical_device, fences.size(), fences.data(), wait_all, timeout);
}

VkResult LogicalDevice::waitForSemaphores(const std::vector<VkSemaphore>& semaphores, const std::vector<uint64_t>& values, VkBool32 wait_any, uint64_t timeout) const
{
	SK_VERIFY(semaphores.size() == values.size());
	VkSemaphoreWaitInfo semaphore_wait_info{};
	semaphore_wait_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
	semaphore_wait_info.pSemaphores = semaphores.data();
	semaphore_wait_info.semaphoreCount = semaphores.size();
	semaphore_wait_info.pValues = values.data();
	semaphore_wait_info.flags = wait_any * VK_SEMAPHORE_WAIT_ANY_BIT;
	return vkWaitSemaphores(logical_device, &semaphore_wait_info, timeout);
}

VkResult LogicalDevice::signalSemaphore(const VkSemaphore& semaphore, uint64_t value) const
{
	VkSemaphoreSignalInfo semaphore_signal_info{};
	semaphore_signal_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
	semaphore_signal_info.semaphore = semaphore;
	semaphore_signal_info.value = value;
	return vkSignalSemaphore(logical_device, &semaphore_signal_info);
}

VkFramebuffer LogicalDevice::createFramebuffer(const VkFramebufferCreateInfo& framebuffer_info) const
{
	VkFramebuffer framebuffer = nullptr;
	Graphics::vulkanAssert(vkCreateFramebuffer(logical_device, &framebuffer_info, nullptr, &framebuffer));
	return framebuffer;
}

void LogicalDevice::destroyFramebuffer(VkFramebuffer framebuffer) const
{
	vkDestroyFramebuffer(logical_device, framebuffer, nullptr);
}

VkDescriptorPool LogicalDevice::createDescriptorPool(const VkDescriptorPoolCreateInfo& descriptor_pool_info) const
{
	VkDescriptorPool descriptor_pool = nullptr;
	Graphics::vulkanAssert(vkCreateDescriptorPool(logical_device, &descriptor_pool_info, nullptr, &descriptor_pool));
	return descriptor_pool;
}	

void LogicalDevice::resetDescriptorPool(VkDescriptorPool descriptor_pool, VkDescriptorPoolResetFlags flags) const
{
	Graphics::vulkanAssert(vkResetDescriptorPool(logical_device, descriptor_pool, flags));
}

void LogicalDevice::destroyDescriptorPool(VkDescriptorPool descriptor_pool) const
{
	vkDestroyDescriptorPool(logical_device, descriptor_pool, nullptr);
}

std::vector<VkDescriptorSet> LogicalDevice::allocateDescriptorSets(const VkDescriptorSetAllocateInfo& descriptor_set_allocate_info) const
{
	std::vector<VkDescriptorSet> descriptor_sets(descriptor_set_allocate_info.descriptorSetCount, nullptr);
	vkAllocateDescriptorSets(logical_device, nullptr, descriptor_sets.data());
	return descriptor_sets;
}
VkResult LogicalDevice::allocateDescriptorSets(const VkDescriptorSetAllocateInfo& alloc_info, VkDescriptorSet& descriptor_set) const
{
	return vkAllocateDescriptorSets(logical_device, &alloc_info, &descriptor_set);
}

void LogicalDevice::updateDescriptorSets(const std::vector<VkWriteDescriptorSet>& writes) const
{
	vkUpdateDescriptorSets(logical_device, writes.size(), writes.data(), 0, nullptr);
}

VkDescriptorSetLayout LogicalDevice::createDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo& descriptor_set_layout_create_info) const
{
	VkDescriptorSetLayout descriptor_set_layout = nullptr;
	Graphics::vulkanAssert(vkCreateDescriptorSetLayout(logical_device, &descriptor_set_layout_create_info, nullptr, &descriptor_set_layout));
	return descriptor_set_layout;
}

void LogicalDevice::destroyDescriptorSetLayout(VkDescriptorSetLayout descriptor_set_layout) const
{
	vkDestroyDescriptorSetLayout(logical_device, descriptor_set_layout, nullptr);
}

VkSubresourceLayout LogicalDevice::getImageSubresourceLayout(VkImage image, const VkImageSubresource& image_subresource) const
{
	VkSubresourceLayout subresource_layout{};
	vkGetImageSubresourceLayout(logical_device, image, &image_subresource, &subresource_layout);
	return subresource_layout;
}

VkImageView LogicalDevice::createImageView(const VkImageViewCreateInfo& image_view_info) const
{
	VkImageView image_view = nullptr;
	Graphics::vulkanAssert(vkCreateImageView(logical_device, &image_view_info, nullptr, &image_view));
	return image_view;
}

void LogicalDevice::destroyImageView(VkImageView image_view) const
{
	vkDestroyImageView(logical_device, image_view, nullptr);
}

VkSampler LogicalDevice::createSampler(const VkSamplerCreateInfo& sampler_info) const
{
	VkSampler sampler = nullptr;
	Graphics::vulkanAssert(vkCreateSampler(logical_device, &sampler_info, nullptr, &sampler));
	return sampler;
}

void LogicalDevice::destroySampler(VkSampler sampler) const
{
	vkDestroySampler(logical_device, sampler, nullptr);
}

VkPipelineLayout LogicalDevice::createPipelineLayout(const VkPipelineLayoutCreateInfo& pipeline_layout_info) const
{
	VkPipelineLayout pipeline_layout = nullptr;
	Graphics::vulkanAssert(vkCreatePipelineLayout(logical_device, &pipeline_layout_info, nullptr, &pipeline_layout));
	return pipeline_layout;
}

void LogicalDevice::LogicalDevice::destroyPipelineLayout(VkPipelineLayout pipeline_layout) const
{
	vkDestroyPipelineLayout(logical_device, pipeline_layout, nullptr);
}

VkPipeline LogicalDevice::createComputePipeline(VkPipelineCache pipeline_cache, const VkComputePipelineCreateInfo& compute_pipeline_info) const
{
	VkPipeline compute_pipeline = nullptr;
	Graphics::vulkanAssert(vkCreateComputePipelines(logical_device, pipeline_cache, 1, &compute_pipeline_info, nullptr, &compute_pipeline));
	return compute_pipeline;
}

VkPipeline LogicalDevice::createGraphicsPipeline(VkPipelineCache pipeline_cache, const VkGraphicsPipelineCreateInfo& graphics_pipeline_info) const
{
	VkPipeline graphics_pipeline = nullptr;
	Graphics::vulkanAssert(vkCreateGraphicsPipelines(logical_device, pipeline_cache, 1, &graphics_pipeline_info, nullptr, &graphics_pipeline));
	return graphics_pipeline;
}

void LogicalDevice::LogicalDevice::destroyPipeline(VkPipeline pipeline) const
{
	vkDestroyPipeline(logical_device, pipeline, nullptr);
}

VkPipelineCache LogicalDevice::createPipelineCache(const VkPipelineCacheCreateInfo& pipeline_cache_info) const
{
	VkPipelineCache pipeline_cache = nullptr;
	vkCreatePipelineCache(logical_device, &pipeline_cache_info, nullptr, &pipeline_cache);
	return pipeline_cache;
}

void LogicalDevice::destroyPipelineCache(VkPipelineCache pipeline_cache) const
{
	vkDestroyPipelineCache(logical_device, pipeline_cache, nullptr);
}

std::vector<uint8_t> LogicalDevice::getPipelineCacheData(VkPipelineCache pipeline_cache) const
{
	size_t data_size = 0;
	vkGetPipelineCacheData(logical_device, pipeline_cache, &data_size, nullptr);
	std::vector<uint8_t> data(data_size);
	vkGetPipelineCacheData(logical_device, pipeline_cache, &data_size, data.data());
	return data;
}

VkRenderPass LogicalDevice::createRenderPass(const VkRenderPassCreateInfo& render_pass_info) const
{
	VkRenderPass render_pass = nullptr;
	Graphics::vulkanAssert(vkCreateRenderPass(logical_device, &render_pass_info, nullptr, &render_pass));
	return render_pass;
}

void LogicalDevice::destroyRenderPass(VkRenderPass render_pass) const
{
	vkDestroyRenderPass(logical_device, render_pass, nullptr);
}

void LogicalDevice::wait() const
{
	vkDeviceWaitIdle(logical_device);
}

VkShaderModule LogicalDevice::createShaderModule(const VkShaderModuleCreateInfo& shader_module_info) const
{
	VkShaderModule shader = nullptr;
	Graphics::vulkanAssert(vkCreateShaderModule(logical_device, &shader_module_info, nullptr, &shader));
	return shader;
}

void LogicalDevice::destroyShaderModule(VkShaderModule shader_module) const
{
	vkDestroyShaderModule(logical_device, shader_module, nullptr);
}

VkSwapchainKHR LogicalDevice::createSwapChain(const VkSwapchainCreateInfoKHR& swap_chain_info) const
{
	VkSwapchainKHR swap_chain = nullptr;
	Graphics::vulkanAssert(vkCreateSwapchainKHR(logical_device, &swap_chain_info, nullptr, &swap_chain));
	return swap_chain;
}

void LogicalDevice::destroySwapChain(VkSwapchainKHR swap_chain) const
{
	vkDestroySwapchainKHR(logical_device, swap_chain, nullptr);
}

VkResult LogicalDevice::acquireNextImage(VkSwapchainKHR swap_chain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* image_index) const
{
	return vkAcquireNextImageKHR(logical_device, swap_chain, timeout, semaphore, fence, image_index);
}

std::vector<VkImage> LogicalDevice::getSwapChainImages(VkSwapchainKHR swap_chain) const
{
	uint32_t image_count = 0;
	vkGetSwapchainImagesKHR(logical_device, swap_chain, &image_count, nullptr);
	std::vector<VkImage> images(image_count);
	vkGetSwapchainImagesKHR(logical_device, swap_chain, &image_count, images.data());
	return images;
}

const Queue& LogicalDevice::getGraphicsQueue() const 
{ 
	return *graphics_queue; 
}

const Queue& LogicalDevice::getTransferQueue() const 
{ 
	return *transfer_queue; 
}

const Queue& LogicalDevice::getPresentQueue() const 
{ 
	return *present_queue; 
}

const Queue& LogicalDevice::getComputeQueue() const 
{ 
	return *compute_queue; 
}

const Queue& LogicalDevice::getQueue(VkQueueFlags queue) const
{
	switch (queue)
	{
	case VK_QUEUE_GRAPHICS_BIT: return Graphics::logical_device->getGraphicsQueue();
	case VK_QUEUE_TRANSFER_BIT: return Graphics::logical_device->getTransferQueue();
	case VK_QUEUE_COMPUTE_BIT: return Graphics::logical_device->getComputeQueue();
	}

	return Graphics::logical_device->getGraphicsQueue(); //Or nullptr
}
