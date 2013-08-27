/*
 Copyright (C) 2010-2013 Kristian Duske
 
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
    Color() :
    Vec<float, 4>(0.0f, 0.0f, 0.0f, 0.0f) {}
    
    Color(const float r, const float g, const float b, const float a) :
    Vec<float, 4>(r, g, b, a) {}
    
    Color(const Color& color, const float a) :
    Vec<float, 4>(color.r(), color.g(), color.b(), a) {}
    
    Color(const std::string& str) :
    Vec<float, 4>(str) {}
    
    inline float r() const {
        return x();
    }

    inline float g() const {
        return y();
    }

    inline float b() const {
        return z();
    }

    inline float a() const {
        return w();
    }
};

#endif
