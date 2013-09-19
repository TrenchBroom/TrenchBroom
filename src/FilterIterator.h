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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TrenchBroom_FilterIterator_h
#define TrenchBroom_FilterIterator_h

namespace TrenchBroom {
    template <class Iterator, class Filter>
    class FilterIterator {
    public:
        typedef std::forward_iterator_tag iterator_category;
        typedef typename Iterator::value_type value_type;
        typedef typename Iterator::difference_type difference_type;
        typedef typename Iterator::pointer pointer;
        typedef typename Iterator::reference reference;
    private:
        Iterator m_cur;
        Iterator m_end;
        Filter m_filter;
    public:
        FilterIterator(const Iterator& cur, const Iterator& end, const Filter& filter) :
        m_cur(cur),
        m_end(end),
        m_filter(filter) {
            advance();
        }
        
        reference operator*() const {
            return *m_cur;
        }
        
        reference operator*() {
            return *m_cur;
        }
        
        pointer operator->() const {
            return &*m_cur;
        }
        
        pointer operator->() {
            return &*m_cur;
        }
        
        // pre-increment
        FilterIterator& operator++() {
            ++m_cur;
            advance();
            return *this;
        }
        
        // post-increment
        FilterIterator operator++(int) {
            FilterIterator<Iterator, Filter> result(*this);
            ++*this;
            return result;
        }
        
        bool operator==(const FilterIterator<Iterator, Filter>& other) const {
            return m_cur == other.m_cur && m_end == other.m_end;
        }
        
        bool operator!=(const FilterIterator<Iterator, Filter>& other) const {
            return !(*this == other);
        }
    private:
        void advance() {
            while (m_cur != m_end && !m_filter(*m_cur))
                ++m_cur;
        }
    };

    template <class Iterator, class Filter>
    FilterIterator<Iterator, Filter> filterIterator(const Iterator& cur, const Iterator& end, const Filter& filter) {
        return FilterIterator<Iterator, Filter>(cur, end, filter);
    }
    
}

#endif
