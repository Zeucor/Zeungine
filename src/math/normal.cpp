#include <zg/math/normal.hpp>
const long double zg::math::normalizedMinMaxValue(const long double& min, const long double& value, const long double& max)
{
    if (min == max)
    {
        if (value > max)
        {
            return 1;
        }
        else if (value < min)
        {
            return 0;
        }
    }
    return (value - min) / (max - min);
}