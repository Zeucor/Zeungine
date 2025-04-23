#include <zg/Registry.hpp>
#include <zg/components/windows/FXAA.hpp>
#include <zg/shaders/ShaderFactory.hpp>
#include <zg/Window.hpp>
using namespace zg::components::windows;
zg::components::windows::WindowComponentCreateInfo zg::components::windows::FXAAFactory(
    float edgeThresholdMin,
	float edgeThreshold,
	float edgeSearchSteps,
	float subpixQuality
)
{
	WindowComponentCreateInfo info{
        .name = "FXAA",
        .onAttachedFunction = [
            edgeThresholdMin,
            edgeThreshold,
            edgeSearchSteps,
            subpixQuality
        ](auto& component)
        {
            auto& window = zg::Registry::getWindow(component.hostIndexStack);
            zg::PostProcessingStageCreateInfo fxaaStageCreateInfo{
                .name = "FXAA",
                .inputs = {"ColorTexture"},
                .outputs = {{"ColorTexture", zg::textures::Framebuffer::AttachmentType::Color}},
                .constants = {"FXAA"},
                .setShaderConstants = [
                    edgeThresholdMin,
                    edgeThreshold,
                    edgeSearchSteps,
                    subpixQuality
                ](shaders::Shader& shader, auto& vao)
                {
                    float values[4] = {
                        edgeThresholdMin,
                        edgeThreshold,
                        edgeSearchSteps,
                        subpixQuality
                    };
                    shader.setBlock("FXAAValues", vao, values);
                    glm::vec2 inverseScreenSize(1.f / shader.iRenderer->platformWindowPointer->renderWindowPointer->windowWidth, 1.f / shader.iRenderer->platformWindowPointer->renderWindowPointer->windowHeight);
                    shader.setBlock("InverseScreenSize", vao, inverseScreenSize);
                },
                .staticOnAttached = []()
                {
                    shaders::ShaderFactory::addHook(
                        shaders::ShaderType::Fragment,
                        "layout",
                        "FXAA",
                        [](shaders::Shader& shader, const auto& constants) -> std::string {
                            uint32_t bindingIndex = 0;
                            bindingIndex = shaders::ShaderFactory::currentBindingIndex++;
                            shader.addUBO(shaders::ShaderType::Fragment, "FXAAValues", bindingIndex, sizeof(float) * 4);
                            std::string string =
                                "layout(binding = " + std::to_string(bindingIndex) + ") uniform FXAAValues {\n" +
                                "  float edgeThresholdMin;\n" +
                                "  float edgeThreshold;\n" +
                                "  float edgeSearchSteps;\n" +
                                "  float subpixQuality;\n" +
                                "} fxaaValues;\n";
                            bindingIndex = shaders::ShaderFactory::currentBindingIndex++;
                            shader.addUBO(shaders::ShaderType::Fragment, "InverseScreenSize", bindingIndex, sizeof(glm::vec2));
                            string +=
                                "layout(binding = " + std::to_string(bindingIndex) + ") uniform InverseScreenSize {\n" +
                                "  vec2 size;\n" +
                                "} inverseScreenSize;\n";
                            string += "float rgb2luma(vec3 rgb);";
                            return string;
                        }
                    );
                    shaders::ShaderFactory::addHook(
                        shaders::ShaderType::Fragment,
                        "postPostInMain",
                        "FXAA",
                        [](auto& shader, const auto& constants) -> std::string {
                            return  "vec3 colorCenter = texture(ColorTexture, inUV).rgb;\n"
                                    "float lumaCenter = rgb2luma(colorCenter);\n"
                                    "float lumaNorth = rgb2luma(texture(ColorTexture, inUV + vec2(0.0, inverseScreenSize.size.y)).rgb);\n"
                                    "float lumaSouth = rgb2luma(texture(ColorTexture, inUV - vec2(0.0, inverseScreenSize.size.y)).rgb);\n"
                                    "float lumaWest  = rgb2luma(texture(ColorTexture, inUV - vec2(inverseScreenSize.size.x, 0.0)).rgb);\n"
                                    "float lumaEast  = rgb2luma(texture(ColorTexture, inUV + vec2(inverseScreenSize.size.x, 0.0)).rgb);\n"
                                    "float lumaMin = min(lumaCenter, min(min(lumaNorth, lumaSouth), min(lumaWest, lumaEast)));\n"
                                    "float lumaMax = max(lumaCenter, max(max(lumaNorth, lumaSouth), max(lumaWest, lumaEast)));\n"
                                    "float lumaRange = lumaMax - lumaMin;\n"
                                    "if (lumaRange < max(fxaaValues.edgeThresholdMin, lumaMax * fxaaValues.edgeThreshold)) {\n"
                                    "  FragColor = vec4(colorCenter, 1.0);\n"
                                    "  return;\n"
                                    "}\n"
                                    "float lumaNW = rgb2luma(texture(ColorTexture, inUV + vec2(-inverseScreenSize.size.x,  inverseScreenSize.size.y)).rgb);\n"
                                    "float lumaNE = rgb2luma(texture(ColorTexture, inUV + vec2( inverseScreenSize.size.x,  inverseScreenSize.size.y)).rgb);\n"
                                    "float lumaSW = rgb2luma(texture(ColorTexture, inUV + vec2(-inverseScreenSize.size.x, -inverseScreenSize.size.y)).rgb);\n"
                                    "float lumaSE = rgb2luma(texture(ColorTexture, inUV + vec2( inverseScreenSize.size.x, -inverseScreenSize.size.y)).rgb);\n"
                                    "float edgeHorizontal = abs((lumaNW + lumaNE) - (lumaSW + lumaSE)) * 2.0 + abs((lumaNorth - lumaSouth) * 2.0);\n"
                                    "float edgeVertical   = abs((lumaNW + lumaSW) - (lumaNE + lumaSE)) * 2.0 + abs((lumaWest - lumaEast) * 2.0);\n"
                                    "bool isHorizontal = (edgeHorizontal >= edgeVertical);\n"
                                    "float pixelN = isHorizontal ? lumaNorth : lumaWest;\n"
                                    "float pixelS = isHorizontal ? lumaSouth : lumaEast;\n"
                                    "float gradientScaled = max(abs(pixelN - lumaCenter), abs(pixelS - lumaCenter)) * 0.25;\n"
                                    "vec2 step = isHorizontal ? vec2(0.0, inverseScreenSize.size.y) : vec2(inverseScreenSize.size.x, 0.0);\n"
                                    "vec2 posP = inUV;\n"
                                    "vec2 dirP = isHorizontal ? vec2(0.0, 1.0) : vec2(1.0, 0.0);\n"
                                    "float lumaEndP = lumaCenter;\n"
                                    "for(int i = 0; i < fxaaValues.edgeSearchSteps; i++) {\n"
                                    "  posP += step;\n"
                                    "  float luma = rgb2luma(texture(ColorTexture, posP).rgb);\n"
                                    "  if(abs(luma - lumaCenter) > gradientScaled) break;\n"
                                    "  lumaEndP = luma;\n"
                                    "}\n"
                                    "vec2 posN = inUV;\n"
                                    "vec2 dirN = -dirP;\n"
                                    "float lumaEndN = lumaCenter;\n"
                                    "for(int i = 0; i < fxaaValues.edgeSearchSteps; i++) {\n"
                                    "    posN -= step;\n"
                                    "    float luma = rgb2luma(texture(ColorTexture, posN).rgb);\n"
                                    "    if(abs(luma - lumaCenter) > gradientScaled) break;\n"
                                    "    lumaEndN = luma;\n"
                                    "}\n"
                                    "float distP = isHorizontal ? (posP.y - inUV.y) : (posP.x - inUV.x);\n"
                                    "float distN = isHorizontal ? (inUV.y - posN.y) : (inUV.x - posN.x);\n"
                                    "bool isDirectionP = (lumaCenter - lumaEndN) >= (lumaEndP - lumaCenter);\n"
                                    "float dist = min(distP, distN);\n"
                                    "float gradient = isDirectionP ? abs(lumaCenter - lumaEndN) : abs(lumaEndP - lumaCenter);\n"
                                    "float pixelOffset = -gradient / (lumaMax - lumaMin + 1e-5);\n"
                                    "pixelOffset = clamp(pixelOffset * fxaaValues.subpixQuality + 0.5, 0.0, 1.0);\n"
                                    "vec2 finalTexCoord = inUV + (isDirectionP ? dirP : dirN) * pixelOffset * dist * inverseScreenSize.size;\n"
                                    "FragColor = vec4(texture(ColorTexture, finalTexCoord).rgb, 1.0);";
                        }
                    );
                    shaders::ShaderFactory::addHook(
                        shaders::ShaderType::Fragment,
                        "postMain",
                        "FXAA",
                        [](auto& shader, const auto& constants) -> std::string {
                            return "float rgb2luma(vec3 rgb) {\n"
                                   "  return sqrt(dot(rgb, vec3(0.299, 0.587, 0.114)));\n"
                                   "}";
                        }
                    );
                }
            };
            window.postProcessingPipeline.addStage(100.f, fxaaStageCreateInfo);
        }
    };
	return info;
}
