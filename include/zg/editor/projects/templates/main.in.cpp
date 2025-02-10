#include <main.hpp>
MainScene::MainScene(Window& window):
	Scene(window, {0, 50, 50}, {0, -1, -1}, 80.f)
{
	clearColor = {0.5, 0, 0.5, 1};
}
ZG_API void OnLoad(Window& window)
{
	window.setScene(std::make_shared<MainScene>(window));
}
ZG_API void OnHotswapLoad(Window& window, hscpp::AllocationResolver &allocationResolver)
{
	window.setScene(std::shared_ptr<MainScene>(allocationResolver.Allocate<MainScene>(window)));
}