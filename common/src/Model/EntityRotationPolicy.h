/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "FloatType.h"

#include <vecmath/forward.h>

#include <string>

namespace TrenchBroom {
    namespace Assets {
        enum class PitchType;
    }

    namespace Model {
        class Entity;
        class EntityNode;

        class EntityRotationPolicy {
        private:
            enum class RotationType {
                None,
                Angle,
                AngleUpDown,
                Euler,
                Euler_PositivePitchDown,
                Mangle
            };
            enum class RotationUsage {
                Allowed,
                BlockRotation
            };

            struct RotationInfo {
                const RotationType type;
                const std::string attribute;
                const RotationUsage usage;
            };
        public:
            static vm::mat4x4 getRotation(const EntityNode* entityNode);
            static void applyRotation(EntityNode* entityNode, const vm::mat4x4& transformation);
            static std::string getAttribute(const EntityNode* entityNode);

            static vm::mat4x4 getRotation(const Entity& entity);
            static void applyRotation(Entity& entity, const vm::mat4x4& transformation);
            static std::string getAttribute(const Entity& entity);
        private:
            static RotationInfo rotationInfo(const EntityNode* entityNode);
            static void setAngle(EntityNode* entityNode, const std::string& attribute, const vm::vec3& direction);

            static RotationInfo rotationInfo(const Entity& entity);
            static void setAngle(Entity& entity, const std::string& attribute, const vm::vec3& direction);

            static FloatType getAngle(vm::vec3 direction);
        public:
            /**
             * Given an arbitrary transform and a rotation matrix, applies the transformation to the
             * rotation matrix and returns the result as euler angles in degrees.
             */
            static vm::vec3 getYawPitchRoll(const vm::mat4x4& transformation, const vm::mat4x4& rotation);
        };
    }
}

#endif /* defined(TrenchBroom_EntityRotationPolicy) */
