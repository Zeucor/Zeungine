#include <zg/Scene.hpp>
#include <zg/components/scenes/Physics.hpp>
using namespace zg::components::scenes;
Physics::Physics(Scene& scene) : scene(scene) {}
void Physics::onAttached() {}
void Physics::onUpdate() {}
void Physics::onDetached() {}
