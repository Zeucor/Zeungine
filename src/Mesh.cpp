#include <zg/Mesh.hpp>
#include <zg/Window.hpp>
using namespace zg;
Mesh::Mesh(const MeshCreateInfo& info, Entity& entity):
    VAO(
        entity.INDEX_STACK,
        info.constants,
        (info.indiceCount ? info.indiceCount(entity) : 0),
        (info.vertexCount ? info.vertexCount(entity) : 0)
    ),
    hash(info.hash),
    keyedTextures(info.keyedTextures),
    indiceCount(info.indiceCount),
    indices(info.indices),
    vertexCount(info.vertexCount),
    vertices(info.vertices),
    colorCount(info.colorCount),
    colors(info.colors),
    uv2Count(info.uv2Count),
    uv2s(info.uv2s),
    uv3Count(info.uv3Count),
    uv3s(info.uv3s)
{
	auto& window = Registry::getWindow(entity.INDEX_STACK);
	if (!indices || !vertices)
        return;
    auto _indices_ = indices(entity);
    auto _vertices_ = vertices(entity);
    std::vector<glm::vec3> normals;
    computeNormals(window.iRenderer->frontFace, _indices_, _vertices_, normals);
    updateIndices(_indices_);
    if (colorCount && colorCount(entity))
        updateElements("Color", colors(entity));
    bool flipUVs = (window.iRenderer->renderer == RENDERER_VULKAN || window.iRenderer->renderer == RENDERER_METAL);
    if (uv2Count && uv2Count(entity))
    {
        auto _uv2s_ = uv2s(entity);
        flipUVsY(_uv2s_);
        updateElements("UV2", _uv2s_);
    }
    if (uv3Count && uv3Count(entity))
    {
        auto _uv3s_ = uv3s(entity);
        flipUVsY(_uv3s_);
        updateElements("UV3", _uv3s_);
    }
    updateElements("Position", _vertices_);
    updateElements("Normal", normals);
}
Mesh::Mesh(const Mesh& other):
    VAO(other),
	ID(other.ID),
	INDEX(other.INDEX),
	INDEX_STACK(other.INDEX_STACK),
    hash(other.hash),
	keyedTextures(other.keyedTextures),
	indiceCount(other.indiceCount),
	indices(other.indices),
	vertexCount(other.vertexCount),
	vertices(other.vertices),
	colorCount(other.colorCount),
	colors(other.colors),
	uv2Count(other.uv2Count),
	uv2s(other.uv2s),
	uv3Count(other.uv3Count),
	uv3s(other.uv3s)
{}
Mesh& Mesh::operator=(const Mesh& other)
{
	((vaos::VAO&)*this) = other;
	ID = other.ID;
	INDEX = other.INDEX;
	INDEX_STACK = other.INDEX_STACK;
    hash = other.hash;
	keyedTextures = other.keyedTextures;
	indiceCount = other.indiceCount;
	indices = other.indices;
	vertexCount = other.vertexCount;
	vertices = other.vertices;
	colorCount = other.colorCount;
	colors = other.colors;
	uv2Count = other.uv2Count;
	uv2s = other.uv2s;
	uv3Count = other.uv3Count;
	uv3s = other.uv3s;
	return *this;
}
void Mesh::render(Entity& entity)
{
	auto shader = addShader();
	shader->bind(*this);
	auto& scene = Registry::getScene(entity.INDEX_STACK);
	{
		if (entity.viewPointer)
			entity.viewPointer->updateMutex.lock();
		else
			scene.viewPointer->updateMutex.lock();
		scene.meshPreRender(*this);
		shader->setBlock("Model", *this, entity.getModelMatrix());
		shader->setBlock("View", *this, entity.viewPointer ? entity.viewPointer->matrix : scene.viewPointer->matrix);
		shader->setBlock("CameraPosition", *this, entity.viewPointer ? entity.viewPointer->position : scene.viewPointer->position, 16);
		if (entity.viewPointer)
			entity.viewPointer->updateMutex.unlock();
		else
			scene.viewPointer->updateMutex.unlock();
	}
	shader->setBlock("Projection", *this, entity.projectionPointer ? entity.projectionPointer->matrix : scene.projectionPointer->matrix);
	auto keyedTexturesSize = keyedTextures.size();
	auto keyedTexturesData = keyedTextures.data();
	for (size_t unit = 0; unit < keyedTexturesSize; ++unit)
		shader->setTexture(keyedTexturesData[unit].first, *this, *keyedTexturesData[unit].second, unit);
	drawVAO();
	shader->unbind();
}