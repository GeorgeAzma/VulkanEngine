#pragma once

#include <shaderc/shaderc.hpp>

class Includer : public shaderc::CompileOptions::IncluderInterface
{
public:

	shaderc_include_result* GetInclude(const char* requested_path, shaderc_include_type type, const char* requesting_path, size_t include_depth) override;

	void ReleaseInclude(shaderc_include_result* data) override;
};