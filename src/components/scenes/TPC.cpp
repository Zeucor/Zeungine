#include <zg/components/scenes/TPC.hpp>
using namespace zg::components::scenes;
TPC::TPC(Scene& scene, const std::string &castaddr):
    scene(scene),
    castaddr(castaddr)
{
    return;
}