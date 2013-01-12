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

        namespace HitType {
            typedef unsigned int Type;
            static const Type NoHit       = 0;
            static const Type EntityHit   = 1 << 0;
            static const Type FaceHit     = 1 << 1;
            static const Type ObjectHit   = EntityHit | FaceHit;
            static const Type Any         = 0xFFFFFFFF;
        }

        class Hit {
        private:
            HitType::Type m_type;
            Vec3f m_hitPoint;
            float m_distance;
        protected:
            Hit(HitType::Type type, const Vec3f& hitPoint, float distance);
        public:
            virtual ~Hit();
            
            inline HitType::Type type() const {
                return m_type;
            }
            
            inline bool hasType(HitType::Type type) const {
                return (m_type & type) != 0;
            }
            
            inline const Vec3f& hitPoint() const {
                return m_hitPoint;
            }
            
            inline float distance() const {
                return m_distance;
            }
            
            virtual bool pickable(Filter& filter) const = 0;
        };
        
        class ObjectHit : public Hit {
        private:
            MapObject& m_object;
        public:
            ObjectHit(HitType::Type type, MapObject& object, const Vec3f& hitPoint, float distance);
            virtual ~ObjectHit();
            
            inline MapObject& object() const {
                return m_object;
            }
        };
        
        class EntityHit : public ObjectHit {
        protected:
            Entity& m_entity;
        public:
            EntityHit(Entity& entity, const Vec3f& hitPoint, float distance);
            
            inline Entity& entity() const {
                return m_entity;
            }

            bool pickable(Filter& filter) const;
        };
        
        class FaceHit : public ObjectHit {
        protected:
            Face& m_face;
        public:
            FaceHit(Face& face, const Vec3f& hitPoint, float distance);

            inline Face& face() const {
                return m_face;
            }

            bool pickable(Filter& filter) const;
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
            
            void add(Hit* hit);
            Hit* first(HitType::Type typeMask, bool ignoreOccluders, Filter& filter);
            HitList hits(HitType::Type typeMask, Filter& filter);
            HitList hits(Filter& filter);
        };
        
        class Picker {
        private:
            Octree& m_octree;
        public:
            Picker(Octree& octree);
            PickResult* pick(const Ray& ray);
        };
    }
}

#endif
