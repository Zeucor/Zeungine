#pragma once
#include "VAO.hpp"
namespace anex::modules::gl::vaos
{
	struct VAOFactory
	{
    using ConstantSizeMap = std::unordered_map<std::string, std::tuple<uint8_t, size_t, GLenum>>;
    using VAOConstantMap = std::unordered_map<std::string, bool>;
    static ConstantSizeMap constantSizes;
    static VAOConstantMap VAOConstants;
    static void generateVAO(const RuntimeConstants &constants, GLuint &vao, GLuint &vbo, GLuint &ebo, const uint32_t &indiceCount);
		static size_t getStride(const RuntimeConstants &constants);
		static size_t getOffset(const RuntimeConstants &constants, const std::string &offsetConstant);
		static bool isVAOConstant(const std::string &constant);
    static void destroyVAO(VAO &vao);
	};
}