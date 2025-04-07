#include <zg/Standard.hpp>
#include <zg/Serial.hpp>
#include <zg/interfaces/ISceneComponent.hpp>
namespace zg::components::scenes
{
    struct TPC : zg::interfaces::ISceneComponent
    {
        Scene& scene;
        std::string castaddr;
        TPC(Scene& scene, const std::string &castaddr);
		void onAttached();
		void onUpdate();
		void onDetached();
    };
}