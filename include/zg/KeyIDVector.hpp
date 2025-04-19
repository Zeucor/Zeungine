#pragma once
#include <unordered_map>
#include <map>
#include <vector>
#include <mutex>
#include <stdexcept>
#include <functional>
#include <string>
namespace zg
{
    #define KEY_ID_VECTOR_KEY_INDEX 0
    #define KEY_ID_VECTOR_ID_INDEX 1
    #define KEY_ID_VECTOR_INDEX_INDEX 2
    #define KEY_ID_VECTOR_VALUE_INDEX 3
    template<typename KeyT, typename ValueT>
    struct KeyIDVector
    {
        using EmplaceBackTuple = std::tuple<KeyT, size_t, size_t*, ValueT*>;
        using GetKeyFunction = std::function<KeyT(const ValueT&)>;
    private:
        size_t m_Count = 0;
        std::unordered_map<KeyT, size_t*> m_KeyIndexMap;
        std::unordered_map<size_t, KeyT*> m_IDKeyMap;
        std::map<size_t, size_t> m_IDIndexMap;
        std::vector<ValueT> m_Values;
        std::shared_ptr<std::recursive_mutex> m_Mutex;
        GetKeyFunction m_GetKeyFunction;
    public:
        KeyIDVector(const GetKeyFunction& getKeyFunction = {}):
            m_Mutex(std::make_shared<std::recursive_mutex>()),
            m_GetKeyFunction(getKeyFunction)
        {};
        KeyIDVector& operator=(const KeyIDVector& other)
        {
            m_Count = other.m_Count;
            m_KeyIndexMap = other.m_KeyIndexMap;
            m_IDKeyMap = other.m_IDKeyMap;
            m_IDIndexMap = other.m_IDIndexMap;
            m_Values = other.m_Values;
            m_Mutex = std::make_shared<std::recursive_mutex>();
            m_GetKeyFunction = other.m_GetKeyFunction;
            return *this;
        }
        ValueT* data()
        {
            return m_Values.data();
        }
        const ValueT* data() const
        {
            return m_Values.data();
        }
        size_t size()
        {
            return m_Values.size();
        }
        const size_t size() const
        {
            return m_Values.size();
        }
        ValueT& operator[](const KeyT& key)
        {
            std::lock_guard lock(*m_Mutex);
            auto key_iter = m_KeyIndexMap.find(key);
            if (key_iter == m_KeyIndexMap.end())
            {
                return constructDefault(key);
            }
            auto& indexRef = *key_iter->second;
            return m_Values[indexRef];
        }
        std::vector<ValueT>::iterator begin()
        {
            return m_Values.begin();
        }
        const std::vector<ValueT>::const_iterator begin() const
        {
            return m_Values.begin();
        }
        std::vector<ValueT>::iterator end()
        {
            return m_Values.end();
        }
        const std::vector<ValueT>::const_iterator end() const
        {
            return m_Values.end();
        }
        std::vector<ValueT>::iterator find_key(const KeyT& key)
        {
            std::lock_guard lock(*m_Mutex);
            auto key_iter = m_KeyIndexMap.find(key);
            if (key_iter == m_KeyIndexMap.end())
            {
                return end();
            }
            auto& indexRef = *key_iter->second;
            return m_Values.begin() + indexRef;
        }
        const std::vector<ValueT>::const_iterator find_key(const KeyT& key) const
        {
            std::lock_guard lock(*m_Mutex);
            auto key_iter = m_KeyIndexMap.find(key);
            if (key_iter == m_KeyIndexMap.end())
            {
                return end();
            }
            auto& indexRef = *key_iter->second;
            return m_Values.begin() + indexRef;
        }
        std::vector<ValueT>::iterator find_id(size_t id)
        {
            std::lock_guard lock(*m_Mutex);
            auto id_iter = m_IDIndexMap.find(id);
            if (id_iter == m_IDIndexMap.end())
            {
                return end();
            }
            auto& index = id_iter->second;
            return m_Values.begin() + index;
        }
        const std::vector<ValueT>::const_iterator find_id(size_t id) const
        {
            std::lock_guard lock(*m_Mutex);
            auto id_iter = m_IDIndexMap.find(id);
            if (id_iter == m_IDIndexMap.end())
            {
                return end();
            }
            auto& index = id_iter->second;
            return m_Values.begin() + index;
        }
        KeyT getKey(std::vector<ValueT>::iterator iter)
        {
            auto length = iter - begin();
            for (auto& keyIndexPair : m_KeyIndexMap)
            {
                auto& index = *keyIndexPair.second;
                if (index == length)
                {
                    return keyIndexPair.first;
                }
            }
            throw std::runtime_error("key not found with iter index [" + std::to_string(length) + "]");
        }
        template<typename... Args>
        EmplaceBackTuple emplace_back(const Args&... args)
        {
            std::lock_guard lock(*m_Mutex);
            auto id = ++m_Count;
            auto index = m_Values.size();
            m_Values.emplace_back(args...);
            auto& indexRef = (m_IDIndexMap[id] = index);
            auto& value = m_Values[index];
            if (m_GetKeyFunction)
            {
                auto key = m_GetKeyFunction(value);
                m_KeyIndexMap[key] = &indexRef;
                return {key, id, &indexRef, &value};
            }
            throw std::runtime_error("GetKeyFunction is not set and emplace_back called without key");
        }
        template<typename... Args>
        EmplaceBackTuple emplace_back_key(const KeyT& key, const Args&... args)
        {
            std::lock_guard lock(*m_Mutex);
            auto id = ++m_Count;
            auto index = m_Values.size();
            m_Values.emplace_back(args...);
            auto& indexRef = (m_IDIndexMap[id] = index);
            auto& value = m_Values[index];
            m_KeyIndexMap[key] = &indexRef;
            return {key, id, &indexRef, &value};
        }
        std::vector<ValueT>::iterator erase(const std::vector<ValueT>::iterator& iter)
        {
            auto index = iter - begin();
            auto nextIter = m_Values.erase(iter);
            auto IDIndexMapEnd = m_IDIndexMap.end();
            for (auto idIndexIter = m_IDIndexMap.begin(); idIndexIter != IDIndexMapEnd;)
            {
                auto& index = idIndexIter->second;
                auto& id = idIndexIter->first;
                if (index > index)
                {
                    index--;
                }
                if (index == index)
                {
                    auto idKeyIter = m_IDKeyMap.find(id);
                    if (idKeyIter == m_IDKeyMap.end())
                    {
                        throw std::runtime_error("IDKey entry not found");
                    }
                    auto& key = *idKeyIter->second;
                    m_KeyIndexMap.erase(key);
                    m_IDKeyMap.erase(idKeyIter);
                    idIndexIter = m_IDIndexMap.erase(idIndexIter);
                    IDIndexMapEnd = m_IDIndexMap.end();
                }
                if (idIndexIter == IDIndexMapEnd)
                    break;
                idIndexIter++;
            }
            return nextIter;
        }
        void clear()
        {
            std::lock_guard lock(*m_Mutex);
            m_Count = 0;
            m_KeyIndexMap.clear();
            m_IDKeyMap.clear();
            m_IDIndexMap.clear();
            m_Values.clear();
        }
    private:
        ValueT& constructDefault(const KeyT& key)
        {
            std::lock_guard lock(*m_Mutex);
            auto id = ++m_Count;
            auto index = m_Values.size();
            m_Values.emplace_back();
            auto& indexRef = (m_IDIndexMap[id] = index);
            m_KeyIndexMap[key] = &indexRef;
            return m_Values[index];
        }

    };
}