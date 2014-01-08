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

#ifndef __TrenchBroom__Picker__
#define __TrenchBroom__Picker__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Holder.h"
#include "Model/ModelTypes.h"
#include "Model/Octree.h"

#include <limits>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class Hit {
        public:
            typedef unsigned long HitType;
            static const HitType NoType = 0;
            static const HitType AnyType = 0xFFFFFFFF;
            static HitType freeHitType();
            
            typedef std::vector<Hit> List;
            static const Hit NoHit;
        private:
            HitType m_type;
            FloatType m_distance;
            Vec3 m_hitPoint;
            BaseHolder::Ptr m_holder;
        public:
            template <typename T>
            Hit(HitType type, FloatType distance, const Vec3& hitPoint, T target) :
            m_type(type),
            m_distance(distance),
            m_hitPoint(hitPoint),
            m_holder(Holder<T>::newHolder(target)) {}

            template <typename T>
            static Hit hit(const HitType type, const FloatType distance, const Vec3& hitPoint, T target) {
                return Hit(type, distance, hitPoint, target);
            }
            
            bool isMatch() const;
            HitType type() const;
            bool hasType(const HitType typeMask) const;
            FloatType distance() const;
            const Vec3& hitPoint() const;
            
            template <typename T>
            T target() const {
                return m_holder->object<T>();
            }
        };
        
        class HitFilter {
        public:
            virtual ~HitFilter();
            virtual bool matches(const Hit& hit) const = 0;
        };

        class PickResult {
        public:
            struct FirstHit {
                bool matches;
                Hit hit;
                FirstHit(bool i_matches, const Hit& i_hit);
            };
        private:
            Hit::List m_hits;
            
            struct CompareHits {
                bool operator() (const Hit& left, const Hit& right) const;
            };
        public:
            FirstHit firstHit(const HitFilter& filter, bool ignoreOccluders) const;
            Hit::List hits(const HitFilter& filter) const;
            Hit::List allHits() const;
            
            void addHit(const Hit& hit);
            void sortHits();
        };
        
        class Picker {
        private:
            Octree<FloatType, Pickable*> m_octree;
        public:
            Picker(const BBox<FloatType, 3>& worldBounds);
            
            void addObject(Pickable* object);
            void removeObject(Pickable* object);

            template <typename Object>
            void addObjects(const std::vector<Object>& objects) {
                typename std::vector<Object>::const_iterator it, end;
                for (it = objects.begin(), end = objects.end(); it != end; ++it) {
                    const Object& object = *it;
                    addObject(object);
                }
            }
            
            template <typename Object>
            void removeObjects(const std::vector<Object>& objects) {
                typename std::vector<Object>::const_iterator it, end;
                for (it = objects.begin(), end = objects.end(); it != end; ++it) {
                    const Object& object = *it;
                    removeObject(object);
                }
            }
            
            PickResult pick(const Ray3& ray);
        };
    }
}

#endif /* defined(__TrenchBroom__Picker__) */
