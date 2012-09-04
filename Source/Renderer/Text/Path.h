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

#ifndef __TrenchBroom__Path__
#define __TrenchBroom__Path__

#include "Utility/VecMath.h"

#include <cassert>
#include <memory>
#include <vector>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Renderer {
        namespace Text {
            class PathContour {
            public:
                typedef enum {
                    NonZero,
                    EvenOdd
                } Winding;
            private:
                Winding m_winding;
                Vec2f::List m_points;
            public:
                PathContour(Winding winding) :
                m_winding(winding) {}
                
                inline Winding winding() const {
                    return m_winding;
                }
                
                inline const Vec2f::List& points() const {
                    return m_points;
                }

                inline void addPoint(const Vec2f& location) {
                    m_points.push_back(location);
                }
            };
            
            typedef std::vector<const PathContour*> PathContours;
            
            class Path {
            private:
                float m_width;
                float m_height;
                PathContours m_contours;
                PathContour* m_current;
            public:
                Path() :
                m_width(-1.0f),
                m_height(-1.0f),
                m_current(NULL) {}

                ~Path() {
                    while (!m_contours.empty()) delete m_contours.back(), m_contours.pop_back();
                    if (m_current != NULL) {
                        delete m_current;
                        m_current = NULL;
                    }
                }
                
                inline float width() const {
                    return m_width;
                }
                
                inline float height() const {
                    return m_height;
                }
                
                inline void setBounds(float width, float height) {
                    m_width = width;
                    m_height = height;
                }
                
                inline void beginContour(PathContour::Winding winding) {
                    if (m_current != NULL)
                        m_contours.push_back(m_current);
                    m_current = new PathContour(winding);
                }
                
                inline void endContour() {
                    assert(m_current != NULL);
                    m_contours.push_back(m_current);
                    m_current = NULL;
                }

                inline void addPoint(const Vec2f& location) {
                    assert(m_current != NULL);
                    m_current->addPoint(location);
                }
            };
            
            typedef std::auto_ptr<Path> PathPtr;
        }
    }
}

#endif /* defined(__TrenchBroom__Path__) */
