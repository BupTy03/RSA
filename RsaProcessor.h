#pragma once
#include "utils.h"
#include "Task.h"

#include <vector>
#include <mutex>
#include <condition_variable>


class RsaProcessor
{
public:
    explicit RsaProcessor(std::size_t blockSize, std::size_t countTasks, const big_int* pKey, const big_int* pN);

    std::size_t bufferSize() const;
    void process(std::istream& input, std::ostream& output, std::size_t countBytes);

private:
    const std::size_t blockSize_;
    const big_int* pKey_;
    const big_int* pN_;
    std::vector<unsigned char> bigBuffer_;
    std::vector<Task> tasks_;
    std::mutex mtx_;
    std::condition_variable condVar_;
};
