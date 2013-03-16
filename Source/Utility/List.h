/*
 Copyright (C) 2010-2012 Kristian Duske
 
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

#ifndef TrenchBroom_List_h
#define TrenchBroom_List_h

#include <algorithm>
#include <functional>
#include <iterator>
#include <set>
#include <vector>

namespace TrenchBroom {
    namespace Utility {
        template <typename T>
        struct DeleteObject {
        public:
            void operator()(const T* ptr) const {
                delete ptr;
            }
        };
        
        template <typename T>
        inline void deleteAll(std::vector<T*>& list, typename std::vector<T*>::iterator start, typename std::vector<T*>::iterator end) {
            std::for_each(start, end, DeleteObject<T>());
            list.erase(start, end);
        }
        
        template <typename T>
        inline void deleteAll(std::vector<T*>& list, typename std::vector<T*>::iterator start) {
            deleteAll(list, start, list.end());
        }
        
        template <typename T>
        inline void deleteAll(std::vector<T*>& list, size_t toSize = 0) {
            typename std::vector<T*>::iterator start(list.begin());
            typename std::vector<T*>::iterator end(list.end());
            if (toSize > 0)
                std::advance(start, std::min(toSize, list.size()));
            
            std::for_each(start, end, DeleteObject<T>());
            list.clear();
        }
        
        template <typename T>
        inline void erase(std::vector<T>& list, T element) {
            list.erase(std::remove(list.begin(), list.end(), element), list.end());
        }
        
        template <typename T>
        inline std::vector<T> concatenate(const std::vector<T>& prefix, const std::vector<T>& suffix) {
            if (prefix.empty())
                return suffix;
            if (suffix.empty())
                return prefix;
            
            std::vector<T> result;
            result.reserve(prefix.size() + suffix.size());
            result.insert(result.end(), prefix.begin(), prefix.end());
            result.insert(result.end(), suffix.begin(), suffix.end());
            return result;
        }

        template <typename T>
        inline std::set<T> makeSet(const std::vector<T>& list) {
            std::set<T> set;
            set.reserve(list.size());
            set.insert(list.begin(), list.end());
            return set;
        }
        
        template <typename T>
        inline std::vector<T> makeList(const std::set<T>& set) {
            std::vector<T> list;
            list.reserve(set.size());
            std::copy(set.begin(), set.end(), std::back_inserter(list));
            return list;
        }
        
        template <typename T, typename O>
        inline std::vector<T> makeList(const std::set<T, O>& set) {
            std::vector<T> list;
            list.reserve(set.size());
            std::copy(set.begin(), set.end(), std::back_inserter(list));
            return list;
        }
    }
}

#endif
