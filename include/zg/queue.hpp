#pragma once
namespace zg::td
{
	template <typename T>
	struct queue
	{
	public:
		T& peek()
		{
			if (!tail)
				throw std::runtime_error("queue has no tail");
			return *tail->value;
		}
		void pop()
		{
			if (!tail)
				throw std::runtime_error("queue has no tail");
			auto _TAIL = tail;
			tail = _TAIL->left;
			delete _TAIL;
		}
		void push(const T& value)
		{
			auto _HEAD = head;
			head = new queue_item{value, 0, _HEAD};
			if (!_HEAD)
			{
				tail = head;
			}
			else
			{
				_HEAD->left = head;
			}
		}
		void clear()
		{
			head = 0;
			auto& current = tail;
			while (current)
			{
				auto copied_current = current;
				current->copied_current->left;
				delete copied_current;
			}
		}
		bool empty() { return head; }

	private:
		struct queue_item
		{
			T value;
			queue_item* left = 0;
			queue_item* right = 0;
		};
		queue_item* head = 0;
		queue_item* tail = 0;
	};
} // namespace zg::std
