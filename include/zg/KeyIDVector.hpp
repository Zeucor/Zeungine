#pragma once
#include <functional>
#include <iostream>
#include <map>
#include <mutex>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
namespace zg
{
#define KEY_ID_VECTOR_KEY_INDEX 0
#define KEY_ID_VECTOR_ID_INDEX 1
#define KEY_ID_VECTOR_INDEX_INDEX 2
#define KEY_ID_VECTOR_VALUE_INDEX 3
	template <typename KeyT, typename ValueT>
	struct KeyIDVector
	{
		using ValueVector = std::vector<ValueT>;
		using KeyIndexMap = std::map<KeyT, size_t*>;
		using IDKeyMap = std::unordered_map<size_t, KeyT>;
		using IDIndexMap = std::map<size_t, size_t>;
		using EmplaceBackTuple = std::tuple<KeyT, size_t, size_t*, ValueT*>;
		using GetKeyFunction = std::function<KeyT(const ValueT&)>;

	private:
		size_t m_Count = 0;
		KeyIndexMap m_KeyIndexMap;
		IDKeyMap m_IDKeyMap;
		IDIndexMap m_IDIndexMap;
		ValueVector m_Values;
		std::shared_ptr<std::recursive_mutex> m_Mutex;
		GetKeyFunction m_GetKeyFunction;

	public:
		KeyIDVector(const GetKeyFunction& getKeyFunction = {}) :
				m_Mutex(std::make_shared<std::recursive_mutex>()), m_GetKeyFunction(getKeyFunction) {};
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
		ValueT* data() { return m_Values.data(); }
		const ValueT* data() const { return m_Values.data(); }
		size_t size() { return m_Values.size(); }
		const size_t size() const { return m_Values.size(); }
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
		ValueVector::iterator begin() { return m_Values.begin(); }
		const ValueVector::const_iterator begin() const { return m_Values.begin(); }
		ValueVector::iterator end() { return m_Values.end(); }
		const ValueVector::const_iterator end() const { return m_Values.end(); }
		ValueVector::iterator find_key(const KeyT& key)
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
		const ValueVector::const_iterator find_key(const KeyT& key) const
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
		ValueVector::iterator find_id(size_t id)
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
		const ValueVector::const_iterator find_id(size_t id) const
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
		KeyT getKey(ValueVector::iterator iter)
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
		template <typename... Args>
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
				m_IDKeyMap[id] = key;
				return {key, id, &indexRef, &value};
			}
			throw std::runtime_error("GetKeyFunction is not set and emplace_back called without key");
		}
		template <typename... Args>
		EmplaceBackTuple emplace_back_key(const KeyT& key, const Args&... args)
		{
			std::lock_guard lock(*m_Mutex);
			auto id = ++m_Count;
			auto index = m_Values.size();
			m_Values.emplace_back(args...);
			auto& indexRef = (m_IDIndexMap[id] = index);
			auto& value = m_Values[index];
			m_KeyIndexMap[key] = &indexRef;
			m_IDKeyMap[id] = key;
			return {key, id, &indexRef, &value};
		}
		ValueVector::iterator erase(const ValueVector::iterator& iter)
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
					auto& key = idKeyIter->second;
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
		std::recursive_mutex& getMutex() { return *m_Mutex; }

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

	public:
		struct key_iterator
		{
			using iterator_category = std::forward_iterator_tag;
			using difference_type = std::ptrdiff_t;
			using value_type = ValueT;
			using pointer = ValueT*;
			using reference = ValueT&;
			using MapIterator = typename KeyIndexMap::iterator;

			KeyIDVector* kiv_ptr = nullptr;
			MapIterator map_iter;

			key_iterator() = default;
			explicit key_iterator(KeyIDVector* kiv, MapIterator it) : kiv_ptr(kiv), map_iter(it) {}

			const KeyT& key() const { return map_iter->first; }

			reference operator*() const
			{
				size_t* index_ptr = map_iter->second;
				if (!index_ptr)
					throw std::logic_error("key_iterator: null index pointer encountered.");
				size_t index = *index_ptr;
				if (index >= kiv_ptr->m_Values.size())
					throw std::out_of_range("key_iterator: index out of bounds.");
				return kiv_ptr->m_Values[index];
			}

			reference value() const { return operator*(); }

			pointer operator->() const { return &(operator*()); }

			key_iterator& operator++()
			{
				++map_iter;
				return *this;
			}

			key_iterator operator++(int)
			{
				key_iterator tmp = *this;
				++(*this);
				return tmp;
			}

			friend bool operator==(const key_iterator& a, const key_iterator& b)
			{
				if (a.kiv_ptr != b.kiv_ptr)
					return false;
				return a.map_iter == b.map_iter;
			};
			friend bool operator!=(const key_iterator& a, const key_iterator& b) { return !(a == b); };
		};

		struct const_key_iterator
		{
			using iterator_category = std::forward_iterator_tag;
			using difference_type = std::ptrdiff_t;
			using value_type = ValueT;
			using pointer = const ValueT*;
			using reference = const ValueT&;
			using MapConstIterator = typename KeyIndexMap::const_iterator;

			const KeyIDVector* kiv_ptr = nullptr;
			MapConstIterator map_iter;

			const_key_iterator() = default;
			explicit const_key_iterator(const KeyIDVector* kiv, MapConstIterator it) : kiv_ptr(kiv), map_iter(it) {}
			const_key_iterator(const key_iterator& other) : kiv_ptr(other.kiv_ptr), map_iter(other.map_iter) {}

			const KeyT& key() const { return map_iter->first; }

			reference operator*() const
			{
				size_t* index_ptr = map_iter->second;
				if (!index_ptr)
					throw std::logic_error("const_key_iterator: null index pointer encountered.");
				size_t index = *index_ptr;
				if (index >= kiv_ptr->m_Values.size())
					throw std::out_of_range("const_key_iterator: index out of bounds.");
				return kiv_ptr->m_Values[index];
			}

			reference value() const { return operator*(); }

			pointer operator->() const { return &(operator*()); }

			const_key_iterator& operator++()
			{
				++map_iter;
				return *this;
			}

			const_key_iterator operator++(int)
			{
				const_key_iterator tmp = *this;
				++(*this);
				return tmp;
			}

			friend bool operator==(const const_key_iterator& a, const const_key_iterator& b)
			{
				if (a.kiv_ptr != b.kiv_ptr)
					return false;
				return a.map_iter == b.map_iter;
			};
			friend bool operator!=(const const_key_iterator& a, const const_key_iterator& b) { return !(a == b); };
		};

		// --- Key Iterator Methods (Unchanged) ---

		key_iterator key_begin() { return key_iterator(this, m_KeyIndexMap.begin()); }
		const_key_iterator key_begin() const { return const_key_iterator(this, m_KeyIndexMap.cbegin()); }
		const_key_iterator ckey_begin() const { return const_key_iterator(this, m_KeyIndexMap.cbegin()); }
		key_iterator key_end() { return key_iterator(this, m_KeyIndexMap.end()); }
		const_key_iterator key_end() const { return const_key_iterator(this, m_KeyIndexMap.cend()); }
		const_key_iterator ckey_end() const { return const_key_iterator(this, m_KeyIndexMap.cend()); }
	};
} // namespace zg
