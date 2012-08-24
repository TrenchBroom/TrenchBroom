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

#ifndef __TrenchBroom__Brush__
#define __TrenchBroom__Brush__

#include "Model/EditState.h"
#include "Model/FaceTypes.h"
#include "Utility/VecMath.h"

#include <vector>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Model {
        class Entity;
        class Face;
        
        class Brush {
        protected:
            Entity* m_entity;
            FaceList m_faces;
            
            EditState m_editState;
            unsigned int m_selectedFaceCount;
            
            const BBox& m_worldBounds;
        public:
            Brush(const BBox& worldBounds);
            ~Brush();
            
            inline Entity* entity() const {
                return m_entity;
            }
            
            void setEntity(Entity* entity);
            
            inline bool partiallySelected() const {
                return m_selectedFaceCount > 0;
            }
            
            inline void incSelectedFaceCount() {
                m_selectedFaceCount++;
            }
            
            inline void decSelectedFaceCount() {
                m_selectedFaceCount--;
            }
            
            inline const BBox& worldBounds() const {
                return m_worldBounds;
            }
        };
    }
}

#endif /* defined(__TrenchBroom__Brush__) */
