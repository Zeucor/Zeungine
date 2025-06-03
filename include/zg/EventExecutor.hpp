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
        Event(const std::any& data, size_t value);
        const std::any& getData() const;
        template <typename T>
        const T& castData() const
        {
            return std::any_cast<const T&>(data);
        }
        size_t getValue() const;
        void markHandled();
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
        EventExecutor() = default;;
        EventExecutor(const EventExecutor& other):
            eventHandlers(other.eventHandlers),
            queuedEvents(other.queuedEvents)
        {}
        EventExecutor& operator=(const EventExecutor& other)
        {
            eventHandlers = other.eventHandlers;
            queuedEvents = other.queuedEvents;
            return *this;
        }
        /**
         * @brief registers an event
         * @param eventType can be any integer representing the ID of an event, use the enum EventType for zg core events
         * @param fn handler function
         * @param priority if -1 (default), priority will be set to 1+ the highest existing, or 1
         * @return size_t ID which can be used to deregister an event
         */
        size_t registerHandler(size_t eventType, const EventHandlerFunction& fn, float priority = -1.f);
        bool deregisterHandler(size_t eventType, size_t ID);
        void queueEvent(size_t eventType, const std::any& data, size_t value = 0);
        void processEvents();
    };
}