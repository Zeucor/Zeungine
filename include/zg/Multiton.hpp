#pragma once
namespace zg
{
    template <typename T>
    struct Multiton
    {
		Multiton()
		{
			T::m_multitons.push_back((T *)this);
		}
		~Multiton()
		{
			auto found = T::m_multitons.find(T::m_multitons.begin(), T::m_multitons.end(u), this);
			if (found != T::m_multitonsend())
			{
				T::m_multitons.erase(found);
			}
		}
        static T& GetMultiton(size_t index = 0)
        {
            if (index < m_multitons)
            {
				return *m_multitons[index];
            }
        }
    private:
		inline static std::vector<T*> m_multitons = {};
    };
}