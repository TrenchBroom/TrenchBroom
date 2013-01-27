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

#ifndef TrenchBroom_Color_h
#define TrenchBroom_Color_h

#include "Utility/VecMath.h"

#include <cassert>
#include <cmath>

namespace TrenchBroom {
    class Color : public Math::Vec4f {
    public:
        Color() : Vec4f() {}
        Color(const std::string& str) : Vec4f(str) {}
        Color(float x, float y, float z, float w) : Vec4f(x, y, z, w) {}
        Color(const Color& color, float w) : Vec4f(color, w) {}
        
        inline static void rgbToHSV(float r, float g, float b, float& h, float& s, float& v) {
            assert(r >= 0.0f && r <= 1.0f);
            assert(g >= 0.0f && g <= 1.0f);
            assert(b >= 0.0f && b <= 1.0f);
            
            float max = std::max(std::max(r, g), b);
            float min = std::min(std::min(r, g), b);
            float dist = max - min;
            
            if (max == min)
                h = 0.0f;
            else if (max == r)
                h = (g - b) / dist;
            else if (max == g)
                h = 2.0f + (b - r) / dist;
            else
                h = 4.0f * (r - g) / dist;
            
            h *= 60.0f;
            if (h < 0.0f)
                h += 360.0f;
            
            if (max == min)
                s = 0.0f;
            else
                s = dist / max;
            
            v = max;
            
            assert(h >= 0.0f && h <= 360.0f);
            assert(s >= 0.0f && s <= 1.0f);
            assert(v >= 0.0f && v <= 1.0f);
        }
        
        inline static void rgbToYIQ(float r, float g, float b, float& y, float& i, float& q) {
            assert(r >= 0.0f && r <= 1.0f);
            assert(g >= 0.0f && g <= 1.0f);
            assert(b >= 0.0f && b <= 1.0f);
            
            Math::Vec3f rgb(r, g, b);
            Math::Vec3f yiq = Math::Mat3f::RGBToYIQ * rgb;
            
            y = yiq.x;
            i = yiq.y;
            q = yiq.z;
        }
    };
}

#endif
