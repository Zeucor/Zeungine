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
    m_transform_keys(entity_mat4_transform_registry::extract_transform_keys(Registry::getMesh(meshID).info.constants))
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
    for (auto& keyedTexture : mesh.info.keyedTextures)
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
    size_t batchID,
    Scene& scene,
    const OpaqueDrawList& opaqueDrawList,
    const TransparentDrawList& transparentDrawList,
    size_t& oldOpaqueHash,
    size_t& oldTransparentHash,
    shaders::Shader* shaderPointer,
    const std::unordered_map<std::string, glm::mat4>& transformOverrides,
    const std::unordered_map<std::string, std::function<void(Scene&, shaders::Shader&, Mesh&)>>& shaderSets
)
{
    auto& shaderBatches = bID_shaderBatches[batchID];
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
        std::vector<glm::vec4> positions;
        // std::vector<glm::vec4> normals;
        std::vector<glm::vec2> uv2s;
        std::vector<glm::vec3> uv3s;
        std::map<size_t, std::pair<size_t, size_t>> position_index_size_map;
        // std::map<size_t, std::pair<size_t, size_t>> normals_index_size_map;
        std::map<size_t, std::pair<size_t, size_t>> uv2s_index_size_map;
        std::map<size_t, std::pair<size_t, size_t>> uv3s_index_size_map;
        for (auto& meshID : shaderPair.second)
        {
            auto& mesh = Registry::getMesh(meshID);
            auto& batch = opaqueBatches.try_emplace(meshID, meshID).first->second;
            batch.update_transforms();
            for (auto& t : batch.mvp_transforms)
            {
                auto& tkey = t.first;
                auto& tvec = transforms[tkey];
                auto oIter = transformOverrides.find(tkey);
                if (oIter != transformOverrides.end())
                {
                    auto tsize = t.second.size();
                    tvec.insert(tvec.end(), tsize, oIter->second);
                }
                else
                {
                    tvec.insert(tvec.end(), t.second.begin(), t.second.end());
                }
            }
            int32_t vertex_offset = -1;
            // int32_t normal_offset = -1;
            uint32_t triangle_count = 0;
            auto indiceCount = mesh.indices.size();
            auto vertexCount = mesh.vertices.size();
            auto uv2Count = mesh.uv2s.size();
            auto uv3Count = mesh.uv3s.size();
            if (indiceCount)
            {
                auto& vertices = mesh.vertices;
                auto vertices_hash = zg::crypto::hashVector(vertices);
                auto pim_iter = position_index_size_map.find(vertices_hash);
                if (pim_iter == position_index_size_map.end())
                {
                    auto vertices_data = vertices.data();
                    auto& indices = mesh.indices;
                    auto indices_data = (glm::ivec3*)indices.data();
                    std::vector<glm::vec4> triangle_vertices;
                    triangle_vertices.reserve(indiceCount / 3);
                    for (size_t ii = 0; ii < indiceCount / 3; ++ii)
                    {
                        triangle_vertices.push_back(glm::vec4(vertices_data[indices_data[ii].x], 0));
                        triangle_vertices.push_back(glm::vec4(vertices_data[indices_data[ii].y], 0));
                        triangle_vertices.push_back(glm::vec4(vertices_data[indices_data[ii].z], 0));
                    }
                    vertex_offset = positions.size();
                    positions.insert(positions.end(), triangle_vertices.begin(), triangle_vertices.end());
                    triangle_count = triangle_vertices.size();
                    position_index_size_map[vertices_hash] = {vertex_offset, triangle_count};
                }
                else
                {
                    vertex_offset = pim_iter->second.first;
                    triangle_count = pim_iter->second.second;
                }
                // auto& mindices = mesh.indices;
                // std::vector<glm::vec3> mnormals;
                // mnormals.resize(triangle_count);
                // computeNormals(window.iRenderer->frontFace, mindices, vertices, mnormals);
                // auto mnormals_hash = zg::crypto::hashVector(mnormals);
                // auto nim_iter = normals_index_size_map.find(mnormals_hash);
                // if (nim_iter == normals_index_size_map.end())
                // {
                //     auto& indices = mesh.indices;
                //     auto normals_data = mnormals.data();
                //     auto indices_data = (glm::ivec3*)indices.data();
                //     std::vector<glm::vec4> triangle_normals;
                //     triangle_normals.reserve(indiceCount / 3);
                //     for (size_t ii = 0; ii < indiceCount / 3; ++ii)
                //     {
                //         triangle_normals.push_back(glm::vec4(normals_data[indices_data[ii].x], 0));
                //         triangle_normals.push_back(glm::vec4(normals_data[indices_data[ii].y], 0));
                //         triangle_normals.push_back(glm::vec4(normals_data[indices_data[ii].z], 0));
                //     }
                //     normal_offset = normals.size();
                //     normals.insert(normals.end(), triangle_normals.begin(), triangle_normals.end());
                //     normals_index_size_map[mnormals_hash] = {normal_offset, triangle_normals.size()};
                // }
                // else
                // {
                //     normal_offset = nim_iter->second.first;
                // }
            }
            for (auto& entityID : batch.entities)
            {
                int32_t uv2_offset = -1;
                int32_t uv3_offset = -1;
                auto& entity = Registry::getEntity(entityID);
                auto& material = entity.meshMaterial(meshID);
                auto material_index = find_material_index(material);
                if (material_index == -1)
                {
                    materials.push_back(material);
                    material_index = materials.size() -1;
                }
                if (uv2Count)
                {
                    auto& muv2s = mesh.uv2s;
                    auto muv2s_hash = zg::crypto::hashVector(muv2s);
                    auto u2m_iter = uv2s_index_size_map.find(muv2s_hash);
                    if (u2m_iter == uv2s_index_size_map.end())
                    {
                        uv2_offset = uv2s.size();
                        uv2s.insert(uv2s.end(), muv2s.begin(), muv2s.end());
                        uv2s_index_size_map[muv2s_hash] = {uv2_offset, muv2s.size()};
                    }
                    else
                    {
                        uv2_offset = u2m_iter->second.first;
                    }
                }
                if (uv3Count)
                {
                    auto& muv3s = mesh.uv3s;
                    auto muv3s_hash = zg::crypto::hashVector(muv3s);
                    auto u3m_iter = uv3s_index_size_map.find(muv3s_hash);
                    if (u3m_iter == uv3s_index_size_map.end())
                    {
                        uv3_offset = uv3s   .size();
                        uv3s.insert(uv3s    .end(), muv3s.begin(), muv3s.end());
                        uv3s_index_size_map[muv3s_hash] = {uv3_offset, muv3s.size()};
                    }
                    else
                    {
                        uv3_offset = u3m_iter->second.first;
                    }
                }
                entities[i++] = {
                    int32_t(mesh.info.shapeType),
                    material_index,
                    vertex_offset,
                    0,
                    uv2_offset,
                    uv3_offset,
                    mesh.info.meta_int,
                    entity.meta_float,
                    entity.meta_vec4
                };
                vertex_offset += triangle_count;
            }
            if (!triangle_count && mesh.info.shapeType != ShapeType::Mesh)
            {
                auto shape_count_iter = shapeVerticeCounts.find(mesh.info.shapeType);
                if (shape_count_iter != shapeVerticeCounts.end())
                {
                    triangle_count = shape_count_iter->second;
                }
            }
            uint32_t batchEntitiesSize = batch.entities.size();
            indirect_commands.push_back({
                triangle_count,
                batchEntitiesSize,
                0,  
                firstInstance
            });
            firstInstance += batchEntitiesSize;
            //
            // set uv/color/normal data
        }
        auto& projection = *scene.projectionPointer;
        shader.setBlock("CameraPosition", firstMesh, scene.viewPointer->position, 16);
        shader.setBlock("Viewport", firstMesh, glm::vec4(0, 0, window.windowWidth, window.windowHeight), 16);
        shader.setBlock("Time", firstMesh, scene.updateTime, 4);
		float nearFar[2] = {
			projection.nearPlane,
			projection.farPlane,
		};
		shader.setBlock("NearFarPlanes", firstMesh, nearFar, 8);
        for (auto& shaderSet : shaderSets)
        {
            auto& constant = shaderSet.first;
            auto shader_constants_end = shader.constants.end();
            auto c_iter = std::find(shader.constants.begin(), shader_constants_end, constant);
            if (c_iter != shader_constants_end)
            {
                shaderSet.second(scene, shader, firstMesh);
            }
        }
        auto entitiesSize = entities.size();
        auto materialsSize = materials.size();
        auto positionsSize = positions.size();
        // auto normalsSize = normals.size();
        auto uv2sSize = uv2s.size();
        auto uv3sSize = uv3s.size();
        if (!positionsSize)
        {
            positions.resize(1);
            positionsSize = positions.size();
        }
        // if (!normalsSize)
        // {
        //     normals.resize(1);
        //     normalsSize = normals.size();
        // }
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
        shader.setSSBO("MeshPositions", firstMesh, positions.data(), sizeof(glm::vec4) * positionsSize);
        // shader.setSSBO("MeshNormals", firstMesh, normals.data(), sizeof(glm::vec4) * normalsSize);
        shader.setSSBO("EntityUV2s", firstMesh, uv2s.data(), sizeof(glm::vec2) * uv2sSize);
        shader.setSSBO("EntityUV3s", firstMesh, uv3s.data(), sizeof(glm::vec3) * uv3sSize);
        for (auto& transform_pair : transforms)
        {
            auto& vector = transform_pair.second;
            auto& tkey = transform_pair.first;
            auto tsize = transform_pair.second.size();
            auto vector_data = vector.data();
            auto ikey = "Instance" + tkey + "s";
            shader.setSSBO(ikey, firstMesh, vector_data, tsize * sizeof(glm::mat4));
            auto inverse_vector = vector;
            auto inverse_vector_data = inverse_vector.data();
            auto iikey = "InverseInstance" + tkey + "s";
            auto oIter = transformOverrides.find(iikey);
            glm::mat4 io;
            bool o = false;
            if (oIter != transformOverrides.end())
            {
                io = oIter->second;
                o = true;
            }
            for (auto& t : inverse_vector)
            {
                t = o ? io : glm::inverse(t);
            }
            shader.setSSBO(iikey, firstMesh, inverse_vector_data, tsize * sizeof(glm::mat4));
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
        for (auto& keyedTexture : mesh.info.keyedTextures)
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
        for (auto& keyedTexture : mesh.info.keyedTextures)
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