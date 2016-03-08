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

#ifndef TrenchBroom_EntityRotationPolicy
#define TrenchBroom_EntityRotationPolicy

#include "Model/ModelTypes.h"

namespace TrenchBroom {
    namespace Model {
        class EntityRotationPolicy {
        private:
            typedef enum {
                RotationType_None,
                RotationType_Angle,
                RotationType_AngleUpDown,
                RotationType_Euler
            } RotationType;

            struct RotationInfo {
                const RotationType type;
                const AttributeName attribute;
                RotationInfo(RotationType i_type, const AttributeName& i_attribute);
            };
        protected:
            EntityRotationPolicy();
            static Mat4x4 getRotation(const Entity* entity);
            static void applyRotation(Entity* entity, const Mat4x4& transformation);
            static AttributeName getAttribute(const Entity* entity);
        private:
            static RotationInfo rotationInfo(const Entity* entity);
            static void setAngle(Entity* entity, const AttributeName& attribute, const Vec3& direction);
            static FloatType getAngle(Vec3 direction);
        };
    }
}

#endif /* defined(TrenchBroom_EntityRotationPolicy) */
