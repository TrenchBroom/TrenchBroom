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
#include <cstdio>
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
        struct CaseSensitiveCharCompare {
        public:
            int operator()(char lhs, char rhs) const {
                return lhs - rhs;
            }
        };
        
        struct CaseInsensitiveCharCompare {
        private:
            const std::locale& m_locale;
        public:
            CaseInsensitiveCharCompare(const std::locale& loc = std::locale::classic()) :
            m_locale(loc) {}
            
            int operator()(char lhs, char rhs) const {
                return std::tolower(lhs, m_locale) - std::tolower(rhs, m_locale);
            }
        };
        
        template <typename Cmp>
        struct CharEqual {
        private:
            Cmp m_compare;
        public:
            bool operator()(char lhs, char rhs) const {
                return m_compare(lhs, rhs) == 0;
            }
        };
        
        template <typename Cmp>
        struct CharLess {
        private:
            Cmp m_compare;
        public:
            bool operator()(char lhs, char rhs) const {
                return m_compare(lhs, rhs) < 0;
            }
        };

        template <typename Cmp>
        struct StringEqual {
        public:
            bool operator()(const String& lhs, const String& rhs) const {
                String::const_iterator lhsEnd, rhsEnd;
                std::advance(lhsEnd = lhs.begin(), std::min(lhs.size(), rhs.size()));
                std::advance(rhsEnd = rhs.begin(), std::min(lhs.size(), rhs.size()));
                return std::lexicographical_compare(lhs.begin(), lhsEnd, rhs.begin(), rhsEnd, CharEqual<Cmp>());
            }
        };
        
        template <typename Cmp>
        struct StringLess {
            bool operator()(const String& lhs, const String& rhs) const {
                String::const_iterator lhsEnd, rhsEnd;
                std::advance(lhsEnd = lhs.begin(), std::min(lhs.size(), rhs.size()));
                std::advance(rhsEnd = rhs.begin(), std::min(lhs.size(), rhs.size()));
                return std::lexicographical_compare(lhs.begin(), lhsEnd, rhs.begin(), rhsEnd, CharLess<Cmp>());
            }
        };
        
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
            size_t lastIndex = 0;
            for (size_t i = 0; i < str.length(); i++) {
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
            for (size_t i = 1; i < strs.size(); i++)
                result << d << strs[i];
            return result.str();
        }
        
        inline void sort(StringList& strs) {
            std::sort(strs.begin(), strs.end());
        }
        
        inline bool isBlank(const String& str) {
            if (str.empty())
                return true;
            
            return str.find_first_not_of(" \n\t\r" + 0) == String::npos;
        }
        
        inline String toLower(const String& str) {
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

        inline bool containsString(const String& haystack, const String& needle, bool caseSensitive = true) {
            String::const_iterator it;
            if (caseSensitive)
                it = std::search(haystack.begin(), haystack.end(), needle.begin(), needle.end(), CharEqual<CaseSensitiveCharCompare>());
            else
                it = std::search(haystack.begin(), haystack.end(), needle.begin(), needle.end(),  CharEqual<CaseInsensitiveCharCompare>());
            return it != haystack.end();
        }
        
        inline bool equalsString(const String& str1, const String& str2, bool caseSensitive = true) {
            if (str1.length() != str2.length())
                return false;
            return containsString(str1, str2, caseSensitive);
        }
        
        inline bool startsWith(const String& haystack, const String& needle, bool caseSensitive = true) {
            if (needle.size() > haystack.size())
                return false;
            
            String::const_iterator hEnd = haystack.begin();
            std::advance(hEnd, needle.size());
            
            if (caseSensitive)
                return std::equal(haystack.begin(), hEnd, needle.begin(), CharEqual<CaseSensitiveCharCompare>());
            return std::equal(haystack.begin(), hEnd, needle.begin(), CharEqual<CaseInsensitiveCharCompare>());
        }
    }
}
#endif
