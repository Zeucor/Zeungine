#pragma once
#include <functional>
template <typename T>
using FunctionX = std::function<T(const T&)>;
template <typename T>
T ForwardDifference(const FunctionX<T>& f, const T& x, const T& h = 1e-5f)
{
	return (f(x + h) - f(x)) / h;
}
template <typename T>
T BackwardDifference(const FunctionX<T>& f, const T& x, const T& h = 1e-5f)
{
	return (f(x) - f(x - h)) / h;
}
template <typename T>
T CentralDifference(const FunctionX<T>& f, const T& x, const T& h = 1e-5f)
{
	return (f(x + h) - f(x - h)) / (2 * h);
}
