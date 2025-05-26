#pragma once
#include <cstdlib>
#include <functional>
#include <utility>
#include <iostream>
#include <map>
#include <mutex>
namespace zg
{
    struct default_delete_single
    {
        template <typename T>
        void operator()(T* p) const
        {
            delete p;
        }
    };
    struct default_delete_array
    {
        template <typename T>
        void operator()(T* p) const
        {
            delete[] p;
        }
    };
    struct default_free_deleter
    {
        template <typename T>
        void operator()(T* p) const
        {
            free(static_cast<void*>(p));
        }
    };
    template <typename T, typename Deleter = default_delete_single>
    struct observable_ptr
    {
    private:
        T* ptr = 0;
        size_t* ref_c = 0;
        std::mutex* mutex = 0;
        Deleter deleter;
        size_t observe_count = 0;
        std::map<size_t, std::function<void(const T&, const T&)>> observe_map;

    public:
        observable_ptr() :
            ptr(new T()),
            ref_c(new size_t(1)),
            mutex(new std::mutex()),
            deleter(Deleter{})
        { }
        observable_ptr(T* external_ptr, Deleter custom_deleter_instance = Deleter{}) :
            ptr(external_ptr),
            ref_c(new size_t(1)),
            mutex(new std::mutex()),
            deleter(custom_deleter_instance)
        { }
        observable_ptr(const observable_ptr& other) :
            ptr(other.ptr),
            ref_c(other.ref_c),
            mutex(other.mutex),
            deleter(other.deleter)
        {
            std::lock_guard lock(*mutex);
            if (ref_c) {
                (*ref_c)++;
            }
        }
        ~observable_ptr()
        {
            bool dp = false;
            std::lock_guard lock(*mutex);
            if (ref_c && --(*ref_c) == 0) {
                if (ptr) {
                    deleter(ptr);
                }
                delete ref_c;
                ref_c = 0;
                ptr = 0;
                dp = true;
            }
            if (dp)
            {
                delete mutex;
                mutex = 0;
            }
        }
        observable_ptr& operator=(const observable_ptr& other) {
            if (this != &other) {
                std::lock_guard lock(*other.mutex);
                if (ref_c && --(*ref_c) == 0) {
                    if (ptr) {
                        deleter(ptr);
                    }
                    delete ref_c;
                }
                ptr = other.ptr;
                ref_c = other.ref_c;
                deleter = other.deleter;
                mutex = other.mutex;
                if (ref_c) {
                    (*ref_c)++;
                }
            }
            return *this;
        }
        T& operator=(const T& other_value)
        {
            auto& ref = *ptr;
            auto old = ref;
            {
                std::lock_guard lock(*mutex);
                ref = other_value;
            }
            notify(old, ref);
            return ref;
        }
        bool operator==(const observable_ptr& other) { return ptr == other.ptr; }
        bool operator!=(const observable_ptr& other) { return ptr != other.ptr; }
        bool operator==(const T& other) { return *ptr == other; }
        bool operator!=(const T& other) { return *ptr != other; }
        T* operator->() const { return ptr; }
        T& operator*() const { return *ptr; }
        explicit operator bool() const { return ptr != 0; }
        size_t observe(std::function<void(const T&, const T&)>& observer)
        {
            std::lock_guard lock(*mutex);
            auto id = ++observe_count;
            observe_map[id] = observer;
            return id;
        }
        bool remove_observer(size_t id)
        {
            std::lock_guard lock(*mutex);
            auto iter = observe_map.find(id);
            if (iter == observe_map.end())
            {
                return false;
            }
            observe_map.erase(iter);
            return true;
        }
        private:
        void notify(const T& old_value, const T& new_value)
        {
            std::lock_guard lock(*mutex);
            for (auto& pair : observe_map)
            {
                pair.second(old_value, new_value);
            }
        }
    };
}
