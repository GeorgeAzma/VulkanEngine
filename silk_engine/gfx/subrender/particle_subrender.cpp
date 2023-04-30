#include "particle_subrender.h"
#include "gfx/particle_system.h"
#include "gfx/renderer.h"
#include "gfx/devices/physical_device.h"
#include "gfx/debug_renderer.h"
#include "gfx/render_context.h"
#include "gfx/material.h"
#include "gfx/pipeline/render_pass.h"

ParticleSubrender::ParticleSubrender(RenderPass& render_pass, uint32_t subpass)
{
    shared<GraphicsPipeline> pipeline = makeShared<GraphicsPipeline>();
    pipeline->setShader(makeShared<Shader>("particle"))
        .setSamples(RenderContext::getPhysicalDevice().getMaxSampleCount())
        .setRenderPass(render_pass)
        .setSubpass(subpass)
        .setDepthCompareOp(GraphicsPipeline::CompareOp::LESS)
        .build();
    material = makeShared<Material>(pipeline);
}

void ParticleSubrender::render()
{
    ParticleSystem::render(*material);
}
