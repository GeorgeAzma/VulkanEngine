#pragma once

class Image;
class InstanceImages;
class ThreadPool;
class Material;
class Buffer;

struct ParticleProps
{
	vec3 position = vec3(0);
	vec3 velocity = vec3(0);
	vec3 velocity_variation = vec3(0);
	vec3 acceleration = vec3(0);
	vec4 color_begin = vec4(1);
	vec4 color_end = vec4(1);
	float size_begin = 1;
	float size_end = 1;
	shared<Image> image = nullptr;
	float life_time = 1.0f;
};

struct ParticleSpoutProps
{
	ParticleProps particle_properties;
	float duration = 0.0f;
	float particles_per_emission = 1;
	vec3(*update_position)() = nullptr;
};

struct Particles
{
	static inline const ParticleProps flame{ {0, 0, 0}, { 0, 3, 0 }, { 1, 1, 1 }, { 0, 0, 0 }, {1, 0.75f, 0, 1}, {1, 0, 0, 0.0f}, .25f, .01f, nullptr, 1.0f };
};

class ParticleSystem
{
	struct Particle
	{
		vec3 position;
		vec3 velocity;
		vec3 acceleration;
		vec4 color_begin;
		vec4 color_end;
		float rotation_begin;
		float rotation_end;
		float size_begin;
		float size_end;
		float life_time = 1.0f;
		float life_remaining = 0;
		uint32_t iamge_index = 0;
	};

	struct ParticleSpout
	{
		ParticleProps particle_properties;
		float duration;
		float particles_per_emission;
		vec3(*update_position)();
		float passed_duration;
	};

	struct ParticleData
	{
		mat4 model = mat4(1);
		vec4 color = vec4(1);
		int32_t iamge_index = -1;
	};

public:
	static constexpr uint32_t MAX_PARTICLES = 65536;

public:
	static void init(VkRenderPass render_pass);

	static void addSpout(const ParticleSpoutProps& props);
	static void emit(const ParticleProps& props);
	
	static void update();
	static void render(Material& material);

	static void destroy();

private:
	static inline std::vector<Particle> particles;
	static inline std::vector<ParticleSpout> particle_spouts;
	static inline std::vector<ParticleData> particle_data;
	static inline shared<Buffer> instance_vbo;
	static inline shared<InstanceImages> instance_images;
	static unique<ThreadPool> thread_pool;
	static inline shared<Material> material = nullptr;
};