#pragma once
#include <array>
#include <zg/Entity.hpp>
#include <zg/glm.hpp>
#include <zg/conversions/ToAssimp.hpp>
#include <zg/interfaces/IFile.hpp>
namespace zg::entities
{
	EntityCreateInfo ModelFactory(
        const interfaces::IFile& modelFile,
        std::string name = "",
        glm::vec3 position = {0, 0, 0},
        glm::quat rotation = {1, 0, 0, 0},
        glm::vec3 scale = {1, 1, 1},
		const shaders::RuntimeConstants& constants = {},
        IRenderer* iRenderer = 0
    );
}
