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

#ifndef TrenchBroom_CastIterator_h
#define TrenchBroom_CastIterator_h

namespace TrenchBroom {
    template <class Iterator, typename OutType>
    class CastIterator {
    public:
        typedef std::forward_iterator_tag iterator_category;
        typedef OutType value_type;
        typedef typename Iterator::difference_type difference_type;
        typedef OutType* pointer;
        typedef OutType& reference;
    private:
        Iterator m_iterator;
    public:
        CastIterator(const Iterator& iterator) :
        m_iterator(iterator) {}
        
        OutType& operator*() const {
            return (OutType&)(*m_iterator);
        }

        OutType* operator->() const {
            return &*m_iterator;
        }

        // pre-increment
        CastIterator& operator++() {
            ++m_iterator;
            return *this;
        }
        
        // post-increment
        CastIterator operator++(int) {
            CastIterator<Iterator, OutType> result(*this);
            ++*this;
            return result;
        }
        
        bool operator==(const CastIterator<Iterator, OutType>& other) const {
            return m_iterator == other.m_iterator;
        }
        
        bool operator!=(const CastIterator<Iterator, OutType>& other) const {
            return !(*this == other);
        }
    };

    template <class OutType>
    struct MakeCastIterator {
        template <class Iterator>
        static CastIterator<Iterator, OutType> castIterator(const Iterator& iterator) {
            return CastIterator<Iterator, OutType>(iterator);
        }
    };
    
}


#endif
