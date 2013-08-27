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

#ifndef TrenchBroom_HitAdapter_h
#define TrenchBroom_HitAdapter_h

#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/Entity.h"
#include "Model/Object.h"
#include "Model/Picker.h"

namespace TrenchBroom {
    namespace Model {
        inline Object* hitAsObject(const Hit& hit) {
            if (hit.type() == Entity::EntityHit)
                return hit.target<Object*>();
            if (hit.type() == Brush::BrushHit) {
                BrushFace* face = hit.target<BrushFace*>();
                return face->parent();
            }
            return NULL;
        }
        
        inline Entity* hitAsEntity(const Hit& hit) {
            if (hit.type() == Entity::EntityHit)
                return hit.target<Entity*>();
            return NULL;
        }
        
        inline Brush* hitAsBrush(const Hit& hit) {
            if (hit.type() == Brush::BrushHit)
                return hit.target<BrushFace*>()->parent();
            return NULL;
        }

        inline BrushFace* hitAsFace(const Hit& hit) {
            if (hit.type() == Brush::BrushHit)
                return hit.target<BrushFace*>();
            return NULL;
        }
    }
}

#endif
