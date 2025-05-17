#pragma once
namespace zg
{
    template <typename T>
    struct Singleton
    {
		Singleton()
		{
			if (!T::m_singleton)
			{
				T::m_singleton = (T *)this;
			}
		}
		~Singleton()
		{
			if (T::m_singleton == this)
			{
				T::m_singleton = (T *)0;
			}
		}
        static T& GetSingleton()
        {
            if (!m_singleton)
            {
                throw std::runtime_error("singleton is not defined");
            }
            return *m_singleton;
        }
    private:
		inline static T *m_singleton = 0;
    };
}