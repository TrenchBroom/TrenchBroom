/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include "Picker.h"

#include "Model/ModelTypes.h"
#include "Model/Pickable.h"

#include <vector>

namespace TrenchBroom {
    namespace Model {
        Picker::Picker(const BBox<FloatType, 3>& worldBounds) :
        m_octree(worldBounds, static_cast<FloatType>(64.0f)) {}

        void Picker::addObject(Pickable* object) {
            m_octree.addObject(object->bounds(), object);
        }
        
        void Picker::removeObject(Pickable* object) {
            m_octree.removeObject(object->bounds(), object);
        }
        
        Hits Picker::pick(const Ray3& ray) const {
            typedef std::vector<Pickable*> PickableList;
            
            Hits hits;
            const PickableList candidates = m_octree.findObjects(ray);
            PickableList::const_iterator it, end;
            for (it = candidates.begin(), end = candidates.end(); it != end; ++it) {
                Pickable* object = *it;
                object->pick(ray, hits);
            }
            
            return hits;
        }
    }
}
