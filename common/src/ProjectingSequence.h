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

#ifndef TrenchBroom_ProjectingSequence_h
#define TrenchBroom_ProjectingSequence_h

#include <iterator>

template <typename T, typename R>
struct ProjectingSequenceProjector {
    using Type = R;
    using ConstType = const R;
};

template <typename C, typename P>
struct ProjectingSequenceIterators {
    class const_iterator {
    public:
        using iterator_category = typename C::const_iterator::iterator_category;
        using value_type = typename C::const_iterator::value_type;
        using difference_type = typename C::const_iterator::difference_type;
        using pointer = typename C::const_iterator::pointer;
        using reference = typename C::const_iterator::reference;
    private:
        using I = typename C::const_iterator;
        I m_iterator;
    public:
        const_iterator() : m_iterator() {}
        explicit const_iterator(I iterator) : m_iterator(iterator) {}

        bool operator<(const const_iterator& other) const { return m_iterator <  other.m_iterator; }
        bool operator>(const const_iterator& other) const { return m_iterator >  other.m_iterator; }
        bool operator==(const const_iterator& other) const { return m_iterator == other.m_iterator; }
        bool operator!=(const const_iterator& other) const { return m_iterator != other.m_iterator; }

        // prefix
        const_iterator& operator++() { ++m_iterator; return *this; }
        const_iterator& operator--() { --m_iterator; return *this; }

        // postfix
        const_iterator operator++(int) { const_iterator result(*this); ++m_iterator; return result; }
        const_iterator operator--(int) { const_iterator result(*this); --m_iterator; return result; }

        typename P::Type const operator*()  const { return P::project(*m_iterator); }
        typename P::Type const operator->() const { return P::project(*m_iterator); }
    };
};

template <typename C, typename P>
class ProjectingSequence {
public:
    using const_iterator = typename ProjectingSequenceIterators<C,P>::const_iterator;
private:
    const C& m_container;
public:
    explicit ProjectingSequence(const C& container) : m_container(container) {}

    size_t size() const {
        return m_container.size();
    }

    const_iterator begin() const {
        return cbegin();
    }

    const_iterator end() const {
        return cend();
    }

    const_iterator cbegin() const {
        return const_iterator(std::cbegin(m_container));
    }

    const_iterator cend() const {
        return const_iterator(std::cend(m_container));
    }
};

#endif
