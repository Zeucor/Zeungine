#pragma once
#include <string>
#include <map>
const float VEC_EPSILON = 1e-5f;             // Small epsilon for floating point comparisons
const float VEC_EPSILON_SQ = VEC_EPSILON * VEC_EPSILON; // Squared epsilon for length checks
namespace zg::math
{
    #define MathematicalEquation std::string
    struct MathematicalEquationResolver
    {
        /**
         * @brief evaluates an expression, given or not given variables, updating variables where found and resolved
         */
        static double solve(const std::string& equation, const std::map<std::string, double>& variables);
    };
}
