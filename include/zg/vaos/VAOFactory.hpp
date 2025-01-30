#pragma once
#include "VAO.hpp"
#include <unordered_map>
namespace zg::vaos
{
	struct VAOFactory
	{
    using ConstantSizeMap = std::unordered_map<std::string_view, std::tuple<uint8_t, size_t, GLenum>>;
    using VAOConstantMap = std::unordered_map<std::string_view, bool>;
    static ConstantSizeMap constantSizes;
    static VAOConstantMap VAOConstants;
    static void generate(VAO &vao);
		static size_t getStride(const RuntimeConstants &constants);
		static size_t getOffset(const RuntimeConstants &constants, const std::string_view offsetConstant);
		static bool isVAOConstant(const std::string_view constant);
    static void destroy(VAO &vao);
	};
}