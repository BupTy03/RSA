#include "Task.h"

#include <cassert>


Task::Task()
        : exit_(false)
        , done_(true)
        , func_()
        , th_()
        , cond_()
        , pNotifier_(nullptr)
{
    th_ = std::thread([this]
    {
        while (true)
        {
            std::unique_lock<std::mutex> lock(mtx_);
            cond_.wait(lock, [this]{ return exit_ || !done_; });

            if(exit_)
                return;

            assert(func_);
            assert(pNotifier_ != nullptr);

            func_();
            done_ = true;
            pNotifier_->notify_all();
        }
    });
}

Task::~Task()
{
    exit_ = true;
    cond_.notify_all();
    th_.join();
}

void Task::setNotifier(std::condition_variable* pNotifier)
{
    assert(pNotifier != nullptr);
    pNotifier_ = pNotifier;
}

void Task::run(std::function<void()> func)
{
    done_ = false;
    func_ = std::move(func);
    cond_.notify_all();
}

bool Task::done() const
{
    return done_;
}
