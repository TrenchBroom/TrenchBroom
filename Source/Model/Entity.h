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

#ifndef __TrenchBroom__Entity__
#define __TrenchBroom__Entity__

#include "Model/BrushTypes.h"
#include "Model/EditState.h"
#include "Model/MapObject.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Model {
        class Brush;
        class EntityDefinition;
        class Map;
        
        class Entity : public MapObject {
        protected:
            Map* m_map;
            BrushList m_brushes;
            
            EntityDefinition* m_definition;
            
            EditState m_editState;
            unsigned int m_selectedBrushCount;
            
            const BBox& m_worldBounds;
            
            bool m_geometryValid;
        public:
            Entity(const BBox& worldBounds);
            ~Entity();
            
            inline Map* map() const {
                return m_map;
            }
            
            inline void setMap(Map* map) {
                m_map = map;
            }
            
            inline const BrushList& brushes() const {
                return m_brushes;
            }
            
            void addBrush(Brush* brush);
            void addBrushes(const BrushList& brushes);
            void removeBrush(Brush* brush);
            
            inline EntityDefinition* definition() const {
                return m_definition;
            }
        
            void setDefinition(EntityDefinition* definition);
            
            inline void incSelectedBrushCount() {
                m_selectedBrushCount++;
            }
            
            inline void decSelectedBrushCount() {
                m_selectedBrushCount--;
            }
            
            inline const BBox& worldBounds() const {
                return m_worldBounds;
            }
            
            inline void invalidateGeometry() {
                m_geometryValid = false;
            }
        };
    }
}

#endif /* defined(__TrenchBroom__Entity__) */
