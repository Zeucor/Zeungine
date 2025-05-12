#include <zg/InstancedDraw.hpp>
#include <zg/Window.hpp>
#include <zg/crypto/vector.hpp>
using namespace zg;
bool entity_mat4_transform_registry::initialized = ([](){
    if (!entity_mat4_transform_registry::set_getter_function("Model", [](Entity& entity) { return entity.getModelMatrix(); })) return false;
    if (!entity_mat4_transform_registry::set_getter_function("InverseModel", [](Entity& entity) { return glm::inverse(entity.getModelMatrix()); })) return false;
    if (!entity_mat4_transform_registry::set_getter_function("Projection", [](Entity& entity) { return entity.projectionPointer ? entity.projectionPointer->matrix : Registry::getScene(entity.INDEX_STACK).projectionPointer->matrix; })) return false;
    if (!entity_mat4_transform_registry::set_getter_function("InverseProjection", [](Entity& entity) { return glm::inverse(entity.projectionPointer ? entity.projectionPointer->matrix : Registry::getScene(entity.INDEX_STACK).projectionPointer->matrix); })) return false;
    if (!entity_mat4_transform_registry::set_getter_function("View", [](Entity& entity) { return entity.viewPointer ? entity.viewPointer->matrix : Registry::getScene(entity.INDEX_STACK).viewPointer->matrix; })) return false;
    if (!entity_mat4_transform_registry::set_getter_function("InverseView", [](Entity& entity) { return glm::inverse(entity.viewPointer ? entity.viewPointer->matrix : Registry::getScene(entity.INDEX_STACK).viewPointer->matrix); })) return false;
    if (!entity_mat4_transform_registry::set_getter_function("InverseView", [](Entity& entity) { return glm::inverse(entity.viewPointer ? entity.viewPointer->matrix : Registry::getScene(entity.INDEX_STACK).viewPointer->matrix); })) return false;
    return true;
})();
InstancedDraw::InstanceBatch::InstanceBatch(size_t meshID):
    m_meshID(meshID),
    m_transform_keys(entity_mat4_transform_registry::extract_transform_keys(Registry::getMesh(meshID).constants))
{
    if (!entity_mat4_transform_registry::initialized)
        throw std::runtime_error("entity_mat4_tfm_rgt is false");
}
void InstancedDraw::InstanceBatch::addEntity(const size_t entity_ID) 
{
    entities.emplace(entity_ID);
}
void InstancedDraw::InstanceBatch::removeEntity(const size_t entity_ID)
{
    entities.erase(entity_ID);
}
void InstancedDraw::InstanceBatch::draw(Scene& scene, shaders::Shader* shaderPointer)
{
    auto& mesh = Registry::getMesh(m_meshID);
    auto shader = mesh.addShader(shaderPointer);
    auto& shaderRef = *shader;
    shaderRef.bind(mesh);
    shaderRef.setBlock("CameraPosition", mesh, scene.viewPointer->position, 16);
    for (auto& transform_pair : mvp_transforms)
    {
        auto transform_array_data = transform_pair.second.data();
        shaderRef.setSSBO("Instance" + transform_pair.first + "s", mesh, transform_array_data, transform_pair.second.size() * sizeof(glm::mat4));
    }
    uint32_t unit = 0;
    for (auto& keyedTexture : mesh.keyedTextures)
        shaderRef.setTexture(keyedTexture.first, mesh, *keyedTexture.second, unit++);
    mesh.drawVAOInstanced(entities.size());
    shaderRef.unbind();
}
void InstancedDraw::InstanceBatch::update_transforms()
{
    for (auto& transform_key : m_transform_keys)
    {
        auto& mvp_transforms_t_k = mvp_transforms[transform_key];
        auto entitiesSize = entities.size();
        auto entitiesIter = entities.begin();
        mvp_transforms_t_k.resize(entitiesSize);
        auto mvp_transforms_t_k_data = mvp_transforms_t_k.data();
        for (size_t index = 0; index < entitiesSize; ++index)
        {
            auto& entityID = *entitiesIter;
            mvp_transforms_t_k_data[index] = entity_mat4_transform_registry::get_value(transform_key, Registry::getEntity(entityID));
            ++entitiesIter;
        }
    }
}
void InstancedDraw::draw(
    Scene& scene,
    const OpaqueDrawList& opaqueDrawList,
    const TransparentDrawList& transparentDrawList,
    size_t& oldOpaqueHash,
    size_t& oldTransparentHash,
    shaders::Shader* shaderPointer
)
{
    for (auto& pair : opaqueDrawList)
        opaqueBatches.try_emplace(pair.second->ID, pair.second->ID).first->second.addEntity(pair.first->ID);
    for (auto& batch : opaqueBatches)
    {
        if (!batch.second.entities.size())
            continue;
        batch.second.update_transforms();
        batch.second.draw(scene, shaderPointer);
    }
    for (auto& pair : transparentDrawList)
        transparentBatches.try_emplace(pair.second->ID, pair.second->ID).first->second.addEntity(pair.first->ID);
    for (auto& batch : transparentBatches)
    {
        if (!batch.second.entities.size())
            return;
        batch.second.update_transforms();
    }
    size_t tbatchsize = transparentBatches.size();
    // std::vector<size_t> tbatchsorted(tbatchsize, 0);
    // for now we'll use an unordered map, a sorted vector may be faster
    std::map<float, std::vector<size_t>> distanceBatches;
    auto tbatchiter = transparentBatches.begin();
    auto& cameraPosition = scene.viewPointer->position;
    for (size_t count = 1; count <= tbatchsize; ++count)
    {
        float tbatchcdist = (std::numeric_limits<float>::max)();
        for (auto& t : tbatchiter->second.mvp_transforms["Model"])
        {
            auto tpos = glm::vec3(t[3][0], t[3][1], t[3][2]);
            float dist = glm::distance(tpos, cameraPosition);
            tbatchcdist = (std::min)(dist, tbatchcdist);
        }
        distanceBatches[tbatchcdist].push_back(tbatchiter->second.m_meshID);
        tbatchiter++;
    }
    auto end = distanceBatches.rend();
    for (auto iter = distanceBatches.rbegin(); iter != end; ++iter)
        for (auto& meshID : iter->second)
           transparentBatches.try_emplace(meshID, meshID).first->second.draw(scene, shaderPointer);
}
void InstancedDraw::drawMulti(
    Scene& scene,
    const OpaqueDrawList& opaqueDrawList,
    const TransparentDrawList& transparentDrawList,
    size_t& oldOpaqueHash,
    size_t& oldTransparentHash,
    shaders::Shader* shaderPointer
)
{
    auto& window = Registry::getWindow(scene.INDEX_STACK);
    for (auto& pair : opaqueDrawList)
        opaqueBatches.try_emplace(pair.second->ID, pair.second->ID).first->second.addEntity(pair.first->ID);
    for (auto& batch : opaqueBatches)
        shaderBatches[Registry::getMesh(batch.second.m_meshID).addShader(shaderPointer)].insert(batch.second.m_meshID);
    for (auto& shaderPair : shaderBatches)
    {
        auto& shader = *shaderPair.first;
        auto meshCount = shaderPair.second.size();
        std::vector<DrawIndirectCommand> indirect_commands;
        indirect_commands.reserve(meshCount);
        uint32_t totalShapes = 0;
        for (auto& meshID : shaderPair.second)
        {
            totalShapes += opaqueBatches.try_emplace(meshID, meshID).first->second.entities.size();  
        }
        std::vector<GLEntity> entities;
        std::unordered_map<std::string, std::vector<glm::mat4>> transforms;
        std::vector<Material> materials;
        auto find_material_index = [&](auto& material) -> int32_t {
            auto iter = std::find_if(materials.begin(), materials.end(), [&](auto& amaterial){
                return amaterial.type == material.type &&
                    amaterial.albedo == material.albedo;
            });
            if (iter != materials.end())
            {
                return (iter - materials.begin());
            }
            return -1;
        };
        entities.resize(totalShapes);
        auto& firstMeshID = *shaderPair.second.begin();
        auto& firstMesh = Registry::getMesh(firstMeshID);
        shader.bind(firstMesh);
        auto& firstBatch = opaqueBatches.try_emplace(firstMeshID, firstMeshID).first->second;
        for (auto& tk : firstBatch.m_transform_keys)
        {
            transforms[tk].reserve(totalShapes);
        }
        uint32_t firstInstance = 0;
        uint32_t i = 0;
        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> uv2s;
        std::vector<glm::vec3> uv3s;
        std::unordered_map<size_t, size_t> position_index_map;
        std::unordered_map<size_t, size_t> normals_index_map;
        std::unordered_map<size_t, size_t> uv2_index_map;
        std::unordered_map<size_t, size_t> uv3_index_map;
        for (auto& meshID : shaderPair.second)
        {
            auto& mesh = Registry::getMesh(meshID);
            auto& batch = opaqueBatches.try_emplace(meshID, meshID).first->second;
            batch.update_transforms();
            for (auto& t : batch.mvp_transforms)
            {
                auto& tvec = transforms[t.first];
                tvec.insert(tvec.end(), t.second.begin(), t.second.end());
            }
            for (auto& entityID : batch.entities)
            {
                int32_t vertex_offset = -1;
                int32_t normal_offset = -1;
                int32_t uv2_offset = -1;
                int32_t uv3_offset = -1;
                auto& entity = Registry::getEntity(entityID);
                auto material = entity.meshMaterial(meshID);
                auto material_index = find_material_index(material);
                if (material_index == -1)
                {
                    materials.push_back(material);
                    material_index = materials.size() -1;
                }
                auto vertexCount = mesh.vertexCount ? mesh.vertexCount(entity) : 0;
                auto uv2Count = mesh.uv2Count ? mesh.uv2Count(entity) : 0;
                auto uv3Count = mesh.uv3Count ? mesh.uv3Count(entity) : 0;
                if (vertexCount)
                {
                    auto vertices = mesh.vertices(entity);
                    auto vertices_hash = zg::crypto::hashVector(vertices);
                    auto pim_iter = position_index_map.find(vertices_hash);
                    if (pim_iter == position_index_map.end())
                    {
                        vertex_offset = positions.size();
                        positions.insert(positions.end(), vertices.begin(), vertices.end());
                        position_index_map.emplace(vertices_hash, vertex_offset); 
                    }
                    else
                    {
                        vertex_offset = pim_iter->second;
                    }
                    auto _indices_ = mesh.indices(entity);
                    std::vector<glm::vec3> mnormals;
                    computeNormals(window.iRenderer->frontFace, _indices_, vertices, mnormals);
                    auto mnormals_hash = zg::crypto::hashVector(mnormals);
                    auto nim_iter = normals_index_map.find(mnormals_hash);
                    if (pim_iter == position_index_map.end())
                    {
                        normal_offset = normals.size();
                        normals.insert(normals.end(), mnormals.begin(), mnormals.end());
                        position_index_map.emplace(mnormals_hash, normal_offset); 
                    }
                    else
                    {
                        normal_offset = pim_iter->second;
                    }
                }
                if (uv2Count)
                {
                    auto muv2s = mesh.uv2s(entity);
                    auto muv2s_hash = zg::crypto::hashVector(muv2s);
                    auto u2m_iter = uv2_index_map.find(muv2s_hash);
                    if (u2m_iter == position_index_map.end())
                    {
                        uv2_offset = uv2s.size();
                        uv2s.insert(uv2s.end(), muv2s.begin(), muv2s.end());
                        position_index_map.emplace(muv2s_hash, uv2_offset); 
                    }
                    else
                    {
                        uv2_offset = u2m_iter->second;
                    }
                }
                if (uv3Count)
                {
                    auto muv3s = mesh.uv3s(entity);
                    auto muv3s_hash = zg::crypto::hashVector(muv3s);
                    auto u3m_iter = uv3_index_map.find(muv3s_hash);
                    if (u3m_iter == position_index_map.end())
                    {
                        uv3_offset = uv3s   .size();
                        uv3s    .insert(uv3s    .end(), muv3s.begin(), muv3s.end());
                        position_index_map.emplace(muv3s_hash, uv3_offset); 
                    }
                    else
                    {
                        uv3_offset = u3m_iter->second;
                    }
                }
                entities[i++] = {
                    int32_t(mesh.shapeType),
                    material_index,
                    vertex_offset,
                    normal_offset,
                    uv2_offset,
                    uv3_offset
                };
                vertex_offset += vertexCount;
            }
            uint32_t vertexCount;
            auto shape_count_iter = shapeVerticeCounts.find(mesh.shapeType);
            if (shape_count_iter != shapeVerticeCounts.end())
            {
                vertexCount = shape_count_iter->second;
            }
            else
            {
                vertexCount = mesh.m_vertexCount;
            }
            uint32_t batchEntitiesSize = batch.entities.size();
            indirect_commands.push_back({
                vertexCount,
                batchEntitiesSize,
                0,
                firstInstance
            });
            firstInstance += batchEntitiesSize;
            //
            // set uv/color/normal data
        }
        shader.setBlock("CameraPosition", firstMesh, scene.viewPointer->position, 16);
        auto entitiesSize = entities.size();
        auto materialsSize = materials.size();
        auto positionsSize = positions.size();
        auto normalsSize = normals.size();
        auto uv2sSize = uv2s.size();
        auto uv3sSize = uv3s.size();
        if (!positionsSize)
        {
            positions.resize(1);
            positionsSize = positions.size();
        }
        if (!normalsSize)
        {
            normals.resize(1);
            normalsSize = normals.size();
        }
        if (!uv2sSize)
        {
            uv2s.resize(1);
            uv2sSize = uv2s.size();
        }
        if (!uv3sSize)
        {
            uv3s.resize(1);
            uv3sSize = uv3s.size();
        }
        shader.setSSBO("Entities", firstMesh, entities.data(), sizeof(GLEntity) * entitiesSize);
        shader.setSSBO("Materials", firstMesh, materials.data(), sizeof(Material) * materialsSize);
        shader.setSSBO("MeshPositions", firstMesh, positions.data(), sizeof(glm::vec3) * positionsSize);
        shader.setSSBO("MeshNormals", firstMesh, normals.data(), sizeof(glm::vec3) * normalsSize);
        shader.setSSBO("EntityUV2s", firstMesh, uv2s.data(), sizeof(glm::vec2) * uv2sSize);
        shader.setSSBO("EntityUV3s", firstMesh, uv3s.data(), sizeof(glm::vec3) * uv3sSize);
        for (auto& transform_pair : transforms)
        {
            auto& vector = transform_pair.second;
            auto& tkey = transform_pair.first;
            auto tsize = transform_pair.second.size();
            auto vector_data = vector.data();
            shader.setSSBO("Instance" + tkey + "s", firstMesh, vector_data, tsize * sizeof(glm::mat4));
            auto inverse_vector = vector;
            auto inverse_vector_data = inverse_vector.data();
            for (auto& t : inverse_vector)
            {
                t = glm::inverse(t);
            }
            shader.setSSBO("InverseInstance" + tkey + "s", firstMesh, inverse_vector_data, tsize * sizeof(glm::mat4));
        }
        window.iRenderer->drawMultiInstanced(&shader, firstMesh, indirect_commands);
        shader.unbind();
    }
}
void InstancedDraw::addEntity(const Entity& entity)
{
    bool isTransparent = entity.isTransparent;
    if (isTransparent)
        goto _add;
    for (auto& meshID : entity.meshIDs)
    {
        auto& mesh = Registry::getMesh(meshID);
        for (auto& keyedTexture : mesh.keyedTextures)
        {
            if (keyedTexture.second->isTransparent)
            {
                isTransparent = true;
                break;
            }
        }
        if (isTransparent)
            break;
    }
_add:
    if (!isTransparent)
        for (auto& meshID : entity.meshIDs)
            opaqueBatches.try_emplace(meshID, meshID).first->second.addEntity(entity.ID);
    else
        for (auto& meshID : entity.meshIDs)
            transparentBatches.try_emplace(meshID, meshID).first->second.addEntity(entity.ID);
}
void InstancedDraw::removeEntity(const Entity& entity)
{
    bool isTransparent = entity.isTransparent;
    if (isTransparent)
        goto _add;
    for (auto& meshID : entity.meshIDs)
    {
        auto& mesh = Registry::getMesh(meshID);
        for (auto& keyedTexture : mesh.keyedTextures)
        {
            if (keyedTexture.second->isTransparent)
            {
                isTransparent = true;
                break;
            }
        }
        if (isTransparent)
            break;
    }
_add:
    for (auto& meshID : entity.meshIDs)
    {
        if (!isTransparent)
        {
            auto mesh_iter = opaqueBatches.find(meshID);
            if (mesh_iter == opaqueBatches.end())
                continue;
            mesh_iter->second.removeEntity(entity.ID);
            if (!mesh_iter->second.entities.size())
                opaqueBatches.erase(mesh_iter);
        }
        else
        {
            auto mesh_iter = transparentBatches.find(meshID);
            if (mesh_iter == transparentBatches.end())
                continue;
            mesh_iter->second.removeEntity(entity.ID);
            if (!mesh_iter->second.entities.size())
                transparentBatches.erase(mesh_iter);
        }
    }
}
void InstancedDraw::removeMesh(size_t meshID)
{
    auto iter = opaqueBatches.find(meshID);
    if (iter != opaqueBatches.end())
    {
        opaqueBatches.erase(iter);
    }
    iter = transparentBatches.find(meshID);
    if (iter != transparentBatches.end())
    {
        transparentBatches.erase(iter);
    }
}