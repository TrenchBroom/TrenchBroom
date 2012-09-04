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

#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        namespace Text {
            class PathPoint {
            public:
                typedef enum {
                    MoveTo,
                    LineTo,
                    ClosePath
                } Type;
            private:
                Type m_type;
                float m_x;
                float m_y;
            public:
                PathPoint(Type type, float x, float y):
                m_type(type),
                m_x(x),
                m_y(y) {}
                
                inline Type type() const {
                    return m_type;
                }
                
                inline float x() const {
                    return m_x;
                }
                
                inline float y() const {
                    return m_y;
                }
            };
            
            typedef std::vector<const PathPoint*> PathPoints;

            class PathContour {
            public:
                typedef enum {
                    NonZero,
                    EvenOdd
                } Winding;
            private:
                Winding m_winding;
                PathPoints m_points;
            public:
                PathContour(Winding winding) :
                m_winding(winding) {}
                
                PathContour() {
                    while (!m_points.empty()) delete m_points.back(), m_points.pop_back();
                }

                inline Winding winding() const {
                    return m_winding;
                }
                
                inline const PathPoints& points() const {
                    return m_points;
                }

                inline void addPoint(PathPoint::Type type, float x, float y) {
                    m_points.push_back(new PathPoint(type, x, y));
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

                inline void addPoint(PathPoint::Type type, float x, float y) {
                    assert(m_current != NULL);
                    m_current->addPoint(type, x, y);
                }
            };
            
            typedef std::auto_ptr<Path> PathPtr;
        }
    }
}

#endif /* defined(__TrenchBroom__Path__) */
