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
#include <cstddef>
#include <utility>

namespace kdl {
    /**
     * A thin wrapper around a statically sized array that allows accessing the elements using an enum as the index.
     *
     * @tparam T the value type
     * @tparam Enum the type of the enum used to index the values
     * @tparam Size the size of the array, must be greater than 0
     */
    template <typename T, typename Enum, std::size_t Size>
    class enum_array {
        static_assert(Size > 0u, "enum_array must have size > 0");
    private:
        T m_array[Size]; // force value initialization to 0
    public:
        using iterator = T*;
        using const_iterator = const T*;
        using reverse_iterator = iterator;
        using const_reverse_iterator = const_iterator;
    public:
        /**
         * Creates a new empty array. The array values are value initialized.
         */
        enum_array() :
        m_array{} {}

        /**
         * Creates a new array with the values in the given initializer list.
         */
        enum_array(std::initializer_list<T> list) :
        m_array(list) {}

        /**
         * Returns a const reference to the element at the given index.
         *
         * Precondition: index < Size

         * @param index the index
         * @return a const reference to the element at the given index
         */
        const T& operator[](const Enum index) const {
            return get(index);
        }

        /**
         * Returns a reference to the element at the given index.
         *
         * Precondition: index < Size

         * @param index the index
         * @return a reference to the element at the given index
         */
        T& operator[](const Enum index) {
            return get(index);
        }

        /**
         * Returns a const reference to the element at the given index.
         *
         * Precondition: index < Size
         *
         * @param index the index
         * @return a const reference to the element at the given index
         */
        const T& get(const Enum index) const {
            const auto i = static_cast<std::size_t>(index);
            assert(i < Size);
            return m_array[i];
        }

        /**
         * Returns a reference to the element at the given index.
         *
         * Precondition: index < Size
         *
         * @param index the index
         * @return a reference to the element at the given index
         */
        T& get(const Enum index) {
            const auto i = static_cast<std::size_t>(index);
            assert(i < Size);
            return m_array[i];
        }

        /**
         * Returns an iterator to the first element.
         */
        iterator begin() {
            return std::begin(m_array);
        }

        /**
         * Returns an iterator the element following the last element of the array.
         */
        iterator end() {
            return std::end(m_array);
        }

        /**
         * Returns an iterator to the first element.
         */
        const_iterator begin() const {
            return cbegin();
        }

        /**
         * Returns an iterator the element following the last element of the array.
         */
        const_iterator end() const {
            return cend();
        }

        /**
         * Returns an iterator to the first element.
         */
        const_iterator cbegin() const {
            return std::cbegin(m_array);
        }

        /**
         * Returns an iterator the element following the last element of the array.
         */
        const_iterator cend() const {
            return std::cend(m_array);
        }

        /**
         * Returns an iterator to the first element of the reversed array.
         */
        reverse_iterator rbegin() {
            return std::rbegin(m_array);
        }

        /**
         * Returns an iterator the element following the last element of the reversed array.
         */
        reverse_iterator rend() {
            return std::rend(m_array);
        }

        /**
         * Returns an iterator to the first element of the reversed array.
         */
        const_reverse_iterator rbegin() const {
            return crbegin();
        }

        /**
         * Returns an iterator the element following the last element of the reversed array.
         */
        const_reverse_iterator rend() const {
            return crend();
        }

        /**
         * Returns an iterator to the first element of the reversed array.
         */
        const_reverse_iterator crbegin() const {
            return std::crbegin(m_array);
        }

        /**
         * Returns an iterator the element following the last element of the reversed array.
         */
        const_reverse_iterator crend() const {
            return std::crend(m_array);
        }
    };
}

#endif //KDL_ENUM_ARRAY_H
