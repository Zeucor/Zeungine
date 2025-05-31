#pragma once
#include <cstdlib>
#include <functional>
#include <utility>
#include <iostream>
#include <map>
#include <mutex>
namespace zg
{
    struct default_noop
    {
        template <typename T>
        void operator()(T* p) const
        {
        }
    };
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
        using ObserveFunction = std::function<void(const T&, const T&)>;
        using ObserveMap = std::map<size_t, ObserveFunction>;
    private:
        T* old_ptr = 0;
        T* ptr = 0;
        size_t* ref_c = 0;
        std::recursive_mutex* mutex = 0;
        Deleter deleter;
        size_t* observe_count = 0;
        ObserveMap* observe_map = 0;

    public:
        observable_ptr() :
            old_ptr(new T()),
            ptr(new T()),
            ref_c(new size_t(1)),
            mutex(new std::recursive_mutex()),
            observe_count(new size_t(0)),
            observe_map(new ObserveMap())
        { }
        observable_ptr(bool create_from_value, const T& value) :
            old_ptr(new T()),
            ptr(new T(value)),
            ref_c(new size_t(1)),
            mutex(new std::recursive_mutex()),
            observe_count(new size_t(0)),
            observe_map(new ObserveMap())
        { }
        observable_ptr(T* external_ptr) :
            old_ptr(new T()),
            ptr(external_ptr),
            ref_c(new size_t(1)),
            mutex(new std::recursive_mutex()),
            observe_count(new size_t(0)),
            observe_map(new ObserveMap())
        { }
        observable_ptr(const observable_ptr& other) :
            old_ptr(other.old_ptr),
            ptr(other.ptr),
            ref_c(other.ref_c),
            mutex(other.mutex),
            observe_count(other.observe_count),
            observe_map(other.observe_map)
        {
            // std::lock_guard lock(*mutex);
            if (ref_c)
            {
                (*ref_c)++;
            }
        }
        ~observable_ptr()
        {
            bool dp = false;
            {
                // std::lock_guard lock(*mutex);
                if (ref_c && --(*ref_c) == 0)
                {
                    if (ptr)
                    {
                        Deleter{}(ptr);
                    }
                    if (old_ptr)
                    {
                        Deleter{}(old_ptr);
                    }
                    delete ref_c;
                    ref_c = 0;
                    ptr = 0;
                    old_ptr = 0;
                    delete observe_count;
                    observe_count = 0;
                    delete observe_map;
                    observe_map = 0;
                    dp = true;
                }
            }
            if (dp)
            {
                delete mutex;
                mutex = 0;
            }
        }
        observable_ptr& operator=(const observable_ptr& other) {
            if (this != &other)
            {
                // std::lock_guard lock(*other.mutex);
                {
                    bool dp = false;
                    {
                        // std::lock_guard lock(*mutex);
                        if (ref_c && --(*ref_c) == 0)
                        {
                            if (old_ptr)
                            {
                                Deleter{}(old_ptr);
                            }
                            if (ptr)
                            {
                                Deleter{}(ptr);
                            }
                            delete ref_c;
                            delete observe_count;
                            delete observe_map;
                            dp = true;
                        }
                    }
                    if (dp)
                        delete mutex;
                }
                old_ptr = other.old_ptr;
                ptr = other.ptr;
                ref_c = other.ref_c;
                deleter = other.deleter;
                mutex = other.mutex;
                observe_count = other.observe_count;
                observe_map = other.observe_map;
                if (ref_c)
                {
                    (*ref_c)++;
                }
            }
            return *this;
        }
        T& operator=(const T& other_value)
        {
            auto& old_ptr_ref = *old_ptr;
            auto& ptr_ref = *ptr;
            if (other_value == ptr_ref)
                return ptr_ref;
            // std::lock_guard lock(*mutex);
            old_ptr_ref = ptr_ref;
            ptr_ref = other_value;
            notify(old_ptr_ref, ptr_ref);
            return ptr_ref;
        }
        bool operator==(const observable_ptr& other) { return ptr == other.ptr; }
        bool operator!=(const observable_ptr& other) { return ptr != other.ptr; }
        bool operator==(const T& other) { return *ptr == other; }
        bool operator!=(const T& other) { return *ptr != other; }
        T* operator->() const { return ptr; }
        T& operator*() const { return *ptr; }
        bool empty() const { return ptr == 0; }
        explicit operator const T&() const { return *ptr; }
        explicit operator T&() { return *ptr; }
        size_t observe(const ObserveFunction& observer, bool call_instantly = false)
        {
            // std::lock_guard lock(*mutex);
            auto id = ++*observe_count;
            observe_map->emplace(id, observer);
            if (call_instantly)
            {
                notify(*ptr, *ptr);
            }
            return id;
        }
        bool remove_observer(size_t id)
        {
            // std::lock_guard lock(*mutex);
            auto& observe_map_ref = *observe_map;
            auto iter = observe_map_ref.find(id);
            if (iter == observe_map_ref.end())
            {
                return false;
            }
            observe_map_ref.erase(iter);
            return true;
        }
        void notify()
        {
            notify(*old_ptr, *ptr);
        }
        private:
        void notify(const T& old_value, const T& new_value)
        {
            // std::lock_guard lock(*mutex);
            auto& observe_map_ref = *observe_map;
            if (observe_map_ref.size() == 0)
                return;
            for (auto& pair : observe_map_ref)
            {
                pair.second(old_value, new_value);
            }
        }
    };
}
