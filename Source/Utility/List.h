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
#include <vector>

namespace TrenchBroom {
    namespace Utility {
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
    }
}

#endif
