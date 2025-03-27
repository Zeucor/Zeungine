#include <zg/math/MER.hpp>
using namespace zg::math;
double MathematicalEquationResolver::evaluate(const std::string& expr, const std::map<char, double>& variables)
{
    std::istringstream input(expr);
    std::stack<double> values;
    std::stack<char> ops;
    std::unordered_map<char, int> precedence{{'+', 1}, {'-', 1}, {'*', 2}, {'/', 2}, {'^', 3}};

    auto apply_op = [](double a, double b, char op) -> double
    {
        switch (op)
        {
        case '+':
            return a + b;
        case '-':
            return a - b;
        case '*':
            return a * b;
        case '/':
            if (b == 0)
                throw std::runtime_error("Division by zero");
            return a / b;
        case '^':
            return std::pow(a, b);
        default:
            throw std::runtime_error("Unknown operator");
        }
    };

    auto process = [&]()
    {
        while (!ops.empty() && values.size() >= 2)
        {
            double b = values.top();
            values.pop();
            double a = values.top();
            values.pop();
            char op = ops.top();
            ops.pop();
            values.push(apply_op(a, b, op));
        }
    };

    while (input)
    {
        if (std::isdigit(input.peek()) || input.peek() == '.')
        {
            double val;
            input >> val;
            values.push(val);
        }
        else if (std::isspace(input.peek()))
        {
            input.get();
        }
        else if (std::isalpha(input.peek()))
        {
            char var = input.get();
            if (variables.find(var) == variables.end())
                throw std::runtime_error("Unknown variable");
            values.push(variables.at(var));
        }
        else if (input.peek() == '(')
        {
            ops.push(input.get());
        }
        else if (input.peek() == ')')
        {
            input.get();
            while (!ops.empty() && ops.top() != '(')
            {
                process();
            }
            if (!ops.empty() && ops.top() == '(')
                ops.pop();
        }
        else
        {
            char op = input.get();
            while (!ops.empty() && precedence[ops.top()] >= precedence[op])
            {
                process();
            }
            ops.push(op);
        }
    }
    process();
    return values.empty() ? 0 : values.top();
}

void MathematicalEquationResolver::solveEquation(const MathematicalEquation& eq, std::map<char, double>& variables)
{
    size_t equalsPos = eq.find('=');
    if (equalsPos == std::string::npos)
        throw std::runtime_error("Equation must contain '='");

    std::string lhs = eq.substr(0, equalsPos);
    std::string rhs = eq.substr(equalsPos + 1);

    std::set<char> foundVars;

    for (char c : eq)
    {
        if (std::isalpha(c))
            foundVars.insert(c);
    }
    if (lhs.size() == 1 && std::isalpha(lhs[0]))
    {
        variables[lhs[0]] = evaluate(rhs, variables);
        return;
    }

    double rhsValue = evaluate(rhs, variables);
    double lhsValue = evaluate(lhs, variables);

    std::map<char, double> solution;
    for (char var : foundVars)
    {
        if (lhs.find(var) != std::string::npos)
        {
            double coeff = evaluate(lhs, {{var, 1}}) - evaluate(lhs, {{var, 0}});
            if (coeff == 0)
                throw std::runtime_error("Cannot solve equation for variable");
            solution[var] = (rhsValue - evaluate(lhs, variables)) / coeff;
        }
    }
    variables = solution;
}