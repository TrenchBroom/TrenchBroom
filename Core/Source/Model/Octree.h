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

#ifndef TrenchBroom_Octree_h
#define TrenchBroom_Octree_h

#include <vector>
#include "Model/Map/BrushTypes.h"
#include "Model/Map/EntityTypes.h"
#include "Utilities/Event.h"
#include "Utilities/VecMath.h"

namespace TrenchBroom {
    namespace Model {
        typedef enum {
            TB_CP_WSB = 0,
            TB_CP_WST = 1,
            TB_CP_WNB = 2,
            TB_CP_WNT = 3,
            TB_CP_ESB = 4,
            TB_CP_EST = 5,
            TB_CP_ENB = 6,
            TB_CP_ENT = 7
        } ENodePosition;
        
        class MapObject;
        class Entity;
        class Brush;
        class Face;
        class Map;
        
        class OctreeNode {
        private:
            int m_minSize;
            BBox m_bounds;
            std::vector<MapObject*> m_objects;
            OctreeNode* m_children[8];
            bool addObject(MapObject& object, int childIndex);
        public:
            OctreeNode(const BBox& bounds, int minSize);
            ~OctreeNode();
            bool addObject(MapObject& object);
            bool removeObject(MapObject& object);
            bool empty();
            void intersect(const Ray& ray, std::vector<MapObject*>& objects);
        };
        
        class Octree {
        private:
            int m_minSize;
            Map& m_map;
            OctreeNode* m_root;

            void entitiesWereAddedOrPropertiesDidChange(const EntityList& entities);
            void entitiesWillBeRemovedOrPropertiesWillChange(const EntityList& entities);
            void brushesWereAddedOrDidChange(const BrushList& brushes);
            void brushesWillBeRemovedOrWillChange(const BrushList& brushes);
            void mapLoaded(Map& map);
            void mapCleared(Map& map);
        public:
            Octree(Map& map, int minSize);
            ~Octree();
            std::vector<MapObject*> intersect(const Ray& ray);
        };
    }
}
#endif
