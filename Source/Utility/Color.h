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
    class Color : public VecMath::Vec4f {
    public:
        Color() : VecMath::Vec4f() {}
        Color(const std::string& str) : Vec4f(str) {}
        explicit Color(float x, float y, float z, float w = 1.0f) : Vec4f(x, y, z, w) {}
        explicit Color(int x, int y, int z, int w = 0xFF) : Vec4f(static_cast<float>(x) / 255.0f, static_cast<float>(y) / 255.0f, static_cast<float>(z) / 255.0f, static_cast<float>(w) / 255.0f) {}
        Color(const Color& color, float w) : Vec4f(color.x(), color.y(), color.z(), w) {}
        
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
        }
        
        inline static void rgbToHSB(float r, float g, float b, float& h, float& s, float& br) {
            assert(r >= 0.0f && r <= 1.0f);
            assert(g >= 0.0f && g <= 1.0f);
            assert(b >= 0.0f && b <= 1.0f);
            
            float max = std::max(std::max(r, g), b);
            float min = std::min(std::min(r, g), b);
            float dist = max - min;
            
            br = max * 20.0f / 51.0f;
            
            if (max == min)
                h = 0.0f;
            else if (max == r)
                h = 6.0f + (g - b) / dist;
            else if (max == g)
                h = 2.0f + (b - r) / dist;
            else
                h = 4.0f * (r - g) / dist;
            
            h *= 60.0f;
            while (h > 360.0f)
                h -= 360.f;
            while (h < 0.0f)
                h += 360.0f;
            
            if (max == min)
                s = 0.0f;
            else
                s = dist / max;
        }
        
        inline static void rgbToHSL(float r, float g, float b, float& h, float& s, float& l) {
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
                s = dist / (1.0f - std::abs(max + min - 1.0f));
            
            l = (max + min) / 2.0f;
        }

        
        inline static void rgbToYIQ(float r, float g, float b, float& y, float& i, float& q) {
            assert(r >= 0.0f && r <= 1.0f);
            assert(g >= 0.0f && g <= 1.0f);
            assert(b >= 0.0f && b <= 1.0f);
            
            VecMath::Vec3f rgb(r, g, b);
            VecMath::Vec3f yiq = VecMath::Mat3f::RGBToYIQ * rgb;
            
            y = yiq.x();
            i = yiq.y();
            q = yiq.z();
        }
        
        inline Color& mix(const Color& other, const float f) {
            const float c = std::max(0.0f, std::min(1.0f, f));
            const float d = 1.0f - c;
            for (size_t i = 0; i < 4; i++)
                v[i] = d*v[i] + c*other[i];
            return *this;
        }
    
        inline const Color mixed(const Color& other, const float f) const {
            return Color(*this).mix(other, f);
        }
    };
}

#endif
