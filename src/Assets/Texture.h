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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__Texture__
#define __TrenchBroom__Texture__

#include "Color.h"

namespace TrenchBroom {
    namespace Assets {
        class Texture {
        public:
            virtual ~Texture();
            virtual size_t width() const = 0;
            virtual size_t height() const = 0;
            virtual const Color& averageColor() const = 0;
            virtual void activate() const = 0;
            virtual void deactivate() const = 0;
        };
    }
}

#endif /* defined(__TrenchBroom__Texture__) */
