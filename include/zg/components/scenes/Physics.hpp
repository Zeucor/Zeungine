#pragma once
#include <zg/interfaces/ISceneComponent.hpp>
namespace zg
{
    struct Scene;
}
namespace zg::components::scenes
{
    struct Physics : interfaces::ISceneComponent
    {
        Scene& scene;
        Physics(Scene& scene);
        void onAttached() override;
		void onUpdate() override;
        void onDetached() override;
    };
}