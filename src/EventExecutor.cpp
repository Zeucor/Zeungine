#include <zg/EventExecutor.hpp>
using namespace zg;
Event::Event(const std::any& data, size_t value):
    data(data),
    value(value)
{};
const std::any& Event::getData() const
{
    return data;
}
size_t Event::getValue() const
{
    return value;
}
void Event::markHandled()
{
    handled = true;
}
size_t EventExecutor::registerHandler(size_t eventType, const EventHandlerFunction& fn, float priority)
{
    auto& kiv = eventHandlers[eventType];
    auto transaction = kiv.startTransaction();
    if (priority == -1.f)
    {
        auto end = kiv.key_end();
        if (end == kiv.key_begin())
        {
            priority = 1.f;
        }
        else
        {
            end = end - 1;
            priority = end.key() + 1.f;
        }
    }
    kiv.commitTransactionKey(transaction, priority, priority, transaction.id, fn);
    return transaction.id;
}
bool EventExecutor::deregisterHandler(size_t eventType, size_t ID)
{
    auto& kiv = eventHandlers[eventType];
    auto id_iter = kiv.find_id(ID);
    if (id_iter == kiv.end())
        return false;
    kiv.erase(id_iter);
    return true;
}
void EventExecutor::queueEvent(size_t eventType, const std::any& data, size_t value)
{
    queuedEvents.push({ eventType, {data, value}});
}
void EventExecutor::processEvents()
{
    while (!queuedEvents.empty())
    {
        auto queuedEvent = queuedEvents.front();
        queuedEvents.pop();
        auto kiv_iter = eventHandlers.find(queuedEvent.first);
        if (kiv_iter == eventHandlers.end())
            continue;
        auto& kiv = kiv_iter->second;
        auto end = kiv.key_end();
        for (auto iter = kiv.key_begin(); iter != end; ++iter)
        {
            iter->fn(queuedEvent.second);
            if (queuedEvent.second.handled)
                break;
        }
        continue;
    }
}