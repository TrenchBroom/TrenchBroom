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

#ifndef TrenchBroom_StringUtils_h
#define TrenchBroom_StringUtils_h

#include <cassert>
#include <string>
#include <sstream>
#include <vector>

typedef std::string String;
typedef std::stringstream StringStream;
typedef std::vector<String> StringList;
static const StringList EmptyStringList;

namespace StringUtils {
    inline String trim(const String& str, const String& chars = " \n\t\r") {
        if (str.length() == 0)
            return str;
        
        size_t first = str.find_first_not_of(chars.c_str());
        if (first == String::npos)
            return "";
        
        size_t last = str.find_last_not_of(chars.c_str());
        if (first > last)
            return "";
        
        return str.substr(first, last - first + 1);
    }

    inline StringList split(const String& str, char d) {
        if (str.empty())
            return EmptyStringList;

        const size_t first = str.find_first_not_of(d);
        if (first == String::npos)
            return EmptyStringList;
        const size_t last = str.find_last_not_of(d);
        assert(last != String::npos);
        assert(first <= last);
        
        StringList result;

        size_t lastPos = first;
        size_t pos = lastPos;
        while ((pos = str.find(d, pos)) < last) {
            result.push_back(str.substr(lastPos, pos - lastPos));
            lastPos = ++pos;
        }
        if (lastPos <= last)
            result.push_back(str.substr(lastPos, last - lastPos + 1));
        return result;
    }
    
    template <typename D>
    inline String join(const StringList& strs, const D& d) {
        if (strs.empty())
            return "";
        if (strs.size() == 1)
            return strs[0];
        
        StringStream result;
        result << strs[0];
        for (size_t i = 1; i < strs.size(); i++)
            result << d << strs[i];
        return result.str();
    }
}

#endif
