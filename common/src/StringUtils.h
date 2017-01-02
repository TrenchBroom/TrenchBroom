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

#ifndef TrenchBroom_StringUtils_h
#define TrenchBroom_StringUtils_h

#include "Macros.h"

#include <cassert>
#include <cstdarg>
#include <locale>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

typedef std::string String;
typedef std::stringstream StringStream;
typedef std::set<String> StringSet;
typedef std::vector<String> StringArray;
typedef std::map<String, String> StringMap;

static const String EmptyString("");
static const StringArray EmptyStringList(0);

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
            return std::equal(std::begin(lhs), std::end(lhs), std::begin(rhs), CharEqual<Cmp>());
        }
    };
    
    template <typename Cmp>
    struct StringLess {
        bool operator()(const String& lhs, const String& rhs) const {
            return std::lexicographical_compare(std::begin(lhs), std::end(lhs), std::begin(rhs), std::end(rhs), CharLess<Cmp>());
        }
    };
    
    typedef StringLess<CaseSensitiveCharCompare> CaseSensitiveStringLess;
    typedef StringLess<CaseInsensitiveCharCompare> CaseInsensitiveStringLess;
    
    template <typename T>
    const String& safePlural(const T count, const String& singular, const String& plural) {
        return count == 1 ? singular : plural;
    }
    
    template <typename T>
    String safePlural(const String& prefix, const T count, const String& singular, const String& plural, const String& suffix = "") {
        return prefix + safePlural(count, singular, plural) + suffix;
    }
    
    template <typename T, typename P>
    String ftos(const T v, const P precision) {
        std::ostringstream strout;
        strout.precision(static_cast<std::streamsize>(precision));
        strout << std::fixed  << v;
        
        String str = strout.str() ;
        size_t end = str.find_last_not_of('0');
        if (str[end] == '.')
            --end;
        return str.erase(end + 1);
    }
    
    String formatString(const char* format, ...);
    String formatStringV(const char* format, va_list arguments);
    String trim(const String& str, const String& chars = " \n\t\r");

    size_t findFirstDifference(const String& str1, const String& str2);
    bool isNumberedPrefix(const String& str, const String& prefix);
    bool isPrefix(const String& str, const String& prefix);
    bool isNumber(const String& str);
    
    bool containsCaseSensitive(const String& haystack, const String& needle);
    bool containsCaseInsensitive(const String& haystack, const String& needle);
    void sortCaseSensitive(StringArray& strs);
    void sortCaseInsensitive(StringArray& strs);
    
    template <typename Cmp>
    bool isEqual(const String& str1, const String& str2, const Cmp& cmp) {
        if (str1.size() != str2.size())
            return false;
        
        for (size_t i = 0; i < str1.length(); ++i) {
            if (cmp(str1[i], str2[i]) != 0)
                return false;
        }
        return true;
    }
    
    template <typename Cmp>
    bool isEqual(const char* s1, const char* e1, const String& str2, const Cmp& cmp) {
        const size_t l1 = static_cast<size_t>(e1 - s1);
        if (l1 != str2.length())
            return false;
        
        for (size_t i = 0; i < str2.length(); ++i) {
            if (cmp(s1[i], str2[i]) != 0)
                return false;
        }
        return true;
    }
    
    bool caseSensitiveEqual(const String& str1, const String& str2);
    bool caseSensitiveEqual(const char* s1, const char* e1, const String& str2);
    bool caseInsensitiveEqual(const String& str1, const String& str2);
    bool caseInsensitiveEqual(const char* s1, const char* e1, const String& str2);

    template <class Cmp>
    bool isPrefix(const String& str, const String& prefix, const Cmp& cmp) {
        if (prefix.length() > str.length())
            return false;
        
        for (size_t i = 0; i < prefix.length(); ++i) {
            if (cmp(str[i], prefix[i]) != 0)
                return false;
        }
        return true;
    }
    
    bool caseSensitivePrefix(const String& str, const String& prefix);
    bool caseInsensitivePrefix(const String& str, const String& prefix);
    
    template <class Cmp>
    bool isSuffix(const String& str, const String& suffix, const Cmp& cmp) {
        if (suffix.length() > str.length())
            return false;
        
        const size_t n = str.length() - suffix.length();
        for (size_t i = suffix.length(); i > 0; --i) {
            if (cmp(str[n + i - 1], suffix[i - 1]) != 0)
                return false;
        }
        return true;
    }

    bool caseSensitiveSuffix(const String& str, const String& suffix);
    bool caseInsensitiveSuffix(const String& str, const String& suffix);

    bool isBlank(const String& str);
    
    template <typename I, typename Eq>
    bool matchesPattern(I strCur, I strEnd, I patCur, I patEnd, const Eq& eq) {
        if (strCur == strEnd && patCur == patEnd)
            return true;

		if (patCur == patEnd)
			return false;

        // Handle escaped characters in pattern.
        if (*patCur == '\\' && (patCur + 1) != patEnd) {
            if (strCur == strEnd)
                return false;
            
            if (*(patCur + 1) == '*' ||
                *(patCur + 1) == '?' ||
                *(patCur + 1) == '\\') {
                if (*strCur != *(patCur + 1))
                    return false;
                return matchesPattern(strCur + 1, strEnd, patCur + 2, patEnd, eq);
            } else {
                return false; // Invalid escape sequence.
            }
        }
        
		// If the pattern is a star and the string is consumed
		if (*patCur == '*' && strCur == strEnd)
			return true;

		// If the pattern is a '?' and the string is consumed
		if (*patCur == '?' && strCur == strEnd)
			return false;

        // Make sure that the characters after '*' are present in the string.
        // This assumes that the pattern will not contain two consecutive '*'
        if (*patCur == '*' && (patCur + 1) != patEnd && strCur == strEnd)
            return false;

        // If the pattern contains '?', or current characters of both strings match
        if (*patCur == '?' || eq(*patCur, *strCur))
            return matchesPattern(strCur + 1, strEnd, patCur + 1, patEnd, eq);
        
        // If there is * in the pattern, then there are two possibilities
        // a) We consider the current character of the string
        // b) We ignore the current character of the string.
        if (*patCur == '*')
            return (matchesPattern(strCur,     strEnd, patCur + 1, patEnd, eq) ||
                    matchesPattern(strCur + 1, strEnd, patCur,     patEnd, eq));
        return false;
    }
    
    bool caseSensitiveMatchesPattern(const String& str, const String& pattern);
    bool caseInsensitiveMatchesPattern(const String& str, const String& pattern);
    
    long makeHash(const String& str);
    String toLower(const String& str);
    String replaceChars(const String& str, const String& needles, const String& replacements);
    String replaceAll(const String& str, const String& needle, const String& replacement);
    String capitalize(const String& str);
    String escape(const String& str, const String& chars);
    String unescape(const String& str, const String& chars);

    int stringToInt(const String& str);
    long stringToLong(const String& str);
    double stringToDouble(const String& str);
    size_t stringToSize(const String& str);
    
    template <typename D>
    StringArray split(const String& str, D d) {
        if (str.empty())
            return EmptyStringList;
        
        const size_t first = str.find_first_not_of(d);
        if (first == String::npos)
            return EmptyStringList;
        const size_t last = str.find_last_not_of(d);
        assert(last != String::npos);
        assert(first <= last);
        
        StringArray result;
        
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
    StringArray splitAndTrim(const String& str, D d) {
        if (str.empty())
            return EmptyStringList;
        
        const size_t first = str.find_first_not_of(d);
        if (first == String::npos)
            return EmptyStringList;
        const size_t last = str.find_last_not_of(d);
        assert(last != String::npos);
        assert(first <= last);
        
        StringArray result;
        
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
    
    template <typename I, typename D1, typename D2, typename D3, typename S>
    String join(I it, I end, const D1& delim, const D2& lastDelim, const D3& delimForTwo, const S& toString) {
        if (it == end)
            return "";
        
        const String first = toString(*it++);
        if (it == end)
            return first;

        StringStream result;
        result << first;
        const String second = toString(*it++);
        if (it == end) {
            result << delimForTwo << second;
            return result.str();
        }
        
        result << delim << second;
        I next = it;
        ++next;
        while (next != end) {
            result << delim << toString(*it);
            it = next;
            ++next;
        }
        result << lastDelim << toString(*it);
        return result.str();
    }
    
    template <typename T, typename D1, typename D2, typename D3, typename S>
    String join(const std::vector<T>& objs, const D1& delim, const D2& lastDelim, const D3& delimForTwo, const S& toString) {
        return join(std::begin(objs), std::end(objs), delim, lastDelim, delimForTwo, toString);
    }
    
    template <typename T, typename D, typename S>
    String join(const std::vector<T>& objs, const D& delim, const S& toString) {
        return join(objs, delim, delim, delim, toString);
    }
    
    StringArray splitAndUnescape(const String& str, char d);
    String escapeAndJoin(const StringArray& strs, char d);
    
    struct StringToString {
        const String& operator()(const String& str) const {
            return str;
        }
    };

    struct StringToSingleQuotedString {
        const String operator()(const String& str) const {
            return "'" + str + "'";
        }
    };
    
    template <typename D1, typename D2, typename D3>
    String join(const StringArray& objs, const D1& delim, const D2& lastDelim, const D3& delimForTwo) {
        return join(objs, delim, lastDelim, delimForTwo, StringToString());
    }

    template <typename D>
    String join(const StringArray& strs, const D& d) {
        return join(strs, d, d, d);
    }

    StringArray makeList(size_t count, const char* str1, ...);
    StringSet makeSet(size_t count, const char* str1, ...);
    
    template <typename Cmp>
    class SimpleStringMatcher {
    private:
        typedef enum {
            Mode_Exact,
            Mode_Prefix,
            Mode_Suffix
        } Mode;
        
        Mode m_mode;
        String m_pattern;
    public:
        SimpleStringMatcher(const String& pattern) {
            assert(!pattern.empty());
            if (pattern[0] == '*') {
                m_mode = Mode_Suffix;
                m_pattern = pattern.substr(1);
            } else if (pattern.size() > 1 &&
                       pattern[pattern.size() - 1] == '*' &&
                       pattern[pattern.size() - 2] != '\\') {
                m_mode = Mode_Prefix;
                m_pattern = pattern.substr(0, pattern.size() - 1);
            } else {
                m_mode = Mode_Exact;
                m_pattern = pattern;
            }
            m_pattern = StringUtils::replaceAll(m_pattern, "\\*", "*");
            assert(!m_pattern.empty());
        }
        
        bool matches(const String& str) const {
            switch (m_mode) {
                case Mode_Exact:
                    return isEqual(str, m_pattern, Cmp());
                case Mode_Prefix:
                    return isPrefix(str, m_pattern, Cmp());
                case Mode_Suffix:
                    return isSuffix(str, m_pattern, Cmp());
                switchDefault()
            }
        }
    };
    
    typedef SimpleStringMatcher<CaseSensitiveCharCompare> CaseSensitiveStringMatcher;
    typedef SimpleStringMatcher<CaseInsensitiveCharCompare> CaseInsensitiveStringMatcher;
}

#endif
