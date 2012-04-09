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
#include "VecMath.h"
#include "Event.h"

using namespace std;

namespace TrenchBroom {
    namespace Model {
        typedef enum {
            CP_WSB = 0,
            CP_WST = 1,
            CP_WNB = 2,
            CP_WNT = 3,
            CP_ESB = 4,
            CP_EST = 5,
            CP_ENB = 6,
            CP_ENT = 7
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
            vector<MapObject*> m_objects;
            OctreeNode* m_children[8];
            bool addObject(MapObject& object, int childIndex);
        public:
            OctreeNode(const BBox& bounds, int minSize);
            ~OctreeNode();
            bool addObject(MapObject& object);
            bool removeObject(MapObject& object);
            void intersect(const Ray& ray, vector<MapObject*>& objects);
        };
        
        class Octree {
        private:
            int m_minSize;
            Map& m_map;
            OctreeNode* m_root;

            void entitiesWereAddedOrPropertiesDidChange(const vector<Entity*>& entities);
            void entitiesWillBeRemovedOrPropertiesWillChange(const vector<Entity*>& entities);
            void brushesWereAddedOrDidChange(const vector<Brush*>& brushes);
            void brushesWillBeRemovedOrWillChange(const vector<Brush*>& brushes);
            void mapLoaded(Map& map);
            void mapCleared(Map& map);
        public:
            Octree(Map& map, int minSize);
            ~Octree();
            vector<MapObject*> intersect(const Ray& ray);
        };
    }
}
#endif
