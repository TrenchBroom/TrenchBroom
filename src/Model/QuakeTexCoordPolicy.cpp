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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "QuakeTexCoordPolicy.h"

#include "Model/BrushFace.h"

namespace TrenchBroom {
    namespace Model {
        const Vec3 QuakeTexCoordPolicy::BaseAxes[] = {
            Vec3( 0.0,  0.0,  1.0), Vec3( 1.0,  0.0,  0.0), Vec3( 0.0, -1.0,  0.0),
            Vec3( 0.0,  0.0, -1.0), Vec3( 1.0,  0.0,  0.0), Vec3( 0.0, -1.0,  0.0),
            Vec3( 1.0,  0.0,  0.0), Vec3( 0.0,  1.0,  0.0), Vec3( 0.0,  0.0, -1.0),
            Vec3(-1.0,  0.0,  0.0), Vec3( 0.0,  1.0,  0.0), Vec3( 0.0,  0.0, -1.0),
            Vec3( 0.0,  1.0,  0.0), Vec3( 1.0,  0.0,  0.0), Vec3( 0.0,  0.0, -1.0),
            Vec3( 0.0, -1.0,  0.0), Vec3( 1.0,  0.0,  0.0), Vec3( 0.0,  0.0, -1.0),
        };

        TextureCoordinateSystem QuakeTexCoordPolicy::textureCoordinateSystem(const Vec3& normal, const float rotation) {
            TextureCoordinateSystem system;
            size_t planeNormIndex, faceNormIndex;
            axesAndIndices(normal, system.xAxis, system.yAxis, planeNormIndex, faceNormIndex);
            rotateAxes(system.xAxis, system.yAxis, Math<FloatType>::radians(rotation), planeNormIndex);
            return system;
        }

        void QuakeTexCoordPolicy::axesAndIndices(const Vec3& normal, Vec3& xAxis, Vec3& yAxis, size_t& planeNormIndex, size_t& faceNormIndex) {
            size_t bestIndex = 0;
            FloatType bestDot = static_cast<FloatType>(0.0);
            for (size_t i = 0; i < 6; ++i) {
                const FloatType dot = normal.dot(BaseAxes[i * 3]);
                if (dot > bestDot) { // no need to use -altaxis for qbsp
                    bestDot = dot;
                    bestIndex = i;
                }
            }
            
            xAxis = BaseAxes[bestIndex * 3 + 1];
            yAxis = BaseAxes[bestIndex * 3 + 2];
            planeNormIndex = (bestIndex / 2) * 6;
            faceNormIndex = bestIndex * 3;
        }
        
        void QuakeTexCoordPolicy::rotateAxes(Vec3& xAxis, Vec3& yAxis, const FloatType angle, const size_t planeNormIndex)  {
            // for some reason, when the texture plane normal is the Y axis, we must rotation clockwise
            const Quat3 rot(BaseAxes[planeNormIndex], planeNormIndex == 12 ? -angle : angle);
            xAxis = rot * xAxis;
            yAxis = rot * yAxis;
        }
    }
}
