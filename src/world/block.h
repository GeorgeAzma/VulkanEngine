#pragma once

enum class Block : uint32_t
{
	AIR = 0,
	STONE,
	GRASS,
	DIRT,
	SAND,
	SANDSTONE,
	WATER,
	LEAF,
	OAK_LOG,
	SNOW,

	LAST,
	ANY,
	NONE = std::numeric_limits<std::underlying_type_t<Block>>::max()
};
static constexpr size_t TOTAL_BLOCKS = ecast(Block::LAST);

static constexpr const char* BLOCK_NAMES[TOTAL_BLOCKS]
{
	"AIR",
	"STONE",
	"GRASS",
	"DIRT",
	"SAND",
	"SANDSTONE",
	"WATER",
	"LEAF",
	"OAK_LOG",
	"SNOW"
};

// SOLID
static constexpr bool BLOCK_SOLID[TOTAL_BLOCKS]
{
	/* AIR		 */  0,
	/* STONE	 */  1,
	/* GRASS	 */  1,
	/* DIRT		 */  1,
	/* SAND		 */  1,
	/* SANDSTONE */  1,
	/* WATER	 */  0,
	/* LEAF		 */  0,
	/* OAK_LOG	 */  1,
	/* SNOW		 */  1
};

static constexpr uint32_t BLOCK_TEXTURE_INDICES[TOTAL_BLOCKS * 6]
{
	/* AIR		 */  0, 0, 0, 0, 0, 0,
	/* STONE	 */  1, 1, 1, 1, 1, 1,
	/* GRASS	 */  4, 3, 3, 3, 3, 2,
	/* DIRT		 */  4, 4, 4, 4, 4, 4,
	/* SAND		 */  5, 5, 5, 5, 5, 5,
	/* SANDSTONE */  6, 6, 6, 6, 6, 7,
	/* WATER	 */  8, 8, 8, 8, 8, 8,
	/* LEAF		 */  9, 9, 9, 9, 9, 9,
	/* OAK_LOG	 */  11, 10, 10, 10, 10, 11,
	/* SNOW		 */  12, 12, 12, 12, 12, 12
};

static inline const auto BLOCK_TEXTURES = makeArray<fs::path>
(
	"res/images/blocks/Air.png",
	"res/images/blocks/Stone.png",
	"res/images/blocks/Grass.png",
	"res/images/blocks/GrassSide.png",
	"res/images/blocks/Dirt.png",
	"res/images/blocks/Sand.png",
	"res/images/blocks/Sandstone.png",
	"res/images/blocks/SandstoneTop.png",
	"res/images/blocks/Water.png",
	"res/images/blocks/Leaf.png",
	"res/images/blocks/OakLog.png",
	"res/images/blocks/OakTop.png",
	"res/images/blocks/SnowBlock.png"
);