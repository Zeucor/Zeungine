#include <anex/modules/fenster/Fenster.hpp>
using namespace anex::modules::fenster;
int main()
{
  FensterGame game(640, 480);
  game.awaitWindowThread();
};