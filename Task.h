#pragma once

#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>


class Task
{
public:
    Task();
    ~Task();

    void setNotifier(std::condition_variable* pNotifier);
    void run(std::function<void()> func);
    bool done() const;

private:
    bool exit_;
    bool done_;
    std::function<void()> func_;
    std::thread th_;
    std::mutex mtx_;
    std::condition_variable cond_;
    std::condition_variable* pNotifier_;
};


struct IsDone {
    bool operator()(const Task& t) const {
        return t.done();
    }
};

