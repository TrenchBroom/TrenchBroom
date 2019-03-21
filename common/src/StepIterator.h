/*
 Copyright (C) 2010-2017 Kristian Duske

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TRENCHBROOM_STEPITERATOR_H
#define TRENCHBROOM_STEPITERATOR_H

#include <iterator>

template <typename I>
class StepIterator {
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
    StepIterator(I cur, I end, const difference_type offset = 0, const difference_type stride = 1) :
    m_cur(cur),
    m_end(end),
    m_stride(stride) {
        increment(offset);
    }

    bool operator<(const StepIterator& other) const  { return m_cur < other.m_cur; }
    bool operator>(const StepIterator& other) const  { return m_cur > other.m_cur; }
    bool operator==(const StepIterator& other) const { return m_cur == other.m_cur; }
    bool operator!=(const StepIterator& other) const { return m_cur != other.m_cur; }

    // prefix increment
    StepIterator& operator++() {
        increment(m_stride);
        return *this;
    }

    // postfix increment
    StepIterator operator++(int) {
        auto result = StepIterator(*this);
        increment(m_stride);
        return result;
    }

    reference operator*() const { return *m_cur; }
    pointer operator->() const { return *m_cur; }
private:
    void increment(const typename I::difference_type distance) {
        std::advance(m_cur, std::min(distance, m_end - m_cur));
    }
};

template <typename I>
StepIterator<I> stepIterator(I cur, I end, const typename I::difference_type offset = 0, const typename I::difference_type stride = 1) {
    return StepIterator<I>(cur, end, offset, stride);
}

#endif //TRENCHBROOM_STEPITERATOR_H
