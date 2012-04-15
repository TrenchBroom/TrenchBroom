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
#ifndef TrenchBroom_Picker_h
#define TrenchBroom_Picker_h

#include <vector>
#include "Utilities/VecMath.h"

using namespace std;

namespace TrenchBroom {
    class Filter;

    namespace Model {
        typedef enum {
            HT_ENTITY           = 1 << 0,
            HT_FACE             = 1 << 1,
            HT_CLOSE_FACE       = 1 << 2,
            HT_VERTEX_HANDLE    = 1 << 3,
            HT_EDGE_HANDLE      = 1 << 4,
            HT_FACE_HANDLE      = 1 << 5
        } EHitType;

        class Entity;
        class Brush;
        class Face;
        class Octree;
        
        class Hit {
        private:
        public:
            void* object;
            int index;
            EHitType type;
            Vec3f hitPoint;
            float distance;
            Hit(void* object, EHitType type, const Vec3f& hitPoint, float distance);
            Hit(void* object, int index, EHitType type, const Vec3f& hitPoint, float distance);

            bool hasType(int typeMask);
            Entity& entity();
            Brush& brush();
            Face& face();
        };
        
        class HitList {
        private:
            vector<Hit*> m_hits;
            bool m_sorted;
            static bool compareHits(const Hit* left, const Hit* right);
            void sortHits();
        public:
            HitList() : m_sorted(false) {}
            ~HitList();
            void add(Hit& hit);
            Hit* first(int typeMask, bool ignoreOccluders);
            vector<Hit*> hits(int typeMask);
            const vector<Hit*>& hits();
        };
        
        class Picker {
        private:
            Octree& m_octree;
            Filter* m_filter;
        public:
            Picker(Octree& octree);
            HitList* pick(const Ray& ray, Filter& filter);
        };
    }
}

#endif
