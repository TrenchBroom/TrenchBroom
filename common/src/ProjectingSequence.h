/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

template <typename T, typename R>
struct ProjectingSequenceProjector {
    typedef R Type;
    typedef const R ConstType;
    static Type project(T& vertex);
};

template <typename C, typename P>
struct ProjectingSequenceIterators {
    class iterator {
    private:
        typedef typename C::iterator I;
        I m_iterator;
    public:
        iterator() : m_iterator() {}
        iterator(I iterator) : m_iterator(iterator) {}
        
        bool operator<(const iterator& other) const { return m_iterator <  other.m_iterator; }
        bool operator>(const iterator& other) const { return m_iterator >  other.m_iterator; }
        bool operator==(const iterator& other) const { return m_iterator == other.m_iterator; }
        bool operator!=(const iterator& other) const { return m_iterator != other.m_iterator; }
        
        // prefix
        iterator& operator++() { ++m_iterator; return *this; }
        iterator& operator--() { --m_iterator; return *this; }
        
        // postfix
        iterator operator++(int) { iterator result(*this); ++m_iterator; return result; }
        iterator operator--(int) { iterator result(*this); --m_iterator; return result; }
        
        typename P::Type operator*()  const { return P::project(*m_iterator); }
        typename P::Type operator->() const { return P::project(*m_iterator); }
    };
    
    class const_iterator {
    private:
        typedef typename C::const_iterator I;
        I m_iterator;
    public:
        const_iterator() : m_iterator() {}
        const_iterator(I iterator) : m_iterator(iterator) {}
        
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
        
        typename P::Type operator*()  const { return P::project(*m_iterator); }
        typename P::Type operator->() const { return P::project(*m_iterator); }
    };
};

template <typename C, typename P>
class ProjectingSequence {
public:
    typedef typename ProjectingSequenceIterators<C,P>::iterator iterator;
    typedef typename ProjectingSequenceIterators<C,P>::const_iterator const_iterator;
private:
    C& m_container;
public:
    ProjectingSequence(C& container) : m_container(container) {}
    
    size_t size() const {
        return m_container.size();
    }
    
    iterator begin() {
        return iterator(m_container.begin());
    }
    
    iterator end() {
        return iterator(m_container.end());
    }

    const_iterator begin() const {
        return const_iterator(m_container.cbegin());
    }
    
    const_iterator end() const {
        return const_iterator(m_container.cend());
    }
};

template <typename C, typename P>
class ConstProjectingSequence {
public:
    typedef typename ProjectingSequenceIterators<C,P>::const_iterator const_iterator;
private:
    const C& m_container;
public:
    ConstProjectingSequence(const C& container) : m_container(container) {}
    
    bool empty() const {
        return size() == 0;
    }
    
    size_t size() const {
        return m_container.size();
    }
    
    const_iterator begin() const {
        return const_iterator(m_container.begin());
    }
    
    const_iterator end() const {
        return const_iterator(m_container.end());
    }
};

#endif
