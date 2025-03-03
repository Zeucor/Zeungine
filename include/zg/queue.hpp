#pragma once
#include <stdexcept>
namespace zg::td
{
	template <typename T>
	struct queue
	{
	public:
		T& front()
		{
			if (!m_right)
				throw std::runtime_error("queue has no right");
			return m_right->value;
		}
		T& back()
		{
			if (!m_left)
				throw std::runtime_error("queue has no left");
			return m_left->value;
		}
		void pop()
		{
			if (!m_right)
				throw std::runtime_error("queue has no right");
			auto M_RIGHT = m_right;
			m_right = M_RIGHT->left;
			if (m_right)
			{
				m_right->right = 0;
			}
			else
			{
				m_left = 0;
			}
			/*
			!@  */
			delete M_RIGHT;
			m_size--;
		}
		void push(const T& value)
		{
			auto M_LEFT = m_left;
			m_left = new queue_item{value, 0, M_LEFT};
			/*
			      !@  */
			m_size++;
			if (!M_LEFT)
			{
				m_right = m_left;
			}
			else
			{
				M_LEFT->left = m_left;
			}
		}
		void clear()
		{
			auto current = m_right;
			auto next = current;
			while (current)
			{
				current = next;
				next = current->left;
				delete current;
			}
			m_right = 0;
			m_left = 0;
			m_size = 0;
		}
        size_t size()
        {
            return m_size;
        }
		bool empty() { return !m_size; }

	private:
		struct queue_item
		{
			T value;
			queue_item* left = 0;
			queue_item* right = 0;
		};
		queue_item* m_left = 0;
		queue_item* m_right = 0;
        size_t m_size = 0;
	};
} // namespace zg::td
