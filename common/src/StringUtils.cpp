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

#include "StringUtils.h"

#include <algorithm>
#include <cstdarg>
#include <cstdio>

namespace StringUtils {
    String formatString(const char* format, ...) {
        va_list(arguments);
        va_start(arguments, format);
        const String message = formatStringV(format, arguments);
        va_end(arguments);
        return message;
    }
    
    String formatStringV(const char* format, va_list arguments) {
        static const int BUFFERSIZE = 8192;
        static char buffer[BUFFERSIZE];
        
        const int count =
#if defined _MSC_VER
        vsprintf_s(buffer, format, arguments);
#else
        vsprintf(buffer, format, arguments);
#endif
        if (count <= 0)
            return EmptyString;
        return String(buffer, static_cast<size_t>(count));
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
    
    size_t findFirstDifference(const String& str1, const String& str2) {
        const size_t max = std::min(str1.size(), str2.size());
        size_t index = 0;
        while (index < max) {
            if (str1[index] != str2[index])
                break;
            ++index;
        }
        return index;
    }

    bool isNumberedPrefix(const String& str, const String& prefix) {
        if (prefix.empty())
            return true;
        if (prefix.size() > str.size())
            return false;
        
        const size_t firstDiff = findFirstDifference(str, prefix);
        if (firstDiff < prefix.size())
            return false;

        return isNumber(String(str, firstDiff, str.length() - firstDiff));
    }

    bool isPrefix(const String& str, const String& prefix) {
        if (prefix.empty())
            return true;
        if (prefix.size() > str.size())
            return false;
        
        const size_t firstDiff = findFirstDifference(str, prefix);
        return firstDiff == prefix.size();
    }
    
    bool isNumber(const String& str) {
        for (size_t i = 0; i < str.size(); ++i) {
            const char c = str[i];
            if (c < '0' || c > '9')
                return false;
        }
        return true;
    }

    bool containsCaseSensitive(const String& haystack, const String& needle) {
        return std::search(std::begin(haystack), std::end(haystack), std::begin(needle), std::end(needle), CharEqual<CaseSensitiveCharCompare>()) != std::end(haystack);
    }
    
    bool containsCaseInsensitive(const String& haystack, const String& needle) {
        return std::search(std::begin(haystack), std::end(haystack), std::begin(needle), std::end(needle),  CharEqual<CaseInsensitiveCharCompare>()) != std::end(haystack);
    }
    
    void sortCaseSensitive(StringList& strs) {
        std::sort(std::begin(strs), std::end(strs), StringLess<CaseSensitiveCharCompare>());
    }
    
    void sortCaseInsensitive(StringList& strs) {
        std::sort(std::begin(strs), std::end(strs), StringLess<CaseInsensitiveCharCompare>());
    }
    
    bool caseSensitiveEqual(const String& str1, const String& str2) {
        return isEqual(str1, str2, CaseSensitiveCharCompare());
    }
    
    bool caseSensitiveEqual(const char* s1, const char* e1, const String& str2) {
        return isEqual(s1, e1, str2, CaseSensitiveCharCompare());
    }
    
    bool caseInsensitiveEqual(const String& str1, const String& str2) {
        return isEqual(str1, str2, CaseInsensitiveCharCompare());
    }
    
    bool caseInsensitiveEqual(const char* s1, const char* e1, const String& str2) {
        return isEqual(s1, e1, str2, CaseInsensitiveCharCompare());
    }

    bool caseSensitivePrefix(const String& str, const String& prefix) {
        return isPrefix(str, prefix, CaseSensitiveCharCompare());
    }
    
    bool caseInsensitivePrefix(const String& str, const String& prefix) {
        return isPrefix(str, prefix, CaseInsensitiveCharCompare());
    }
    
    bool caseSensitiveSuffix(const String& str, const String& suffix) {
        return isSuffix(str, suffix, CaseSensitiveCharCompare());
    }
    
    bool caseInsensitiveSuffix(const String& str, const String& suffix) {
        return isSuffix(str, suffix, CaseInsensitiveCharCompare());
    }

    bool isBlank(const String& str) {
        return str.find_first_not_of(" \t\n\r") == String::npos;
    }

    bool caseSensitiveMatchesPattern(const String& str, const String& pattern) {
        return matchesPattern(std::begin(str), std::end(str), std::begin(pattern), std::end(pattern), StringUtils::CharEqual<StringUtils::CaseSensitiveCharCompare>());
    }
    
    bool caseInsensitiveMatchesPattern(const String& str, const String& pattern) {
        return matchesPattern(std::begin(str), std::end(str), std::begin(pattern), std::end(pattern), StringUtils::CharEqual<StringUtils::CaseInsensitiveCharCompare>());
    }

    long makeHash(const String& str) {
        long hash = 0;
        for (size_t i = 0; i < str.size(); ++i)
            hash = static_cast<long>(str[i]) + (hash << 6) + (hash << 16) - hash;
        return hash;
    }
    
    String toLower(const String& str) {
        String result(str);
        std::transform(std::begin(result), std::end(result), std::begin(result), tolower);
        return result;
    }
    
    String replaceChars(const String& str, const String& needles, const String& replacements) {
        if (replacements.empty() || needles.empty() || str.empty())
            return str;
        
        String result = str;
        for (size_t i = 0; i < needles.size(); ++i) {
            if (result[i] == needles[i])
                result[i] = replacements[std::max(i, replacements.size())];
        }
        return result;
    }
    
    String replaceAll(const String& str, const String& needle, const String& replacement) {
        String result = str;
        size_t pos = result.find(needle);
        while (pos != String::npos) {
            result.replace(pos, needle.length(), replacement);
            pos = result.find(needle, pos + replacement.length());
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
    
    String escape(const String& str, const String& chars, const char esc) {
        if (str.empty())
            return str;
        
        StringStream buffer;
        for (size_t i = 0; i < str.size(); ++i) {
            const char c = str[i];
            if (c == esc || chars.find_first_of(c) != String::npos)
                buffer << esc;
            buffer << c;
        }
        return buffer.str();
    }

    String unescape(const String& str, const String& chars, const char esc) {
        if (str.empty())
            return str;
        
        bool escaped = false;
        StringStream buffer;
        for (size_t i = 0; i < str.size(); ++i) {
            const char c = str[i];
            if (c == esc) {
                if (escaped)
                    buffer << c;
                escaped = !escaped;
            } else {
                if (escaped && chars.find_first_of(c) == String::npos)
                    buffer << '\\';
                buffer << c;
                escaped = false;
            }
        }
        
        if (escaped)
            buffer << '\\';
        
        return buffer.str();
    }

    int stringToInt(const String& str) {
        return std::atoi(str.c_str());
    }
    
    long stringToLong(const String& str) {
        return std::atol(str.c_str());
    }
    
    double stringToDouble(const String& str) {
        return std::atof(str.c_str());
    }

    size_t stringToSize(const String& str) {
        const long longValue = stringToLong(str);
        assert(longValue >= 0);
        return static_cast<size_t>(longValue);
    }
    
    StringList splitAndUnescape(const String& str, const char d) {
        StringStream escapedStr;
        escapedStr << d << '\\';
        const String escaped = escapedStr.str();
        
        StringList result;
        char l = 0;
        char ll = 0;
        size_t li = 0;
        for (size_t i = 0; i < str.size(); ++i) {
            const char c = str[i];
            
            if (c == d && (l != '\\' || ll == '\\')) {
                result.push_back(unescape(str.substr(li, i-li), escaped));
                li = i+1;
            }
            
            ll = l;
            l = c;
        }
        
        if (!str.empty() && li <= str.size())
            result.push_back(unescape(str.substr(li), escaped));
        
        return result;
    }
    
    String escapeAndJoin(const StringList& strs, const char d) {
        StringStream escapedStr;
        escapedStr << d << '\\';
        const String escaped = escapedStr.str();
        
        StringStream buffer;

        for (size_t i = 0; i < strs.size(); ++i) {
            const String& str = strs[i];
            buffer << escape(str, escaped);
            if (i < strs.size() - 1)
                buffer << d;
        }
        
        return buffer.str();
    }

    StringList makeList(const size_t count, const char* str1, ...) {
        StringList result;
        result.reserve(count);
        result.push_back(str1);
        
        va_list(strs);
        va_start(strs, str1);
        for (size_t i = 0; i < count - 1; ++i)
            result.push_back(va_arg(strs, const char*));
        va_end(strs);
        
        return result;
    }
    
    StringSet makeSet(const size_t count, const char* str1, ...) {
        StringSet result;
        result.insert(str1);
        
        va_list(strs);
        va_start(strs, str1);
        for (size_t i = 0; i < count - 1; ++i)
            result.insert(va_arg(strs, const char*));
        va_end(strs);
        
        return result;
    }

}
