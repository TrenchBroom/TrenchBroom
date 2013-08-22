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

#include "Palette.h"

#include <algorithm>
#include <cstring>
#include <fstream>

namespace TrenchBroom {
    namespace Renderer {
        Palette::Palette(const String& path) {
            std::ifstream stream(path.c_str(), std::ios::binary | std::ios::in);
            assert(stream.is_open());

            stream.seekg(0, std::ios::end);
            m_size = static_cast<size_t>(stream.tellg());
            stream.seekg(0, std::ios::beg);
            m_data = new unsigned char[m_size];

            stream.read(reinterpret_cast<char*>(m_data), static_cast<std::streamsize>(m_size));
            stream.close();
        }

        Palette::Palette(const Palette& other) :
        m_data(NULL),
        m_size(other.m_size) {
            m_data = new unsigned char[m_size];
            memcpy(m_data, other.m_data, m_size);
        }

        void Palette::operator= (Palette other) {
            std::swap(m_data, other.m_data);
            std::swap(m_size, other.m_size);
        }

        Palette::~Palette() {
            delete[] m_data;
        }
    }
}
