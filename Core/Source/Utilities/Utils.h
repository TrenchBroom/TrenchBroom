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

#ifndef TrenchBroom_Utils_h
#define TrenchBroom_Utils_h

#include <algorithm>
#include <set>
#include <string>
#include <vector>

namespace TrenchBroom {
    std::string trim(const std::string& str);
    std::vector<std::string> split(const std::string& str, char d);
    bool isBlank(const std::string& str);

    std::string toLower(std::string str);
    bool caseInsensitiveCharEqual(char c1, char c2);
    bool caseSensitiveCharEqual(char c1, char c2);
    bool containsString(const std::string& haystack, const std::string& needle, bool caseSensitive = true);
    
    template <typename T>
    std::vector<T> difference(const std::vector<T> list1, const std::vector<T> list2) {
        if (list1.empty() || list2.empty())
        return list1;
    
        std::set<T> set1(list1.begin(), list1.end());
        std::set<T> set2(list2.begin(), list2.end());
        std::vector<T> result(set1.size() + set2.size());
        typename std::vector<T>::iterator end;
        
        end = std::set_difference(set1.begin(), set1.end(), set2.begin(), set2.end(), result.begin());
        result.resize(end - result.begin());
        return result;
    }
}

#endif
