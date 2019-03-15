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
    I m_delegate;
    difference_type m_offset;
    difference_type m_stride;
    bool m_initialStep;
public:
    StepIterator(I delegate, const difference_type offset = 0, const difference_type stride = 1) :
    m_delegate(delegate),
    m_offset(offset),
    m_stride(stride),
    m_initialStep(true) {}

    bool operator<(const StepIterator& other) const  { return m_delegate < other.m_delegate; }
    bool operator>(const StepIterator& other) const  { return m_delegate > other.m_delegate; }
    bool operator==(const StepIterator& other) const { return m_delegate == other.m_delegate; }
    bool operator!=(const StepIterator& other) const { return m_delegate != other.m_delegate; }

    // prefix increment
    StepIterator& operator++() {
        increment();
        return *this;
    }

    // postfix increment
    StepIterator operator++(int) {
        auto result = StepIterator(*this);
        increment();
        return result;
    }

    reference operator*() const { return *m_delegate; }
    pointer operator->() const { return *m_delegate; }
private:
    void increment() {
        if (m_initialStep) {
            std::advance(m_delegate, m_offset);
            m_initialStep = false;
        } else {
            std::advance(m_delegate, m_stride);
        }
    }
};

#endif //TRENCHBROOM_STEPITERATOR_H
