#include <zg/Serial.hpp>
#include <zg/entities/Curve.hpp>
// template <>
// Serial& serialize(Serial& serial, const std::shared_ptr<zg::entities::NDParametricCurve<3>>& curvePointer)
// {
// 	if (!curvePointer)
// 	{
// 		serial << false;
// 		return serial;
// 	}
// 	auto& curve = *curvePointer;
// 	serial << true;
// 	auto& position = curve.position;
// 	auto& rotation = curve.rotation;
// 	auto& scale = curve.scale;
// 	auto& color = curve.color;
// 	auto& constants = curve.constants;
// 	auto& tStart = curve.tStart;
// 	auto& tEnd = curve.tEnd;
// 	auto& tStep = curve.tStep;
// 	auto& radius = curve.radius;
// 	auto& vars = curve.vars;
// 	auto& equations = curve.equations;
// 	auto& name = curve.name;
// 	serial << position << rotation << scale << color;
// 	auto constantsSize = constants.size();
// 	serial << constantsSize;
// 	for (auto j = 0; j < constantsSize; j++)
// 		serial << constants[j];
// 	serial << radius;
// 	auto varsSize = vars.size();
// 	serial << varsSize;
// 	for (auto i = 1; i <= varsSize; ++i)
// 	{
// 		std::string key = 0;
// 		double value = 0;
// 		serial << key << value;
// 		vars[key] = value;
// 	}
// 	serial << tStart << tEnd << tStep;
// 	auto n = 3;
// 	serial << n;
// 	for (auto j = 0; j < n; j++)
// 	{
// 		serial << equations[j];
// 	}
// 	serial << name;
// 	return serial;
// }
// template <>
// Serial& deserialize(Serial& serial, std::shared_ptr<zg::entities::NDParametricCurve<3, float>>& curvePointer)
// {
// 	bool wroteBit = false;
// 	serial >> wroteBit;
// 	if (!wroteBit)
// 		return serial;
// 	zg::Window* windowPointer = (zg::Window*)serial.getContextPointer("Window");
// 	zg::Scene* scenePointer = (zg::Scene*)serial.getContextPointer("Scene");
// 	glm::vec3 position(0);
// 	glm::quat rotation(0, 0, 0, 0);
// 	glm::vec3 scale(0);
// 	glm::vec4 color(0);
// 	zg::shaders::RuntimeConstants constants;
// 	double tStart = 0;
// 	double tEnd = 0;
// 	double tStep = 0;
// 	float radius = 0;
// 	std::map<std::string, double> vars;
// 	std::array<std::string, 3> equations;
// 	serial >> position >> rotation >> scale >> color;
// 	auto constantsSize = constants.size();
// 	serial >> constantsSize;
// 	constants.resize(constantsSize);
// 	for (auto j = 0; j < constantsSize; j++)
// 		serial >> constants[j];
// 	serial >> radius;
// 	auto varsSize = vars.size();
// 	serial >> varsSize;
// 	for (auto i = 1; i <= varsSize; ++i)
// 	{
// 		std::string key = 0;
// 		double value = 0;
// 		serial >> key >> value;
// 		vars[key] = value;
// 	}
// 	serial >> tStart >> tEnd >> tStep;
// 	auto n = 3;
// 	serial >> n;
// 	for (auto j = 0; j < n; j++)
// 	{
// 		serial >> equations[j];
// 	}
// 	std::string name;
// 	serial >> name;
// 	curvePointer = std::make_shared<zg::entities::NDParametricCurve<3>>(*windowPointer, *scenePointer, position, rotation,
// 																																			scale, color, constants, name, radius, vars, tStart,
// 																																			tEnd, tStep, equations);
// 	return serial;
// }
