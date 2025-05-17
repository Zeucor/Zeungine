#include <zg/entities/Model.hpp>
#include <zg/utilities.hpp>
#include <zg/textures/Texture.hpp>
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#define STB_IMAGE_RESIZE_STATIC
#include "stb_image.h"
#include "stb_image_resize2.h"
#include <optional>
using namespace zg;
size_t totalToLoad = 0;
size_t totalLoaded = 0;
const aiScene* importSceneFromFile(interfaces::IFile& modelFile, Assimp::Importer &importer)
{
	auto bytes = modelFile.toBytes();
	auto modelFileSize = modelFile.size();
	const aiScene *scene = importer.ReadFileFromMemory(bytes.get(), modelFileSize, aiProcess_Triangulate | aiProcessPreset_TargetRealtime_Fast | aiProcess_FlipUVs | aiProcess_CalcTangentSpace, nullptr);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cerr << importer.GetErrorString() << std::endl;
		return nullptr;
	}

	return scene;
}
void decomposeMatrix(const glm::mat4 &matrix, glm::vec3 &position, glm::quat &rotation, glm::vec3 &scale)
{
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::decompose(matrix, scale, rotation, position, skew, perspective);
}
std::shared_ptr<uint8_t> textureBytesFromMemory(uint8_t *data, int32_t length, int32_t &textureWidth, int32_t &textureHeight)
{
	int nrChannels;
	uint8_t *imageData = stbi_load_from_memory(data, length, &textureWidth, &textureHeight, &nrChannels, 4);
	if (!imageData)
	{
		throw std::runtime_error("Failed to load texture from memory.");
	}
	auto resizedBytes = stbir_resize_uint8_srgb(imageData, textureWidth, textureHeight, 0, 0, textureWidth / 4.f, textureHeight / 4.f, 0, STBIR_RGBA);
	if (!resizedBytes)
	{
		throw std::runtime_error("Failed to resize image");
	}
    textureWidth /= 4.f;
    textureHeight /= 4.f;
    stbi_image_free(imageData);
	return std::shared_ptr<uint8_t>(resizedBytes, [](auto pointer) {
        stbi_image_free(pointer);
    });
}
std::vector<std::shared_ptr<textures::Texture>> loadMaterialTextures(
    const aiScene *aiscene,
    aiMaterial *material,
    const aiTextureType &type,
    const std::string &typeName,
    IRenderer* iRenderer
)
{
	std::vector<std::shared_ptr<textures::Texture>> textures;
	for (uint32_t i = 0; i < material->GetTextureCount(type); i++)
	{
		aiString str;
		material->GetTexture(type, i, &str);
		std::shared_ptr<uint8_t> textureBytes;
		int32_t textureWidth = 0, textureHeight = 0;
		// Check if the texture is embedded
		if (str.data[0] == '*')
		{
			uint32_t textureIndex = atoi(&str.data[1]);
			aiTexture *aiTex = aiscene->mTextures[textureIndex];
			if (aiTex->mHeight == 0)
			{
				// The embedded texture is compressed (e.g., PNG or JPG in memory)
				textureBytes = textureBytesFromMemory((uint8_t *)aiTex->pcData, aiTex->mWidth, textureWidth, textureHeight);
			}
			else
			{
				// The embedded texture is uncompressed raw data
				throw std::runtime_error("A embedded texture is uncompressed raw data, we currently only support compressed textures such as PNG or JPG");
			}
		}
		else
		{
			// External texture
			throw std::runtime_error("A texture is an external texture, we currently only support compressed embedded textures such as PNG or JPG");
		}
		textures.push_back(std::make_shared<textures::Texture>(
            iRenderer, glm::ivec4(textureWidth, textureHeight, 1, 0), (const void*)textureBytes.get(), textures::Texture::Format::RGBA8,
            textures::Texture::Type::UnsignedByte, textures::Texture::FilterType::Linear,
										 false, textures::Texture::Multisampling::x1));
	}
	return textures;
}
void processMesh(
    const aiScene *aiscene,
    aiMesh *mesh,
    zg::FRONTFACE frontFace,
    zg::MeshCreateInfo& info,
    IRenderer* iRenderer,
	const shaders::RuntimeConstants& constants
)
{
	glm::vec4 color;
	bool hasColor = false;
    std::vector<std::pair<std::string, std::shared_ptr<textures::Texture>>> keyedTextures;
	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial *material = aiscene->mMaterials[mesh->mMaterialIndex];
		// Load base color (albedo) texture
		std::vector<std::shared_ptr<textures::Texture>> baseColorMaps = loadMaterialTextures(aiscene, material, aiTextureType_BASE_COLOR, "baseColor", iRenderer);
        for (auto& texture : baseColorMaps)
        {
            keyedTextures.push_back({"ColorTexture", texture});
        }
		// Load diffuse textures
		std::vector<std::shared_ptr<textures::Texture>> diffuseMaps;
		if (!baseColorMaps.size())
		{
			diffuseMaps = loadMaterialTextures(aiscene, material, aiTextureType_DIFFUSE, "diffuse", iRenderer);
            for (auto& texture : diffuseMaps)
            {
                keyedTextures.push_back({"ColorTexture", texture});
            }
		}
	// 	// Load specular textures
	// 	std::vector<std::shared_ptr<textures::Texture>> specularMaps = loadMaterialTextures(aiscene, material, aiTextureType_SPECULAR, "specular");
	// 	textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	// 	// Load normal maps
	// 	std::vector<std::shared_ptr<textures::Texture>> normalMaps = loadMaterialTextures(aiscene, material, aiTextureType_NORMALS, "normal");
	// 	textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
	// 	// Load height maps
	// 	std::vector<std::shared_ptr<textures::Texture>> heightMaps = loadMaterialTextures(aiscene, material, aiTextureType_HEIGHT, "height");
	// 	textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
	// 	// Load ambient occlusion maps
	// 	std::vector<std::shared_ptr<textures::Texture>> aoMaps = loadMaterialTextures(aiscene, material, aiTextureType_AMBIENT_OCCLUSION, "ambientOcclusion");
	// 	textures.insert(textures.end(), aoMaps.begin(), aoMaps.end());
	// 	//
	// 	if (!baseColorMaps.size() && !diffuseMaps.size())
	// 	{
	// 		aiColor4D aicolor;
	// 		if (AI_SUCCESS == aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &aicolor))
	// 		{
	// 			hasColor = true;
	// 			aiColor4D_to_glmvec4(aicolor, color);
	// 		}
	// 	}
	}
	
	std::vector<glm::vec3> vertices;
	// std::vector<glm::vec3> normals;
	std::vector<glm::vec4> colors;
	std::vector<glm::vec2> uv2s;
	std::vector<uint32_t> indices;
	auto meshHasNormals = mesh->HasNormals();
	//
	for (uint32_t i = 0; i < mesh->mNumVertices; i++)
	{
		// Positions
		vertices.push_back({mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z});
		// Normals
		// if (meshHasNormals)
		// {
		// 	normals.push_back({mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z});
		// }
		// Colors
		if (hasColor)
		{
			colors.push_back(color);
		}
		// Texture coordinates
		if (!hasColor && mesh->mTextureCoords[0])
		{
			// for (size_t j = 0; j < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++j)
			// {
			// 	auto tc = mesh->mTextureCoords[j];
			// 	if (tc && (tc[i].x > 1 || tc[i].y > 1))
			// 		std::cout << "mesh->mTextureCoords[" << j << "][" << i << "] = " << glm::to_string(glm::vec2(tc[i].x, tc[i].y)) << std::endl;
			// }
			uv2s.push_back({mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y});
		}
	}
    auto i0 = 2;
    auto i1 = 1;
    auto i2 = 0;
	// Process indices
	for (uint32_t i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace &face = mesh->mFaces[i];
		if (face.mNumIndices == 3)
		{
			indices.push_back(face.mIndices[i0]);
            indices.push_back(face.mIndices[i1]);
            indices.push_back(face.mIndices[i2]);
		}
		else
		{
			throw std::runtime_error("expected number of indices in face to equal 3, equals " + std::to_string(face.mNumIndices));
		}
	}
    auto indiceCount = uint32_t(indices.size());
    auto vertexCount = uint32_t(vertices.size());
    auto colorCount = uint32_t(colors.size());
    auto uv2Count = uint32_t(uv2s.size());
	info.shapeType = ShapeType::Mesh;
	info.info = [indices, vertices, colors, uv2s](auto&) -> MeshInfo {
		return {
			.indices = indices,
			.vertices = vertices,
			.colors = colors,
			.uv2s = uv2s,
		};
	};
	info.keyedTextures = keyedTextures;
	info.constants = constants;
	//
	// if (mesh->mNumBones)
	// {
	// 	glm::ivec4 *boneIDs = (glm::ivec4 *)this->operator()<int32_t>(IEntity::Quanta::BoneIDs, vertices.size());
	// 	glm::vec4 *boneWeights = (glm::vec4 *)this->operator()<Floating32>(IEntity::Quanta::BoneWeights, vertices.size());
	// 	glm::bvec4 *bonesEnabled = (glm::bvec4 *)this->operator()<bool>(IEntity::Quanta::BonesEnabled, vertices.size());
	// 	for (uint32_t i = 0; i < mesh->mNumBones; i++)
	// 	{
	// 		uint32_t boneIndex = 0;
	// 		String boneName(mesh->mBones[i]->mName.C_Str());
	// 		auto boneIter = boneMapping.find(boneName);
	// 		if (boneIter == boneMapping.end())
	// 		{
	// 			boneIndex = numBones;
	// 			numBones++;
	// 			auto boneNode = findBoneNode(aiscene->mRootNode, boneName);
	// 			glm::mat4 bindPoseTransform = aiMatrix4x4ToGlm(boneNode->mTransformation);
	// 			glm::mat4 boneOffset = glm::inverse(bindPoseTransform);
	// 			boneOffsets.push_back(boneOffset); // aiMatrix4x4ToGlm(mesh->mBones[i]->mOffsetMatrix)
	// 			boneLocalTransformations.push_back(glm::mat4(1));
	// 			boneFinalTransformations.push_back(glm::mat4(1));
	// 			bonePoseTransforms.push_back(bindPoseTransform);
	// 			boneParentNames.push_back(getParentBoneName(aiscene, boneName));
	// 			boneMapping[boneName] = boneIndex;
	// 		}
	// 		else
	// 		{
	// 			boneIndex = boneIter->value;
	// 		}

	// 		for (uint32_t j = 0; j < mesh->mBones[i]->mNumWeights; j++)
	// 		{
	// 			const auto &vertexID = mesh->mBones[i]->mWeights[j].mVertexId;
	// 			const auto &weight = mesh->mBones[i]->mWeights[j].mWeight;
	// 			addBoneData(boneIndex, weight, boneIDs[vertexID], boneWeights[vertexID], bonesEnabled[vertexID]);
	// 		}
	// 	}
	// 	boneParentIndices = std::vector<int32_t>(boneOffsets.size(), -1);
	// 	for (int i = 0; i < boneOffsets.size(); ++i)
	// 	{
	// 		const auto &boneName = boneParentNames[i];
	// 		if (!boneName.empty() && boneMapping.find(boneName) != boneMapping.end())
	// 		{
	// 			boneParentIndices[i] = boneMapping[boneName];
	// 		}
	// 	}
	// }
}
void totalNodes(const aiScene* aiscene,
    aiNode* node)
{
	totalToLoad++;
	for (uint32_t i = 0; i < node->mNumChildren; i++)
	{
        totalNodes(aiscene, node->mChildren[i]);
    }
}
std::optional<zg::EntityCreateInfo> processNode(
    const aiScene *aiscene,
    aiNode *node,
    zg::FRONTFACE frontFace,
    const shaders::RuntimeConstants& constants,
    IRenderer* iRenderer
)
{
	glm::mat4 transformation = ToAssimp<aiMatrix4x4, glm::mat4>(node->mTransformation);
	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 _scale;
	decomposeMatrix(transformation, position, rotation, _scale);
    zg::EntityCreateInfo info{
        .typeName = "Model",
        .position = position,
        .rotation = rotation,
        .scale = _scale,
        .name = aiscene->mName.C_Str()
    };
	zg::MeshCreateInfo meshInfo;
	auto infoCopy = info;
    bool valid = false;
	for (size_t i = 0; i < node->mNumMeshes; ++i)
	{
		auto thisMeshInfo = meshInfo;
        valid = true;
		aiMesh *aimesh = aiscene->mMeshes[node->mMeshes[i]];
		processMesh(aiscene, aimesh, frontFace, thisMeshInfo, iRenderer, constants);
		info.meshInfos.push_back(thisMeshInfo);
	}
	for (uint32_t i = 0; i < node->mNumChildren; i++)
	{
        if (!valid)
            valid = true;
        auto childInfo = processNode(aiscene, node->mChildren[i], frontFace, constants, iRenderer);
        if (childInfo.has_value())
    		info.childrenInfos.push_back(childInfo.value());
	}
    totalLoaded++;
    std::cout << "Processed Node: " << totalLoaded << "/" << totalToLoad << std::endl;
    if (valid)
        return info;
    return {};
};
zg::EntityCreateInfo zg::entities::ModelFactory(
    const interfaces::IFile& modelFile,
    std::string name,
    glm::vec3 position,
    glm::quat rotation,
    glm::vec3 scale,
    const shaders::RuntimeConstants& constants,
    IRenderer* iRenderer
)
{
	Assimp::Importer importer;
	auto aiscene = importSceneFromFile((interfaces::IFile&)modelFile, importer);
    totalNodes(aiscene, aiscene->mRootNode);
	auto info = processNode(aiscene, aiscene->mRootNode, iRenderer->frontFace, constants, iRenderer);
    auto& infoValue = info.value();
    infoValue.name = name;
    infoValue.position = position;
    infoValue.rotation = rotation;
    infoValue.scale = scale;
	importer.FreeScene();
    return infoValue;
}