#include "descriptor_set_layout.h"
#include "gfx/render_context.h"
#include "gfx/devices/logical_device.h"

DescriptorSetLayout::DescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings)
{
	this->bindings.reserve(bindings.size());
	for (size_t i = 0; i < bindings.size(); ++i)
		this->bindings.emplace(bindings[i].binding, bindings[i]);

	VkDescriptorSetLayoutCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	ci.bindingCount = bindings.size();
	ci.pBindings = bindings.data();
	layout = RenderContext::getLogicalDevice().createDescriptorSetLayout(ci);
}

DescriptorSetLayout::~DescriptorSetLayout()
{
	RenderContext::getLogicalDevice().destroyDescriptorSetLayout(layout);
}