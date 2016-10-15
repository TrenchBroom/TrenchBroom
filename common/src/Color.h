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

#ifndef TrenchBroom_Color_h
#define TrenchBroom_Color_h

#include "Vec.h"

class Color : public Vec<float, 4> {
public:
    static Color parse(const std::string& str);
    
    Color();
    Color(const Vec<float,4>& vec);
    Color(float r, float g, float b, float a = 1.0f);
    Color(const Color& color, float a);
    Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 0xFF);
    Color(const int r, const int g, const int b, const float a);
    Color(int r, int g, int b, int a = 0xFF);
    
    float r() const;
    float g() const;
    float b() const;
    float a() const;
    
    template <typename T>
    Color& mix(const Color& other, const T f) {
        const float c = static_cast<float>(std::max(static_cast<T>(0.0), std::min(static_cast<T>(1.0), f)));
        const float d = 1.0f - c;
        for (size_t i = 0; i < 4; i++)
            v[i] = d*v[i] + c*other[i];
        return *this;
    }
    
    const Color mixed(const Color& other, const float f) const {
        return Color(*this).mix(other, f);
    }

    static void rgbToHSB(float r, float g, float b, float& h, float& s, float& br);
};

#endif
