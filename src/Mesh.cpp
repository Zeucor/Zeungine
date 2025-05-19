#include <zg/Mesh.hpp>
#include <zg/Window.hpp>
using namespace zg;
Mesh::Mesh(const MeshCreateInfo& info, Entity& entity):
	MeshInfo(((MeshCreateInfo&)info).entity_id_mesh_infos[entity.ID]),
    VAO(
        entity.INDEX_STACK,
        info.constants,
        indices.size(),
        vertices.size()
    ),
	info(info)
{
	hash = info.hash;
	auto meshInfoConstantsEnd = info.constants.end();
	if (std::find(info.constants.begin(), meshInfoConstantsEnd, "Shape") != meshInfoConstantsEnd)
		return;
	auto& window = Registry::GetSingleton().getWindow(entity.INDEX_STACK);
	if (!indices.size() || !vertices.size())
        return;
    updateIndices(indices);
    if (!colors.empty())
        updateElements("Color", colors);
    bool flipUVs = (window.iRenderer->renderer == RENDERER_VULKAN || window.iRenderer->renderer == RENDERER_METAL);
    if (!uv2s.empty())
    {
        flipUVsY(uv2s);
        updateElements("UV2", uv2s);
    }
    if (!uv3s.empty())
    {
        flipUVsY(uv3s);
        updateElements("UV3", uv3s);
    }
    updateElements("Position", vertices);
}
Mesh::Mesh(const Mesh& other):
	MeshInfo(other),
    VAO(other),
	ID(other.ID),
	INDEX(other.INDEX),
	INDEX_STACK(other.INDEX_STACK),
	info(other.info)
{}
Mesh& Mesh::operator=(const Mesh& other)
{
	((MeshInfo&)*this) = other;
	((vaos::VAO&)*this) = other;
	ID = other.ID;
	INDEX = other.INDEX;
	INDEX_STACK = other.INDEX_STACK;
	info = other.info;
	return *this;
}
void Mesh::render(Entity& entity)
{
	auto& window = Registry::GetSingleton().getWindow(entity.INDEX_STACK);
	auto shader = addShader();
	shader->bind(*this);
	auto& scene = Registry::GetSingleton().getScene(entity.INDEX_STACK);
	{
		if (entity.viewPointer)
			entity.viewPointer->updateMutex.lock();
		else
			scene.viewPointer->updateMutex.lock();
		scene.meshPreRender(*this);
		auto& model_matrix = entity.getModelMatrix();
		auto& view_matrix = (entity.viewPointer ? entity.viewPointer->matrix : scene.viewPointer->matrix);
		auto& camera_position = entity.viewPointer ? entity.viewPointer->position : scene.viewPointer->position;
		shader->setSSBO("InstanceModels", *this, &model_matrix, sizeof(glm::mat4));
		auto inverse_model_matrix = glm::inverse(model_matrix);
		shader->setSSBO("InverseInstanceModels", *this, &inverse_model_matrix, sizeof(glm::mat4));
		shader->setSSBO("InstanceViews", *this, &view_matrix, sizeof(glm::mat4));
		auto inverse_view_matrix = glm::inverse(view_matrix);
		shader->setSSBO("InverseInstanceViews", *this, &inverse_view_matrix, sizeof(glm::mat4));
		auto& projection = (entity.projectionPointer ? entity.projectionPointer : scene.projectionPointer);
		auto& projection_matrix = (projection->matrix);
		shader->setSSBO("InstanceProjections", *this, &projection_matrix, sizeof(glm::mat4));
		auto inverse_projection_matrix = glm::inverse(projection_matrix);
		shader->setSSBO("InverseInstanceProjections", *this, &inverse_projection_matrix, sizeof(glm::mat4));
		shader->setBlock("Viewport", *this, window.viewport, 16);
		shader->setBlock("Time", *this, scene.updateTime, 4);
		float nearFar[2] = {
			projection->nearPlane,
			projection->farPlane,
		};
		shader->setBlock("NearFarPlanes", *this, nearFar, 8);
		shader->setBlock("CameraPosition", *this, camera_position, 16);
		if (entity.viewPointer)
			entity.viewPointer->updateMutex.unlock();
		else
			scene.viewPointer->updateMutex.unlock();
	}
	auto keyedTexturesSize = info.keyedTextures.size();
	auto keyedTexturesData = info.keyedTextures.data();
	for (size_t unit = 0; unit < keyedTexturesSize; ++unit)
		shader->setTexture(keyedTexturesData[unit].first, *this, *keyedTexturesData[unit].second, unit);
	drawVAO();
	shader->unbind();
}