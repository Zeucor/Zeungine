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
#include <zg/GlobalUID.hpp>
#include <cassert>
#include <memory>
#include <map>
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
		using KeyIndexMap = std::map<KeyT, size_t*, std::less<KeyT>>;
		using IDKeyMap = std::unordered_map<size_t, KeyT, std::hash<size_t>>;
		using IDIndexMap = std::map<size_t, std::shared_ptr<size_t>>;
		using IndexIDMap = std::unordered_map<size_t, size_t, std::hash<size_t>, std::equal_to<size_t>>;
		using EmplaceBackTuple = std::tuple<KeyT, size_t, size_t*, ValueT*>;
		using GetKeyFunction = std::function<KeyT(const ValueT&)>;
		using DuplicateKeyFunction = std::function<KeyT(const KeyT&)>;
		using ValueIndexIterator = ValueVector::iterator;
		using ConstValueIndexIterator = ValueVector::const_iterator;

	public:
		struct Transaction
		{
			friend KeyIDVector;
			size_t* index;
			size_t id;
			KeyT key;

		protected:
			bool committed = false;
			bool rolledback = false;
		};
		Transaction startTransaction()
		{
			m_Mutex->lock();
			size_t id = GlobalUID::GetNew();
			size_t index = m_Values.size();
			auto id_indexit = m_IDIndexMap.emplace(id, std::make_shared<size_t>(index));
			auto& indexRef = *id_indexit.first->second;
			m_IndexIDMap[index] = id;
			Transaction t;
			t.index = &indexRef;
			t.id = id;
			return t;
		}
		template <typename... Args>
		ValueT& commitTransaction(Transaction& transaction, Args&... args)
		{
			if (transaction.committed)
			{
				m_Mutex->unlock();
				throw std::runtime_error("Cannot commit a transaction more than once");
			}
			if (transaction.rolledback)
			{
				m_Mutex->unlock();
				throw std::runtime_error("Cannot commit a transaction after it has been rolled back");
			}
			auto& value = m_Values.emplace_back(args...);
			assert(m_Values.size() - 1 == *transaction.index);
			transaction.key = FindNextKey_locked(value);
			m_IDKeyMap.emplace(transaction.id, transaction.key);
			auto key_indexit = m_KeyIndexMap.emplace(transaction.key, transaction.index);
			transaction.committed = true;
			m_Mutex->unlock();
			return value;
		}
		template <typename... Args>
		ValueT& commitTransactionKey(Transaction& transaction, const KeyT& key, Args&... args)
		{
			if (transaction.committed)
			{
				m_Mutex->unlock();
				throw std::runtime_error("Cannot commit a transaction more than once");
			}
			if (transaction.rolledback)
			{
				m_Mutex->unlock();
				throw std::runtime_error("Cannot commit a transaction after it has been rolled back");
			}
			auto& value = m_Values.emplace_back(args...);
			assert(m_Values.size() - 1 == *transaction.index);
			transaction.key = key;
			m_IDKeyMap.emplace(transaction.id, transaction.key);
			auto key_indexit = m_KeyIndexMap.emplace(transaction.key, transaction.index);
			transaction.committed = true;
			m_Mutex->unlock();
			return value;
		}
		void rollback(Transaction& transaction)
		{
			if (transaction.committed)
			{
				m_Mutex->unlock();
				throw std::runtime_error("unable to rollback committed transactions, use erase instead!");
			}
			if (transaction.rolledback)
			{
				m_Mutex->unlock();
				throw std::runtime_error("unable to rollback transaction more than once!");
			}
			m_IDIndexMap.erase(transaction.id);
			m_IndexIDMap.erase(*transaction.index);
			transaction.rolledback = true;
			m_Mutex->unlock();
		}

	private:
		KeyIndexMap m_KeyIndexMap;
		IDKeyMap m_IDKeyMap;
		IDIndexMap m_IDIndexMap;
		IndexIDMap m_IndexIDMap;
		ValueVector m_Values;
		std::shared_ptr<std::recursive_mutex> m_Mutex;
		GetKeyFunction m_GetKeyFunction;
		DuplicateKeyFunction m_DuplicateKeyFunction;
		bool m_GetKeyFunctionSet = false;
		bool m_DuplicateKeyFunctionSet = false;

	public:
		size_t DuplicateKeyReKeyAttemps = 1;

	public:
		KeyIDVector(const GetKeyFunction& getKeyFunction = {}, const DuplicateKeyFunction& duplicateKeyFunction = {}) :
				m_Mutex(std::make_shared<std::recursive_mutex>()), m_GetKeyFunction(getKeyFunction),
				m_DuplicateKeyFunction(duplicateKeyFunction), m_GetKeyFunctionSet(m_GetKeyFunction),
				m_DuplicateKeyFunctionSet(m_DuplicateKeyFunction) {};
		KeyIDVector(const KeyIDVector& other)
		{
			std::lock_guard lock_other(*other.m_Mutex);
			m_Values = other.m_Values;
			m_IDKeyMap = other.m_IDKeyMap;
			m_GetKeyFunction = other.m_GetKeyFunction;
			m_DuplicateKeyFunction = other.m_DuplicateKeyFunction;
			m_GetKeyFunctionSet = other.m_GetKeyFunctionSet;
			m_DuplicateKeyFunctionSet = other.m_DuplicateKeyFunctionSet;
			m_Mutex = std::make_shared<std::recursive_mutex>();
			m_IDIndexMap = other.m_IDIndexMap;
			m_KeyIndexMap = other.m_KeyIndexMap;
			m_IndexIDMap = other.m_IndexIDMap;
		}
		KeyIDVector& operator=(const KeyIDVector& other)
		{
			if (this == &other)
				return *this;

			std::lock(*m_Mutex, *other.m_Mutex);
			std::lock_guard lock_this(*m_Mutex, std::adopt_lock);
			std::lock_guard lock_other(*other.m_Mutex, std::adopt_lock);
			m_Values = other.m_Values;
			m_IDKeyMap = other.m_IDKeyMap;
			m_GetKeyFunction = other.m_GetKeyFunction;
			m_DuplicateKeyFunction = other.m_DuplicateKeyFunction;
			m_GetKeyFunctionSet = other.m_GetKeyFunctionSet;
			m_DuplicateKeyFunctionSet = other.m_DuplicateKeyFunctionSet;
			m_IDIndexMap = other.m_IDIndexMap;
			m_KeyIndexMap = other.m_KeyIndexMap;
			m_IndexIDMap = other.m_IndexIDMap;
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
		template <typename... Args>
		EmplaceBackTuple emplace_back(Args&... args)
		{
			if (!m_GetKeyFunctionSet)
				throw std::logic_error("KeyIDVector::emplace_back: GetKeyFunction is not set.");
			std::lock_guard lock(*m_Mutex);
			auto& value = m_Values.emplace_back(args...);
			KeyT key = FindNextKey_locked(value);
		_id:
			size_t id = GlobalUID::GetNew();
			size_t index = m_Values.size() - 1;
			auto id_indexit = m_IDIndexMap.emplace(id, std::make_shared<size_t>(index));
			auto& indexRef = *id_indexit.first->second;
			auto key_indexit = m_KeyIndexMap.emplace(key, &indexRef);
			m_IDKeyMap.emplace(id, key);
			m_IndexIDMap[index] = id;
			return {key, id, &indexRef, &value};
		}
		template <typename... Args>
		EmplaceBackTuple emplace_back_key(KeyT key, Args&... args)
		{
			std::lock_guard lock(*m_Mutex);
			size_t id = GlobalUID::GetNew();
			size_t index = m_Values.size();
			key = FindNextKey_locked_withKey(key);
			auto& value = m_Values.emplace_back(args...);
			auto id_indexit = m_IDIndexMap.emplace(id, std::make_shared<size_t>(index));
			auto& indexRef = *id_indexit.first->second;
			m_KeyIndexMap.emplace(key, &indexRef);
			m_IDKeyMap.emplace(id, key);
			m_IndexIDMap[index] = id;
			return {key, id, &indexRef, &value};
		}

		KeyT FindNextKey_locked(const ValueT& value)
		{
			KeyT key = m_GetKeyFunction(value);
			if (m_KeyIndexMap.find(key) != m_KeyIndexMap.end())
			{
				if (m_DuplicateKeyFunctionSet)
				{
					size_t count = 0;
					while (count < DuplicateKeyReKeyAttemps)
					{
						key = m_DuplicateKeyFunction(key);
						if (m_KeyIndexMap.find(key) == m_KeyIndexMap.end())
							goto _return_key;
						count++;
					}
				}
				m_Values.pop_back();
				throw std::runtime_error("KeyIDVector::emplace_back: Key already exists.");
			}
		_return_key:
			return key;
		}

		KeyT FindNextKey_locked_withKey(KeyT key)
		{
			if (m_KeyIndexMap.find(key) != m_KeyIndexMap.end())
			{
				if (m_DuplicateKeyFunctionSet)
				{
					size_t count = 0;
					while (count < DuplicateKeyReKeyAttemps)
					{
						key = m_DuplicateKeyFunction(key);
						if (m_KeyIndexMap.find(key) == m_KeyIndexMap.end())
							goto _return_key;
						count++;
					}
				}
				throw std::runtime_error("KeyIDVector::emplace_back: Key already exists.");
			}
		_return_key:
			return key;
		}

		template <typename IteratorT>
		IteratorT eraseValueIterator(IteratorT iter)
		{
			IteratorT end, begin;
			if constexpr (std::is_same_v<IteratorT, ValueIndexIterator>)
			{
				end = m_Values.end();
				begin = m_Values.begin();
			}
			else if constexpr (std::is_same_v<IteratorT, ConstValueIndexIterator>)
			{
				end = m_Values.cend();
				begin = m_Values.cbegin();
			}
			else
			{
				throw std::runtime_error("eraseValueIterator: Unsupported IteratorT");
			}
			std::lock_guard lock(*m_Mutex);
			if (iter == m_Values.cend() || m_Values.empty())
				return end;
			size_t erased_index = static_cast<size_t>(std::distance(begin, iter));
			if (erased_index >= m_Values.size())
				throw std::out_of_range("KeyIDVector::erase: Iterator out of range or invalidated.");
			auto idx_id_it = m_IndexIDMap.find(erased_index);
			if (idx_id_it == m_IndexIDMap.end())
				throw std::logic_error(
					"KeyIDVector::erase: Inconsistent state - No ID found for erased_index in m_IndexIDMap.");
			size_t erased_id = idx_id_it->second;
			auto id_key_it = m_IDKeyMap.find(erased_id);
			if (id_key_it == m_IDKeyMap.end())
				throw std::logic_error(
					"KeyIDVector::erase: Inconsistent state - ID found but no corresponding Key in m_IDKeyMap.");
			KeyT erased_key = id_key_it->second;
			size_t last_index = m_Values.size() - 1;
			if (erased_index != last_index)
			{
				auto last_idx_id_it = m_IndexIDMap.find(last_index);
				if (last_idx_id_it == m_IndexIDMap.end())
					throw std::logic_error(
						"KeyIDVector::erase: Inconsistent state - No ID found for last_index in m_IndexIDMap.");
				size_t last_id = last_idx_id_it->second;
				std::swap(m_Values[erased_index], m_Values[last_index]);
				auto moved_id_idx_it = m_IDIndexMap.find(last_id);
				if (moved_id_idx_it == m_IDIndexMap.end())
					throw std::logic_error("KeyIDVector::erase: Inconsistent state - No ID->Index map entry for moved element.");
				*moved_id_idx_it->second = erased_index;
				m_IndexIDMap[erased_index] = last_id;
				*m_IDIndexMap[last_id] = erased_index;
				m_IndexIDMap.erase(last_idx_id_it);
			}
			else
			{
				m_IndexIDMap.erase(erased_index);
			}
			m_KeyIndexMap.erase(erased_key);
			m_IDKeyMap.erase(erased_id);
			m_IDIndexMap.erase(erased_id);
			if (erased_index != last_index)
				m_IndexIDMap.erase(last_index);
			m_Values.pop_back();
			if (erased_index >= m_Values.size())
				return end;
			else
				return begin + erased_index;
		}
		void clear()
		{
			std::lock_guard lock(*m_Mutex);
			m_KeyIndexMap.clear();
			m_IDKeyMap.clear();
			m_IDIndexMap.clear();
			m_IndexIDMap.clear();
			m_Values.clear();
		}
		std::recursive_mutex& getMutex() { return *m_Mutex; }

	private:
		ValueT& constructDefault(const KeyT& key)
		{
			size_t id = GlobalUID::GetNew();
			size_t index = m_Values.size();
			auto& value = m_Values.emplace_back();
			auto id_indexit = m_IDIndexMap.emplace(id, std::make_shared<size_t>(index));
			auto& indexRef = *id_indexit.first->second;
			m_KeyIndexMap.emplace(key, &indexRef);
			m_IDKeyMap.emplace(id, key);
			m_IndexIDMap[index] = id;
			return value;
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
			key_iterator(KeyIDVector* kiv, MapIterator it) : kiv_ptr(kiv), map_iter(it) {}

			const KeyT& key() const { return map_iter->first; }
			size_t index() const
			{
				auto iter = kiv_ptr->m_KeyIndexMap.find(map_iter->first);
				if (iter == kiv_ptr->m_KeyIndexMap.end())
					return -1;
				return *iter->second;
			}
			size_t id() const
			{
				auto _index = index();
				auto iter = kiv_ptr->m_IndexIDMap.find(_index);
				if (iter == kiv_ptr->m_IndexIDMap.end())
					return 0;
				return iter->second;
			}

			reference operator*() const
			{
				auto index_ptr = map_iter->second;
				if (!index_ptr)
					throw std::logic_error("key_iterator: null index pointer encountered.");
				auto& _index = *index_ptr;
				if (_index >= kiv_ptr->m_Values.size())
					throw std::out_of_range("key_iterator: index out of bounds.");
				return kiv_ptr->m_Values[_index];
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
			key_iterator& operator--()
			{
				--map_iter;
				return *this;
			}

			key_iterator operator--(int)
			{
				key_iterator tmp = *this;
				--(*this);
				return tmp;
			}
			key_iterator operator+(int32_t amount) const
			{
				key_iterator tmp = *this;
				for (int32_t c = 1; c <= amount; c++)
					++tmp;
				return tmp;
			};
			key_iterator operator-(int32_t amount) const
			{
				key_iterator tmp = *this;
				for (int32_t c = 1; c <= amount; c++)
					--tmp;
				return tmp;
			};

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
			const_key_iterator(const KeyIDVector* kiv, MapConstIterator it) : kiv_ptr(kiv), map_iter(it) {}
			const_key_iterator(const key_iterator& other) : kiv_ptr(other.kiv_ptr), map_iter(other.map_iter) {}

			const KeyT& key() const { return map_iter->first; }
			size_t index() const
			{
				auto iter = kiv_ptr->m_KeyIndexMap.find(map_iter->first);
				if (iter == kiv_ptr->m_KeyIndexMap.end())
					return 0;
				return *iter->second;
			}
			size_t id() const
			{
				auto _index = index();
				auto iter = kiv_ptr->m_IndexIDMap.find(_index);
				if (iter == kiv_ptr->m_IndexIDMap.end())
					return 0;
				return iter->second;
			}
			reference operator*() const
			{
				auto index_ptr = map_iter->second;
				if (!index_ptr)
					throw std::logic_error("const_key_iterator: null index pointer encountered.");
				auto& _index = *index_ptr;
				if (_index >= kiv_ptr->m_Values.size())
					throw std::out_of_range("const_key_iterator: index out of bounds.");
				return kiv_ptr->m_Values[_index];
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
			const_key_iterator& operator--()
			{
				--map_iter;
				return *this;
			}

			const_key_iterator operator--(int)
			{
				const_key_iterator tmp = *this;
				--(*this);
				return tmp;
			}
			const_key_iterator operator+(int32_t amount) const
			{
				const_key_iterator tmp = *this;
				for (int32_t c = 1; c <= amount; c++)
					++tmp;
				return tmp;
			};
			const_key_iterator operator-(int32_t amount) const
			{
				const_key_iterator tmp = *this;
				for (int32_t c = 1; c <= amount; c++)
					--tmp;
				return tmp;
			};

			friend bool operator==(const const_key_iterator& a, const const_key_iterator& b)
			{
				if (a.kiv_ptr != b.kiv_ptr)
					return false;
				return a.map_iter == b.map_iter;
			};
			friend bool operator!=(const const_key_iterator& a, const const_key_iterator& b) { return !(a == b); };
		};

		struct id_iterator
		{
			using iterator_category = std::forward_iterator_tag;
			using difference_type = std::ptrdiff_t;
			using value_type = ValueT;
			using pointer = ValueT*;
			using reference = ValueT&;
			using MapIterator = typename IDIndexMap::iterator;

			KeyIDVector* kiv_ptr = nullptr;
			MapIterator map_iter;

			id_iterator() = default;
			id_iterator(KeyIDVector* kiv, MapIterator it) : kiv_ptr(kiv), map_iter(it) {}

			const KeyT& key() const
			{
				auto keyIter = kiv_ptr->m_IDKeyMap.find(id());
				if (keyIter == kiv_ptr->m_IDKeyMap.end())
					throw std::logic_error("ID has no Key!");
				return keyIter->second;
			}
			size_t index() const { return *map_iter->second; }
			size_t id() const
			{
				auto _index = index();
				auto indexIDIter = kiv_ptr->m_IndexIDMap.find(_index);
				if (indexIDIter == kiv_ptr->m_IndexIDMap.end())
					throw std::logic_error("index has no ID!");
				return indexIDIter->second;
				;
			}

			reference operator*() const
			{
				auto _index = index();
				if (_index >= kiv_ptr->m_Values.size())
					throw std::out_of_range("id_iterator: index out of bounds.");
				return kiv_ptr->m_Values[_index];
			}

			reference value() const { return operator*(); }

			pointer operator->() const { return &(operator*()); }

			id_iterator& operator++()
			{
				++map_iter;
				return *this;
			}

			id_iterator operator++(int)
			{
				id_iterator tmp = *this;
				++(*this);
				return tmp;
			}
			id_iterator operator+(int32_t amount) const
			{
				id_iterator tmp = *this;
				for (int32_t c = 1; c <= amount; c++)
					++tmp;
				return tmp;
			};
			id_iterator operator-(int32_t amount) const
			{
				id_iterator tmp = *this;
				for (int32_t c = 1; c <= amount; c++)
					--tmp;
				return tmp;
			};

			friend bool operator==(const id_iterator& a, const id_iterator& b)
			{
				if (a.kiv_ptr != b.kiv_ptr)
					return false;
				return a.map_iter == b.map_iter;
			};
			friend bool operator!=(const id_iterator& a, const id_iterator& b) { return !(a == b); };
		};

		struct const_id_iterator
		{
			using iterator_category = std::forward_iterator_tag;
			using difference_type = std::ptrdiff_t;
			using value_type = ValueT;
			using pointer = const ValueT*;
			using reference = const ValueT&;
			using MapConstIterator = typename IDIndexMap::const_iterator;

			const KeyIDVector* kiv_ptr = nullptr;
			MapConstIterator map_iter;

			const_id_iterator() = default;
			const_id_iterator(const KeyIDVector* kiv, MapConstIterator it) : kiv_ptr(kiv), map_iter(it) {}
			const_id_iterator(const id_iterator& other) : kiv_ptr(other.kiv_ptr), map_iter(other.map_iter) {}

			const KeyT& key() const
			{
				auto keyIter = kiv_ptr->m_IDKeyMap.find(id());
				if (keyIter == kiv_ptr->m_IDKeyMap.end())
					throw std::logic_error("ID has no Key!");
				return keyIter->second;
			}
			size_t index() const { return *map_iter->second; }
			size_t id() const
			{
				auto _index = index();
				auto indexIDIter = kiv_ptr->m_IndexIDMap.find(_index);
				if (indexIDIter == kiv_ptr->m_IndexIDMap.end())
					throw std::logic_error("index has no ID!");
				return indexIDIter->second;
				;
			}

			reference operator*() const
			{
				auto _index = index();
				if (_index >= kiv_ptr->m_Values.size())
					throw std::out_of_range("id_iterator: index out of bounds.");
				return kiv_ptr->m_Values[_index];
			}

			reference value() const { return operator*(); }

			pointer operator->() const { return &(operator*()); }

			const_id_iterator& operator++()
			{
				++map_iter;
				return *this;
			}

			const_id_iterator operator++(int)
			{
				const_id_iterator tmp = *this;
				++(*this);
				return tmp;
			}
			const_id_iterator operator+(int32_t amount) const
			{
				const_id_iterator tmp = *this;
				for (int32_t c = 1; c <= amount; c++)
					++tmp;
				return tmp;
			};
			const_id_iterator operator-(int32_t amount) const
			{
				const_id_iterator tmp = *this;
				for (int32_t c = 1; c <= amount; c++)
					--tmp;
				return tmp;
			};

			friend bool operator==(const const_id_iterator& a, const const_id_iterator& b)
			{
				if (a.kiv_ptr != b.kiv_ptr)
					return false;
				return a.map_iter == b.map_iter;
			};
			friend bool operator!=(const const_id_iterator& a, const const_id_iterator& b) { return !(a == b); };
		};

		struct value_iterator
		{
			using iterator_category = std::forward_iterator_tag;
			using difference_type = std::ptrdiff_t;
			using value_type = ValueT;
			using pointer = ValueT*;
			using reference = ValueT&;
			using VectorIterator = ValueVector::iterator;

			KeyIDVector* kiv_ptr = nullptr;
			VectorIterator map_iter;

			value_iterator() = default;
			value_iterator(KeyIDVector* kiv, VectorIterator it) : kiv_ptr(kiv), map_iter(it) {}

			const KeyT& key() const
			{
				auto keyIter = kiv_ptr->m_IDKeyVector.find(id());
				if (keyIter == kiv_ptr->m_IDKeyVector.end())
					throw std::logic_error("ID has no Key!");
				return keyIter->second;
			}
			size_t index() const { return std::distance(kiv_ptr->m_Values.begin(), map_iter); }
			size_t id() const
			{
				auto _index = index();
				auto indexIDIter = kiv_ptr->m_IndexIDMap.find(_index);
				if (indexIDIter == kiv_ptr->m_IndexIDMap.end())
					throw std::logic_error("index has no ID!");
				return indexIDIter->second;
				;
			}

			reference operator*() const
			{
				auto _index = index();
				if (_index >= kiv_ptr->m_Values.size())
					throw std::out_of_range("value_iterator: index out of bounds.");
				return kiv_ptr->m_Values[_index];
			}

			reference value() const { return operator*(); }

			pointer operator->() const { return &(operator*()); }

			value_iterator& operator++()
			{
				++map_iter;
				return *this;
			}

			value_iterator operator++(int)
			{
				value_iterator tmp = *this;
				++(*this);
				return tmp;
			}
			value_iterator operator+(int32_t amount) const
			{
				value_iterator tmp = *this;
				for (int32_t c = 1; c <= amount; c++)
					++tmp;
				return tmp;
			};
			value_iterator operator-(int32_t amount) const
			{
				value_iterator tmp = *this;
				for (int32_t c = 1; c <= amount; c++)
					--tmp;
				return tmp;
			};

			friend bool operator==(const value_iterator& a, const value_iterator& b)
			{
				if (a.kiv_ptr != b.kiv_ptr)
					return false;
				return a.map_iter == b.map_iter;
			};
			friend bool operator!=(const value_iterator& a, const value_iterator& b) { return !(a == b); };
		};

		struct const_value_iterator
		{
			using iterator_category = std::forward_iterator_tag;
			using difference_type = std::ptrdiff_t;
			using value_type = ValueT;
			using pointer = const ValueT*;
			using reference = const ValueT&;
			using VectorConstIterator = ValueVector::const_iterator;

			const KeyIDVector* kiv_ptr = nullptr;
			VectorConstIterator map_iter;

			const_value_iterator() = default;
			const_value_iterator(const KeyIDVector* kiv, VectorConstIterator it) : kiv_ptr(kiv), map_iter(it) {}
			const_value_iterator(const value_iterator& other) : kiv_ptr(other.kiv_ptr), map_iter(other.map_iter) {}

			const KeyT& key() const
			{
				auto keyIter = kiv_ptr->m_IDKeyVector.find(id());
				if (keyIter == kiv_ptr->m_IDKeyVector.end())
					throw std::logic_error("ID has no Key!");
				return keyIter->second;
			}
			size_t index() const { return std::distance(kiv_ptr->m_Values.begin(), map_iter); }
			size_t id() const
			{
				auto _index = index();
				auto indexIDIter = kiv_ptr->m_IndexIDMap.find(_index);
				if (indexIDIter == kiv_ptr->m_IndexIDMap.end())
					throw std::logic_error("index has no ID!");
				return indexIDIter->second;
				;
			}

			reference operator*() const
			{
				auto _index = index();
				if (_index >= kiv_ptr->m_Values.size())
					throw std::out_of_range("value_iterator: index out of bounds.");
				return kiv_ptr->m_Values[_index];
			}

			reference value() const { return operator*(); }

			pointer operator->() const { return &(operator*()); }

			const_value_iterator& operator++()
			{
				++map_iter;
				return *this;
			}

			const_value_iterator operator++(int)
			{
				const_value_iterator tmp = *this;
				++(*this);
				return tmp;
			}
			const_value_iterator operator+(int32_t amount) const
			{
				const_value_iterator tmp = *this;
				for (int32_t c = 1; c <= amount; c++)
					++tmp;
				return tmp;
			};
			const_value_iterator operator-(int32_t amount) const
			{
				const_value_iterator tmp = *this;
				for (int32_t c = 1; c <= amount; c++)
					--tmp;
				return tmp;
			};

			friend bool operator==(const const_value_iterator& a, const const_value_iterator& b)
			{
				if (a.kiv_ptr != b.kiv_ptr)
					return false;
				return a.map_iter == b.map_iter;
			};
			friend bool operator!=(const const_value_iterator& a, const const_value_iterator& b) { return !(a == b); };
		};

		// --- erase constexpr ---
		template <typename IteratorT>
		IteratorT erase(IteratorT iter)
		{
			if constexpr (std::is_same_v<IteratorT, ValueIndexIterator> || std::is_same_v<IteratorT, ConstValueIndexIterator>)
			{
				return eraseValueIterator(iter);
			}
			else if constexpr (std::is_same_v<key_iterator, IteratorT> || std::is_same_v<const_key_iterator, IteratorT> ||
												 std::is_same_v<id_iterator, IteratorT> || std::is_same_v<const_id_iterator, IteratorT> ||
												 std::is_same_v<value_iterator, IteratorT> || std::is_same_v<const_value_iterator, IteratorT>)
			{
				KeyT nextKey = KeyT();
				size_t nextID = 0;
				bool nextIsEnd = false;
				try
				{
					if constexpr (std::is_same_v<key_iterator, IteratorT> || std::is_same_v<const_key_iterator, IteratorT>)
						nextKey = (iter + 1).key();
					else if constexpr (std::is_same_v<id_iterator, IteratorT> || std::is_same_v<const_id_iterator, IteratorT>)
						nextID = (iter + 1).id();
				}
				catch (...)
				{
					nextIsEnd = true;
				}
				auto index = iter.index();
				auto valueIter = eraseValueIterator(m_Values.begin() + index);
				if constexpr (std::is_same_v<key_iterator, IteratorT>)
					return key_iterator(this, nextIsEnd ? m_KeyIndexMap.end() : m_KeyIndexMap.find(nextKey));
				else if constexpr (std::is_same_v<const_key_iterator, IteratorT>)
					return key_iterator(this,
															nextIsEnd ? m_KeyIndexMap.cend() : ((const KeyIndexMap&)m_KeyIndexMap).find(nextKey));
				if constexpr (std::is_same_v<id_iterator, IteratorT>)
					return id_iterator(this, (nextIsEnd || !nextID) ? m_IDIndexMap.end() : m_IDIndexMap.find(nextID));
				else if constexpr (std::is_same_v<const_id_iterator, IteratorT>)
					return id_iterator(
						this, (nextIsEnd || !nextID) ? m_IDIndexMap.cend() : ((const IDIndexMap&)m_IDIndexMap).find(nextID));
				if constexpr (std::is_same_v<value_iterator, IteratorT>)
					return value_iterator(this, (index < m_Values.size() ? m_Values.end() : m_Values.begin() + index));
				else if constexpr (std::is_same_v<const_value_iterator, IteratorT>)
					return value_iterator(this, (index < m_Values.size() ? m_Values.cend() : m_Values.cbegin() + index));
			}
			throw std::logic_error("unsupported IteratorT!");
		}

		// --- Key Iterator Methods ---

		key_iterator key_begin() { return key_iterator(this, m_KeyIndexMap.begin()); }
		const_key_iterator key_begin() const { return const_key_iterator(this, m_KeyIndexMap.cbegin()); }
		const_key_iterator ckey_begin() const { return const_key_iterator(this, m_KeyIndexMap.cbegin()); }
		key_iterator key_end() { return key_iterator(this, m_KeyIndexMap.end()); }
		const_key_iterator key_end() const { return const_key_iterator(this, m_KeyIndexMap.cend()); }
		const_key_iterator ckey_end() const { return const_key_iterator(this, m_KeyIndexMap.cend()); }

		id_iterator id_begin() { return id_iterator(this, m_IDIndexMap.begin()); }
		const_id_iterator id_begin() const { return const_id_iterator(this, m_IDIndexMap.cbegin()); }
		const_id_iterator cid_begin() const { return const_id_iterator(this, m_IDIndexMap.cbegin()); }
		id_iterator id_end() { return id_iterator(this, m_IDIndexMap.end()); }
		const_id_iterator id_end() const { return const_id_iterator(this, m_IDIndexMap.cend()); }
		const_id_iterator cid_end() const { return const_id_iterator(this, m_IDIndexMap.cend()); }

		value_iterator begin() { return {this, m_Values.begin()}; }
		const const_value_iterator begin() const { return {this, m_Values.begin()}; }
		value_iterator end() { return {this, m_Values.end()}; }
		const const_value_iterator end() const { return {this, m_Values.end()}; }

		value_iterator find_key(const KeyT& key)
		{
			std::lock_guard lock(*m_Mutex);
			auto key_iter = m_KeyIndexMap.find(key);
			if (key_iter == m_KeyIndexMap.end())
			{
				return end();
			}
			auto& indexRef = *key_iter->second;
			return {this, m_Values.begin() + indexRef};
		}
		const const_value_iterator find_key(const KeyT& key) const
		{
			std::lock_guard lock(*m_Mutex);
			auto key_iter = m_KeyIndexMap.find(key);
			if (key_iter == m_KeyIndexMap.end())
			{
				return end();
			}
			auto& indexRef = *key_iter->second;
			return {this, m_Values.begin() + indexRef};
		}
		value_iterator find_id(size_t id)
		{
			std::lock_guard lock(*m_Mutex);
			auto id_iter = m_IDIndexMap.find(id);
			if (id_iter == m_IDIndexMap.end())
			{
				return end();
			}
			auto& index = *id_iter->second;
			return {this, m_Values.begin() + index};
		}
		const const_value_iterator find_id(size_t id) const
		{
			std::lock_guard lock(*m_Mutex);
			auto id_iter = m_IDIndexMap.find(id);
			if (id_iter == m_IDIndexMap.end())
			{
				return end();
			}
			auto& index = *id_iter->second;
			return {this, m_Values.begin() + index};
		}
		KeyT getKey(value_iterator iter)
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

	public:
		void reserve(size_t n)
		{
			m_Values.reserve(n);
			m_IDKeyMap.reserve(n);
			m_IndexIDMap.reserve(n);
		}
	};
} // namespace zg
