#pragma once

#include "silk_engine/gfx/images/image.h"

class Framebuffer;
class SwapChain;

struct AttachmentProps
{
	Format format = Format::BGRA;
	VkImageLayout final_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;

	VkAttachmentLoadOp load_operation = VK_ATTACHMENT_LOAD_OP_CLEAR;
	VkAttachmentStoreOp store_operation = VK_ATTACHMENT_STORE_OP_STORE;
	VkAttachmentLoadOp stencil_load_operation = VK_ATTACHMENT_LOAD_OP_MAX_ENUM; // VK_ATTACHMENT_LOAD_OP_MAX_ENUM sets stencil load_operation to same thing as load_operation if format is a stencil/depth_stencil format
	VkAttachmentStoreOp stencil_store_operation = VK_ATTACHMENT_STORE_OP_MAX_ENUM; // VK_ATTACHMENT_STORE_OP_MAX_ENUM sets stencil store_operation to same thing as store_operation if format is a stencil/depth_stencil format
	VkImageLayout initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
	bool preserve = false;
};

class RenderPass : NoCopy
{
	struct SubpassInfo
	{
		std::vector<VkAttachmentReference> resolve_attachment_references;
		std::vector<VkAttachmentReference> color_attachment_references;
		VkAttachmentReference depth_attachment_reference = { VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_UNDEFINED };
		std::vector<uint32_t> preserve_attachment_references;
		std::vector<VkAttachmentReference> input_attachment_references;
	};

public:
	~RenderPass();

	size_t addSubpass();
	void addInputAttachment(uint32_t index);
	size_t addAttachment(const AttachmentProps& attachment_props);
	void addSubpassDependency(uint32_t source, uint32_t destination, PipelineStage source_stage, PipelineStage destination_stage, VkAccessFlags source_access_mask, VkAccessFlags destination_access_mask, VkDependencyFlags dependency_flags = VkDependencyFlags(0));

	void build();

	void begin(VkSubpassContents subpass_contents = VK_SUBPASS_CONTENTS_INLINE);
	void nextSubpass(VkSubpassContents subpass_contents = VK_SUBPASS_CONTENTS_INLINE);
	void end();

	void setViewport(const ivec2& viewport) { this->viewport = viewport; }
	void setClearColorValue(size_t index, const VkClearColorValue& color_clear_value) 
	{ 
		// For multisampled attachments render pass creates 2 images
		// first is resolved image and second is multisampled image which comes right after resolved image
		// we never need to clear resolved image, so add +1 to index to jump to multisampled image
		clear_values[index + (attachment_descriptions[(index + 1) < attachment_descriptions.size() ? index + 1 : index].samples != VK_SAMPLE_COUNT_1_BIT)].color = color_clear_value; 
	}
	void setClearDepthStencilValue(size_t index, const VkClearDepthStencilValue& depth_stencil_clear_value) 
	{ 
		clear_values[index].depthStencil = depth_stencil_clear_value;
	}
	void resize(const SwapChain& swap_chain);

	bool isInputAttachment(uint32_t attachment) const { return attachments_used_as_inputs.contains(attachment); }
	size_t getSubpassCount() const { return subpass_infos.size(); }
	const std::vector<VkAttachmentDescription>& getAttachmentDescriptions() const { return attachment_descriptions; }
	operator const VkRenderPass& () const { return render_pass; }
	const shared<Framebuffer>& getFramebuffer() const { return framebuffer; }
	const std::vector<shared<Image>>& getAttachments() const;
	uint32_t getWidth() const;
	uint32_t getHeight() const;

private:
	VkRenderPass render_pass = nullptr;
	std::vector<SubpassInfo> subpass_infos; 
	std::vector<VkSubpassDependency> subpass_dependencies;
	std::vector<VkAttachmentDescription> attachment_descriptions;
	std::unordered_set<uint32_t> attachments_used_as_inputs;
	std::vector<VkClearValue> clear_values;
	bool any_cleared = false;
	ivec2 viewport = ivec2(0);
	shared<Framebuffer> framebuffer = nullptr;
};