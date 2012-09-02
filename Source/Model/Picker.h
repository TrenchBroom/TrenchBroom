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

#include "Model/Filter.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Model {
        class Entity;
        class Brush;
        class Face;
        class Filter;
        class Octree;
        
        class Hit {
        public:
            typedef enum {
                EntityHit       = 1 << 0,
                FaceHit         = 1 << 1,
                NearFaceHit     = 1 << 2,
                VertexHandleHit = 1 << 3,
                EdgeHandleHit   = 1 << 4,
                FaceHandleHit   = 1 << 5
            } Type;
        private:
            void* m_object;
            int m_index;
            Type m_type;
            Vec3f m_hitPoint;
            float m_distance;
        public:
            Hit(void* object, Type type, const Vec3f& hitPoint, float distance);
            Hit(void* object, int index, Type type, const Vec3f& hitPoint, float distance);
            
            inline int index() const {
                return m_index;
            }
            
            inline Type type() const {
                return m_type;
            }
            
            inline const Vec3f& hitPoint() const {
                return m_hitPoint;
            }
            
            inline float distance() const {
                return m_distance;
            }
            
            inline bool hasType(int typeMask) const {
                return (m_type & typeMask) != 0;
            }
            
            inline Entity& entity() const {
                assert(m_type == EntityHit);
                return *(reinterpret_cast<Entity*>(m_object));
            }
            
            inline Brush& brush() const {
                assert(m_type == VertexHandleHit || m_type == EdgeHandleHit || m_type == FaceHandleHit);
                return *(reinterpret_cast<Brush*>(m_object));
            }
            
            inline Face& face() const {
                assert(m_type == FaceHit || m_type == NearFaceHit);
                return *(reinterpret_cast<Face*>(m_object));
            }
        };
        
        typedef std::vector<Hit*> HitList;
        
        class CompareHitsByDistance {
        public:
            inline bool operator() (const Hit* left, const Hit* right) const {
                return left->distance() < right->distance();
            }
        };

        class PickResult {
        private:
            HitList m_hits;
            bool m_sorted;
            void sortHits();
        public:
            PickResult() : m_sorted(false) {}
            ~PickResult();
            
            void add(Hit& hit);
            Hit* first(int typeMask, bool ignoreOccluders);
            HitList hits(int typeMask);
            const HitList& hits();
        };
        
        class Picker {
        private:
            Octree& m_octree;
        public:
            Picker(Octree& octree);
            PickResult* pick(const Ray& ray, Filter* filter = new Filter());
        };
    }
}

#endif
