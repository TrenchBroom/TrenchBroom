/*
 Copyright (C) 2010-2017 Kristian Duske

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
#include "StringList.h"
#include "StringSet.h"
#include "StringStream.h"
#include "StringType.h"

#include <cassert>
#include <cstdarg>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

static const StringList EmptyStringList(0);

namespace StringUtils {
    const String& choose(bool predicate, const String& positive, const String& negative);

    template <typename T>
    const String& safePlural(const T count, const String& singular, const String& plural) {
        return choose(count == 1, singular, plural);
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
        if (str[end] == '.') {
            --end;
        }
        return str.erase(end + 1);
    }

    // remove in favor of using variadic template
    String formatString(const char* format, ...);

    // remove in favor of using variadic template
    String formatStringV(const char* format, va_list arguments);

    String trim(const String& str, const String& chars = " \n\t\r");

    bool isNumber(const String& str);

    bool isBlank(const String& str);

    long makeHash(const String& str); // unused
    String toLower(const String& str);
    String toUpper(const String& str);
    String replaceChars(const String& str, const String& needles, const String& replacements); // unused
    String replaceAll(const String& str, const String& needle, const String& replacement);
    String capitalize(const String& str);
    String escape(const String& str, const String& chars, char esc = '\\');
    String escapeIfNecessary(const String& str, const String& chars, char esc = '\\');
    String unescape(const String& str, const String& chars, char esc = '\\');

    // use std::to_string?
    template <typename T>
    String toString(const T& t) {
        StringStream str;
        str << t;
        return str.str();
    }

    int stringToInt(const String& str); // nused
    long stringToLong(const String& str); // only used in stringToSize
    double stringToDouble(const String& str); // unused
    size_t stringToSize(const String& str); // only used in Autosaver

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

    // should just be using a general identity mapping
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

    template <typename C, typename D1, typename D2, typename D3, typename S = StringToString>
    String join(const C& objs, const D1& delim, const D2& lastDelim, const D3& delimForTwo, const S& toString = S()) {
        return join(std::begin(objs), std::end(objs), delim, lastDelim, delimForTwo, toString);
    }

    template <typename C, typename D = String, typename S = StringToString>
    String join(const C& objs, const D& delim = D(", "), const S& toString = S()) {
        return join(std::begin(objs), std::end(objs), delim, delim, delim, toString);
    }

    StringList splitAndUnescape(const String& str, char d);
    String escapeAndJoin(const StringList& strs, char d);

    // remove in favor of initialization list
    StringList makeList(size_t count, const char* str1, ...);

    // remove in favor of initialization list
    StringSet makeSet(size_t count, const char* str1, ...);
}

#endif
