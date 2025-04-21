#pragma once
#include <zg/Entity.hpp>
#include <zg/Window.hpp>
#include <zg/entities/Curve.hpp>
#include <zg/entities/Frame.hpp>
#include <zg/entities/Plane.hpp>
#include <zg/entities/TypeID.hpp>
#include <zg/fonts/freetype/Freetype.hpp>
#include <zg/Scene.hpp>
#include <zg/zgfilesystem/File.hpp>
namespace zg::entities
{
	EntityCreateInfo DeltaVisualizerFactory(glm::vec2 size, long double& targetDelta, long double& currentDelta,
											glm::vec3 position, glm::quat rotation, glm::vec3 _scale,
											const shaders::RuntimeConstants& constants = {}, std::string_view name = "");
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
	// 	float step;
	// 	float startX;
	// 	static inline size_t deltaVisualizersCount = 0;
	// 	static constexpr size_t maxpoints = 42;
	// 	size_t getTypeID() override { return EntityTypeID<DeltaVisualizer>::id; };
	// 	uint32_t getIndiceCount() { return 6; }
	// 	std::vector<uint32_t> getIndices(zg::FRONTFACE frontFace)
	// 	{
	// 		if (frontFace == zg::CLOCKWISE)
	// 			return {{2, 1, 0, 0, 3, 2}};
	// 		else
	// 			return {{0, 1, 2, 2, 3, 0}};
	// 	}
	// 	uint32_t getvertexCount() { return 4; }
	// 	std::vector<glm::vec3> getElements(glm::vec2 size)
	// 	{
	// 		return {{glm::vec3(size.x / 2.f, size.y / 2.f, 0), glm::vec3(size.x / 2.f, -(size.y / 2.f), 0),
	// 						 glm::vec3(-(size.x / 2.f), -(size.y / 2.f), 0), glm::vec3(-(size.x / 2.f), size.y / 2.f, 0)}};
	// 	}
	// 	DeltaVisualizer(Window& window, ) :
	// 			Entity(window, scene,
	// 						 zg::mergeVectors<std::string>(
	// 							 {{"Color", "Position", "Normal", "View", "Projection", "Model", "CameraPosition"}}, constants),
	// 						 getIndiceCount(), getIndices(window.iRenderer->frontFace), getvertexCount(), getElements(size),
	// 						 position, rotation, _scale,
	// 						 (name.empty() ? ("DeltaVisualizer " + std::to_string(++deltaVisualizersCount)) : name)),
	// 			robotoFile(
	// 				zgfilesystem::File(zgfilesystem::File::getProgramDirectoryPath() / "fonts" / "Roboto" / "Roboto-Regular.ttf",
	// 													 zg::enums::EFileLocation::Absolute, "r")),
	// 			robotoRegular(window, robotoFile), targetDelta(targetDelta), currentDelta(currentDelta), size(size),
	// 			step(size.x / maxpoints), startX((-(size.x) / 2.f) - step)
	// 	{

	// 		// frame
	// 		{
	// 			auto angle = glm::angleAxis(glm::radians(0.f), glm::vec3(0.f, 1.f, 0.f));
	// 			frame = std::make_shared<zg::entities::Frame>(window, scene, glm::vec3(0, 1.0, -0.1), angle, glm::vec3(1),
	// 																										glm::vec2(5.1, 2.0f), 0.06f, glm::vec4(0.869, 0.860, 0.864, 1),
	// 																										constants, "Window Curve Frame");
	// 			frameID = addChild(frame);
	// 			median = std::make_shared<zg::entities::Plane>(window, scene, glm::vec3(0, 1.0, -0.05), angle, glm::vec3(1),
	// 																										 glm::vec2(5.1, 0.05f), glm::vec4(0.208, 0.990, 0.586, 1.f),
	// 																										 constants, "Window Curve Median");
	// 			addChild(median);
	// 			overwork = std::make_shared<zg::entities::Plane>(window, scene, glm::vec3(0, 1.5, -0.05), angle, glm::vec3(1),
	// 																											 glm::vec2(5.1, 0.05f), glm::vec4(0.868, 0.960, 0.0384, 1.f),
	// 																											 constants, "Window Curve Overwork");
	// 			addChild(overwork);
	// 			underwork = std::make_shared<zg::entities::Plane>(window, scene, glm::vec3(0, 0.5, -0.05), angle, glm::vec3(1),
	// 																												glm::vec2(5.1, 0.05f), glm::vec4(0.700, 0.715, 1.00, 1.f),
	// 																												constants, "Window Curve Underwork");
	// 			addChild(underwork);
	// 			//
	// 			std::string frametimeString = "Frametime: " + std::to_string(targetDelta);
	// 			float frametimeFontsize = 42, frametimeLineheight = 0;
	// 			auto frametimeBounds = glm::vec2(0);
	// 			auto frametimeBreakstyle = zg::enums::EBreakStyle::None;
	// 			auto frametimeScale = glm::vec3(1.f / (frametimeFontsize * 8.f), 1.f / (frametimeFontsize * 8.f), 1.f);
	// 			auto frametimeSize = robotoRegular.stringSize(frametimeString, frametimeFontsize, frametimeLineheight,
	// 																										frametimeBounds, frametimeBreakstyle);
	// 			robotoRegular.stringToEntity(
	// 				frametimeString,
	// 				glm::vec3((size.x / 2.f) - (frametimeSize.x * frametimeScale.x),
	// 									1.0 + ((frametimeLineheight * frametimeScale.y) / 2.f), -0.15),
	// 				glm::vec4(0.869, 0.860, 0.864, 1), angle, frametimeScale, frametimeFontsize, frametimeLineheight,
	// 				frametimeSize * (glm::vec2(frametimeScale) + glm::vec2(1e-3f)), frametimeBreakstyle, scene, *this,
	// 				frametimeEntities, frametimeCursorIndex, frametimeCursor);
	// 			currentPoint = std::make_shared<zg::entities::Plane>(
	// 				window, scene, glm::vec3(0), glm::quat(1, 0, 0, 0), glm::vec3(1), glm::vec2(size.x / 25.f, size.x / 25.f),
	// 				glm::vec4(0.9, 0.2, 0.4, 1), constants, "Delta Curve Current Point");
	// 			addChild(currentPoint);
	// 		}
	// 		computeNormals(window.iRenderer->frontFace, indices, vertices, normals);
	// 		updateIndices(indices);
	// 		setColor(glm::vec4(0.2f, 0.3f, 0.4f, 0.7f));
	// 		updateElements("Position", vertices);
	// 		updateElements("Normal", normals);
	// 	}
	// 	void setColor(glm::vec4 newColor)
	// 	{
	// 		colors = {newColor, newColor, newColor, newColor};
	// 		updateElements("Color", colors);
	// 	}
	// 	void preUpdate() override
	// 	{

	// 		// curves (hehe)
	// 		{
	// 			if (!curve1ID)
	// 			{
	// 				curve1Points.resize(maxpoints, glm::vec2(0));
	// 				auto angle = glm::angleAxis(glm::radians(0.f), glm::vec3(0.f, 1.f, 0.f));
	// 				curve1 = std::make_shared<zg::entities::NDParametricCurve<2, float>>(
	// 					window, scene, glm::vec3(0), angle, glm::vec3(1), glm::vec4(1, 1, 1, 1), constants, "Window Curve", 0.02f,
	// 					std::map<std::string, double>(), curve1Points);
	// 				curve1ID = addChild(curve1);
	// 			}
	// 			if (curveFrameOffset > 0)
	// 			{
	// 				curveFrameOffset++;
	// 				if (curveFrameOffset == 5)
	// 				{
	// 					curveFrameOffset = 0;
	// 				}
	// 				return;
	// 			}
	// 			curveFrameOffset++;
	// 			auto curve1PointsData = curve1Points.data();
	// 			if (curve1PointsIndex >= maxpoints)
	// 			{
	// 				auto i = 0;
	// 				for (; i < curve1PointsIndex - 1; i++)
	// 				{
	// 					auto& p = curve1PointsData[i + 1];
	// 					p.x -= step;
	// 					curve1PointsData[i] = p;
	// 				}
	// 				curve1PointsData[i] = glm::vec2(0);
	// 				curve1PointsIndex--;
	// 			}
	// 			auto lastX = curve1PointsIndex ? curve1Points[curve1PointsIndex - 1].x : startX;
	// 			auto thisY = (currentDelta / targetDelta);
	// 			// std::cout << "currentDelta: " << currentDelta << ", deltaTime: " << targetDelta << ", thisY: "
	// 			// << thisY
	// 			// 					<< std::endl;
	// 			auto point = glm::vec2(lastX + step, thisY);
	// 			currentPoint->position = glm::vec3(point, -0.15);
	// 			curve1PointsData[curve1PointsIndex++] = point;
	// 			curve1->generateAndUpdateCurve(curve1Points);
	// 			// update lastDeltaText

	// 			std::string lastDeltaTextString = "Delta: " + std::to_string(currentDelta);
	// 			float lastDeltaTextFontsize = 42, lastDeltaTextLineheight = 0;
	// 			auto lastDeltaTextBounds = glm::vec2(0);
	// 			auto lastDeltaTextBreakstyle = zg::enums::EBreakStyle::None;
	// 			auto lastDeltaTextScale =
	// 				glm::vec3(1.f / (lastDeltaTextFontsize * 8.f), 1.f / (lastDeltaTextFontsize * 8.f), 1.f);
	// 			auto lastDeltaTextSize =
	// 				robotoRegular.stringSize(lastDeltaTextString, lastDeltaTextFontsize, lastDeltaTextLineheight,
	// 																 lastDeltaTextBounds, lastDeltaTextBreakstyle);
	// 			robotoRegular.stringToEntity(
	// 				lastDeltaTextString,
	// 				glm::vec3((size.x / 2.f) - (lastDeltaTextSize.x * lastDeltaTextScale.x),
	// 									-(lastDeltaTextSize.y * lastDeltaTextScale.y), -0.15),
	// 				glm::vec4(0.369, 0.760, 0.564, 1), glm::quat(1, 0, 0, 0), lastDeltaTextScale, lastDeltaTextFontsize,
	// 				lastDeltaTextLineheight, lastDeltaTextSize * (glm::vec2(lastDeltaTextScale) + glm::vec2(1e-3f)),
	// 				lastDeltaTextBreakstyle, scene, *this, lastDeltaTextEntities, lastDeltaTextCursorIndex, lastDeltaTextCursor);
	// 		}
	// 	}
	// }; // namespace zg::entities
} // namespace zg::entities
