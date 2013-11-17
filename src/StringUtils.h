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

#ifndef TrenchBroom_StringUtils_h
#define TrenchBroom_StringUtils_h

#include <cassert>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

typedef std::string String;
typedef std::stringstream StringStream;
typedef std::set<String> StringSet;
typedef std::vector<String> StringList;
typedef std::map<String, String> StringMap;
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

    typedef StringLess<CaseSensitiveCharCompare> CaseSensitiveStringLess;
    typedef StringLess<CaseSensitiveCharCompare> CaseInsensitiveStringLess;
    
    String formatString(const char* format, va_list arguments);
    String trim(const String& str, const String& chars = " \n\t\r");

    bool isPrefix(const String& str, const String& prefix);
    bool containsCaseSensitive(const String& haystack, const String& needle);
    bool containsCaseInsensitive(const String& haystack, const String& needle);
    void sortCaseSensitive(StringList& strs);
    void sortCaseInsensitive(StringList& strs);
    bool caseSensitiveEqual(const String& str1, const String& str2);
    bool caseInsensitiveEqual(const String& str1, const String& str2);
    long makeHash(const String& str);
    String toLower(const String& str);
    String replaceChars(const String& str, const String& needles, const String& replacements);
    String capitalize(const String& str);
    String escape(const String& str, const String& chars);
    String unescape(const String& str, const String& chars);

    template <typename D>
    StringList split(const String& str, D d) {
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
    StringList splitAndTrim(const String& str, D d) {
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
            const String item = trim(str.substr(lastPos, pos - lastPos));
            if (!item.empty())
                result.push_back(item);
            lastPos = ++pos;
        }
        if (lastPos <= last) {
            const String item = trim(str.substr(lastPos, last - lastPos + 1));
            if (!item.empty())
                result.push_back(item);
        }
        return result;
    }
    
    template <typename D1, typename D2, typename D3>
    String join(const StringList& strs, const D1& delim, const D2& lastDelim, const D3& delimForTwo) {
        if (strs.empty())
            return "";
        if (strs.size() == 1)
            return strs[0];

        StringStream result;
        if (strs.size() == 2) {
            result << strs[0] << delimForTwo << strs[1];
            return result.str();
        }
        
        result << strs[0];
        for (size_t i = 1; i < strs.size() - 1; i++)
            result << delim << strs[i];
        result << lastDelim << strs.back();
        return result.str();
    }
    
    template <typename D>
    String join(const StringList& strs, const D& d) {
        return join(strs, d, d, d);
    }
}

#endif
