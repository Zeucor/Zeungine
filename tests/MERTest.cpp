#include <zg/Logger.hpp>
#include <zg/math/MER.hpp>
using namespace zg::math;
int main()
{
	MathematicalEquation eq("x=2/1/2/2");
	MathematicalEquation ez("y=3*x");
    std::map<char, double> eqvars;
	MathematicalEquationResolver::solveEquation(eq, eqvars);
    std::map<char, double> ezvars{{'x', 3}};
	MathematicalEquationResolver::solveEquation(ez, ezvars);
	zg::Logger::print(zg::Logger::Blank, "eq:");
	for (auto& pair : eqvars)
		zg::Logger::print(zg::Logger::Blank, pair.first, ": ", pair.second);
	zg::Logger::print(zg::Logger::Blank, "ez:");
	for (auto& pair : ezvars)
		zg::Logger::print(zg::Logger::Blank, pair.first, ": ", pair.second);
}
