#pragma once
#include <chrono>
#include <string>

namespace SoftRenderer
{
    class Timer {
    public:
        Timer() {}

        ~Timer() {}

        void Start() { start_ = std::chrono::steady_clock::now(); }

        void Stop() { end_ = std::chrono::steady_clock::now(); }

        int64_t ElapseMillis() const
        {
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_ - start_);
            return duration.count();
        }

    private:
        std::chrono::time_point<std::chrono::steady_clock> start_;
        std::chrono::time_point<std::chrono::steady_clock> end_;
    };

    class ScopedTimer {
    public:

        ScopedTimer(const char* str)
            : tag_(str) 
        {
            timer_.Start();
        }

        ~ScopedTimer() 
        {
            timer_.Stop();
            printf("%s: %lld ms\n", tag_.c_str(), timer_.ElapseMillis());
        }

        operator bool() 
        {
            return true;
        }

    private:
        Timer timer_;
        std::string tag_;
    };
}