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

#include <atomic>
#include <future> // for std::async
#include <thread>
#include <utility> // for std::declval
#include <vector>

namespace kdl {
    /**
     * Applies the given lambda to each element of the input (passing elements as const lvalue references),
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
     * @param transform the lambda to apply, must be of type `auto(const T&)`
     * @return a vector containing the transformed values
     */
    template<class T, class L>
    auto vec_parallel_transform(const std::vector<T>& input, L&& transform) {
        using ResultType = decltype(transform(std::declval<const T&>()));

        std::vector<ResultType> result;
        result.resize(input.size());

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
                    if (ourIndex >= input.size()) {
                        break;
                    }
                    result[ourIndex] = transform(input[ourIndex]);
                }
            });
        }

        for (size_t i = 0; i < numThreads; ++i) {
            threads[i].wait();
        }

        return result;
    }
}

#endif //KDL_PARALLEL_H
