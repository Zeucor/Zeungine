#pragma once
#include <zg/Entity.hpp>
namespace zg::entities
{
	EntityCreateInfo DeltaVisualizerFactory(glm::vec2 size, long double& targetDelta, long double& currentDelta,
											glm::vec3 position = {0, 0, 0}, glm::quat rotation = {1, 0, 0, 0}, glm::vec3 _scale = {1, 1, 1},
											const shaders::RuntimeConstants constants = {}, std::string name = "", zg::FRONTFACE frontFace = IRenderer::DEFAULTFRONTFACE);
	// struct DeltaVisualizer : Entity
	// {
	// 	std::vector<glm::vec4> colors;
	// 	std::vector<glm::vec3> normals;
	// 	zgfilesystem::File robotoFile;
	// 	zg::fonts::freetype::FreetypeFont robotoRegular;
	// 	Entity* frame;
	// 	zg::UniqueIdentifier frameID;
	// 	Entity* median;
	// 	Entity* overwork;
	// 	Entity* underwork;
	// 	Entity* currentPoint;
	// 	Entity* curve1;
	// 	std::vector<glm::vec2> curve1Points;
	// 	size_t curve1PointsIndex = 0;
	// 	zg::UniqueIdentifier curve1ID = 0;
	// 	char curveFrameOffset = 0;
	// 	std::vector<Entity*> frametimeEntities;
	// 	int64_t frametimeCursorIndex = -1;
	// 	Entity* frametimeCursor;
	// 	std::vector<Entity*> lastDeltaTextEntities;
	// 	int64_t lastDeltaTextCursorIndex = -1;
	// 	Entity* lastDeltaTextCursor;
	// 	long double& targetDelta;
	// 	long double& currentDelta;
	// 	glm::vec2 size;
	// 	size_t getTypeID() override { return EntityTypeID<DeltaVisualizer>::id; };
	// 	uint32_t getIndiceCount() { return 6; }
	// 	std::vector<uint32_t> getIndices(zg::FRONTFACE frontFace)
	// 	{
	// 	}
	// 	uint32_t getvertexCount() { return 4; }
	// 	std::vector<glm::vec3> getElements(glm::vec2 size)
	// 	{
	// 	}
	// 	DeltaVisualizer(Window& window, ) :
	// 			Entity(window, scene,
	// 						 zg::mergeVectors<std::string>(
	// 							 {{"Color", "Position", "Normal", "View", "Projection", "Model", "CameraPosition"}}, constants),
	// 						 getIndiceCount(), getIndices(window.iRenderer->frontFace), getvertexCount(), getElements(size),
	// 						 position, rotation, _scale,
	// 						 (name.empty() ? ("DeltaVisualizer " + std::to_string(++deltaVisualizersCount)) : name)),
	// 	{

	// 		// frame
	// 		{
	// 		}
	// 		updateIndices(indices);
	// 		setColor();
	// 		updateElements("Position", vertices);
	// 	}
	// 	void setColor(glm::vec4 newColor)
	// 	{
	// 		colors = {newColor, newColor, newColor, newColor};
	// 		updateElements("Color", colors);
	// 	}
	// 	void preUpdate() override
	// 	{

	// 	}
	// }; // namespace zg::entities
} // namespace zg::entities
