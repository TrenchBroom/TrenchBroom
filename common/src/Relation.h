/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#ifndef Relation_h
#define Relation_h

#include <cassert>
#include <map>
#include <set>

#include "Macros.h"

template <typename L, typename R, typename Cmp_L = std::less<L>, typename Cmp_R = std::less<R> >
class relation {
private:
    typedef std::set<L, Cmp_L> left_set;
    typedef std::set<R, Cmp_R> right_set;
    typedef std::map<L, right_set, Cmp_L> left_right_map;
    typedef std::map<R, left_set, Cmp_R> right_left_map;
public:
    typedef typename left_set::const_iterator const_left_iterator;
    typedef typename right_set::const_iterator const_right_iterator;
    typedef std::pair<const_left_iterator, const_left_iterator> const_left_range;
    typedef std::pair<const_right_iterator, const_right_iterator> const_right_range;
private:
    left_right_map m_left_right_map;
    right_left_map m_right_left_map;
    size_t m_size;
public:
    relation() : m_size(0) {}
    relation(const std::map<L, R, Cmp_L>& entries) :
    m_size(0) {
        for (const auto& entry : entries)
            insert(entry.first, entry.second);
    }
public:
    void insert(const relation<L, R>& other) {
        typename left_right_map::const_iterator o_lr_it = std::begin(other.m_left_right_map);
        typename left_right_map::const_iterator o_lr_end = std::end(other.m_left_right_map);
        while (o_lr_it != o_lr_end) {
            const L& l = o_lr_it->first;
            const right_set& o_r = o_lr_it->second;
            
            right_set& m_r = find_or_insert_right(l);
            const size_t m_r_s = m_r.size();
            
            m_r.insert(std::begin(o_r), std::end(o_r));
            m_size += (m_r.size() - m_r_s);
            
            ++o_lr_it;
        }
        
        typename right_left_map::const_iterator o_rl_it = std::begin(other.m_right_left_map);
        typename right_left_map::const_iterator o_rl_end = std::end(other.m_right_left_map);
        while (o_rl_it != o_rl_end) {
            const R& r = o_rl_it->first;
            const left_set& o_l = o_rl_it->second;
            left_set& m_l = find_or_insert_left(r);
            m_l.insert(std::begin(o_l), std::end(o_l));
            ++o_rl_it;
        }
    }
    
    template <typename I>
    void insert(const L& l, const std::pair<I, I> r_range) {
        insert(l, r_range.first, r_range.second);
    }
    
    template <typename I>
    void insert(const L& l, I r_cur, I r_end) {
        typename left_right_map::iterator lrIt = m_left_right_map.find(l);
        if (lrIt == std::end(m_left_right_map))
            lrIt = m_left_right_map.insert(std::make_pair(l, right_set())).first;
        
        while (r_cur != r_end) {
            const R& r = *r_cur;
            if (lrIt->second.insert(r).second) {
                insert_right_to_left(l, r);
                ++m_size;
            }
            ++r_cur;
        }
    }
    
    template <typename I>
    void insert(const std::pair<I, I> l_range, const R& r) {
        insert(l_range.first, l_range.second, r);
    }
    
    template <typename I>
    void insert(I l_cur, I l_end, const R& r) {
        typename right_left_map::iterator rlIt = m_right_left_map.find(r);
        if (rlIt == std::end(m_right_left_map))
            rlIt = m_right_left_map.insert(std::make_pair(r, left_set())).first;
        
        while (l_cur != l_end) {
            const L& l = *l_cur;
            if (rlIt->second.insert(l).second) {
                insert_left_to_right(l, r);
                ++m_size;
            }
            ++l_cur;
        }
    }
    
    bool insert(const L& l, const R& r) {
        if (!insert_left_to_right(l, r))
            return false;
        assertResult(insert_right_to_left(l, r));
        ++m_size;
        return true;
    }
private:
    bool insert_left_to_right(const L& l, const R& r) {
        return find_or_insert_right(l).insert(r).second;
    }
    
    bool insert_right_to_left(const L& l, const R& r) {
        return find_or_insert_left(r).insert(l).second;
    }
public:
    bool erase(const L& l, const R& r) {
        typename left_right_map::iterator lrIt = m_left_right_map.find(l);
        if (lrIt == std::end(m_left_right_map))
            return false;
        
        right_set& right = lrIt->second;
        if (right.erase(r) > 0) {
            typename right_left_map::iterator rlIt = m_right_left_map.find(r);
            assert(rlIt != std::end(m_right_left_map));
            
            left_set& left = rlIt->second;
            assertResult(left.erase(l) > 0);
            --m_size;
            
            return true;
        } else {
            assert(find_left(r).count(l) == 0);
            return false;
        }
    }
public:
    bool empty() const {
        return size() == 0;
    }
    
    size_t size() const {
        return m_size;
    }
    
    bool contains(const L& l, const R& r) {
        return find_right(l).count(r) > 0;
    }
    
    size_t count_left(const R& r) const {
        return find_left(r).size();
    }
    
    size_t count_right(const L& l) const {
        return find_right(l).size();
    }
    
    const_left_range left_range(const R& r) const {
        const left_set& left = find_left(r);
        return std::make_pair(std::begin(left), std::end(left));
    }
    
    const_left_iterator left_begin(const R& r) const {
        return find_left(r).begin();
    }
    
    const_left_iterator left_end(const R& r) const {
        return find_left(r).end();
    }
    
    const_right_range right_range(const L& l) const {
        const right_set& right = find_right(l);
        return std::make_pair(std::begin(right), std::end(right));
    }
    
    const_right_iterator right_begin(const L& l) const {
        return find_right(l).begin();
    }
    
    const_right_iterator right_end(const L& l) const {
        return find_right(l).end();
    }
private:
    const left_set& find_left(const R& r) const {
        static const left_set EMPTY_LEFT_SET;
        
        typename right_left_map::const_iterator rlIt = m_right_left_map.find(r);
        if (rlIt == std::end(m_right_left_map))
            return EMPTY_LEFT_SET;
        return rlIt->second;
    }
    
    const right_set& find_right(const L& l) const {
        static const right_set EMPTY_RIGHT_SET;
        
        typename left_right_map::const_iterator lrIt = m_left_right_map.find(l);
        if (lrIt == std::end(m_left_right_map))
            return EMPTY_RIGHT_SET;
        return lrIt->second;
    }
private:
    left_set& find_or_insert_left(const R& r) {
        typename right_left_map::iterator rlIt = m_right_left_map.find(r);
        if (rlIt == std::end(m_right_left_map))
            rlIt = m_right_left_map.insert(std::make_pair(r, left_set())).first;
        return rlIt->second;
    }
    
    right_set& find_or_insert_right(const L& l) {
        typename left_right_map::iterator lrIt = m_left_right_map.find(l);
        if (lrIt == std::end(m_left_right_map))
            lrIt = m_left_right_map.insert(std::make_pair(l, right_set())).first;
        return lrIt->second;
    }
};

#endif /* Relation_h */
