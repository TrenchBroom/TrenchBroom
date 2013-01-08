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

#ifndef TrenchBroom_ModelUtils_h
#define TrenchBroom_ModelUtils_h

#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Utility/Grid.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        inline Vec3f referencePoint(const Model::EntityList& entities, const Model::BrushList& brushes, const Utility::Grid& grid) {
            assert(!entities.empty() || !brushes.empty());
            
            BBox bounds;
            if (!entities.empty()) {
                Model::EntityList::const_iterator entityIt = entities.begin();
                Model::EntityList::const_iterator entityEnd = entities.end();
                
                bounds = (*entityIt++)->bounds();
                while (entityIt != entityEnd)
                    bounds.mergeWith((*entityIt++)->bounds());
            }
            
            if (!brushes.empty()) {
                Model::BrushList::const_iterator brushIt = brushes.begin();
                Model::BrushList::const_iterator brushEnd = brushes.end();
                
                if (entities.empty())
                    bounds = (*brushIt++)->bounds();
                while (brushIt != brushEnd)
                    bounds.mergeWith((*brushIt++)->bounds());
            }

            return grid.snap(bounds.center());
        }
    }
}

#endif
