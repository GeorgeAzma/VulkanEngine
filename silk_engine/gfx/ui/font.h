#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H  
#include "silk_engine/gfx/images/image.h"

class Font
{
	struct CharacterInfo
	{
		ivec2 size;
		ivec2 bearing;
		uvec2 advance;
		vec4 texture_coordinate;
	};

	struct Character
	{
		vec4 position;
		vec4 texture_coordinate;
	};

public:
	static void init();
	static void destroy();

public:
	static constexpr size_t MAX_CHARACTER_COUNT = 256;

public:
	Font(const fs::path& file, uint32_t size = 64);
	~Font();

	shared<Image> getAtlas() const { return texture_atlas; }

	std::vector<Character> getCharacterLayout(std::string_view str);

private:
	static inline FT_Library free_type_library;

private:
	FT_Face face;
	fs::path file;
	std::vector<CharacterInfo> characters;
	size_t size;

	shared<Image> texture_atlas;

public:
	static shared<Font> get(std::string_view name) { if (auto it = fonts.find(name); it != fonts.end()) return it->second; else return nullptr; }
	static shared<Font> add(std::string_view name, const shared<Font> font) { return fonts.insert_or_assign(name, font).first->second; }

private:
	static inline std::unordered_map<std::string_view, shared<Font>> fonts{};
};