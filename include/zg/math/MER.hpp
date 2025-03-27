#pragma once
#include <cctype>
#include <cmath>
#include <map>
#include <set>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>
namespace zg::math
{
    #define MathematicalEquation std::string
    struct MathematicalEquationResolver
    {
        /**
         * @brief evaluates an expression, given variables, resolving to a double
         */
        static double evaluate(const std::string& expr, const std::map<char, double>& variables);
        /**
         * @brief evaluates an expression, given or not given variables, updating variables where found and resolved
         */
        static void solveEquation(const MathematicalEquation& eq, std::map<char, double>& variables);
    };
}
