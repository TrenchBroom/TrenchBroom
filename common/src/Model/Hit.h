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

#ifndef TrenchBroom_Hit
#define TrenchBroom_Hit

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Reference.h"

#include <list>

namespace TrenchBroom {
    namespace Model {
        class Hit {
        public:
            typedef std::list<Hit> List;
            
            typedef unsigned long HitType;
            static const HitType NoType;
            static const HitType AnyType;
            static HitType freeHitType();
            static const Hit NoHit;
        private:
            HitType m_type;
            FloatType m_distance;
            Vec3 m_hitPoint;
            UntypedReference m_target;
            FloatType m_error;
        public:
            template <typename T>
            Hit(const HitType type, const FloatType distance, const Vec3& hitPoint, const T& target, const FloatType error = 0.0) :
            m_type(type),
            m_distance(distance),
            m_hitPoint(hitPoint),
            m_target(Reference::copy(target)),
            m_error(error) {}
            
            template <typename T>
            static Hit hit(const HitType type, const FloatType distance, const Vec3& hitPoint, const T& target, const FloatType error = 0.0) {
                return Hit(type, distance, hitPoint, target);
            }
            
            bool isMatch() const;
            HitType type() const;
            bool hasType(const HitType typeMask) const;
            FloatType distance() const;
            const Vec3& hitPoint() const;
            FloatType error() const;
            
            template <typename T>
            const T& target() const {
                TypedReference<T> target(m_target);
                return target.get();
            }

            template <typename T>
            T& target() {
                TypedReference<T> target(m_target);
                return target.get();
            }
        };
    }
}

#endif /* defined(TrenchBroom_Hit) */
