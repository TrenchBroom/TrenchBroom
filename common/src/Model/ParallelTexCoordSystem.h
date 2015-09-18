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
            ParallelTexCoordSystem* m_coordSystem;
            Vec3 m_xAxis;
            Vec3 m_yAxis;
        public:
            ParallelTexCoordSystemSnapshot(ParallelTexCoordSystem* coordSystem);
        private:
            void doRestore();
        };
        
        class ParallelTexCoordSystem : public TexCoordSystem {
        private:
            Vec3 m_xAxis;
            Vec3 m_yAxis;
            
            friend class ParallelTexCoordSystemSnapshot;
        public:
            ParallelTexCoordSystem(const Vec3& point0, const Vec3& point1, const Vec3& point2, const BrushFaceAttributes& attribs);
            ParallelTexCoordSystem(const Vec3& xAxis, const Vec3& yAxis, const BrushFaceAttributes& attribs);
        private:
            TexCoordSystem* doClone() const;
            TexCoordSystemSnapshot* doTakeSnapshot();
            
            Vec3 getXAxis() const;
            Vec3 getYAxis() const;
            Vec3 getZAxis() const;

            void doResetTextureAxes(const Vec3& normal);
            void doResetTextureAxesToParaxial(const Vec3& normal, float angle);
            void doResetTextureAxesToParallel(const Vec3& normal, float angle);

            bool isRotationInverted(const Vec3& normal) const;
            Vec2f doGetTexCoords(const Vec3& point, const BrushFaceAttributes& attribs) const;
            
            void doSetRotation(const Vec3& normal, float oldAngle, float newAngle);
            void applyRotation(const Vec3& normal, FloatType angle);
            
            void doTransform(const Plane3& oldBoundary, const Mat4x4& transformation, BrushFaceAttributes& attribs, bool lockTexture, const Vec3& invariant);
            float computeTextureAngle(const Plane3& oldBoundary, const Mat4x4& transformation) const;
            Mat4x4 computeNonTextureRotation(const Vec3& oldNormal, const Vec3& newNormal, const Mat4x4& rotation) const;
            
            void doUpdateNormal(const Vec3& oldNormal, const Vec3& newNormal, const BrushFaceAttributes& attribs);

            void doShearTexture(const Vec3& normal, const Vec2f& factors);

            float doMeasureAngle(float currentAngle, const Vec2f& center, const Vec2f& point) const;
            void computeInitialAxes(const Vec3& normal, Vec3& xAxis, Vec3& yAxis) const;
        };
    }
}

#endif /* defined(TrenchBroom_ParallelTexCoordSystem) */
