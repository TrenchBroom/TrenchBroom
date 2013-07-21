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

#include <algorithm>
#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <locale>
#include <string>
#include <sstream>
#include <vector>

typedef std::string String;
typedef std::stringstream StringStream;
typedef std::vector<String> StringList;
static const StringList EmptyStringList;

namespace StringUtils {
    struct CaseSensitiveCharCompare {
    public:
        int operator()(const char& lhs, const char& rhs) const {
            return lhs - rhs;
        }
    };
    
    struct CaseInsensitiveCharCompare {
    private:
        std::locale m_locale;
    public:
        CaseInsensitiveCharCompare(std::locale loc = std::locale::classic()) :
        m_locale(loc) {}
        
        int operator()(const char& lhs, const char& rhs) const {
            return std::tolower(lhs, m_locale) - std::tolower(rhs, m_locale);
        }
    };

    template <typename Cmp>
    struct CharEqual {
    private:
        Cmp m_compare;
    public:
        bool operator()(const char& lhs, const char& rhs) const {
            return m_compare(lhs, rhs) == 0;
        }
    };
    
    template <typename Cmp>
    struct CharLess {
    private:
        Cmp m_compare;
    public:
        bool operator()(const char& lhs, const char& rhs) const {
            return m_compare(lhs, rhs) < 0;
        }
    };
    
    template <typename Cmp>
    struct StringEqual {
    public:
        bool operator()(const String& lhs, const String& rhs) const {
            if (lhs.size() != rhs.size())
                return false;
            return std::equal(lhs.begin(), lhs.end(), rhs.begin(), CharEqual<Cmp>());
        }
    };
    
    template <typename Cmp>
    struct StringLess {
        bool operator()(const String& lhs, const String& rhs) const {
            typedef String::iterator::difference_type StringDiff;

            String::const_iterator lhsEnd, rhsEnd;
            const size_t minSize = std::min(lhs.size(), rhs.size());
            StringDiff difference = static_cast<StringDiff>(minSize);
            
            std::advance(lhsEnd = lhs.begin(), difference);
            std::advance(rhsEnd = rhs.begin(), difference);
            return std::lexicographical_compare(lhs.begin(), lhsEnd, rhs.begin(), rhsEnd, CharLess<Cmp>());
        }
    };

    inline String formatString(const char* format, va_list arguments) {
        static char buffer[4096];
        
#if defined _MSC_VER
        vsprintf_s(buffer, format, arguments);
#else
        vsprintf(buffer, format, arguments);
#endif
        
        return buffer;
    }

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

    template <typename D>
    inline StringList split(const String& str, D d) {
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
        while ((pos = str.find_first_of(d, pos)) < last) {
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
    
    inline bool isPrefix(const String& str, const String& prefix) {
        if (prefix.empty())
            return true;
        if (prefix.size() > str.size())
            return false;
        
        for (size_t i = 0; i < prefix.size(); i++)
            if (prefix[i] != str[i])
                return false;
        return true;
    }
    
    inline void sortCaseSensitive(StringList& strs) {
        std::sort(strs.begin(), strs.end(), StringLess<CaseSensitiveCharCompare>());
    }
    
    inline void sortCaseInsensitive(StringList& strs) {
        std::sort(strs.begin(), strs.end(), StringLess<CaseInsensitiveCharCompare>());
    }
    
    inline bool caseSensitiveEqual(const String& str1, const String& str2) {
        StringEqual<CaseSensitiveCharCompare> equality;
        return equality(str1, str2);
    }
    
    inline bool caseInsensitiveEqual(const String& str1, const String& str2) {
        StringEqual<CaseInsensitiveCharCompare> equality;
        return equality(str1, str2);
    }
}

#endif
