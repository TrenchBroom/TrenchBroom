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

#ifndef TrenchBroom_ParaxialTexCoordSystem
#define TrenchBroom_ParaxialTexCoordSystem

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Model/TexCoordSystem.h"

namespace TrenchBroom {
    namespace Model {
        class BrushFaceAttributes;

        class ParaxialTexCoordSystem : public TexCoordSystem {
        private:
            static const vec3 BaseAxes[];
            
            size_t m_index;
            vec3 m_xAxis;
            vec3 m_yAxis;
        public:
            ParaxialTexCoordSystem(const vec3& point0, const vec3& point1, const vec3& point2, const BrushFaceAttributes& attribs);
            ParaxialTexCoordSystem(const vec3& normal, const BrushFaceAttributes& attribs);

            static size_t planeNormalIndex(const vec3& normal);
            static void axes(size_t index, vec3& xAxis, vec3& yAxis);
            static void axes(size_t index, vec3& xAxis, vec3& yAxis, vec3& projectionAxis);
        private:
            ParaxialTexCoordSystem(size_t index, const vec3& xAxis, const vec3& yAxis);
        private:
            TexCoordSystem* doClone() const override;
            TexCoordSystemSnapshot* doTakeSnapshot() override;
            void doRestoreSnapshot(const TexCoordSystemSnapshot& snapshot) override;

            vec3 getXAxis() const override;
            vec3 getYAxis() const override;
            vec3 getZAxis() const override;

            void doResetCache(const vec3& point0, const vec3& point1, const vec3& point2, const BrushFaceAttributes& attribs) override;
            void doResetTextureAxes(const vec3& normal) override;
            void doResetTextureAxesToParaxial(const vec3& normal, float angle) override;
            void doResetTextureAxesToParallel(const vec3& normal, float angle) override;

            bool isRotationInverted(const vec3& normal) const override;
            vm::vec2f doGetTexCoords(const vec3& point, const BrushFaceAttributes& attribs) const override;
            
            void doSetRotation(const vec3& normal, float oldAngle, float newAngle) override;
            void doTransform(const plane3& oldBoundary, const plane3& newBoundary, const mat4x4& transformation, BrushFaceAttributes& attribs, bool lockTexture, const vec3& invariant) override;
            
            void doUpdateNormalWithProjection(const vec3& oldNormal, const vec3& newNormal, const BrushFaceAttributes& attribs) override;
            void doUpdateNormalWithRotation(const vec3& oldNormal, const vec3& newNormal, const BrushFaceAttributes& attribs) override;

            void doShearTexture(const vec3& normal, const vm::vec2f& factors) override;
            
            float doMeasureAngle(float currentAngle, const vm::vec2f& center, const vm::vec2f& point) const override;
        private:
            void rotateAxes(vec3& xAxis, vec3& yAxis, FloatType angleInRadians, size_t planeNormIndex) const;
        private:
            ParaxialTexCoordSystem(const ParaxialTexCoordSystem& other);
            ParaxialTexCoordSystem& operator=(const ParaxialTexCoordSystem& other);
        };
    }
}

#endif /* defined(TrenchBroom_QuakeTexCoordPolicy) */
