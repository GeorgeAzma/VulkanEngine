#include "mesh_subrender.h"
#include "gfx/devices/physical_device.h"
#include "gfx/pipeline/graphics_pipeline.h"
#include "gfx/debug_renderer.h"
#include "gfx/buffers/command_buffer.h"
#include "gfx/buffers/buffer.h"
#include "gfx/render_context.h"
#include "gfx/pipeline/render_pass.h"

MeshSubrender::MeshSubrender(RenderPass& render_pass, uint32_t subpass)
{
    shared<GraphicsPipeline> graphics_pipeline = makeShared<GraphicsPipeline>();
    graphics_pipeline->setShader(makeShared<Shader>("3D"))
        .setSamples(RenderContext::getPhysicalDevice().getMaxSampleCount())
        .setRenderPass(render_pass)
        .setSubpass(subpass)
        .setDepthCompareOp(GraphicsPipeline::CompareOp::LESS)
        .build();
    GraphicsPipeline::add("3D", graphics_pipeline);

    graphics_pipeline = makeShared<GraphicsPipeline>();
    graphics_pipeline->setShader(makeShared<Shader>("2D"))
        .setSamples(RenderContext::getPhysicalDevice().getMaxSampleCount())
        .setRenderPass(render_pass)
        .setSubpass(subpass)
        .setDepthCompareOp(GraphicsPipeline::CompareOp::LESS_OR_EQUAL)
        .build();
    GraphicsPipeline::add("2D", graphics_pipeline);

    graphics_pipeline = makeShared<GraphicsPipeline>();
    graphics_pipeline->setShader(makeShared<Shader>("font"))
        .setSamples(RenderContext::getPhysicalDevice().getMaxSampleCount())
        .setRenderPass(render_pass)
        .setSubpass(subpass)
        .setDepthCompareOp(GraphicsPipeline::CompareOp::ALWAYS)
        .build();
    GraphicsPipeline::add("Font", graphics_pipeline);
}

void MeshSubrender::render()
{
    DebugRenderer::render();
}