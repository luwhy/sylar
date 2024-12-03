#ifndef SINGLE_TON
#define SINGLE_TON
#include <memory>
namespace sylar
{
    template <class T, class X = void, int n = 0>
    class Singleton
    {
    public:
        static T *GetInstance()
        {
            static T v;
            return &v;
        }
    };
    template <class T, class X = void, int N = 0>
    class SingleTonPtr
    {
    public:
        static std::shared_ptr<T> GetInstance()
        {
            static std::shared_ptr<T> v(new T);
            return v;
        }
    };
}

#endif