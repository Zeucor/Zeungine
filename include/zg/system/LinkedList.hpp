#pragma once
namespace zg::system
{
    template<typename T>
    struct ILinkedList
    {
        virtual ~ILinkedList() = default;
        virtual size_t size() = 0;
        virtual T& front() = 0;
        virtual T& back() = 0;
        virtual void pop_front() = 0;
        virtual void pop_back() = 0;
        virtual void push_back() = 0;
        virtual void push_front() = 0;
        virtual void insert(size_t index, const T& value) = 0;
        virtual void erase(size_t index);
    };
    template<typename T>
    class LinkedList : ILinkedList<T>
    {
    private:
        struct Item
        {
            T value;
            Item* left = 0;
            Item* right = 0;
            Item(const T& t):
                value(t)
            {}
        };
        size_t m_size = 0;
        Item *head = 0;
        Item *tail = 0;
    public:
        LinkedList():
        {}
        ~LinkedList()
        {
            clear();
        }
        clear()
        {
            auto current = head;
            while (true)
            {
                auto& currentRef = *current;
                current = currentRef.right;
                delete &currentRef;
            }
            m_size = 0;
        }
    }

}