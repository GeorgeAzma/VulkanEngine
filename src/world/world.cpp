#include "world.h"
#include "silk_engine/gfx/material.h"
#include "silk_engine/gfx/pipeline/graphics_pipeline.h"
#include "silk_engine/gfx/render_context.h"
#include "silk_engine/gfx/pipeline/render_graph/render_graph.h"
#include "silk_engine/gfx/pipeline/render_pass.h"
#include "silk_engine/gfx/debug_renderer.h"
#include "silk_engine/gfx/devices/logical_device.h"
#include "silk_engine/scene/camera/camera_controller.h"
#include "silk_engine/scene/components.h"
#include "silk_engine/utils/debug_timer.h"

World::World()
{
	player = Scene::getActive()->createEntity();
	player->add<CameraComponent>();
	player->add<ScriptComponent>().bind<CameraController>();
	player->get<CameraComponent>().camera.position = vec3(0.0f, 0.0f, 8.0f);
	camera = &player->get<CameraComponent>().camera;

	VkRenderPass render_pass = RenderContext::getRenderGraph().getPass("Geometry").getRenderPass();
	shared<GraphicsPipeline> chunk_pipeline = makeShared<GraphicsPipeline>();
	chunk_pipeline->setShader(makeShared<Shader>("chunk", Shader::Defines{
		{ "SIZE", std::to_string(Chunk::SIZE) },
		{ "AREA", std::to_string(Chunk::AREA) },
		{ "VOLUME", std::to_string(Chunk::VOLUME) } }))
		.setRenderPass(render_pass)
		.setSamples(RenderContext::getPhysicalDevice().getMaxSampleCount())
		.enableTag(GraphicsPipeline::EnableTag::DEPTH_WRITE)
		.enableTag(GraphicsPipeline::EnableTag::DEPTH_TEST)
		.setCullMode(GraphicsPipeline::CullMode::FRONT)
		.setDepthCompareOp(GraphicsPipeline::CompareOp::LESS);
	chunk_pipeline->build();
	material = makeShared<Material>(chunk_pipeline);
	Image::Props props{};
	props.sampler_props.mag_filter = VK_FILTER_NEAREST;
	props.sampler_props.min_filter = VK_FILTER_NEAREST;
	texture_atlas = makeShared<Image>(block_textures, props);
	texture_atlas->transitionLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

#define MULTITHREAD 0

void World::update()
{
	const vec3& origin = camera->position;
	const Chunk::Coord& chunk_origin = toChunkCoord((World::Coord)round(origin));

	// Delete far chunks
	RenderContext::getLogicalDevice().wait();
	constexpr float max_chunk_distance = 8;
	constexpr float max_chunk_distance2 = max_chunk_distance * max_chunk_distance;
	for (int32_t i = 0; i < chunks.size(); ++i)
	{
		if (distance2(vec3(chunks[i]->getPosition()), vec3(chunk_origin)) > max_chunk_distance2)
		{
			// TODO: When removing chunk, other chunks neighbors are made invalid, fix that
			//std::swap(chunks[i], chunks.back());
			//chunks.pop_back();
			//--i;
		}
	}

	// Build chunks and regenerate chunks with new neighbors
#if MULTITHREAD
	pool.forEach(chunks.size(), [&](size_t i) {
		if (!isChunkVisible(chunks[i]->getPosition()))
			return;
		chunks[i]->generateMesh();
	});
#else
	for (size_t i = 0; i < chunks.size(); ++i)
	{
		if (!isChunkVisible(chunks[i]->getPosition()))
			continue;
		chunks[i]->generateMesh();
	}
#endif

	constexpr size_t max_chunks = 16 * 16 * 16; // 4096
	if (chunks.size() >= max_chunks)
		return;

	// Queue up to be generated chunks
	std::ranges::sort(chunks, [&](const shared<Chunk>& lhs, const shared<Chunk>& rhs) {
		vec3 modifier_lhs = vec3(1, 2, 1) * ((lhs->getFill() != Block::NONE) * 16.0f + 1.0f);
		vec3 modifier_rhs = vec3(1, 2, 1) * ((rhs->getFill() != Block::NONE) * 16.0f + 1.0f);
		return distance2(vec3(chunk_origin) * modifier_lhs, vec3(lhs->getPosition()) * modifier_lhs) <
			   distance2(vec3(chunk_origin) * modifier_rhs, vec3(rhs->getPosition()) * modifier_rhs);
	});

	std::vector<Chunk::Coord> queued_chunks;
	if (!findChunk(chunk_origin))
		queued_chunks.emplace_back(chunk_origin);
	constexpr size_t max_queued_chunks = 1;
	for (const auto& chunk : chunks)
	{
		if (distance2(vec3(chunk_origin), vec3(chunk->getPosition())) > max_chunk_distance2 || !isChunkVisible(chunk->getPosition()))
			continue;
		const std::vector<Chunk::Coord> missing_neighbors = chunk->getMissingNeighborLocations();
		for (const auto& missing : missing_neighbors)
		{
			if (findChunk(chunk->getPosition() + missing))
				continue;
			queued_chunks.emplace_back(chunk->getPosition() + missing);
			if (queued_chunks.size() >= max_queued_chunks)
				break;
		}
		if (queued_chunks.size() >= max_queued_chunks)
			break;
	}

#if MULTITHREAD
	for (const auto& chunk : queued_chunks)
	{
		chunks.emplace_back(makeShared<Chunk>(chunk, material->getPipeline()));
		std::array<Chunk::Coord, 26> neighbors = Chunk::getNeighborCoords();
		for (size_t i = 0; i < neighbors.size(); ++i)
			chunks.back()->addNeighbor(i, findChunk(chunk + neighbors[i]));
	}
	pool.forEach(queued_chunks.size(), [this](size_t i) { chunks[chunks.size() - 1 - i]->allocate(); chunks[chunks.size() - 1 - i]->generate(); });
#else
	for (const auto& chunk : queued_chunks)
	{
		chunks.emplace_back(makeShared<Chunk>(chunk, material->getPipeline()));
		std::array<Chunk::Coord, 26> neighbors = Chunk::getNeighborCoords();
		for (size_t i = 0; i < neighbors.size(); ++i)
			chunks.back()->addNeighbor(i, findChunk(chunk + neighbors[i]));
		chunks.back()->allocate(); 
		chunks.back()->generate();
	}
#endif
}

void World::render()
{
	material->set("GlobalUniform", *DebugRenderer::getGlobalUniformBuffer());
	material->set("texture_atlas", *texture_atlas);
	material->bind();
	for (const auto& chunk : chunks)
	{
		if (!isChunkVisible(chunk->getPosition()))
			continue;
		chunk->render();
	}
}

bool World::isChunkVisible(const Chunk::Coord& position) const
{
	return camera->frustum.isBoxVisible(toWorldCoord(position), toWorldCoord(position) + Chunk::DIM);
}
