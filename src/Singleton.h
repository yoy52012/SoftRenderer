#pragma once

namespace SoftRenderer
{
    class Noncopyable 
    {
    protected:
        Noncopyable() = default;
        ~Noncopyable() = default;
        Noncopyable(Noncopyable &&) = delete;                 // Move construct
        Noncopyable(const Noncopyable &) = delete;            // Copy construct
        Noncopyable &operator=(const Noncopyable &) = delete; // Copy assign
        Noncopyable &operator=(Noncopyable &&) = delete;      // Move assign
    };

    template <typename T>
    class Singleton : public Noncopyable 
    {
    public:
        static T &instance() 
        {
            static T instance;
            return instance;
        }
    };
}

