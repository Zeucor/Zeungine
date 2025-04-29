#include <zg/FullscreenQuad.hpp>
#include <zg/utilities.hpp>
#include <zg/Window.hpp>
using namespace zg;
FullscreenQuad::FullscreenQuad(const FullscreenQuad& other):
    vaos::VAO(other.VAO_INDEX_STACK, other.constants, other.indiceCount, other.vertexCount),
    model(other.model)
{
    generateQuad();
};
FullscreenQuad::FullscreenQuad(const std::vector<size_t*>& INDEX_STACK, const shaders::RuntimeConstants& constants):
    vaos::VAO(INDEX_STACK, zg::mergeVectors({ "UV2", "Position", "Model" }, constants), 6, 4),
    model(glm::translate(glm::vec3(0, 0, -(0.1 * (INDEX_STACK.size() > 1 ? *INDEX_STACK[1] : *INDEX_STACK[0])))))
{
    generateQuad();
}
FullscreenQuad& FullscreenQuad::operator=(const FullscreenQuad& other)
{
    model = other.model;
    return *this;
}
void FullscreenQuad::generateQuad()
{
    std::vector<uint32_t> indices;
	if (vaoIRenderer->frontFace == zg::COUNTERCLOCKWISE)
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
	bool flipUVs = (vaoIRenderer->renderer == RENDERER_VULKAN || vaoIRenderer->renderer == RENDERER_METAL);
    zg::Entity::flipUVsY(uv2s);
    updateElements("UV2", uv2s);
}
void FullscreenQuad::render(const std::vector<std::pair<std::string, std::shared_ptr<zg::textures::Texture>>>& inputTextures, bool shaderAlreadyBound)
{
	auto shader = addShader();
    if (!shaderAlreadyBound)
    	shader->bind(*this);
	shader->setBlock("Model", *this, model);
    uint32_t unit = 0;
    auto constantsBegin = constants.begin();
    auto constantsEnd = constants.end();
    for (auto& pair : inputTextures)
    {
        auto found_iter = std::find_if(constantsBegin, constantsEnd, [&](auto& val)
        {
            return val == pair.first;
        });
        if (found_iter != constantsEnd)
            shader->setTexture(pair.first, *this, *pair.second, unit++);
    }
	drawVAO();
	shader->unbind();
}
template <>
Serial& serialize(Serial& serial, const std::shared_ptr<FullscreenQuad>& fsq)
{
    return serial << fsq->constants;
}
template <>
Serial& deserialize(Serial& serial, std::shared_ptr<FullscreenQuad>& fsq)
{
    auto windowPointer = (Window*)serial.getContextPointer("Window");
    auto scenePointer = (Scene*)serial.getContextPointer("Scene");
    std::vector<size_t*> INDEX_STACK;
    if (scenePointer)
        INDEX_STACK = scenePointer->INDEX_STACK;
    if (windowPointer)
        INDEX_STACK = windowPointer->INDEX_STACK;
    shaders::RuntimeConstants constants;
    serial >> constants;
    fsq = std::make_shared<FullscreenQuad>(INDEX_STACK, constants);
    return serial;
}