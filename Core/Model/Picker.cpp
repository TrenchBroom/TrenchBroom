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

#include "Picker.h"

namespace TrenchBroom {
    namespace Model {
        Picker::Picker(Map* map) : m_map(map) {}
        
        PickingHitList* Picker::pick(TRay ray, Filter* filter) {
            PickingHitList* hits = new PickingHitList();
            Octree& octree = m_map->octree();
            vector<MapObject&> objects = octree.intersect(ray);
            for (int i = 0; i < objects.size(); i++) {
                MapObject& object = objects[i];
                Brush* brush = dynamic_cast<Brush*>(&object);
                if (brush != NULL) {
                } else {
                    Entity* entity = dynamic_cast<Entity*>(&object);
                    if (entity != NULL) {
                    }
                }
            }
        }
        
        void Picker::pickCloseFaces(TRay ray, const vector<Brush*>& brushes, float maxDistance, PickingHitList& hits, Filter* filter) {
        }
        
        void Picker::pickVertices(TRay ray, const vector<Brush*>& brushes, float handleRadius, PickingHitList& hits, Filter* filter) {
        }
    }
}