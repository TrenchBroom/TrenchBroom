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

#ifndef TrenchBroom_ParallelTexCoordSystem
#define TrenchBroom_ParallelTexCoordSystem

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Model/TexCoordSystem.h"

namespace TrenchBroom {
    namespace Model {
        class BrushFaceAttributes;
        class ParallelTexCoordSystem;
        
        class ParallelTexCoordSystemSnapshot : public TexCoordSystemSnapshot {
        private:
            Vec3 m_xAxis;
            Vec3 m_yAxis;
        public:
            ParallelTexCoordSystemSnapshot(const Vec3& xAxis, const Vec3& yAxis);
            ParallelTexCoordSystemSnapshot(ParallelTexCoordSystem* coordSystem);
        private:
            TexCoordSystemSnapshot* doClone() const override;
            void doRestore(ParallelTexCoordSystem* coordSystem) const override;
            void doRestore(ParaxialTexCoordSystem* coordSystem) const override;
        };
        
        class ParallelTexCoordSystem : public TexCoordSystem {
        private:
            Vec3 m_xAxis;
            Vec3 m_yAxis;
            
            friend class ParallelTexCoordSystemSnapshot;
        public:
            ParallelTexCoordSystem(const Vec3& point0, const Vec3& point1, const Vec3& point2, const BrushFaceAttributes& attribs);
            ParallelTexCoordSystem(const Vec3& xAxis, const Vec3& yAxis);
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
            void applyRotation(const Vec3& normal, FloatType angle);
            
            void doTransform(const Plane3& oldBoundary, const Plane3& newBoundary, const Mat4x4& transformation, BrushFaceAttributes& attribs, bool lockTexture, const Vec3& invariant) override;
            float computeTextureAngle(const Plane3& oldBoundary, const Mat4x4& transformation) const;
            Mat4x4 computeNonTextureRotation(const Vec3& oldNormal, const Vec3& newNormal, const Mat4x4& rotation) const;
            
            void doUpdateNormalWithProjection(const Vec3& oldNormal, const Vec3& newNormal, const BrushFaceAttributes& attribs) override;
            void doUpdateNormalWithRotation(const Vec3& oldNormal, const Vec3& newNormal, const BrushFaceAttributes& attribs) override;

            void doShearTexture(const Vec3& normal, const Vec2f& factors) override;

            float doMeasureAngle(float currentAngle, const Vec2f& center, const Vec2f& point) const override;
            void computeInitialAxes(const Vec3& normal, Vec3& xAxis, Vec3& yAxis) const;
        private:
            ParallelTexCoordSystem(const ParallelTexCoordSystem& other);
            ParallelTexCoordSystem& operator=(const ParallelTexCoordSystem& other);
        };
    }
}

#endif /* defined(TrenchBroom_ParallelTexCoordSystem) */
