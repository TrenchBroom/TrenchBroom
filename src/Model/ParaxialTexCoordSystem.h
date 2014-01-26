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

#ifndef __TrenchBroom__ParaxialTexCoordSystem__
#define __TrenchBroom__ParaxialTexCoordSystem__

#include "TrenchBroom.h"
#include "VecMath.h"

namespace TrenchBroom {
    namespace Model {
        class BrushFaceAttribs;
        class ParaxialTexCoordSystem {
        
        private:
            static const Vec3 BaseAxes[];
            
            Vec3 m_xAxis;
            Vec3 m_yAxis;
        public:
            ParaxialTexCoordSystem(const Vec3& point0, const Vec3& point1, const Vec3& point2);

            const Vec3& xAxis() const;
            const Vec3& yAxis() const;
            Vec3 projectedXAxis(const Vec3& normal) const;
            Vec3 projectedYAxis(const Vec3& normal) const;
            void update(const Vec3& normal, float rotation);
            
            static size_t planeNormalIndex(const Vec3& normal);
            static void axes(size_t index, Vec3& xAxis, Vec3& yAxis);
            static void axes(size_t index, Vec3& xAxis, Vec3& yAxis, Vec3& projectionAxis);
            
            void compensateTransformation(const Vec3& faceNormal, const Vec3& faceCenter, const Mat4x4& transformation, BrushFaceAttribs& attribs);
            
            static bool invertRotation(const Vec3& normal);
        private:
            static Vec3 transformAxis(const Vec3& normal, const Vec3& axis, const Mat4x4& transformation);
            static Vec3 projectAxis(const Vec3& normal, const Vec3& axis);
            static void rotateAxes(Vec3& xAxis, Vec3& yAxis, FloatType angle, size_t planeNormIndex);
            static Vec3 safeScaleAxis(const Vec3& axis, float factor);
        };
    }
}

#endif /* defined(__TrenchBroom__QuakeTexCoordPolicy__) */
