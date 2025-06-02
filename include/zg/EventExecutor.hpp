#pragma once
#include <unordered_map>
#include <any>
#include <map>
#include <functional>
#include <zg/KeyIDVector.hpp>
#include <queue>
namespace zg
{
    struct EventExecutor;
    struct Event
    {
        friend EventExecutor;
    protected:
        std::any data;
        size_t value;
        bool handled = false;
    public:
        Event(const std::any& data, size_t value):
            data(data),
            value(value)
        {};
        const std::any& getData() const
        {
            return data;
        }
        template <typename T>
        const T& castData() const
        {
            return std::any_cast<const T&>(data);
        }
        size_t getValue() const
        {
            return value;
        }
        void markHandled()
        {
            handled = true;
        }
    };
    using EventHandlerFunction = std::function<void(Event&)>;
    struct EventHandlerEntry
    {
        float priority;
        size_t ID;
        EventHandlerFunction fn;
    };
    enum EventType : size_t
    {
        EVENT_MOUSE_MOVE = 1,
        EVENT_MOUSE_PRESS,
        EVENT_MOUSE_HOVER,
        EVENT_KEY_PRESS,
        EVENT_RESIZE,
        EVENT_FOCUS,
        EVENT_SHUTDOWN,
        EVENT_CORE_MAX
    };
    struct EventExecutor
    {
    private:
        std::unordered_map<size_t, KeyIDVector<float, EventHandlerEntry>> eventHandlers;
        std::queue<std::pair<size_t, Event>> queuedEvents;
    public:
        /**
         * @brief registers an event
         * @param eventType can be any integer representing the ID of an event, use the enum EventType for zg core events
         * @param fn handler function
         * @param priority if -1 (default), priority will be set to 1+ the highest existing, or 1
         * @return size_t ID which can be used to deregister an event
         */
        size_t registerHandler(size_t eventType, const EventHandlerFunction& fn, float priority = -1.f)
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
        bool deregisterHandler(size_t eventType, size_t ID)
        {
            auto& kiv = eventHandlers[eventType];
            auto id_iter = kiv.find_id(ID);
            if (id_iter == kiv.end())
                return false;
            kiv.erase(id_iter);
            return true;
        }
        void queueEvent(size_t eventType, const std::any& data, size_t value = 0)
        {
            queuedEvents.push({ eventType, {data, value}});
        }
        void processEvents()
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
    };
}