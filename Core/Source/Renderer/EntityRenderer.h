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

#ifndef TrenchBroom_EntityRenderer_h
#define TrenchBroom_EntityRenderer_h

#include "Utilities/VecMath.h"

namespace TrenchBroom {
    namespace Model {
        class Entity;
    }
    
    namespace Renderer {
        class RenderContext;
        
        class EntityRenderer {
        public:
            virtual ~EntityRenderer() {};
            virtual void render(Model::Entity& entity) = 0;
            virtual void render(const Vec3f& position, float angle, float scale = 1.0f) = 0;
            virtual const Vec3f& center() = 0;
            virtual const BBox& bounds() = 0;
            virtual const BBox& maxBounds() = 0;
        };
    }
}

#endif
