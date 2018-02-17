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
            static const Vec3 BaseAxes[];
            
            size_t m_index;
            Vec3 m_xAxis;
            Vec3 m_yAxis;
        public:
            ParaxialTexCoordSystem(const Vec3& point0, const Vec3& point1, const Vec3& point2, const BrushFaceAttributes& attribs);
            ParaxialTexCoordSystem(const Vec3& normal, const BrushFaceAttributes& attribs);

            static size_t planeNormalIndex(const Vec3& normal);
            static void axes(size_t index, Vec3& xAxis, Vec3& yAxis);
            static void axes(size_t index, Vec3& xAxis, Vec3& yAxis, Vec3& projectionAxis);
        private:
            ParaxialTexCoordSystem(size_t index, const Vec3& xAxis, const Vec3& yAxis);
        private:
            TexCoordSystem* doClone() const override;
            TexCoordSystemSnapshot* doTakeSnapshot() override;
            void doRestoreSnapshot(const TexCoordSystemSnapshot& snapshot) override;

            Vec3 getXAxis() const override;
            Vec3 getYAxis() const override;
            Vec3 getZAxis() const override;

            void doResetCache(const Vec3& point0, const Vec3& point1, const Vec3& point2, const BrushFaceAttributes& attribs) override;
            void doResetTextureAxes(const Vec3& normal) override;
            void doResetTextureAxesToParaxial(const Vec3& normal, float angle) override;
            void doResetTextureAxesToParallel(const Vec3& normal, float angle) override;

            bool isRotationInverted(const Vec3& normal) const override;
            Vec2f doGetTexCoords(const Vec3& point, const BrushFaceAttributes& attribs) const override;
            
            void doSetRotation(const Vec3& normal, float oldAngle, float newAngle) override;
            void doTransform(const Plane3& oldBoundary, const Plane3& newBoundary, const Mat4x4& transformation, BrushFaceAttributes& attribs, bool lockTexture, const Vec3& invariant) override;
            
            void doUpdateNormalWithProjection(const Vec3& oldNormal, const Vec3& newNormal, const BrushFaceAttributes& attribs) override;
            void doUpdateNormalWithRotation(const Vec3& oldNormal, const Vec3& newNormal, const BrushFaceAttributes& attribs) override;

            void doShearTexture(const Vec3& normal, const Vec2f& factors) override;
            
            float doMeasureAngle(float currentAngle, const Vec2f& center, const Vec2f& point) const override;
        private:
            void rotateAxes(Vec3& xAxis, Vec3& yAxis, FloatType angleInRadians, size_t planeNormIndex) const;
        private:
            ParaxialTexCoordSystem(const ParaxialTexCoordSystem& other);
            ParaxialTexCoordSystem& operator=(const ParaxialTexCoordSystem& other);
        };
    }
}

#endif /* defined(TrenchBroom_QuakeTexCoordPolicy) */
