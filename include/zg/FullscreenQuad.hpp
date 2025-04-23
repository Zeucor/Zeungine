#pragma once
#include <zg/vaos/VAO.hpp>
#include <zg/glm.hpp>
#include <zg/textures/Texture.hpp>
#include <zg/shaders/RuntimeConstants.hpp>
namespace zg
{
    struct FullscreenQuad : vaos::VAO
    {
        glm::mat4 model = glm::mat4(1.0f);
        FullscreenQuad(IRenderer* iRenderer, const shaders::RuntimeConstants& constants);
        void render(const std::vector<std::pair<std::string, std::shared_ptr<zg::textures::Texture>>>& inputTextures, bool shaderAlreadyBound = false);
    };
}