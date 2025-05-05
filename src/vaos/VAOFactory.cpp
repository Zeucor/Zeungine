#include <stdexcept>
#include <zg/renderers/GLRenderer.hpp>
#include <zg/vaos/VAOFactory.hpp>
// #ifdef USE_VULKAN
#include <zg/renderers/VulkanRenderer.hpp>
// #endif
using namespace zg::vaos;
VAOFactory::ConstantSizeMap VAOFactory::constantSizes = {{"Indice", {3, sizeof(uint32_t), ZG_UNSIGNED_INT}},
														 {"Color", {4, sizeof(float), ZG_FLOAT4}},
														 {"Position", {3, sizeof(float), ZG_FLOAT3}},
														 {"Normal", {3, sizeof(float), ZG_FLOAT3}},
														 {"UV2", {2, sizeof(float), ZG_FLOAT2}},
														 {"UV3", {3, sizeof(float), ZG_FLOAT3}}};
VAOFactory::VAOConstantMap VAOFactory::VAOConstants = {
	{"Color", true}, {"Position", true}, {"Normal", true}, {"UV2", true}, {"UV3", true}};
void VAOFactory::generate(VAO &vao) { vao.vaoIRenderer->generateVAO(vao); };
void VAOFactory::copy(VAO &dest, const VAO& src) { src.vaoIRenderer->copyVAO(dest, src); };
size_t VAOFactory::getStride(const RuntimeConstants &constants)
{
	size_t stride = 0;
	for (auto &constant : constants)
	{
		if (!isVAOConstant(constant))
			continue;
		auto constantIter = constantSizes.find(constant);
		if (constantIter == constantSizes.end())
			continue;
		auto &constantSize = constantIter->second;
		stride += std::get<0>(constantSize) * std::get<1>(constantSize);
	};
	return stride;
};
size_t VAOFactory::getOffset(const RuntimeConstants &constants, const std::string_view offsetConstant)
{
	size_t stride = 0;
	for (auto &constant : constants)
	{
		if (!isVAOConstant(constant))
			continue;
		if (constant == offsetConstant)
			return stride;
		auto constantIter = constantSizes.find(constant);
		if (constantIter == constantSizes.end())
			continue;
		auto &constantSize = constantIter->second;
		stride += std::get<0>(constantSize) * std::get<1>(constantSize);
	};
	throw std::runtime_error("No such constant");
};
bool VAOFactory::isVAOConstant(const std::string_view constant)
{
	auto iter = VAOConstants.find(constant);
	return (iter != VAOConstants.end());
}
void VAOFactory::destroy(VAO &vao) { vao.vaoIRenderer->destroyVAO(vao); };
