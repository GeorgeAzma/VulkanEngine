#include "render_pass.h"
#include "gfx/render_context.h"
#include "gfx/devices/logical_device.h"
#include "gfx/buffers/framebuffer.h"
#include "gfx/window/swap_chain.h"

// TODO:
// figure stuff out with VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT
// Because of resolve attachments getting automatically added to attachment_descriptions, SubpassProps::inputs may not function as user would think, make it simpler
// Support subpass input depth attachments

void RenderPass::addSubpass()
{
    subpass_infos.emplace_back();
}

void RenderPass::addInputAttachment(uint32_t index)
{
    // RULE: Subpass input attachments are always either VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL or VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL or 
    // VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL or VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, depending on final layout of the input attachment
    auto& subpass_info = subpass_infos.back();
    subpass_info.input_attachment_references.emplace_back();
    auto& input_attachment = subpass_info.input_attachment_references.back();
    input_attachment.attachment = index;

    // RULE: Subpasses that are used in a subsequent subpass as an input attachment always have STORE_OP_STORE
    auto& previous_output_attachment = attachment_descriptions[index];
    previous_output_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    switch (previous_output_attachment.finalLayout)
    {
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        input_attachment.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        break;
    case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
        input_attachment.layout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
        break;
    case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
        input_attachment.layout = VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL;
        break;
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        input_attachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        break;
    default:
        SK_ERROR("Previous subpass output attachment had an incompatible final layout to be used as an input attachment to subsequent subpasses");
    }
}

void RenderPass::addAttachment(const AttachmentProps& attachment_props)
{
    auto& subpass_info = subpass_infos.back();
    size_t attachment_index = attachment_descriptions.size();
 
    VkAttachmentDescription attachment_description{};
    attachment_description.format = VkFormat(attachment_props.format);
    attachment_description.samples = attachment_props.samples;
    attachment_description.initialLayout = attachment_props.initial_layout;
    attachment_description.storeOp = attachment_props.store_operation;
    attachment_description.loadOp = attachment_props.load_operation;
    attachment_description.stencilLoadOp = attachment_props.stencil_load_operation;
    attachment_description.stencilStoreOp = attachment_props.stencil_store_operation;

    // RULE: Preserved subpass attachments always have LOAD_OP_LOAD and STORE_OP_STORE
    if (attachment_props.preserve)
    {
        attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        subpass_info.preserve_attachment_references.emplace_back(attachment_index);
    }

    // RULE: If not using stencil, stencil load and store ops should be set to DONT_CARE
    bool stencil = Image::isStencilFormat(attachment_props.format);
    if (!stencil)
    {
        attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    }

    bool multisampled = attachment_props.samples != VK_SAMPLE_COUNT_1_BIT;
    if (multisampled)
    {
        if (attachment_description.loadOp == VK_ATTACHMENT_LOAD_OP_LOAD)
            attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    }

    static constexpr VkClearColorValue default_color_clear_value { 0.0f, 0.0f, 0.0f, 1.0f };
    static constexpr VkClearDepthStencilValue default_depth_stencil_clear_value { 1.0f, 0 };
    VkClearValue clear_value{};
    bool cleared = attachment_description.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR || attachment_description.stencilLoadOp == VK_ATTACHMENT_LOAD_OP_CLEAR;
    if (cleared)
    {
        any_cleared = true;
        if (attachment_props.clear_value)
            clear_value = *attachment_props.clear_value;
        else if (Image::isColorFormat(attachment_props.format))
            clear_value.color = default_color_clear_value;
        else
            clear_value.depthStencil = default_depth_stencil_clear_value;
    }

    if (Image::isColorFormat(attachment_props.format))
    {
        if (multisampled)
        {
            attachment_description.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            size_t resolve_attachment_index = attachment_descriptions.size();
            VkAttachmentDescription resolve_attachment_description = attachment_description;
            resolve_attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
            resolve_attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            resolve_attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            resolve_attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            resolve_attachment_description.finalLayout = attachment_props.final_layout;
            subpass_info.resolve_attachment_references.emplace_back(resolve_attachment_index, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            clear_values.emplace_back();
            attachment_descriptions.emplace_back(std::move(resolve_attachment_description));
            ++attachment_index;
        }
        else
        {
            attachment_description.finalLayout = attachment_props.final_layout;
            subpass_info.resolve_attachment_references.emplace_back(VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_UNDEFINED);
        }
        clear_values.emplace_back(clear_value);
        subpass_info.color_attachment_references.emplace_back(attachment_index, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    }
    else // Depth | Stencil
    {
        if (Image::isDepthOnlyFormat(attachment_props.format))
            attachment_description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        else if (Image::isStencilOnlyFormat(attachment_props.format))
            attachment_description.finalLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
        else if (Image::isDepthStencilFormat(attachment_props.format))
            attachment_description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        else SK_ERROR("Render pass attachment is in incompatible format");
        subpass_info.depth_attachment_reference.attachment = attachment_index;
        subpass_info.depth_attachment_reference.layout = attachment_description.finalLayout;
        clear_values.emplace_back(clear_value);
    }
    attachment_descriptions.emplace_back(std::move(attachment_description));
}

void RenderPass::addSubpassDependency(const VkSubpassDependency& dependency)
{
    subpass_dependencies.emplace_back(dependency);
}

void RenderPass::build(const std::vector<VkSubpassDependency>& dependencies)
{
    // TODO: Subpass dependencies (good reference: https://www.reddit.com/r/vulkan/comments/s80reu/subpass_dependencies_what_are_those_and_why_do_i/)
    VkSubpassDependency subpass_dependency{};
    subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpass_dependency.dstSubpass = 0;
    subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subpass_dependency.srcAccessMask = VK_ACCESS_NONE;
    subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    subpass_dependencies.emplace_back(std::move(subpass_dependency));

    subpass_dependency.srcSubpass = 0;
    subpass_dependency.dstSubpass = 0;
    subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.srcAccessMask = VK_ACCESS_NONE;
    subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpass_dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    subpass_dependencies.emplace_back(std::move(subpass_dependency));

    subpass_dependencies.insert(subpass_dependencies.end(), dependencies.begin(), dependencies.end());

    std::vector<VkSubpassDescription> subpass_descriptions(subpass_infos.size());
    for (size_t i = 0; i < subpass_infos.size(); ++i)
    {
        const auto& subpass_info = subpass_infos[i];
        for (const auto& input_attachment_reference : subpass_info.input_attachment_references)
            attachments_used_as_inputs.emplace(input_attachment_reference.attachment);

        auto& subpass_description = subpass_descriptions[i];
        subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass_description.inputAttachmentCount = subpass_info.input_attachment_references.size();
        subpass_description.pInputAttachments = subpass_info.input_attachment_references.data();
        subpass_description.colorAttachmentCount = subpass_info.color_attachment_references.size();
        subpass_description.pColorAttachments = subpass_info.color_attachment_references.data();
        subpass_description.pResolveAttachments = subpass_info.resolve_attachment_references.data();
        subpass_description.pDepthStencilAttachment = &subpass_info.depth_attachment_reference;
        subpass_description.preserveAttachmentCount = subpass_info.preserve_attachment_references.size();
        subpass_description.pPreserveAttachments = subpass_info.preserve_attachment_references.data();
    }

    VkRenderPassCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    ci.attachmentCount = attachment_descriptions.size();
    ci.pAttachments = attachment_descriptions.data();
    ci.subpassCount = subpass_descriptions.size();
    ci.pSubpasses = subpass_descriptions.data();
    ci.dependencyCount = subpass_dependencies.size();
    ci.pDependencies = subpass_dependencies.data();
    render_pass = RenderContext::getLogicalDevice().createRenderPass(ci);

    if (!any_cleared)
        clear_values.clear();
}

RenderPass::~RenderPass()
{
    RenderContext::getLogicalDevice().destroyRenderPass(render_pass);
}

void RenderPass::begin(VkSubpassContents subpass_contents)
{
    VkRenderPassBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    begin_info.renderPass = render_pass;
    begin_info.framebuffer = *framebuffer;

    begin_info.renderArea.offset = { 0, 0 };
    begin_info.renderArea.extent.width = framebuffer->getWidth();
    begin_info.renderArea.extent.height = framebuffer->getHeight();

    std::vector<VkClearValue> clear_values = this->clear_values;
    begin_info.clearValueCount = clear_values.size();
    begin_info.pClearValues = clear_values.data();

    RenderContext::getCommandBuffer().beginRenderPass(begin_info, subpass_contents);
}

void RenderPass::nextSubpass(VkSubpassContents subpass_contents)
{
    if (RenderContext::getCommandBuffer().getActive().subpass >= (subpass_infos.size() - 1))
        return;

    RenderContext::getCommandBuffer().nextSubpass(subpass_contents);
}

void RenderPass::end()
{
    RenderContext::getCommandBuffer().endRenderPass();
    // TODO: set framebuffer attachment final layouts to what they will be transitioned to, so that they can be used in a descriptor set?
}

void RenderPass::resize(const SwapChain& swap_chain)
{
    framebuffer = makeShared<Framebuffer>(swap_chain, *this, viewport.x ? viewport.x : swap_chain.getWidth(), viewport.y ? viewport.y : swap_chain.getHeight());
}
