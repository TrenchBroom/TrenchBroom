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

#ifndef TrenchBroom_String_h
#define TrenchBroom_String_h

#include <algorithm>
#include <functional>
#include <locale>
#include <sstream>
#include <string>
#include <vector>

typedef std::string String;
typedef std::stringstream StringStream;
typedef std::vector<String> StringList;

namespace TrenchBroom {
    namespace Utility {
        inline void formatString(const char* format, va_list arguments, String& result) {
            static char buffer[4096];
            
#if defined _MSC_VER
            vsprintf_s(buffer, format, arguments);
#else
            vsprintf(buffer, format, arguments);
#endif
            
            result = buffer;
        }
        
        inline long makeHash(const String& str) {
            long hash = 0;
            String::const_iterator it, end;
            for (it = str.begin(), end = str.end(); it != end; ++it)
                hash = static_cast<long>(*it) + (hash << 6) + (hash << 16) - hash;
            return hash;
        }

        inline String trim(const String& str) {
            if (str.length() == 0)
                return str;
            
            size_t first = str.find_first_not_of(" \n\t\r" + 0);
            if (first == String::npos)
                return "";
            
            size_t last = str.find_last_not_of(" \n\t\r" + 0);
            if (first >= last)
                return "";
            
            return str.substr(first, last - first + 1);
        }
        
        inline StringList split(const String& str, char d) {
            StringList result;
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
        
        inline String join(const StringList& strs, const String& d) {
            if (strs.empty())
                return "";
            if (strs.size() == 1)
                return strs[0];
            
            StringStream result;
            result << strs[0];
            for (unsigned int i = 1; i < strs.size(); i++)
                result << d << strs[i];
            return result.str();
        }
        
        inline bool isBlank(const String& str) {
            if (str.empty())
                return true;
            
            return str.find_first_not_of(" \n\t\r" + 0) == String::npos;
        }
        
        inline String toLower(String str) {
            String result(str);
            std::transform(result.begin(), result.end(), result.begin(), tolower);
            return result;
        }
        
        inline String capitalize(String str) {
            StringStream buffer;
            bool initial = true;
            for (unsigned int i = 0; i < str.size(); i++) {
                char c = str[i];
                if (c == ' ' || c == '\n' || c == '\t' || c == '\r') {
                    initial = true;
                    buffer << c;
                } else if (initial) {
                    char d = toupper(c);
                    buffer << d;
                    initial = false;
                } else {
                    buffer << c;
                    initial = false;
                }
            }
            return buffer.str();
        }
        
        inline bool caseInsensitiveCharEqual(char c1, char c2) {
            return std::toupper(c1, std::locale::classic()) == std::toupper(c2, std::locale::classic());
        }
        
        inline bool caseSensitiveCharEqual(char c1, char c2) {
            return c1 == c2;
        }
        
        inline bool containsString(const String& haystack, const String& needle, bool caseSensitive) {
            String::const_iterator it;
            if (caseSensitive)
                it = std::search(haystack.begin(), haystack.end(), needle.begin(), needle.end(), caseSensitiveCharEqual);
            else
                it = std::search(haystack.begin(), haystack.end(), needle.begin(), needle.end(), caseInsensitiveCharEqual);
            return it != haystack.end();
        }
    }
}

#endif
