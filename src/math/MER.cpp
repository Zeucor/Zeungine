#include <zg/math/MER.hpp>
#include <exprtk.hpp>
using namespace zg::math;
exprtk::parser<double> parser;
double MathematicalEquationResolver::solve(const std::string& equation, const std::map<std::string, double>& variables)
{
	exprtk::symbol_table<double> symbol_table;
	for (auto& pair : variables)
		symbol_table.add_variable(pair.first, (double&)pair.second);
	exprtk::expression<double> expression;
	expression.register_symbol_table(symbol_table);
	if (!parser.compile(equation, expression))
	{
		throw std::runtime_error("Could not compile equation");
	}
	return expression.value();
}

// double MathematicalEquationResolver::evaluate(const std::string& expr, const std::map<char, double>& variables)
// {
// 	std::istringstream input(expr);
// 	std::stack<double> values;
// 	std::stack<char> ops;
// 	std::unordered_map<char, int> precedence{{'+', 1}, {'-', 1}, {'*', 2}, {'/', 2}, {'^', 3}};

// 	auto apply_op = [&](double a, double b, char op) -> double
// 	{
// 		switch (op)
// 		{
// 		case '+':
// 			return a + b;
// 		case '-':
// 			return a - b;
// 		case '*':
// 			return a * b;
// 		case '/':
// 			if (b == 0)
// 				throw std::runtime_error("Division by zero");
// 			return a / b;
// 		case '^':
// 			return std::pow(a, b);
// 		default:
// 			throw std::runtime_error("Unknown operator");
// 		}
// 	};

// 	auto process = [&]()
// 	{
// 		while (!ops.empty() && values.size() >= 2)
// 		{
// 			double b = values.top();
// 			values.pop();
// 			double a = values.top();
// 			values.pop();
// 			char op = ops.top();
// 			ops.pop();
// 			values.push(apply_op(a, b, op));
// 		}
// 	};

// 	while (input)
// 	{
// 		if (std::isdigit(input.peek()) || input.peek() == '.')
// 		{
// 			double val;
// 			input >> val;
// 			values.push(val);
// 		}
// 		else if (std::isspace(input.peek()))
// 		{
// 			input.get();
// 		}
// 		else if (std::isalpha(input.peek()))
// 		{
// 			char var = input.get();
// 			if (variables.find(var) == variables.end())
// 				throw std::runtime_error("Unknown variable: " + std::string(1, var));
// 			values.push(variables.at(var));
// 		}
// 		else if (input.peek() == '(')
// 		{
// 			ops.push(input.get());
// 		}
// 		else if (input.peek() == ')')
// 		{
// 			input.get();
// 			while (!ops.empty() && ops.top() != '(')
// 				process();
// 			if (!ops.empty() && ops.top() == '(')
// 				ops.pop();
// 		}
// 		else
// 		{
// 			char op = input.get();
// 			while (!ops.empty() && precedence[ops.top()] >= precedence[op])
// 				process();
// 			ops.push(op);
// 		}
// 	}
// 	process();
// 	return values.empty() ? 0 : values.top();
// }

// std::map<char, std::vector<std::complex<double>>>
// MathematicalEquationResolver::solveEquation(const MathematicalEquation& eq, std::map<char, double>& variables)
// {
// 	size_t equalsPos = eq.find('=');
// 	if (equalsPos == std::string::npos)
// 		throw std::runtime_error("Equation must contain '='");

// 	std::string lhs = eq.substr(0, equalsPos);
// 	std::string rhs = eq.substr(equalsPos + 1);

// 	std::set<char> allVars;
// 	for (char c : eq)
// 		if (std::isalpha(c))
// 			allVars.insert(c);

// 	std::map<char, std::vector<std::complex<double>>> solutions;
// 	double rhsValue = evaluate(rhs, variables);

// 	for (char var : allVars)
// 	{
// 		std::vector<double> coeffs;
// 		int maxPower = allVars.size();
// 		for (int power = 1; power <= maxPower; ++power)
// 		{
// 			std::map<char, double> tempVars = variables;
// 			tempVars[var] = power;
// 			coeffs.push_back(evaluate(lhs, tempVars));
// 		}
// 		double a = 0, b = 0, c = 0;
// 		if (coeffs.size() == 3)
// 		{
// 			a = (coeffs[2] - 2 * coeffs[1] + coeffs[0]) / 2;
// 			b = coeffs[1] - coeffs[0] - 2 * a;
// 			c = coeffs[0] - rhsValue;
// 		}
//         else if (coeffs.size() == 2)
//         {
//             // a = 
//             // b = 
//         }
//         else if (coeffs.size() == 1)
//         {
//             a = rhsValue;
//         }

// 		if (coeffs.size() == 3)
// 		{
// 			if (a == 0 && b == 0)
// 			{
// 				if (c != 0)
// 					solutions[var] = {};
// 				else
// 					solutions[var] = {0};
// 			}
// 			else if (a == 0)
// 			{
// 				if (b != 0)
// 					solutions[var] = {-c / b};
// 				else
// 					solutions[var] = {};
// 			}
// 			else
// 			{
// 				double discriminant = b * b - 4 * a * c;
// 				if (discriminant < 0)
// 					solutions[var] = {std::complex<double>(-b, std::sqrt(-discriminant)) / (2 * a),
// 														std::complex<double>(-b, -std::sqrt(-discriminant)) / (2 * a)};
// 				else if (discriminant == 0)
// 					solutions[var] = {-b / (2 * a)};
// 				else
// 					solutions[var] = {(-b + std::sqrt(discriminant)) / (2 * a), (-b - std::sqrt(discriminant)) / (2 * a)};
// 			}
// 		}
// 		else if (coeffs.size() == 2)
// 		{
// 			if (coeffs[1] == 0)
// 			{
// 				if (coeffs[0] == 0)
// 					solutions[var] = {0};
// 				else
// 					solutions[var] = {};
// 			}
// 			else
// 			{
// 				solutions[var] = {-coeffs[0] / coeffs[1]};
// 			}
// 		}
// 		else if (coeffs.size() == 1)
// 		{
// 			solutions[var] = {a};
// 		}
// 		else
// 		{
// 			solutions[var] = {};
// 		}
// 	}
// 	return solutions;
// }
