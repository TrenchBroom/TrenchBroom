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

#include "Palette.h"

#include "Exceptions.h"
#include "StringUtils.h"

#include <algorithm>
#include <cstring>
#include <fstream>

namespace TrenchBroom {
    namespace Assets {
        Palette::Palette(unsigned char* data, const size_t size) :
        m_data(data),
        m_size(size) {
            assert(m_data != NULL);
            assert(m_size > 0);
        }

        Palette::~Palette() {
            delete[] m_data;
        }
    }
}
