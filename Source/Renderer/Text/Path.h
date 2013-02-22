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

#include "Utility/List.h"
#include "Utility/VecMath.h"

#include <cassert>
#include <list>
#include <memory>
#include <limits>
#include <vector>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Renderer {
        namespace Text {
            typedef std::vector<Vec2f> PathPoints;

            class PathContour {
            private:
                bool m_clockwise;
                PathPoints m_points;
                Vec2f m_leftMost;
            public:
                PathContour() :
                m_leftMost(std::numeric_limits<float>::max(), 0.0f) {}

                inline const PathPoints& points() const {
                    return m_points;
                }

                inline void addPoint(const Vec2f& point) {
                    m_points.push_back(point);
                    if (point.x < m_leftMost.x)
                        m_leftMost = point;
                }

                inline const Vec2f& leftMost() const {
                    return m_leftMost;
                }

                inline bool clockwise() const {
                    return m_clockwise;
                }

                inline void setClockwise(bool clockwise) {
                    m_clockwise = clockwise;
                }

                inline void reverse() {
                    size_t count = m_points.size();
                    for (unsigned int i = 0; i < count / 2; i++)
                        std::swap(m_points[i], m_points[count - i - 1]);
                }
            };

            typedef std::vector<PathContour*> PathContours;

            class PathPolygon {
            public:
                typedef enum {
                    NonZero,
                    EvenOdd
                } Winding;
            private:
                Winding m_winding;
                PathContours m_contours;
                PathContour* m_current;
            public:
                PathPolygon(Winding winding) :
                m_winding(winding),
                m_current(NULL) {}
                ~PathPolygon() {
                    Utility::deleteAll(m_contours);
                    delete m_current;
                    m_current = NULL;
                }

                inline Winding winding() const {
                    return m_winding;
                }

                inline const PathContours& contours() const {
                    return m_contours;
                }

                inline void beginContour() {
                    if (m_current != NULL)
                        m_contours.push_back(m_current);
                    m_current = new PathContour();
                }

                inline void endContour(bool clockwise) {
                    assert(m_current != NULL);
                    m_current->setClockwise(clockwise);
                    m_contours.push_back(m_current);
                    m_current = NULL;
                }

                inline void addPoint(const Vec2f& point) {
                    assert(m_current != NULL);
                    m_current->addPoint(point);
                }

                inline void fixOrientations() {
                    Vec2f v0;
                    Vec2f v1;
                    for (unsigned int i = 0; i < m_contours.size(); i++) {
                        PathContour& contour = *m_contours[i];
                        const Vec2f& origin = contour.leftMost();
                        unsigned int c = 0;

                        for (unsigned int j = i + 1; j < m_contours.size(); j++) {
                            PathContour& candidate = *m_contours[j];
                            const PathPoints& points = candidate.points();
                            v0 = points.back() - origin;

                            for (unsigned int k = 0; k < points.size(); k++) {
                                v1 = points[k] - origin;

                                /*
                                 * A polygon edge intersects with the positive X axis if the
                                 * following conditions are met: The Y coordinates of its
                                 * vertices must have different signs (we assign a negative sign
                                 * to 0 here in order to count it as a negative number) and one
                                 * of the following two conditions must be met: Either the X
                                 * coordinates of the vertices are both positive or the X
                                 * coordinates of the edge have different signs (again, we
                                 * assign a negative sign to 0 here). In the latter case, we
                                 * must calculate the point of intersection between the edge and
                                 * the X axis and determine whether its X coordinate is positive
                                 * or zero.
                                 */

                                // do the Y coordinates have different signs?
                                if ((v0.y > 0 && v1.y <= 0) || (v0.y <= 0 && v1.y > 0)) {
                                    // Is segment entirely on the positive side of the X axis?
                                    if (v0.x > 0 && v1.x > 0) {
                                        c += 1; // edge intersects with the X axis
                                        // if not, do the X coordinates have different signs?
                                    } else if ((v0.x > 0 && v1.x <= 0) || (v0.x <= 0 && v1.x > 0)) {
                                        // calculate the point of intersection between the edge
                                        // and the X axis
                                        float x = -v0.y * (v1.x - v0.x) / (v1.y - v0.y) + v0.x;
                                        if (x >= 0)
                                            c += 1; // edge intersects with the X axis
                                    }
                                }

                                v0 = v1;
                            }
                        }

                        if ((c % 2 == 0) != contour.clockwise())
                            contour.reverse();
                    }
                }
            };

            typedef std::vector<const PathPolygon*> PathPolygons;

            class Path {
            private:
                float m_width;
                float m_height;
                PathPolygons m_polygons;
                PathPolygon* m_current;
            public:
                Path() :
                m_width(-1.0f),
                m_height(-1.0f),
                m_current(NULL) {}

                ~Path() {
                    Utility::deleteAll(m_polygons);
                    delete m_current;
                    m_current = NULL;
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

                inline const PathPolygons& polygons() const {
                    return m_polygons;
                }

                inline void beginPolygon(PathPolygon::Winding winding) {
                    assert(m_current == NULL);
                    m_current = new PathPolygon(winding);
                }

                inline void endPolygon() {
                    assert(m_current != NULL);
                    // m_current->fixOrientations(); doesn't seem necessary, but leaving it in, maybe we'll need it
                    m_polygons.push_back(m_current);
                    m_current = NULL;
                }

                inline void beginContour() {
                    assert(m_current != NULL);
                    m_current->beginContour();
                }

                inline void endContour(bool clockwise) {
                    assert(m_current != NULL);
                    m_current->endContour(clockwise);
                }

                inline void addPoint(const Vec2f& point) {
                    assert(m_current != NULL);
                    m_current->addPoint(point);
                }
            };

            typedef std::auto_ptr<const Path> PathPtr;
        }
    }
}

#endif /* defined(__TrenchBroom__Path__) */
