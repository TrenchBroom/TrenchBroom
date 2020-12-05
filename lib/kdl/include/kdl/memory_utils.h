/*
 Copyright 2010-2019 Kristian Duske

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

#pragma once

#include <cassert>
#include <memory>

namespace kdl {
    /**
     * Checks whether the given shared pointer is expired. Always returns false.
     *
     * @tparam T the type of the pointee
     * @return false
     */
    template<typename T>
    bool mem_expired(const std::shared_ptr<T>& /* ptr */) {
        return false;
    }

    /**
     * Checks whether the given weak pointer is expired.
     *
     * @tparam T the type of the pointee
     * @param ptr the pointer to check
     * @return true if the given pointer is expired, and false otherwise
     */
    template<typename T>
    bool mem_expired(const std::weak_ptr<T>& ptr) {
        return ptr.expired();
    }

    /**
     * Locks the given shared pointer. Always returns the given pointer.
     *
     * @tparam T the type of the pointee
     * @param ptr the pointer to check
     * @return the given pointer
     */
    template<typename T>
    std::shared_ptr<T> mem_lock(std::shared_ptr<T> ptr) {
        return ptr;
    }

    /**
     * Locks the given weak pointer.
     *
     * Precondition: the given weak pointer is not expired
     *
     * @tparam T the type of the pointee
     * @param ptr the pointer to lock
     * @return the underlying shared pointer
     */
    template<typename T>
    std::shared_ptr<T> mem_lock(std::weak_ptr<T> ptr) {
        assert(!mem_expired(ptr));
        return ptr.lock();
    }
}

#endif //KDL_MEMORY_UTILS_H
