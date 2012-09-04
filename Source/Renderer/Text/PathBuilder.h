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

#ifndef __TrenchBroom__PathBuilder__
#define __TrenchBroom__PathBuilder__

#include "Renderer/Text/Path.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Renderer {
        namespace Text {
            class PathBuilder {
            private:
                Path* m_path;
                unsigned int m_bezierSegments;
            public:
                PathBuilder(Path* path, unsigned int bezierSegments = 6);
                
                inline void beginContour(PathContour::Winding winding) {
                    m_path->beginContour(winding);
                }
                
                inline void endContour() {
                    m_path->endContour();
                }
                
                inline void addPoint(const Vec2f& point) {
                    m_path->addPoint(point);
                }
                
                void addQuadraticBezierCurve(const Vec2f& a, const Vec2f& b, const Vec2f& c);
                void addCubicBezierCurve(const Vec2f& a, const Vec2f& b, const Vec2f& c, const Vec2f& d);
            };
        }
    }
}

#endif /* defined(__TrenchBroom__PathBuilder__) */
