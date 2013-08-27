/*
 Copyright (C) 2010-2013 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TrenchBroom_NestedHierarchyIterator_h
#define TrenchBroom_NestedHierarchyIterator_h

#include <iterator>

namespace TrenchBroom {
    template <typename OuterIterator, class InnerAdapter, typename ValueType>
    class NestedHierarchyIterator {
    private:
        typedef typename InnerAdapter::InnerIterator InnerIterator;
    public:
        typedef std::forward_iterator_tag iterator_category;
        typedef ValueType value_type;
        typedef typename InnerIterator::difference_type difference_type;
        typedef ValueType* pointer;
        typedef ValueType& reference;
    private:
        InnerAdapter m_adapter;
        OuterIterator m_outerCur;
        OuterIterator m_outerEnd;
        InnerIterator m_innerCur;
        InnerIterator m_innerEnd;
        bool m_returnOuter;
    public:
        NestedHierarchyIterator() {}
        
        NestedHierarchyIterator(const OuterIterator& outerCur) :
        m_outerCur(outerCur),
        m_outerEnd(m_outerCur),
        m_returnOuter(true) {}
        
        NestedHierarchyIterator(const OuterIterator& outerCur, const OuterIterator& outerEnd) :
        m_outerCur(outerCur),
        m_outerEnd(outerEnd),
        m_returnOuter(true) {}
        
        inline reference operator*() const {
            if (m_returnOuter)
                return static_cast<reference>(*m_outerCur);
            return static_cast<reference>(*m_innerCur);
        }
        
        inline ValueType& operator*() {
            if (m_returnOuter)
                return static_cast<ValueType&>(*m_outerCur);
            return static_cast<ValueType&>(*m_innerCur);
        }
        
        inline ValueType* operator->() const {
            if (m_returnOuter)
                return &*m_outerCur;
            return &*m_innerCur;
        }
        
        // pre-increment
        inline NestedHierarchyIterator& operator++() {
            ++m_innerCur;
            m_returnOuter = false;
            if (m_innerCur == m_innerEnd)
                advanceOuter();
            return *this;
        }
        
        // post-increment
        inline NestedHierarchyIterator operator++(int) {
            NestedHierarchyIterator<OuterIterator, InnerIterator, ValueType> result(*this);
            ++*this;
            return result;
        }
    
        inline bool operator==(const NestedHierarchyIterator<OuterIterator, InnerAdapter, ValueType>& other) const {
            if (m_outerCur != other.m_outerCur)
                return false;
            if (m_outerCur != m_outerEnd &&
                other.m_outerCur != other.m_outerEnd &&
                m_innerCur != other.m_innerCur)
                return false;
            return true;
        }

        inline bool operator!=(const NestedHierarchyIterator<OuterIterator, InnerAdapter, ValueType>& other) const {
            return !(*this == other);
        }
    private:
        inline void advanceOuter() {
            if (m_outerCur == m_outerEnd)
                return;
            ++m_outerCur;
            m_innerCur = m_adapter.beginInner(m_outerCur);
            m_innerEnd = m_adapter.endInner(m_outerCur);
            m_returnOuter = true;
        }
    };
}

#endif
