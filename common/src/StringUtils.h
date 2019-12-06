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

    bool isNumber(const String& str);

    bool isBlank(const String& str);

    String replaceAll(const String& str, const String& needle, const String& replacement);

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
}

#endif
