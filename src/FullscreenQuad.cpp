#include <zg/FullscreenQuad.hpp>
#include <zg/utilities.hpp>
#include <zg/Entity.hpp>
using namespace zg;
FullscreenQuad::FullscreenQuad(IRenderer* iRenderer, const shaders::RuntimeConstants& constants):
    vaos::VAO(iRenderer, zg::mergeVectors(constants, { "UV2", "Position", "Model" }), 6, 4)
{
    std::vector<uint32_t> indices;
	if (iRenderer->frontFace == zg::COUNTERCLOCKWISE)
		indices = {
			2,	1,	0,	0,	3,	2, // Front face
		};
	else
		indices = {
			0,	1,	2,	2,	3,	0, // Front face
		};
    glm::vec2 size(2, 2);
    std::vector<glm::vec3> vertices({{
        {-size.x / 2, -size.y / 2, 0},	 {size.x / 2, -size.y / 2, 0},
        {size.x / 2, size.y / 2, 0},		 {-size.x / 2, size.y / 2, 0} // Front
    }});
    updateIndices(indices);
    updateElements("Position", vertices);
	std::vector<glm::vec2> uv2s({
        {0, 1},
        {1, 1},
        {1, 0},
        {0, 0}
    });
	bool flipUVs = (iRenderer->renderer == RENDERER_VULKAN || iRenderer->renderer == RENDERER_METAL);
    zg::Entity::flipUVsY(uv2s);
    updateElements("UV2", uv2s);
}
void FullscreenQuad::render(const std::vector<std::pair<std::string, std::shared_ptr<zg::textures::Texture>>>& inputTextures)
{
	auto shader = addShader();
	shader->bind(*this);
	shader->setBlock("Model", *this, model);
    uint32_t unit = 0;
    for (auto& pair : inputTextures)
        shader->setTexture(pair.first, *this, *pair.second, unit++);
	drawVAO();
	shader->unbind();
}