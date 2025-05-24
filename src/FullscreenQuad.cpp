#include <zg/FullscreenQuad.hpp>
#include <zg/utilities.hpp>
#include <zg/Window.hpp>
#include <zg/Mesh.hpp>
using namespace zg;
FullscreenQuad::FullscreenQuad(const FullscreenQuad& other):
    vaos::VAO(other),
    view(other.view),
    projection(other.projection),
    model(other.model)
{
};
FullscreenQuad::FullscreenQuad(const std::vector<size_t*>& INDEX_STACK, const shaders::RuntimeConstants& constants):
    vaos::VAO(INDEX_STACK, zg::mergeVectors({ "Viewport", "Shape", "UV2", "Position", "Normal", "Model", "View", "Projection" }, constants), 6, 6),
    model(
        glm::scale(glm::translate(glm::vec3(0, 0, -(0.1 * (INDEX_STACK.size() > 1 ? *INDEX_STACK[1] : *INDEX_STACK[0])))), glm::vec3(2, 2, 1)))
{
    auto& window = Registry::GetSingleton().getWindow(INDEX_STACK);
    vp::View _view({0, 0, 2}, {0, 0, -1}, {0, 1, 0});
    view = _view.matrix;
    vp::Projection _projection(window, {2, 2}, 0.1f, 10.f);
    projection = _projection.matrix;
}
FullscreenQuad& FullscreenQuad::operator=(const FullscreenQuad& other)
{
    model = other.model;
    return *this;
}
void FullscreenQuad::generateQuad()
{
}
void FullscreenQuad::render(const std::vector<std::pair<std::string, std::shared_ptr<zg::textures::Texture>>>& inputTextures, bool shaderAlreadyBound)
{
	auto shader = addShader();
    if (!shaderAlreadyBound)
    	shader->bind(*this);
    auto& window = Registry::GetSingleton().getWindow(VAO_INDEX_STACK);
    shader->setBlock("Viewport", *this, window.viewport, 16);
	shader->setSSBO("InstanceModels", *this, &model, sizeof(glm::mat4));
    auto inversemodel = glm::inverse(model);
	shader->setSSBO("InverseInstanceModels", *this, &inversemodel, sizeof(glm::mat4));
	shader->setSSBO("InstanceViews", *this, &view, sizeof(glm::mat4));
    auto inverseview = glm::inverse(view);
	shader->setSSBO("InverseInstanceViews", *this, &inverseview, sizeof(glm::mat4));
	shader->setSSBO("InstanceProjections", *this, &projection, sizeof(glm::mat4));
    auto inverseprojection = glm::inverse(projection);
	shader->setSSBO("InverseInstanceProjections", *this, &inverseprojection, sizeof(glm::mat4));
    GLEntity gl_entity{
        .shape_type = int32_t(ShapeType::PlaneXY),
        .material_index = 0,
        .vertex_offset = 0,
        .padding = 0,
        .uv2_offset = 0,
        .uv3_offset = 0,
        .meta_int = 0,
        .meta_float = 0.f,
        .meta_vec4 = glm::vec4(0)
    };
    shader->setSSBO("Entities", *this, &gl_entity, sizeof(GLEntity) * 1);
    Material material{
        .albedo = glm::vec4(1),
        .type = 1
    };
    shader->setSSBO("Materials", *this, &material, sizeof(Material) * 1);
    glm::vec4 vec(0);
    shader->setSSBO("MeshPositions", *this, &vec, sizeof(glm::vec4) * 1);
	static std::vector<glm::vec2> uv2s({
        {0, 1},
        {1, 1},
        {1, 0},
        {0, 0}
    });
	bool flipUVs = (vaoIRenderer->renderer == RENDERER_VULKAN || vaoIRenderer->renderer == RENDERER_METAL);
    zg::Mesh::flipUVsY(uv2s);
    shader->setSSBO("EntityUV2s", *this, uv2s.data(), sizeof(glm::vec2) * uv2s.size());
    shader->setSSBO("EntityUV3s", *this, &vec, sizeof(glm::vec4) * 1);
    uint32_t unit = 0;
    auto constantsBegin = vaoConstants.begin();
    auto constantsEnd = vaoConstants.end();
    for (auto& pair : inputTextures)
    {
        auto found_iter = std::find_if(constantsBegin, constantsEnd, [&](auto& val)
        {
            return val == pair.first;
        });
        if (found_iter != constantsEnd)
            shader->setTexture(pair.first, *this, *pair.second, unit++);
    }
	drawVAO(shader);
	shader->unbind();
}
template <>
Serial& serialize(Serial& serial, const std::shared_ptr<FullscreenQuad>& fsq)
{
    return serial << fsq->vaoConstants;
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