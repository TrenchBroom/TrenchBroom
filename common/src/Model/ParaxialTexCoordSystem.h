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
            void doTransform(const Plane3& oldBoundary, const Mat4x4& transformation, BrushFaceAttributes& attribs, bool lockTexture, const Vec3& invariant);
            
            void doUpdateNormal(const Vec3& oldNormal, const Vec3& newNormal, const BrushFaceAttributes& attribs);
            
            void doShearTexture(const Vec3& normal, const Vec2f& factors);
            
            float doMeasureAngle(float currentAngle, const Vec2f& center, const Vec2f& point) const;
        private:
            void rotateAxes(Vec3& xAxis, Vec3& yAxis, FloatType angleInRadians, size_t planeNormIndex) const;
        };
    }
}

#endif /* defined(TrenchBroom_QuakeTexCoordPolicy) */
