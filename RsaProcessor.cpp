#include "RsaProcessor.h"

#include "rsa.h"

#include <algorithm>


void RsaProcessor::process(std::istream& input, std::ostream& output, std::size_t countBytes)
{
    assert(countBytes <= bigBuffer_.size());

    input.read(reinterpret_cast<char*>(bigBuffer_.data()), countBytes);

    const auto countTasks = countBytes / blockSize_;
    auto lastWritten = bigBuffer_.data();
    for(std::size_t taskIdx = 0; taskIdx < countTasks; ++taskIdx)
    {
        auto beginChunk = lastWritten;
        auto endChunk = beginChunk + blockSize_;
        lastWritten = endChunk;

        tasks_.at(taskIdx).run([this, beginChunk, endChunk] {
            auto endWritten = rsa(beginChunk, endChunk, beginChunk, *pKey_, *pN_);
            if(endWritten != endChunk)
            {
                std::rotate(beginChunk, endWritten, endChunk);
                std::fill_n(beginChunk, endChunk - endWritten, 0);
            }

            condVar_.notify_all();
        });
    }

    const auto remainingBytes = (bigBuffer_.data() + countBytes) - lastWritten;
    if(remainingBytes > 0)
        lastWritten = rsa(lastWritten, lastWritten + remainingBytes, lastWritten, *pKey_, *pN_);

    std::unique_lock<std::mutex> lock(mtx_);
    condVar_.wait(lock, [this, countTasks]{
        return std::all_of(tasks_.cbegin(), tasks_.cbegin() + countTasks, [](const auto& t) { return t.done(); });
    });

    output.write(reinterpret_cast<char*>(bigBuffer_.data()), lastWritten - bigBuffer_.data());
}
