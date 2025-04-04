#pragma once
#include <string>
#include <map>
namespace zg::math
{
    #define MathematicalEquation std::string
    struct MathematicalEquationResolver
    {
        /**
         * @brief evaluates an expression, given or not given variables, updating variables where found and resolved
         */
        static double solve(const std::string& equation, const std::map<char, double>& variables);
    };
}
