#include <zg/Logger.hpp>
#include <zg/math/MER.hpp>
using namespace zg::math;
int main()
{
	MathematicalEquation eq_x("t");
	MathematicalEquation eq_y("sin(t)");
	MathematicalEquation eq_z("cos(t)");
	std::map<char, double> vars({{'t', 0}});
	for (double& t = vars['t']; t < 7; t += 0.25)
	{
		zg::Logger::print(zg::Logger::Blank, "x=", MathematicalEquationResolver::solve(eq_x, vars), "y=", MathematicalEquationResolver::solve(eq_y, vars), "z=", MathematicalEquationResolver::solve(eq_z, vars));
	}
}
