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

#include "Utils.h"
#include <algorithm>
#include <locale>

namespace TrenchBroom {
    std::string trim(const std::string& str) {
        if (str.length() == 0) return str;
        size_t first = str.find_first_not_of(" \n\t\r" + 0);
        size_t last = str.find_last_not_of(" \n\t\r" + 0);
        if (first >= last) return "";
        return str.substr(first, last - first + 1);
    }
    
    std::vector<std::string> split(const std::string& str, char d) {
        std::vector<std::string> result;
        unsigned int lastIndex = 0;
        for (unsigned int i = 0; i < str.length(); i++) {
            char c = str[i];
            if (c == d && lastIndex < i) {
                result.push_back(str.substr(lastIndex, i - lastIndex));
                lastIndex = i + 1;
            }
        }
        if (lastIndex < str.length() - 1)
            result.push_back(str.substr(lastIndex, str.length() - lastIndex));
        return result;
    }
    
    bool caseInsensitiveCharEqual(char c1, char c2) {
		return std::toupper(c1, std::locale::classic()) == std::toupper(c2, std::locale::classic());
    }
    
    bool caseSensitiveCharEqual(char c1, char c2) {
        return c1 == c2;
    }
    
    bool containsString(const std::string& haystack, const std::string& needle, bool caseSensitive) {
        std::string::const_iterator it;
        if (caseSensitive)
            it = std::search(haystack.begin(), haystack.end(), needle.begin(), needle.end(), caseSensitiveCharEqual);
        else
            it = std::search(haystack.begin(), haystack.end(), needle.begin(), needle.end(), caseInsensitiveCharEqual);
        return it != haystack.end();
    }
}