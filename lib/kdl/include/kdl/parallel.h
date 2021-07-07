/*
 Copyright 2020 Eric Wasylishen

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef KDL_PARALLEL_H
#define KDL_PARALLEL_H

#include "kdl/vector_utils.h"

#ifdef _WIN32
#include <ppl.h>
#endif

#include <atomic>
#include <future> // for std::async
#include <optional>
#include <thread>
#include <utility> // for std::declval
#include <vector>

namespace kdl {
    /**
     * Runs the given lambda `count` times, passing it indices `0` through `count - 1`.
     *
     * Lambda is executed in parallel, using the number of threads returned by std::thread::hardware_concurrency().
     *
     * Because the threads are spawned with std::async(std::launch::async, ...) and no thread pool is used,
     * there is a relatively large overhead and this should only be used on large/slow to process data sets.
     *
     * @tparam L type of lambda
     * @param count the maximum value (exclusive) to pass to lambda
     * @param lambda the lambda to run
     */
    template<class L>
    void parallel_for(const size_t count, L&& lambda) {
#ifdef _WIN32
        concurrency::parallel_for<size_t>(0, count, lambda);
#else
        size_t numThreads = static_cast<size_t>(std::thread::hardware_concurrency());
        if (numThreads == 0) {
            numThreads = 1;
        }

        std::atomic<size_t> nextIndex(0);

        std::vector<std::future<void>> threads;
        threads.resize(numThreads);

        for (size_t i = 0; i < numThreads; ++i) {
            threads[i] = std::async(std::launch::async, [&]() {
                while (true) {
                    const size_t ourIndex = std::atomic_fetch_add(&nextIndex, static_cast<size_t>(1));
                    if (ourIndex >= count) {
                        break;
                    }
                    lambda(ourIndex);
                }
            });
        }

        for (size_t i = 0; i < numThreads; ++i) {
            threads[i].wait();
        }
#endif
    }

    /**
     * Applies the given lambda to each element of the input (passing elements as rvalue references),
     * and returns a vector of the resulting values, in their original order.
     * 
     * The lambda is executed in parallel, using the number of threads returned by std::thread::hardware_concurrency().
     * 
     * Because the threads are spawned with std::async(std::launch::async, ...) and no thread pool is used,
     * there is a relatively large overhead and this should only be used on large/slow to process data sets.
     *
     * @tparam T the type of the vector elements
     * @tparam L the type of the lambda to apply
     * @param input the vector
     * @param transform the lambda to apply, must be of type `auto(T&&)`
     * @return a vector containing the transformed values
     */
    template<class T, class L>
    auto vec_parallel_transform(std::vector<T> input, L&& transform) {
        using ResultType = std::optional<decltype(transform(std::declval<T&&>()))>;

        std::vector<ResultType> result;
        result.resize(input.size());

        parallel_for(input.size(), [&](const size_t index) {
            result[index] = transform(std::move(input[index]));
        });

        return vec_transform(std::move(result), [](ResultType&& x) { return std::move(*x); });
    }
}

#endif //KDL_PARALLEL_H
