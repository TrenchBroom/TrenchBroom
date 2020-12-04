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

#include <iterator>

namespace kdl {
    /**
     * Wraps a range, allowing the start iterator of the range to be incremented in steps greater than 1, and
     * with an initial offset.
     *
     * Initially, the skip iterator is incremented by the offset, and on each subsequence increment, it is incremented
     * by the stride. Both the offset and the stride are passed to the constructor.
     *
     * The skip iterator can be compared to the wrapped iterator.
     *
     * @tparam I the type of the wrapped iterators
     */
    template <typename I>
    class skip_iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = typename I::difference_type;
        using value_type = typename I::value_type;
        using pointer = typename I::pointer;
        using reference = typename I::reference;
    private:
        I m_cur;
        I m_end;
        difference_type m_stride;
    public:
        /**
         * Creates a skip iterator for the given range [cur, end) with the given offset and stride.
         *
         * @param cur the start of the wrapped range
         * @param end the end of the wrapped range (past-the-end iterator)
         * @param offset the initial offset
         * @param stride the stride of the iterator
         */
        skip_iterator(I cur, I end, const difference_type offset = 0, const difference_type stride = 1) :
        m_cur(cur),
        m_end(end),
        m_stride(stride) {
            advance(offset);
        }

        friend bool operator<(const skip_iterator& lhs, const skip_iterator& rhs) { return lhs.m_cur < rhs.m_cur; }
        friend bool operator<(const skip_iterator& lhs, const I& rhs)             { return lhs.m_cur < rhs;       }
        friend bool operator<(const I& lhs, const skip_iterator& rhs)             { return lhs       < rhs.m_cur; }

        friend bool operator>(const skip_iterator& lhs, const skip_iterator& rhs) { return lhs.m_cur > rhs.m_cur; }
        friend bool operator>(const skip_iterator& lhs, const I& rhs)             { return lhs.m_cur > rhs;       }
        friend bool operator>(const I& lhs, const skip_iterator& rhs)             { return lhs       > rhs.m_cur; }

        friend bool operator==(const skip_iterator& lhs, const skip_iterator& rhs) { return lhs.m_cur == rhs.m_cur; }
        friend bool operator==(const skip_iterator& lhs, const I& rhs)             { return lhs.m_cur == rhs;       }
        friend bool operator==(const I& lhs, const skip_iterator& rhs)             { return lhs       == rhs.m_cur; }

        friend bool operator!=(const skip_iterator& lhs, const skip_iterator& rhs) { return lhs.m_cur != rhs.m_cur; }
        friend bool operator!=(const skip_iterator& lhs, const I& rhs)             { return lhs.m_cur != rhs;       }
        friend bool operator!=(const I& lhs, const skip_iterator& rhs)             { return lhs       != rhs.m_cur; }

        skip_iterator& operator++() {
            advance(m_stride);
            return *this;
        }

        skip_iterator operator++(int) {
            auto result = skip_iterator(*this);
            advance(m_stride);
            return result;
        }

        reference operator*()  const { return *m_cur; }
        pointer   operator->() const { return *m_cur; }
    private:
        void advance(const difference_type distance) {
            std::advance(m_cur, std::min(distance, std::distance(m_cur, m_end)));
        }
    };

    /**
     * Deduction guide.
     */
    template <typename I>
    skip_iterator(I cur, I end, typename I::difference_type offset,  typename I::difference_type stride) -> skip_iterator<I>;
}


