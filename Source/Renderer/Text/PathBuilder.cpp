/*
 Copyright (C) 2010-2012 Kristian Duske
 
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

#include "PathBuilder.h"

namespace TrenchBroom {
    namespace Renderer {
        namespace Text {
            PathBuilder::PathBuilder(Path* path, unsigned int bezierSegments) :
            m_path(path),
            m_bezierSegments(bezierSegments) {}
            
            void PathBuilder::addQuadraticBezierCurve(const Vec2f& a, const Vec2f& b, const Vec2f& c) {
                for (unsigned int i = 1; i < m_bezierSegments; i++) {
                    float t = static_cast<float>(i) / m_bezierSegments;
                    Vec2f u = a * (1.0f - t) + b * t;
                    Vec2f v = b * (1.0f - t) + c * t;
                    addPoint(u * (1.0f - t) + v * t);
                }
            }
            
            void PathBuilder::addCubicBezierCurve(const Vec2f& a, const Vec2f& b, const Vec2f& c, const Vec2f& d) {
                for (unsigned int i = 0; i < m_bezierSegments; i++) {
                    float t = static_cast<float>(i) / m_bezierSegments;
                    Vec2f u = a * (1.0f - t) + b * t;
                    Vec2f v = b * (1.0f - t) + c * t;
                    Vec2f w = c * (1.0f - t) + d * t;
                    Vec2f m = u * (1.0f - t) + v * t;
                    Vec2f n = v * (1.0f - t) + w * t;
                    addPoint(m * (1.0f - t) + n * t);
                }
            }
        }
    }
}
