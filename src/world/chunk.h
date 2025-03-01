#pragma once

#include "block.h"

class Buffer;

class Chunk : NoCopy
{
	friend class World;

public:
	using Coord = ivec3;
	using Vertex = uint64_t;

public:
	static constexpr int32_t SIZE = 64;
	static constexpr int32_t EDGE = SIZE - 1;
	static constexpr int32_t AREA = SIZE * SIZE;
	static constexpr int32_t VOLUME = SIZE * AREA;
	static constexpr Coord DIM = Coord(SIZE);

	static constexpr int32_t SHARED_SIZE = SIZE + 2;
	static constexpr int32_t SHARED_EDGE = SHARED_SIZE - 1;
	static constexpr int32_t SHARED_AREA = SHARED_SIZE * SHARED_SIZE;
	static constexpr int32_t SHARED_VOLUME = SHARED_SIZE * SHARED_AREA;
	static constexpr Coord SHARED_DIM = Coord(SHARED_SIZE);

	static constexpr size_t MAX_VERTICES = VOLUME * 6 * 6 / 2;
	static constexpr Chunk::Coord NEIGHBORS[26]
	{
		{ -1, -1, -1 }, //  0 ^
		{ 00, -1, -1 }, //  1 -
		{ +1, -1, -1 }, //  2 ^
		{ -1, -1, 00 }, //  3 -
		{ 00, -1, 00 }, //  4 *
		{ +1, -1, 00 }, //  5 -
		{ -1, -1, +1 }, //  6 ^
		{ 00, -1, +1 }, //  7 -
		{ +1, -1, +1 }, //  8 ^
		{ -1, 00, -1 }, //  9 -
		{ 00, 00, -1 }, // 10 *
		{ +1, 00, -1 }, // 11 -
		{ -1, 00, 00 }, // 12 *
		{ +1, 00, 00 }, // 13 *
		{ -1, 00, +1 }, // 14 -
		{ 00, 00, +1 }, // 15 *
		{ +1, 00, +1 }, // 16 -
		{ -1, +1, -1 }, // 17 ^
		{ 00, +1, -1 }, // 18 -
		{ +1, +1, -1 }, // 19 ^
		{ -1, +1, 00 }, // 20 -
		{ 00, +1, 00 }, // 21 *
		{ +1, +1, 00 }, // 22 -
		{ -1, +1, +1 }, // 23 ^
		{ 00, +1, +1 }, // 24 -
		{ +1, +1, +1 }  // 25 ^
	};
	static constexpr Chunk::Coord ADJACENT_NEIGHBORS[6]
	{
		{ 00, -1, 00 },
		{ 00, 00, -1 },
		{ -1, 00, 00 },
		{ +1, 00, 00 },
		{ 00, 00, +1 },
		{ 00, +1, 00 }
	};

public:
	Chunk(const Coord& position)
		: position(position) {}
	~Chunk();

	void generateStart();
	void generateEnd();
	void generateMesh();
	void render() const;

	void addNeighbor(size_t index, Chunk* neighbor)
	{
		if (neighbors[index] == neighbor)
			return;
		neighbors[index] = neighbor;
		dirty = true; 
		updateNeighboringBlocks(index);
		neighbor->addNeighbor(getNeighborIndexFromCoord(position - neighbor->getPosition()), this);
	}

	std::vector<Chunk::Coord> getMissingAdjacentNeighborLocations() const
	{
		std::vector<Chunk::Coord> missing_neighbors;
		for (size_t i = 0; i < 6; ++i)
			if (!neighbors[getNeighborIndexFromCoord(ADJACENT_NEIGHBORS[i])])
				missing_neighbors.emplace_back(ADJACENT_NEIGHBORS[i]);
		return missing_neighbors;
	}

	Block& at(uint32_t x, uint32_t y, uint32_t z) { return blocks.size() ? blocks[idx(x, y, z)] : fill; }
	Block at(uint32_t x, uint32_t y, uint32_t z) const { return blocks.size() ? blocks[idx(x, y, z)] : fill; }
	const Coord& getPosition() const { return position; }
	const shared<Buffer>& getVertexBuffer() const { return vertex_buffer; }
	uint32_t getVertexCount() const { return vertex_count; }
	Block getFill() const { return fill; }

	bool operator==(const Chunk& other) const { return position == other.position; }
	bool operator==(const Chunk::Coord& chunk_coord) const { return position == chunk_coord; }

private:
	void updateNeighboringBlocks(size_t index);

public:
	static size_t idx(uint32_t x, uint32_t y, uint32_t z) { return (y + 1) * SHARED_AREA + (z + 1) * SHARED_SIZE + (x + 1); }

	static uint32_t getNeighborIndexFromCoord(const Chunk::Coord& position)
	{
		uint32_t index = (position.x + 1) + (position.y + 1) * 9 + (position.z + 1) * 3;
		return index - (index >= 13); // Excluding self
	}

	static Chunk::Coord toChunkCoord(const Chunk::Coord& position)
	{
		return Chunk::Coord(
			(position.x < 0) ? ((position.x + 1 - Chunk::SIZE) / Chunk::SIZE) : (position.x / Chunk::SIZE),
			(position.y < 0) ? ((position.y + 1 - Chunk::SIZE) / Chunk::SIZE) : (position.y / Chunk::SIZE),
			(position.z < 0) ? ((position.z + 1 - Chunk::SIZE) / Chunk::SIZE) : (position.z / Chunk::SIZE)
		);
	}
	static Chunk::Coord toBlockCoord(const Chunk::Coord& position) { return (Chunk::DIM + (position % Chunk::DIM)) % Chunk::DIM; }
	static Chunk::Coord toWorldCoord(const Chunk::Coord& position) { return position * Chunk::DIM; }

private:
	static uint64_t getAO(uint32_t side1, uint32_t side2, uint32_t corner)
	{
		return (side1 && side2) ? 0 : 3 - (side1 + side2 + corner);
	}

public:
	bool visible = true;

private:
	Coord position = Coord(0);
	std::vector<Block> blocks = {};
	uint32_t vertex_count = 0;
	shared<Buffer> vertex_buffer = nullptr;
	shared<Buffer> block_buffer = nullptr;
	std::array<Chunk*, 26> neighbors = {};
	bool dirty = true;
	Block fill = Block::ANY;
};