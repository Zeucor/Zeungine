#pragma once
#include <zg/Entity.hpp>
#include <zg/Scene.hpp>
#include <zg/glm.hpp>
#include "SceneComponent.hpp"
#include <zg/vp/View.hpp>
namespace zg
{
	struct Entity;
}
namespace zg::components::scenes
{
	components::scenes::SceneComponentCreateInfo EntityThirdPersonCameraFactory(Entity& entity);
} // namespace zg::components::scenes
