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

#ifndef __TrenchBroom__ParallelTexCoordSystem__
#define __TrenchBroom__ParallelTexCoordSystem__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Model/TexCoordSystem.h"

namespace TrenchBroom {
    namespace Model {
        class BrushFaceAttribs;
        
        class ParallelTexCoordSystem : public TexCoordSystem {
        private:
            Vec3 m_xAxis;
            Vec3 m_yAxis;
        public:
            ParallelTexCoordSystem(const Vec3& xAxis, const Vec3& yAxis, const Vec3& normal, float rotation);
            ParallelTexCoordSystem(const Vec3& point0, const Vec3& point1, const Vec3& point2);
        private:
            TexCoordSystem* doClone() const;
            
            Vec3 getXAxis() const;
            Vec3 getYAxis() const;
            Vec3 getZAxis() const;

            bool isRotationInverted(const Vec3& normal) const;
            Vec2f doGetTexCoords(const Vec3& point, const BrushFaceAttribs& attribs) const;
            
            void doSetRotation(const Vec3& normal, float oldAngle, float newAngle);
            void doTransform(const Plane3& oldBoundary, const Mat4x4& transformation, BrushFaceAttribs& attribs, bool lockTexture);

            float doMeasureAngle(float currentAngle, const Vec2f& center, const Vec2f& point) const;
            void computeInitialAxes(const Vec3& normal, Vec3& xAxis, Vec3& yAxis) const;
        };
    }
}

#endif /* defined(__TrenchBroom__ParallelTexCoordSystem__) */
