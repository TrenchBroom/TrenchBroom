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
#include "Model/BrushTypes.h"
#include "Model/EntityTypes.h"
#include "Model/MapObjectTypes.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Model {
        class Map;
        
        class OctreeNode {
        private:
            typedef enum {
                WSB,
                WST,
                WNB,
                WNT,
                ESB,
                EST,
                ENB,
                ENT
            } NodePosition;
            
            int m_minSize;
            BBox m_bounds;
            MapObjectList m_objects;
            OctreeNode* m_children[8];
            bool addObject(MapObject& object, int childIndex);
        public:
            OctreeNode(const BBox& bounds, int minSize);
            ~OctreeNode();
            bool addObject(MapObject& object);
            bool removeObject(MapObject& object);
            bool empty();
            void intersect(const Ray& ray, MapObjectList& objects);
        };
        
        class Octree {
        private:
            int m_minSize;
            Map& m_map;
            OctreeNode* m_root;
        public:
            Octree(Map& map, int minSize = 64);
            ~Octree();
            
            void loadMap();
            void clear();
            void addObject(MapObject& object);
            void addObjects(const MapObjectList& objects);
            void removeObject(MapObject& object);
            void removeObjects(const MapObjectList& objects);
            void updateObjects(const MapObjectList& objects);

            MapObjectList intersect(const Ray& ray);
        };
    }
}
#endif
