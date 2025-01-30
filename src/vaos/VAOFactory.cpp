#include <stdexcept>
#include <zg/interfaces/IEntity.hpp>
#include <zg/renderers/GLRenderer.hpp>
#include <zg/vaos/VAOFactory.hpp>
using namespace zg::vaos;
VAOFactory::ConstantSizeMap VAOFactory::constantSizes = {{"Indice", {3, sizeof(uint32_t), GL_UNSIGNED_INT}},
																												 {"Color", {4, sizeof(float), GL_FLOAT}},
																												 {"Position", {3, sizeof(float), GL_FLOAT}},
																												 {"Normal", {3, sizeof(float), GL_FLOAT}},
																												 {"UV2", {2, sizeof(float), GL_FLOAT}},
																												 {"UV3", {3, sizeof(float), GL_FLOAT}}};
VAOFactory::VAOConstantMap VAOFactory::VAOConstants = {
	{"Indice", false}, {"Color", true},			{"Position", true},					{"Normal", true}, {"UV2", true},
	{"UV3", true},		 {"View", false},			{"Projection", false},			{"Model", false}, {"CameraPosition", false},
	{"Fog", false},		 {"Lighting", false}, {"LightSpaceMatrix", false}};
void VAOFactory::generate(VAO &vao)
{
	vao.vaoWindow.iRenderer->generateVAO(vao);
};
size_t VAOFactory::getStride(const RuntimeConstants& constants)
{
	size_t stride = 0;
	for (auto& constant : constants)
	{
		if (!isVAOConstant(constant))
			continue;
		auto& constantSize = constantSizes[constant];
		stride += std::get<0>(constantSize) * std::get<1>(constantSize);
	};
	return stride;
};
size_t VAOFactory::getOffset(const RuntimeConstants& constants, const std::string_view offsetConstant)
{
	size_t stride = 0;
	for (auto& constant : constants)
	{
		if (!isVAOConstant(constant))
			continue;
		if (constant == offsetConstant)
			return stride;
		auto& constantSize = constantSizes[constant];
		stride += std::get<0>(constantSize) * std::get<1>(constantSize);
	};
	throw std::runtime_error("No such constant");
};
bool VAOFactory::isVAOConstant(const std::string_view constant) { return VAOConstants[constant]; }
void VAOFactory::destroy(VAO &vao)
{
	vao.vaoWindow.iRenderer->destroyVAO(vao);
};
