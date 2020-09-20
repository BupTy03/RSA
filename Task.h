#pragma once
#include <atomic>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>


class Task
{
public:
    explicit Task()
        : exit_(false)
        , done_(true)
        , func_()
        , th_()
        , cond_()
    {
        th_ = std::thread([this]{
           while (true)
           {
               std::unique_lock<std::mutex> lock(mtx_);
               cond_.wait(lock, [this]{ return exit_ || !done_; });

               if(exit_)
                   return;

               func_();
               done_ = true;
           }
        });
    }

    ~Task() { exit_ = true; cond_.notify_all(); th_.join(); }

    void run(std::function<void()> func)
    {
        done_ = false;
        func_ = std::move(func);
        cond_.notify_all();
    }

    bool done() const { return done_; }

private:
    bool exit_;
    bool done_;
    std::function<void()> func_;
    std::thread th_;
    std::mutex mtx_;
    std::condition_variable cond_;
};
