#include <zg/Mesh.hpp>
#include <zg/Window.hpp>
using namespace zg;
Mesh::Mesh(const MeshCreateInfo& info, Entity& entity):
	MeshInfo(((MeshCreateInfo&)info).entity_id_mesh_infos[entity.ID]),
    VAO(
        entity.INDEX_STACK,
        info.constants,
        getIndicesSize(info.shapeType),
        getVerticesSize(info.shapeType)
    ),
	info(info)
{
	hash = info.hash;
	// auto meshInfoConstantsEnd = info.constants.end();
	// if (std::find(info.constants.begin(), meshInfoConstantsEnd, "Shape") != meshInfoConstantsEnd)
	// 	return;
	// auto& window = Registry::GetSingleton().getWindow(entity.INDEX_STACK);
	// if (!vertices.size())
    //     return;
    // updateIndices(indices);
    // if (!colors.empty())
    //     updateElements("Color", colors);
    // bool flipUVs = (window.iRenderer->renderer == RENDERER_VULKAN || window.iRenderer->renderer == RENDERER_METAL);
    // if (!uv2s.empty())
    // {
    //     flipUVsY(uv2s);
    //     updateElements("UV2", uv2s);
    // }
    // if (!uv3s.empty())
    // {
    //     flipUVsY(uv3s);
    //     updateElements("UV3", uv3s);
    // }
    // updateElements("Position", vertices);
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
uint32_t Mesh::getIndicesSize(ShapeType shapeType)
{
	switch (shapeType)
	{
	case ShapeType::PlaneXY_Center:
	case ShapeType::PlaneXZ_Center:
	case ShapeType::PlaneYZ_Center:
	case ShapeType::PlaneXY_BottomLeft:
	case ShapeType::SDF:
		return 6;
	case ShapeType::Box:
		return 36;
	case ShapeType::Mesh:
		return indices.size();
	default: return 0;
	}
}
uint32_t Mesh::getVerticesSize(ShapeType shapeType)
{
	switch (shapeType)
	{
	case ShapeType::PlaneXY_Center:
	case ShapeType::PlaneXZ_Center:
	case ShapeType::PlaneYZ_Center:
	case ShapeType::PlaneXY_BottomLeft:
	case ShapeType::SDF:
		return 6;
	case ShapeType::Box:
		return 36;
	case ShapeType::Mesh:
		return indices.size();
	default: return 0;
	}
}
void Mesh::render(Entity& entity)
{
	uid = entity.ID;
	auto& window = Registry::GetSingleton().getWindow(entity.INDEX_STACK);
	if (window.iRenderer->summingDraw)
		window.summingDrawCount += 1;
	auto shader = addShader();
	shader->bind(*this);
	size_t ID_index = -1;
	for (auto& meshID : entity.meshIDs)
	{
		ID_index++;
		if (meshID == ID)
			break;
	}
	auto& meshInfo = entity.meshInfos[ID_index];
	if (window.iRenderer->summingDraw)
		window.summingTriangleCount += getIndicesSize(meshInfo.shapeType) / 3;
	auto& scene = Registry::GetSingleton().getScene(entity.INDEX_STACK);
	{
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
		auto& projection = (entity.projectionPointer ? *entity.projectionPointer : *scene.projectionPointer);
		auto& projection_matrix = (projection.matrix);
		shader->setSSBO("InstanceProjections", *this, &projection_matrix, sizeof(glm::mat4));
		auto inverse_projection_matrix = glm::inverse(projection_matrix);
		shader->setSSBO("InverseInstanceProjections", *this, &inverse_projection_matrix, sizeof(glm::mat4));
		shader->setBlock("Viewport", *this, *window.viewport, 16);
		shader->setBlock("Time", *this, scene.updateTime, 4);
		float nearFar[2] = {
			projection.nearPlane,
			projection.farPlane,
		};
		shader->setBlock("NearFarPlanes", *this, nearFar, 8);
		shader->setBlock("CameraPosition", *this, camera_position, 16);
        GLEntity gl_entity{
			.shape_type = int32_t(meshInfo.shapeType),
			.material_index = 0,
			.vertex_offset = vertices.empty() ? -1 : 0,
			.padding = 0,
			.uv2_offset = uv2s.empty() ? -1 : 0,
			.uv3_offset = uv3s.empty() ? -1 : 0,
			.meta_int = meshInfo.meta_int,
			.meta_float = entity.meta_float,
			.meta_vec4 = entity.meta_vec4
		};
        shader->setSSBO("Entities", *this, &gl_entity, sizeof(GLEntity) * 1);
		auto& material = meshInfo.material;
        shader->setSSBO("Materials", *this, &material, sizeof(Material) * 1);
		if (vertices.size())
		{
			std::vector<glm::vec4> vertices4;
			vertices4.reserve(vertices.size());
			for (auto& v : vertices)
				vertices4.push_back(glm::vec4(v, 0.f));
			shader->setSSBO("MeshPositions", *this, vertices4.data(), sizeof(glm::vec4) * vertices4.size());
		}
		else
		{
			glm::vec4 vec(0);
			shader->setSSBO("MeshPositions", *this, &vec, sizeof(glm::vec4) * 1);
		}
		if (uv2s.size())
		{
			shader->setSSBO("EntityUV2s", *this, uv2s.data(), sizeof(glm::vec2) * uv2s.size());
		}
		else
		{
			glm::vec2 vec(0);
			shader->setSSBO("EntityUV2s", *this, &vec, sizeof(glm::vec2) * 1);
		}
		if (uv3s.size())
		{
			std::vector<glm::vec4> uvs4;
			uvs4.reserve(vertices.size());
			for (auto& uv : uv3s)
				uvs4.push_back(glm::vec4(uv, 0.f));
			shader->setSSBO("EntityUV3s", *this, uvs4.data(), sizeof(glm::vec4) * uvs4.size());
		}
		else
		{
			glm::vec4 vec(0);
			shader->setSSBO("EntityUV3s", *this, &vec, sizeof(glm::vec4) * 1);
		}
		auto constants_begin = vaoConstants.begin();
		auto constants_end = vaoConstants.end();
		for (auto& pairPair : entity.runtimeConstantValueShaderSetters)
		{
			auto& key = pairPair.first;
			auto& pair = pairPair.second;
			if (std::find(constants_begin, constants_end, key) == constants_end)
				continue;
			pair.second(*this, *shader, *pair.first);
		}
	}
	auto keyedTexturesSize = meshInfo.keyedTextures.size();
	auto keyedTexturesData = meshInfo.keyedTextures.data();
	for (size_t unit = 0; unit < keyedTexturesSize; ++unit)
		shader->setTexture(keyedTexturesData[unit].first, *this, *keyedTexturesData[unit].second, unit);
	drawVAO(shader);
	shader->unbind();
}