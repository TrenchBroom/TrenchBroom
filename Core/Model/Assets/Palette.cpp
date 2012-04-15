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
#include <fstream>
#include <cassert>

namespace TrenchBroom {
    namespace Model {
        namespace Assets {
            Palette::Palette(const string& path) {
                ifstream stream(path.c_str());
                assert(stream.is_open());
                
                stream.seekg(0, ios::end);
                m_size = stream.tellg();
                stream.seekg(0, ios::beg);
                m_data = new unsigned char[m_size];
                stream.read((char*)m_data, m_size);
                stream.close();
            }
            
            Palette::~Palette() {
                delete[] m_data;
            }
            
            void Palette::indexToRgb(const unsigned char* indexedImage, unsigned char* rgbImage, int pixelCount) const {
                for (int i = 0; i < pixelCount; i++) {
                    unsigned char index = indexedImage[i];
                    assert(index < m_size);
                    for (int j = 0; j < 3; j++)
                        rgbImage[i * 3 + j] = m_data[index * 3 + j];
                }
            }
        }
    }
}