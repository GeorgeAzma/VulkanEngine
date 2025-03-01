#include "sampler.h"
#include "silk_engine/gfx/render_context.h"
#include "silk_engine/gfx/devices/physical_device.h"
#include "silk_engine/gfx/devices/logical_device.h"

Sampler::Sampler(const Props& props)
{
	VkSamplerCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	ci.magFilter = VkFilter(props.mag_filter);
	ci.minFilter = VkFilter(props.min_filter);
	ci.addressModeU = VkSamplerAddressMode(props.u_wrap);
	ci.addressModeV = VkSamplerAddressMode(props.v_wrap);
	ci.addressModeW = VkSamplerAddressMode(props.w_wrap);
	if (props.anisotropy != 1.0f)
	{
		ci.anisotropyEnable = VK_TRUE;
		ci.maxAnisotropy = props.anisotropy == 0.0f ? RenderContext::getPhysicalDevice().getProperties().limits.maxSamplerAnisotropy : props.anisotropy;
	}
	else
	{
		ci.anisotropyEnable = VK_FALSE;
		ci.maxAnisotropy = props.anisotropy;
	}
	ci.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	ci.unnormalizedCoordinates = VK_FALSE;
	ci.compareEnable = VK_FALSE;
	ci.compareOp = VK_COMPARE_OP_ALWAYS;
	ci.mipmapMode = props.mipmap_mode != MipmapMode::NONE ? VkSamplerMipmapMode(props.mipmap_mode) : VK_SAMPLER_MIPMAP_MODE_LINEAR;
	ci.mipLodBias = 0.0f;
	ci.minLod = 0.0f; 
	ci.maxLod = VK_LOD_CLAMP_NONE;
	sampler = RenderContext::getLogicalDevice().createSampler(ci);
}

Sampler::~Sampler()
{
	RenderContext::getLogicalDevice().destroySampler(sampler);
}