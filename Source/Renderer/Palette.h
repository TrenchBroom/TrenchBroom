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

#ifndef __TrenchBroom__Palette__
#define __TrenchBroom__Palette__

#include "Utility/Color.h"
#include "Utility/String.h"

#include <cassert>

namespace TrenchBroom {
    namespace Renderer {
        class Palette {
        private:
            unsigned char* m_data;
            size_t m_size;
        public:
            Palette(const String& path);
            Palette(const Palette& other);
            ~Palette();
            
            void operator= (Palette other);
            
            inline void indexedToRgb(const unsigned char* indexedImage, unsigned char* rgbImage, size_t pixelCount, Color& averageColor) const {
                double avg[3];
                avg[0] = avg[1] = avg[2] = 0;
                for (unsigned int i = 0; i < pixelCount; i++) {
                    unsigned int index = indexedImage[i];
                    assert(index < m_size);
                    for (unsigned int j = 0; j < 3; j++) {
                        unsigned char c = m_data[index * 3 + j];
                        rgbImage[i * 3 + j] = c;
                        avg[j] += static_cast<double>(c);
                    }
                }

                for (unsigned int i = 0; i < 3; i++)
                    averageColor[i] = static_cast<float>(avg[i] / pixelCount / 0xFF);
                averageColor[3] = 1.0f;
            }
        };
    }
}

#endif /* defined(__TrenchBroom__Palette__) */
