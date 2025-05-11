#include <zg/entities/DeltaVisualizer.hpp>
#include <zg/entities/Curve.hpp>
#include <zg/entities/Frame.hpp>
#include <zg/entities/Plane.hpp>
#include <zg/fonts/freetype/Freetype.hpp>
#include <zg/zgfilesystem/File.hpp>
#include <zg/utilities.hpp>
#include <zg/Registry.hpp>
#include <zg/Window.hpp>
using namespace zg;
zg::EntityCreateInfo zg::entities::DeltaVisualizerFactory(glm::vec2 size, long double& targetDelta, long double& currentDelta,
    glm::vec3 position, glm::quat rotation, glm::vec3 scale,
    const shaders::RuntimeConstants& constants, std::string name, zg::FRONTFACE frontFace)
{
    auto targetDeltaPointer = &targetDelta;
    auto currentDeltaPointer = &currentDelta;
	auto color = glm::vec4(0.2f, 0.3f, 0.4f, 0.7f);
    MeshCreateInfo meshInfo{
        .indiceCount = [](auto&) { return 6; },
        .indices = [frontFace](auto&) -> std::vector<uint32_t>
        {
            if (frontFace == zg::CLOCKWISE)
                return {0, 1, 2, 2, 3, 0};
            else
                return {2, 1, 0, 0, 3, 2};
        },
        .vertexCount = [](auto&) { return 4; },
        .vertices = [](auto& entity) -> std::vector<glm::vec3>
        {
            auto& size = entity.template getData<glm::vec2>("Size");
            return {{glm::vec3(size.x / 2.f, size.y / 2.f, 0), glm::vec3(size.x / 2.f, -(size.y / 2.f), 0),
                glm::vec3(-(size.x / 2.f), -(size.y / 2.f), 0), glm::vec3(-(size.x / 2.f), size.y / 2.f, 0)}};
        },
        .colorCount = [](auto&) { return 4; },
        .colors = [](auto& entity) -> std::vector<glm::vec4>
        {
            auto& color = entity.template getData<glm::vec4>("Color");
            return {4, color};
        },
        .constants = zg::mergeVectors<std::string>(
			{{"Color", "Position", "Normal", "View", "Projection", "Model", "CameraPosition"}}, constants)
    };
    EntityCreateInfo info{
        .typeName = "DeltaVisualizer",
        .position = position,
        .rotation = rotation,
        .scale = scale,
        .name = name,
        .preUpdateFunction = [constants, size](auto& entity)
        {
            auto& currentDelta = *entity.template getData<long double*>("currentDelta");
            if (currentDelta > 100000 || currentDelta < 0)
                return;
            auto& robotoFile = *entity.template getData<zgfilesystem::File*>("RobotoFile");
            auto& robotoRegular = *entity.template getData<zg::fonts::freetype::FreetypeFont*>("RobotoRegularFont");
            auto& maxpoints = entity.template getData<size_t>("maxpoints");
            auto& step = entity.template getData<float>("step");
            auto& startX = entity.template getData<float>("startX");
            auto& deltaVisualizersCount = entity.template getData<size_t>("deltaVisualizersCount");
            
		    auto& curvePointsIndex = entity.template getData<size_t>("curvePointsIndex");
            auto& curveID = entity.template getData<size_t>("curveID");
		    auto& curveFrameOffset = entity.template getData<char>("curveFrameOffset");
		    auto& frametimeEntities = entity.template getData<std::vector<size_t>>("frametimeEntities");
		    auto& frametimeCursorIndex = entity.template getData<int64_t>("frametimeCursorIndex");
		    auto& frametimeCursor = entity.template getData<size_t>("frametimeCursor");
		    auto& lastDeltaTextEntities = entity.template getData<std::vector<size_t>>("lastDeltaTextEntities");
		    auto& lastDeltaTextCursorIndex = entity.template getData<int64_t>("lastDeltaTextCursorIndex");
		    auto& lastDeltaTextCursor = entity.template getData<size_t>("lastDeltaTextCursor");
		    auto& thisYTextEntities = entity.template getData<std::vector<size_t>>("thisYTextEntities");
		    auto& thisYTextCursorIndex = entity.template getData<int64_t>("thisYTextCursorIndex");
		    auto& thisYTextCursor = entity.template getData<size_t>("thisYTextCursor");
            auto& currentPoint = entity.template getData<size_t>("currentPoint");
            auto& targetDelta = *entity.template getData<long double*>("targetDelta");
            auto& averageDeltaSum = entity.template getData<long double>("averageDeltaSum");
			{
                averageDeltaSum += currentDelta;
				curveFrameOffset++;
				if (!curveID)
				{
					std::vector<glm::vec2> curvePoints(maxpoints, glm::vec2(0));
					auto curveInfo = zg::entities::NDParametricCurveFactory<2>(
						glm::vec3(0, 0, 0.15), {1, 0, 0, 0}, glm::vec3(1), glm::vec4(1, 1, 1, 1), constants, "Delta Visualizer Curve", size.y / 128.f,
						curvePoints);
					auto curve_tuple = entity.addChild(curveInfo);
                    curveID = std::get<KEY_ID_VECTOR_ID_INDEX>(curve_tuple);
                    return;
				}
                auto& curve = Registry::getEntity(curveID);
                auto& curvePoints = curve.template getData<std::vector<glm::vec2>>("Points");
				if (curveFrameOffset > 0)
				{
					if (curveFrameOffset == 10)
					{
						curveFrameOffset = 0;
                        goto _nextPoint;
					}
					return;
				}
            _nextPoint:
                auto _currentDelta = averageDeltaSum / 10;
                averageDeltaSum = 0.0L;
				auto curvePointsData = curvePoints.data();
				if (curvePointsIndex >= maxpoints)
				{
					auto i = 0;
					for (; i < curvePointsIndex - 1; i++)
					{
						auto& p = curvePointsData[i + 1];
						p.x -= step;
						curvePointsData[i] = p;
					}
					curvePointsData[i] = glm::vec2(0);
					curvePointsIndex--;
				}
				auto lastX = curvePointsIndex ? curvePoints[curvePointsIndex - 1].x : startX;
				auto thisY = ((_currentDelta / targetDelta) * size.y / 2.f) - size.y / 2.f;
				// std::cout << "currentDelta: " << currentDelta << ", deltaTime: " << targetDelta << ", thisY: "
				// << thisY
				// 					<< std::endl;
				auto point = glm::vec2(lastX + step, thisY);
                auto& currentPointEntity = Registry::getEntity(currentPoint);
				currentPointEntity.position = glm::vec3(point, 0.15);
				curvePointsData[curvePointsIndex++] = point;
				curve.refreshMeshes();

				// update lastDeltaText
				std::string lastDeltaTextString = "Delta: " + std::to_string(currentDelta);
				float lastDeltaTextFontsize = 42, lastDeltaTextLineheight = 0;
				auto lastDeltaTextBounds = glm::vec2(0);
				auto lastDeltaTextBreakstyle = zg::enums::EBreakStyle::None;
				auto lastDeltaTextScale =
					glm::vec3(1.f / (lastDeltaTextFontsize * 32.f), 1.f / (lastDeltaTextFontsize * 32.f), 1.f);
				auto lastDeltaTextSize =
					robotoRegular.stringSize(lastDeltaTextString, lastDeltaTextFontsize, lastDeltaTextLineheight,
																	 lastDeltaTextBounds, lastDeltaTextBreakstyle);
                auto& scene = Registry::getScene(entity.INDEX_STACK);
                auto deltaTextPosition = glm::vec3((size.x / 2.f) - (lastDeltaTextSize.x * lastDeltaTextScale.x),
                (-size.y / 3.f) - (lastDeltaTextSize.y * lastDeltaTextScale.y), 0.15);
				robotoRegular.stringToEntity(
					lastDeltaTextString,
					deltaTextPosition,
					glm::vec4(0.369, 0.760, 0.564, 1), glm::quat(1, 0, 0, 0), lastDeltaTextScale, lastDeltaTextFontsize,
					lastDeltaTextLineheight, lastDeltaTextSize * (glm::vec2(lastDeltaTextScale) + glm::vec2(1e-3f)),
					lastDeltaTextBreakstyle, scene, entity, lastDeltaTextEntities, lastDeltaTextCursorIndex, lastDeltaTextCursor);
                // update thisYText
                std::string thisYTextString = "Y: " + std::to_string(thisY);
                float thisYTextFontsize = 42, thisYTextLineheight = 0;
                auto thisYTextBounds = glm::vec2(0);
                auto thisYTextBreakstyle = zg::enums::EBreakStyle::None;
                auto thisYTextScale =
                    glm::vec3(1.f / (thisYTextFontsize * 32.f), 1.f / (thisYTextFontsize * 32.f), 1.f);
                auto thisYTextSize =
                    robotoRegular.stringSize(thisYTextString, thisYTextFontsize, thisYTextLineheight,
                                                                        thisYTextBounds, thisYTextBreakstyle);
                auto yTextPosition = glm::vec3((size.x / 2.f) - (thisYTextSize.x * thisYTextScale.x),
                deltaTextPosition.y - (thisYTextSize.y * thisYTextScale.y), 0.15);
                robotoRegular.stringToEntity(
                    thisYTextString,
                    yTextPosition,
                    glm::vec4(0.369, 0.760, 0.564, 1), glm::quat(1, 0, 0, 0), thisYTextScale, thisYTextFontsize,
                    thisYTextLineheight, thisYTextSize * (glm::vec2(thisYTextScale) + glm::vec2(1e-3f)),
                    thisYTextBreakstyle, scene, entity, thisYTextEntities, thisYTextCursorIndex, thisYTextCursor);
			}
        },
        .onAddedFunction = [constants, targetDeltaPointer, currentDeltaPointer, size, frontFace](auto& entity)
        {
            auto& targetDelta = *entity.template make<long double*>("targetDelta", targetDeltaPointer);
            auto& currentDelta = *entity.template make<long double*>("currentDelta", currentDeltaPointer);
            auto& averageDeltaSum = entity.template make<long double>("averageDeltaSum", 0.0L);
            auto& robotoFilePointer = entity.template make<zgfilesystem::File*>(
                "RobotoFile",
                new zgfilesystem::File(
                    zgfilesystem::File::getProgramDirectoryPath() / "fonts" / "Roboto" / "Roboto-Regular.ttf",
                    zg::enums::EFileLocation::Absolute,
                    "r"
                )
            );
            auto& window = Registry::getWindow(entity.INDEX_STACK);
            auto& robotoRegularPointer = entity.template make<zg::fonts::freetype::FreetypeFont*>(
                "RobotoRegularFont",
                new zg::fonts::freetype::FreetypeFont(
                    window.iRenderer,
                    *robotoFilePointer
                )
            );
            auto& robotoRegular = *robotoRegularPointer;
            auto& maxpoints = entity.template make<size_t>("maxpoints", 42);
            auto& step = entity.template make<float>("step", size.x / maxpoints);
            auto& startX = entity.template make<float>("startX", (-(size.x) / 2.f) - step);
            auto& deltaVisualizersCount = entity.template make<size_t>("deltaVisualizersCount", 0);
            
		    auto& curvePoints = entity.template make<std::vector<glm::vec2>>("curvePoints");
		    auto& curvePointsIndex = entity.template make<size_t>("curvePointsIndex");
            auto& curveID = entity.template make<size_t>("curveID");
		    auto& curveFrameOffset = entity.template make<char>("curveFrameOffset");
		    auto& frametimeEntities = entity.template make<std::vector<size_t>>("frametimeEntities");
		    auto& frametimeCursorIndex = entity.template make<int64_t>("frametimeCursorIndex", -1);
		    auto& frametimeCursor = entity.template make<size_t>("frametimeCursor");
		    auto& lastDeltaTextEntities = entity.template make<std::vector<size_t>>("lastDeltaTextEntities");
		    auto& lastDeltaTextCursorIndex = entity.template make<int64_t>("lastDeltaTextCursorIndex", -1);
		    auto& lastDeltaTextCursor = entity.template make<size_t>("lastDeltaTextCursor");
		    auto& thisYTextEntities = entity.template make<std::vector<size_t>>("thisYTextEntities");
		    auto& thisYTextCursorIndex = entity.template make<int64_t>("thisYTextCursorIndex", -1);
		    auto& thisYTextCursor = entity.template make<size_t>("thisYTextCursor");
            auto& currentPoint = entity.template make<size_t>("currentPoint");
            auto angle = glm::angleAxis(glm::radians(0.f), glm::vec3(0.f, 1.f, 0.f));
            // auto frameCreateInfo = entities::FrameFactory(glm::vec4(0.869, 0.860, 0.864, 1), "Window Curve Frame",
            //                                             glm::vec3(0, 1.0, -0.1), angle, glm::vec3(1),
            //                                             glm::vec2(5.1, 2.0f), 0.06f,
            //                                             constants);
            // entity.addChild(frameCreateInfo);
            auto medianCreateInfo = entities::PlaneFactory(glm::vec4(0.208, 0.990, 0.586, 1.f), "Window Curve Median",
                                                        glm::vec3(0, 0, 0.05), angle,
                                                        glm::vec3(size.x, size.y / 32.f, 1.f),
                                                        constants);
            entity.addChild(medianCreateInfo);
            auto overworkCreateInfo = entities::PlaneFactory(glm::vec4(0.868, 0.960, 0.0384, 1.f), "Window Curve Overwork",
                                                            glm::vec3(0, (size.y / 3.f) - (size.y / 64.f), 0.05), angle,
                                                            glm::vec3(size.x, size.y / 32.f, 1.f),
                                                            constants);
            entity.addChild(overworkCreateInfo);
            auto underworkCreateInfo = entities::PlaneFactory(glm::vec4(0.700, 0.715, 1.00, 1.f), "Window Curve Underwork",
                                                            glm::vec3(0, (-size.y / 3.f) + (size.y / 64.f), 0.05), angle,
                                                            glm::vec3(size.x, size.y / 32.f, 1.f),
                                                            constants);
            entity.addChild(underworkCreateInfo);
            //
            std::string frametimeString = "Frametime: " + std::to_string(*targetDeltaPointer);
            float frametimeFontsize = 42, frametimeLineheight = 0;
            auto frametimeBounds = glm::vec2(0);
            auto frametimeBreakstyle = zg::enums::EBreakStyle::None;
            auto frametimeScale = glm::vec3(1.f / (frametimeFontsize * 32.f), 1.f / (frametimeFontsize * 32.f), 1.f);
            auto frametimeSize = robotoRegular.stringSize(frametimeString, frametimeFontsize, frametimeLineheight,
                                                                                                        frametimeBounds, frametimeBreakstyle);
            auto& scene = Registry::getScene(entity.INDEX_STACK);
            robotoRegular.stringToEntity(
                frametimeString,
                glm::vec3((size.x / 2.f) - (frametimeSize.x * frametimeScale.x),
                                    ((frametimeLineheight * frametimeScale.y) / 2.f), +0.15),
                glm::vec4(0.869, 0.860, 0.864, 1), angle, frametimeScale, frametimeFontsize, frametimeLineheight,
                frametimeSize * (glm::vec2(frametimeScale) + glm::vec2(1e-3f)), frametimeBreakstyle, scene, entity,
                frametimeEntities, frametimeCursorIndex, frametimeCursor);
            auto currentPointInfo = entities::PlaneFactory(glm::vec4(0.9, 0.2, 0.4, 1), "Delta Visualizer Current Point", glm::vec3(0), glm::quat(1, 0, 0, 0), glm::vec3(size.x / 25.f, size.x / 25.f, 1.f), constants, frontFace);
            auto currentPoint_tuple = entity.addChild(currentPointInfo);
            currentPoint = std::get<KEY_ID_VECTOR_ID_INDEX>(currentPoint_tuple);
        },
        .onRemovedFunction = [](auto& entity)
        {
            auto& robotoFilePointer = entity.template getData<zgfilesystem::File*>("RobotoFile" );
            auto& robotoRegularPointer = entity.template getData<zg::fonts::freetype::FreetypeFont*>("RobotoRegularFont");
            delete robotoFilePointer;
            delete robotoRegularPointer;
        },
        .dataMap = {
            {"Size", size},
            {"Color", color}
        },
        .meshInfos = {meshInfo}
    };
    return info;
}