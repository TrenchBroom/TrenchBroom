/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#ifndef __TrenchBroom__QuakeEntityRotator__
#define __TrenchBroom__QuakeEntityRotator__

#include "Model/Entity.h"

namespace TrenchBroom {
    namespace Model {
        class QuakeEntityRotationPolicy {
        private:
            typedef enum {
                RTNone,
                RTZAngle,
                RTZAngleWithUpDown,
                RTEulerAngles
            } RotationType;

            struct RotationInfo {
                const RotationType type;
                const PropertyKey property;
                
                RotationInfo(RotationType i_type, const PropertyKey& i_property) :
                type(i_type),
                property(i_property) {}
            };
        public:
            static Quat3 getRotation(const Entity& entity);
            static RotationInfo rotationInfo(const Entity& entity);
            static void applyRotation(Entity& entity, const Mat4x4& transformation);
        private:
            static void setAngle(Entity& entity, const PropertyKey& key, const Vec3& direction);
            static FloatType getAngle(Vec3 direction);
            QuakeEntityRotationPolicy();
        };
    }
}

#endif /* defined(__TrenchBroom__QuakeEntityRotator__) */
