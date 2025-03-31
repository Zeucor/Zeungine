#pragma once
namespace zg::phyzix
{
	struct Wavelength
	{
		static float get(const float& constant, const float& mass, const float& velocity)
		{
			return constant / (mass * velocity);
		};
	};
} // namespace zg::phyzix
