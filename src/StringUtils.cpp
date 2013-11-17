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

#include "StringUtils.h"

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <locale>

namespace StringUtils {
    String formatString(const char* format, va_list arguments) {
        static char buffer[4096];
        
#if defined _MSC_VER
        vsprintf_s(buffer, format, arguments);
#else
        vsprintf(buffer, format, arguments);
#endif
        
        return buffer;
    }
    
    String trim(const String& str, const String& chars) {
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
    
    bool isPrefix(const String& str, const String& prefix) {
        if (prefix.empty())
            return true;
        if (prefix.size() > str.size())
            return false;
        
        for (size_t i = 0; i < prefix.size(); i++)
            if (prefix[i] != str[i])
                return false;
        return true;
    }
    
    bool containsCaseSensitive(const String& haystack, const String& needle) {
        return std::search(haystack.begin(), haystack.end(), needle.begin(), needle.end(), CharEqual<CaseSensitiveCharCompare>()) != haystack.end();
    }
    
    bool containsCaseInsensitive(const String& haystack, const String& needle) {
        return std::search(haystack.begin(), haystack.end(), needle.begin(), needle.end(),  CharEqual<CaseInsensitiveCharCompare>()) != haystack.end();
    }
    
    void sortCaseSensitive(StringList& strs) {
        std::sort(strs.begin(), strs.end(), StringLess<CaseSensitiveCharCompare>());
    }
    
    void sortCaseInsensitive(StringList& strs) {
        std::sort(strs.begin(), strs.end(), StringLess<CaseInsensitiveCharCompare>());
    }
    
    bool caseSensitiveEqual(const String& str1, const String& str2) {
        StringEqual<CaseSensitiveCharCompare> equality;
        return equality(str1, str2);
    }
    
    bool caseInsensitiveEqual(const String& str1, const String& str2) {
        StringEqual<CaseInsensitiveCharCompare> equality;
        return equality(str1, str2);
    }
    
    long makeHash(const String& str) {
        long hash = 0;
        String::const_iterator it, end;
        for (it = str.begin(), end = str.end(); it != end; ++it)
            hash = static_cast<long>(*it) + (hash << 6) + (hash << 16) - hash;
        return hash;
    }
    
    String toLower(const String& str) {
        String result(str);
        std::transform(result.begin(), result.end(), result.begin(), tolower);
        return result;
    }
    
    String replaceChars(const String& str, const String& needles, const String& replacements) {
        if (needles.size() != replacements.size() || needles.empty() || str.empty())
            return str;
        
        String result = str;
        for (size_t i = 0; i < needles.size(); ++i) {
            if (result[i] == needles[i])
                result[i] = replacements[i];
        }
        return result;
    }
    
    String capitalize(const String& str) {
        StringStream buffer;
        bool initial = true;
        for (size_t i = 0; i < str.size(); i++) {
            char c = str[i];
            if (c == ' ' || c == '\n' || c == '\t' || c == '\r') {
                initial = true;
                buffer << c;
            } else if (initial) {
                char d = static_cast<char>(toupper(c));
                buffer << d;
                initial = false;
            } else {
                buffer << c;
                initial = false;
            }
        }
        return buffer.str();
    }
    
    String escape(const String& str, const String& chars) {
        if (str.empty())
            return str;
        
        StringStream buffer;
        for (size_t i = 0; i < str.size(); ++i) {
            const char c = str[i];
            if (c == '\\' || chars.find_first_of(c) != String::npos)
                buffer << '\\';
            buffer << c;
        }
        return buffer.str();
    }

    String unescape(const String& str, const String& chars) {
        if (str.empty())
            return str;
        
        StringStream buffer;
        for (size_t i = 0; i < str.size(); ++i) {
            const char c = str[i];
            if (c == '\\' && i < str.size() - 1) {
                const char d = str[i+1];
                if (d != '\\' && chars.find_first_of(d) == String::npos)
                    buffer << c;
            } else {
                buffer << c;
            }
        }
        return buffer.str();
    }
}
