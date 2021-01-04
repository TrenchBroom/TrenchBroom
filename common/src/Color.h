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

#pragma once

#include <vecmath/vec.h>

#include <optional>
#include <string>

namespace TrenchBroom {
    class Color : public vm::vec<float, 4> {
    public:
        static std::optional<Color> parse(const std::string& str);
        std::string toString() const;

        Color();
        Color(const vec<float,4>& v);
        Color(float r, float g, float b, float a = 1.0f);
        Color(const Color& color, float a);
        Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 0xFF);
        Color(int r, int g, int b, float a);
        Color(int r, int g, int b, int a = 0xFF);

        float r() const;
        float g() const;
        float b() const;
        float a() const;

        template <typename T>
        Color& mix(const Color& other, const T f) {
            const float c = static_cast<float>(vm::max(static_cast<T>(0.0), vm::min(static_cast<T>(1.0), f)));
            const float d = 1.0f - c;
            for (size_t i = 0; i < 4; i++)
                v[i] = d*v[i] + c*other[i];
            return *this;
        }

        Color mixed(const Color& other, const float f) const {
            return Color(*this).mix(other, f);
        }

        friend Color mixAlpha(const Color& color, const float f) {
            return Color(color.r(), color.g(), color.b(), f * color.a());
        }

        static void rgbToHSB(float r, float g, float b, float& h, float& s, float& br);
    };
}

