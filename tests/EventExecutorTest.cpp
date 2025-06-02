#include <zg/EventExecutor.hpp>
#include <iostream>
using namespace zg;
int main()
{
    EventExecutor ee;
    ee.registerHandler(EVENT_FOCUS, [](auto& event) {
        auto focused = event.template castData<bool>();
        if (focused)
        {
            std::cout << "5.f focused" << std::endl;
            event.markHandled();
        }
    }, 5.f);
    ee.registerHandler(EVENT_FOCUS, [](auto& event) {
        auto focused = event.template castData<bool>();
        if (!focused)
        {
            std::cout << "5.5f not focused" << std::endl;
            // event.markHandled();
        }
    }, 5.5f);
    ee.registerHandler(EVENT_FOCUS, [](auto& event) {
        auto focused = event.template castData<bool>();
        std::cout << "focused: " << focused << std::endl;
    });
    ee.queueEvent(EVENT_FOCUS, true);
    ee.queueEvent(EVENT_FOCUS, false);
    ee.queueEvent(EVENT_FOCUS, true);
    ee.processEvents();
    return 0;
}