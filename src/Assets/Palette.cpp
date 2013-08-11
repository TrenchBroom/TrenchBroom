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

#include "Exceptions.h"
#include "StringUtils.h"

#include <algorithm>
#include <cstring>
#include <fstream>

namespace TrenchBroom {
    namespace Assets {
        Palette::Palette(const IO::Path& path) {
            if (StringUtils::caseInsensitiveEqual(path.extension(), "lmp"))
                loadLmpPalette(path);
            else if (StringUtils::caseInsensitiveEqual(path.extension(), "pcx"))
                loadPcxPalette(path);
            else
                throw FileSystemException("Unknown palette format " + path.asString());
        }

        Palette::Palette(const Palette& other) :
        m_data(NULL),
        m_size(other.m_size) {
            m_data = new char[m_size];
            memcpy(m_data, other.m_data, m_size);
        }

        void Palette::operator= (Palette other) {
            using std::swap;
            swap(m_data, other.m_data);
            swap(m_size, other.m_size);
        }

        Palette::~Palette() {
            delete[] m_data;
        }

        void Palette::indexedToRgb(const Buffer<char>& indexedImage, const size_t pixelCount, Buffer<unsigned char>& rgbImage, Color& averageColor) const {
            indexedToRgb(&indexedImage[0], pixelCount, rgbImage, averageColor);
        }

        void Palette::indexedToRgb(const char* indexedImage, const size_t pixelCount, Buffer<unsigned char>& rgbImage, Color& averageColor) const {
            double avg[3];
            avg[0] = avg[1] = avg[2] = 0.0;
            for (size_t i = 0; i < pixelCount; ++i) {
                const size_t index = static_cast<size_t>(static_cast<unsigned char>(indexedImage[i]));
                assert(index < m_size);
                for (size_t j = 0; j < 3; ++j) {
                    const size_t c = m_data[index * 3 + j];
                    rgbImage[i * 3 + j] = c;
                    avg[j] += static_cast<double>(c);
                }
            }
            
            for (size_t i = 0; i < 3; ++i)
                averageColor[i] = static_cast<float>(avg[i] / pixelCount / 0xFF);
            averageColor[3] = 1.0f;
        }

        void Palette::loadLmpPalette(const IO::Path& path) {
            std::ifstream stream(path.asString().c_str(), std::ios::binary | std::ios::in);
            if (!stream.is_open())
                throw FileSystemException("Cannot load palette " + path.asString());
            
            stream.seekg(0, std::ios::end);
            m_size = static_cast<size_t>(stream.tellg());
            stream.seekg(0, std::ios::beg);
            m_data = new char[m_size];
            
            stream.read(reinterpret_cast<char*>(m_data), static_cast<std::streamsize>(m_size));
        }
        
        void Palette::loadPcxPalette(const IO::Path& path) {
            std::ifstream stream(path.asString().c_str(), std::ios::binary | std::ios::in);
            if (!stream.is_open())
                throw FileSystemException("Cannot load palette " + path.asString());
            
            m_size = 768;
            stream.seekg(m_size + 1, std::ios::end);
            
            char magic;
            stream >> magic;
            // if (magic != 0x0C)
               // throw FileSystemException("Cannot load palette " + path.asString());
            
            m_data = new char[m_size];
            stream.read(reinterpret_cast<char*>(m_data), static_cast<std::streamsize>(m_size));
        }
    }
}
