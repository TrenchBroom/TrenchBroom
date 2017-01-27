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

#include "MathUtils.h"

namespace Math {
    size_t succ(const size_t index, const size_t count, const size_t offset) {
        return (index + offset) % count;
    }
    
    size_t pred(const size_t index, const size_t count, const size_t offset) {
        return ((index + count) - (offset % count)) % count;
    }

    double nextgreater(double value) {
#ifdef _MSC_VER
        return _nextafter(value, std::numeric_limits<double>::infinity());
#else
        return ::nextafter(value, std::numeric_limits<double>::infinity());
#endif
    }
}
