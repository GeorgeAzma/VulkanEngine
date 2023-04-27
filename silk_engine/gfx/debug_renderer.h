#pragma once

#include "utils/color.h"
#include "scene/light.h"
#include "scene/instance_images.h"
#include "scene/camera/camera.h"

class Image;
class Font;
class Buffer;
class VertexBuffer;
class Mesh;
class GraphicsPipeline;
class Material;

class DebugRenderer
{
public:
	static constexpr uint32_t MAX_IMAGE_SLOTS = 16;
	static constexpr uint32_t MAX_LIGHTS = 16;

	struct InstanceData
	{
		mat4 transform = mat4(1);
		vec4 color = vec4(1);
		uint32_t image_index = 0;

		bool operator==(const InstanceData& other) const
		{
			return transform == other.transform
				&& image_index == other.image_index
				&& color == color;
		}
	};

private:
	static inline struct Active
	{
		vec4 color = vec4(1);
		std::vector<shared<Image>> images{};
		shared<Font> font = nullptr;
		mat4 transform = mat4(1);
		bool transformed = false;
		float depth = 1.0f;
	} active;

	// TODO: move this in command buffer? and keep statistics only on debug mode
	static inline struct Statistics
	{
		size_t instance_batches = 0;
		size_t instances = 0;
		size_t vertices = 0;
		size_t indices = 0;
	} stats{};

	struct GlobalUniformData
	{
		mat4 projection_view;
		mat4 projection_view2D;
		mat4 projection;
		mat4 view;
		vec3 camera_position;
		float time;
		vec3 camera_direction;
		float delta_time;
		uvec2 resolution;
		uint32_t frame;
		uint32_t padding;
		std::array<Light, MAX_LIGHTS> lights;
	};

	struct CullData
	{
		vec3 min;
		uint32_t index;
		vec3 max;
		uint32_t count;
		std::array<vec4, 6> planes;
	};

	struct RenderedInstance
	{
		size_t image_count = 0;
		size_t instance_data_index = std::numeric_limits<size_t>::max();
		size_t instance_batch_index = std::numeric_limits<size_t>::max();
	};

	class RenderContextBase
	{
		struct InstanceBatch
		{
			shared<Mesh> mesh = nullptr;
			std::vector<InstanceData> instance_data;
			InstanceImages instance_images{};
			shared<Material> material = nullptr;
			bool needs_update = true;
		};
	public:
		void init();
		void destroy()
		{
			instance_buffer = nullptr;
			indirect_buffer = nullptr;
		}

		void update();
		void render();


		RenderedInstance createInstance(const shared<Mesh>& mesh, const InstanceData& instance_data, const shared<GraphicsPipeline>& pipeline = nullptr, const std::vector<shared<Image>>& images = {});
		void updateInstance(const RenderedInstance& instance, const InstanceData& instance_data)
		{
			instance_batches[instance.instance_batch_index].needs_update = true;
			instance_batches[instance.instance_batch_index].instance_data[instance.instance_data_index] = instance_data;
		}

	public:
		shared<VertexBuffer> instance_buffer;
		shared<VertexBuffer> vertex_buffer;
		shared<Buffer> indirect_buffer;
		std::vector<VkDrawIndexedIndirectCommand> draw_commands;
		std::vector<InstanceBatch> instance_batches;
	};

	class RenderContext : public RenderContextBase
	{
	public:
		shared<RenderedInstance> createInstance(const shared<Mesh>& mesh, const InstanceData& instance_data, const shared<GraphicsPipeline>& pipeline = nullptr, const std::vector<shared<Image>>& images = {});
		void destroyInstance(const RenderedInstance& instance);

	private:
		std::vector<std::vector<shared<RenderedInstance>>> instances;
	};

	class ImmediateRenderContext : public RenderContextBase
	{
	public:
		void clear()
		{
			for (auto& instance_batch : instance_batches)
			{
				instance_batch.instance_data.clear();
				instance_batch.instance_images.clear();
			}
			for (auto& instance : instances)
				instance.clear();
		}

		void createInstance(const shared<Mesh>& mesh, const InstanceData& instance_data, const shared<GraphicsPipeline>& pipeline = nullptr, const std::vector<shared<Image>>& images = {});

	private:
		std::vector<std::vector<RenderedInstance>> instances;
	};

public:
	static void init();
	static void destroy();
	static void reset();
	static void update(Camera* camera);
	static void render();

	static const Active& getActive() { return active; }
	static const shared<Image>& getWhiteImage() { return white_image; }
	static const shared<Buffer>& getGlobalUniformBuffer() { return global_uniform_buffer; }

	// States
	static void transform(const mat4& transform = mat4(1)) { active.transformed = transform != mat4(1); active.transform = transform; }
	static void color(const Color& color = Colors::WHITE) { active.color = color; }
	static void image(const shared<Image>& image = white_image) { active.images[0] = image; }
	static void depth(float depth) { active.depth = depth; }

	// 2D
	static void triangle(float x, float y, float width, float height);
	static void triangle(float x, float y, float size);
	static void triangle(float x1, float y1, float x2, float y2, float x3, float y3);
	static void rectangle(float x, float y, float width, float height);
	static void roundedRectangle(float x, float y, float width, float height);
	static void square(float x, float y, float size);
	static void roundedSquare(float x, float y, float size);
	static void ellipse(float x, float y, float width, float height);
	static void ellipseOutline(float x, float y, float width, float height);
	static void circle(float x, float y, float radius);
	static void circleOutline(float x, float y, float radius);
	static void line(const std::vector<vec2>& points, float width = 1.0f);
	static void line(float x1, float y1, float x2, float y2, float width = 1.0f);
	static void bezier(float x1, float y1, float px, float py, float x2, float y2, float width);
	static void bezier(float x1, float y1, float px1, float py1, float px2, float py2, float x2, float y2, float width);
	static void text(const std::string& text, float x, float y, float width, float height);
	static void text(const std::string& text, float x, float y, float size);
	static void image(const shared<Image>& image, float x, float y, float width, float height);
	static void image(const shared<Image>& image, float x, float y, float size);
	static void mesh(const shared<Mesh>& mesh, float x, float y, float width, float height);

	// 3D
	static void tetrahedron(float x, float y, float z, float size);
	static void cube(float x, float y, float z, float size);
	static void cuboid(float x, float y, float z, float width, float height, float depth);
	static void sphere(float x, float y, float z, float radius);
	static void ellipsoid(float x, float y, float z, float width, float height, float depth);

	// Slow function for quickly drawing stuff
	static void draw(const shared<GraphicsPipeline>& graphics_pipeline, const shared<Mesh>& mesh, const mat4& transform);
	static void draw(const shared<GraphicsPipeline>& graphics_pipeline, const shared<Mesh>& mesh, float x, float y, float z, float width, float height, float depth = 1.0f);
	static void draw(const shared<GraphicsPipeline>& graphics_pipeline, const shared<Mesh>& mesh, float x, float y, float width, float height);

	static Light* addLight(const Light& light);

	shared<RenderedInstance> createInstance(const shared<Mesh>& mesh, const InstanceData& instance_data, const shared<GraphicsPipeline>& pipeline = nullptr, const std::vector<shared<Image>>& images = {}) { render_context.createInstance(mesh, instance_data, pipeline, images); }
	void updateInstance(const RenderedInstance& instance, const InstanceData& instance_data) { render_context.updateInstance(instance, instance_data); }
	void destroyInstance(const RenderedInstance& instance) { render_context.destroyInstance(instance); }

private:
	static inline RenderContext render_context;
	static inline ImmediateRenderContext immediate_render_context;
	static shared<Buffer> global_uniform_buffer;
	static std::array<Light, MAX_LIGHTS> lights;
	static shared<Image> white_image;
	static shared<GraphicsPipeline> graphics_pipeline_2D;
	static shared<GraphicsPipeline> graphics_pipeline_3D;
};