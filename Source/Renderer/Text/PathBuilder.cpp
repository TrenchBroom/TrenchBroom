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
                    Vec2f u = (1.0f - t) * a + t * b;
                    Vec2f v = (1.0f - t) * b + t * c;
                    addPoint((1.0f - t) * u + t * v);
                }
            }
            
            void PathBuilder::addCubicBezierCurve(const Vec2f& a, const Vec2f& b, const Vec2f& c, const Vec2f& d) {
                for (unsigned int i = 1; i < m_bezierSegments; i++) {
                    float t = static_cast<float>(i) / m_bezierSegments;
                    Vec2f u = (1.0f - t) * a + t * b;
                    Vec2f v = (1.0f - t) * b + t * c;
                    Vec2f w = (1.0f - t) * c + t * d;
                    Vec2f m = (1.0f - t) * u + t * v;
                    Vec2f n = (1.0f - t) * v + t * w;
                    addPoint((1.0f - t) * m + t * n);
                }
            }
        }
    }
}
