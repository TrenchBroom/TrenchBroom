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

#include "StringUtils.h"

#include <algorithm>
#include <cstdarg>
#include <cctype> // for std::tolower

namespace StringUtils {
    String trim(const String& str, const String& chars) {
        if (str.length() == 0)
            return str;

        size_t first = str.find_first_not_of(chars);
        if (first == String::npos)
            return "";

        size_t last = str.find_last_not_of(chars);
        if (first > last)
            return "";

        return str.substr(first, last - first + 1);
    }

    bool isNumber(const String& str) {
        for (size_t i = 0; i < str.size(); ++i) {
            const char c = str[i];
            if (c < '0' || c > '9')
                return false;
        }
        return true;
    }

    bool isBlank(const String& str) {
        return str.find_first_not_of(" \t\n\r") == String::npos;
    }

    long makeHash(const String& str) {
        long hash = 0;
        for (size_t i = 0; i < str.size(); ++i)
            hash = static_cast<long>(str[i]) + (hash << 6) + (hash << 16) - hash;
        return hash;
    }

    String toLower(const String& str) {
        String result(str);
        std::transform(std::begin(result), std::end(result), std::begin(result), [](const char c) { return static_cast<char>(std::tolower(static_cast<int>(c))); });
        return result;
    }

    String toUpper(const String& str) {
        String result(str);
        std::transform(std::begin(result), std::end(result), std::begin(result), [](const char c) { return static_cast<char>(std::toupper(static_cast<int>(c))); });
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

    String escapeIfNecessary(const String& str, const String& chars, char esc) {
        if (str.empty())
            return str;

        StringStream buffer;
        const size_t length = str.length();
        for (size_t i = 0; i < length; ++i) {
            const char c = str[i];
            const bool cNeedsEscaping = (chars.find(c) != String::npos);
            if (cNeedsEscaping) {
                // if `c` is not prefixed by `esc`, insert an `esc`
                if (i == 0 || str[i - 1] != esc) {
                    buffer << esc;
                }
            }
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
}
