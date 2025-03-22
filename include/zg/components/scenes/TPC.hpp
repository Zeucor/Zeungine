#include <zg/Standard.hpp>
#include <zg/Serial.hpp>
#include <zg/interfaces/ISceneComponent.hpp>
namespace zg::components::scenes
{
    struct TPC : zg::interfaces::ISceneComponent
    {
        std::string castaddr;
        TPC(const std::string &castaddr);
		void onUpdate(zg::Scene& scene);
		void onAdded(zg::Scene& scene);
		void onRemoved(zg::Scene& scene);
    };
}