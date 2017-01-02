
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

#include "Color.h"

Color Color::parse(const std::string& str) {
    const Vec<float, 4> vec = Vec<float, 4>::parse(str);
    return Color(vec.x(), vec.y(), vec.z(), vec.w());
}

Color::Color() :
Vec<float, 4>(0.0f, 0.0f, 0.0f, 0.0f) {}

Color::Color(const Vec<float,4>& vec) :
Vec<float, 4>(vec) {}

Color::Color(const float r, const float g, const float b, const float a) :
Vec<float, 4>(r, g, b, a) {}

Color::Color(const Color& color, const float a) :
Vec<float, 4>(color.r(), color.g(), color.b(), a) {}

Color::Color(const unsigned char r, const unsigned char g, const unsigned char b, const unsigned char a) :
Vec<float, 4>(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f) {}

Color::Color(const int r, const int g, const int b, const int a) :
Vec<float, 4>(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f) {}

Color::Color(const int r, const int g, const int b, const float a) :
Vec<float, 4>(r / 255.0f, g / 255.0f, b / 255.0f, a) {}

float Color::r() const {
    return x();
}

float Color::g() const {
    return y();
}

float Color::b() const {
    return z();
}

float Color::a() const {
    return w();
}

void Color::rgbToHSB(const float r, const float g, const float b, float& h, float& s, float& br) {
    assert(r >= 0.0f && r <= 1.0f);
    assert(g >= 0.0f && g <= 1.0f);
    assert(b >= 0.0f && b <= 1.0f);
    
    const float max = std::max(std::max(r, g), b);
    const float min = std::min(std::min(r, g), b);
    const float dist = max - min;
    
    br = max;
    if (br != 0.0f)
        s = dist / max;
    else
        s = 0.0f;
    
    if (s == 0.0f) {
        h = 0.0f;
    } else {
        const float rc = (max - r) / dist;
        const float gc = (max - g) / dist;
        const float bc = (max - b) / dist;
        if (r == max)
            h = bc - gc;
        else if (g == max)
            h = 2.0f + rc - bc;
        else
            h = 4.0f + gc - rc;
        h = h / 6.0f;
        if (h < 0)
            h = h + 1.0f;
    }
}

