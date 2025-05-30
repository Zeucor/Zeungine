#pragma once
#include <zg/glm.hpp>
namespace zg
{

	template <typename T>
	T lerp(const T& start, const T& finish, float t)
	{
		return start + (finish - start) * t;
	}

	namespace Easing
	{
		float linear(float t) { return t; }

		float easeInQuad(float t) { return t * t; }

		float easeOutQuad(float t) { return 1.0f - (1.0f - t) * (1.0f - t); }

		float easeInOutQuad(float t) { return t < 0.5f ? 2.0f * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) / 2.0f; }

		float easeInCubic(float t) { return t * t * t; }

		float easeOutCubic(float t) { return 1.0f - std::pow(1.0f - t, 3.0f); }

		float easeInOutCubic(float t)
		{
			return t < 0.5f ? 4.0f * t * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 3.0f) / 2.0f;
		}

		float easeInSine(float t) { return 1.0f - std::cos(t * ZG_PI_2); }

		float easeOutSine(float t) { return std::sin(t * ZG_PI_2); }

		float easeInOutSine(float t) { return -(std::cos(ZG_PI * t) - 1.0f) / 2.0f; }
	} // namespace Easing

	template <typename T, typename EasingFunc>
	T TimingFunction(const T& start, const T& finish, float t, EasingFunc easing)
	{
		float eased_t = easing(t);
		return lerp<T>(start, finish, eased_t);
	}

	template <typename W, typename T, typename EasingFunc>
	std::shared_ptr<bool> StartTimingFunction(W& window, const T& start, const T& finish, T* value_pointer, float start_t, float duration_seconds, EasingFunc easing)
	{
		std::shared_ptr<std::function<void(Window&)>> timingRunnable = std::make_shared<std::function<void(Window&)>>();
		std::shared_ptr<float> duration_seconds_current = std::make_shared<float>(0.f);
		std::shared_ptr<bool> runningBool = std::make_shared<bool>(true);
		*timingRunnable = [runningBool, timingRunnable, start, finish, start_t, duration_seconds_current, duration_seconds, value_pointer, easing](auto& window) {
			if (!*runningBool)
			{
				return;
			}
			auto deltaTime = *window.lastFrameDeltaTime;
			// std::cout << "deltaTime: " << *window.deltaTime << ", lastFrameDeltaTime: " << *window.lastFrameDeltaTime << std::endl;
			auto& duration_seconds_current_ref = *duration_seconds_current;
			auto t = duration_seconds_current_ref / duration_seconds;
			*value_pointer = TimingFunction<T>(start, finish, t, easing);
			duration_seconds_current_ref += deltaTime;
			if (duration_seconds_current_ref > duration_seconds)
			{
				t = 1.0f;
				*value_pointer = TimingFunction(start, finish, t, easing);
				duration_seconds_current_ref = duration_seconds;
				*runningBool = false;
			}
			else
			{
				window.runOnThread(*timingRunnable);
			}
		};
		window.runOnThread(*timingRunnable);
		return runningBool;
	}
} // namespace zg
