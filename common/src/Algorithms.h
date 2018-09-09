/*
 Copyright (C) 2010-2017 Kristian Duske
 
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

#ifndef TrenchBroom_Algorithms_h
#define TrenchBroom_Algorithms_h

#include "vec_decl.h"
#include "plane_decl.h"
#include "ray_decl.h"

// TODO 2201: refactor
namespace vm {
    /*
     Returns > 0 if p3.xy() is to the left of the line through p1.xy() and p2.xy(),
     < 0 if it is to the right of that line, or
     = 0 if it is on the line.
     */
    template <typename T, size_t S>
    int isLeft(const vec<T,S>& p1, const vec<T,S>& p2, const vec<T,S>& p3) {
        assert(S >= 2);
        const T result = ( (p2.x() - p1.x()) * (p3.y() - p1.y())
                          -(p3.x() - p1.x()) * (p2.y() - p1.y()));
        if (result < 0.0)
            return -1;
        if (result > 0.0)
            return 1;
        return 0;
    }


    template <typename T>
    class ConvexHull2D {
    private:
        class LessThanByAngle {
        private:
            const vec<T,3>& m_anchor;
        public:
            LessThanByAngle(const vec<T,3>& anchor) : m_anchor(anchor) {}
        public:
            bool operator()(const vec<T,3>& lhs, const vec<T,3>& rhs) const {
                const int side = isLeft(m_anchor, lhs, rhs);
                if (side > 0)
                    return true;
                if (side < 0)
                    return false;

                // the points are colinear, the one that is further from the anchor is considered less
                const T dxl = abs(lhs.x() - m_anchor.x());
                const T dxr = abs(rhs.x() - m_anchor.x());
                if (dxl == dxr) {
                    const T dyl = abs(lhs.y() - m_anchor.y());
                    const T dyr = abs(rhs.y() - m_anchor.y());
                    return dyl > dyr;
                }
                return dxl > dxr;
            }
        };

        typename vec<T,3>::List m_points;
        bool m_hasResult;
    public:
        ConvexHull2D(const typename vec<T,3>::List& points) :
        m_points(points),
        m_hasResult(m_points.size() > 2) {
            if (m_hasResult) {
                const size_t thirdPointIndex = findLinearlyIndependentPoint();
                m_hasResult = (thirdPointIndex < m_points.size());

                if (m_hasResult) {
                    const axis::type axis = computeAxis(thirdPointIndex);
                    swizzleTo(axis);

                    findAnchor();
                    sortPoints();
                    m_hasResult = (m_points.size() > 2);
                    if (m_hasResult)
                        buildHull();

                    swizzleFrom(axis);
                }
            }
        }

        bool hasResult() const {
            return m_hasResult;
        }

        const typename vec<T,3>::List& result() const {
            assert(m_hasResult);
            return m_points;
        }
    private:
        size_t findLinearlyIndependentPoint() const {
            size_t index = 2;
            while (index < m_points.size() && colinear(m_points[0], m_points[1], m_points[index])) {
                ++index;
            }
            return index;
        }

        axis::type computeAxis(const size_t thirdPointIndex) const {
            const vec<T,3> ortho = cross(m_points[thirdPointIndex] - m_points[0], m_points[1] - m_points[0]);
            return firstComponent(ortho);
        }

        void swizzleTo(const axis::type axis) {
            for (size_t i = 0; i < m_points.size(); ++i) {
                m_points[i] = swizzle(m_points[i], axis);
            }
        }

        void swizzleFrom(const axis::type axis) {
            swizzleTo(axis);
            swizzleTo(axis);
        }

        void findAnchor() {
            size_t anchor = 0;
            for (size_t i = 1; i < m_points.size(); ++i) {
                if ((m_points[i].y() < m_points[anchor].y()) ||
                    (m_points[i].y() == m_points[anchor].y() &&
                     m_points[i].x() >  m_points[anchor].x())) {
                    anchor = i;
                }
            }

            if (anchor > 0) {
                using std::swap;
                swap(m_points[0], m_points[anchor]);
            }
        }

        void sortPoints() {
            const vec<T,3>& anchor = m_points[0];
            std::sort(std::begin(m_points) + 1, std::end(m_points), LessThanByAngle(anchor));

            // now remove the duplicates
            auto i = std::begin(m_points) + 1;
            while (i != std::end(m_points)) {
                const vec<T,3>& p1 = *(i++);
                while (i != std::end(m_points)) {
                    const vec<T,3>& p2 = *i;
                    if (isLeft(anchor, p1, p2) == 0) {
                        i = m_points.erase(i);
                    } else {
                        break;
                    }
                };
            }
        }

        void buildHull() {
            typename vec<T,3>::List stack;
            stack.reserve(m_points.size());
            stack.push_back(m_points[0]);
            stack.push_back(m_points[1]);

            for (size_t i = 2; i < m_points.size(); ++i) {
                const vec<T,3>& p = m_points[i];
                popStalePoints(stack, p);
                stack.push_back(p);
            }

            using std::swap;
            swap(m_points, stack);
            assert(m_points.size() > 2);
        }

        void popStalePoints(typename vec<T,3>::List& stack, const vec<T,3>& p) {
            if (stack.size() > 1) {
                const vec<T,3>& t1 = stack[stack.size() - 2];
                const vec<T,3>& t2 = stack[stack.size() - 1];
                const int side = isLeft(t1, t2, p);
                if (side < 0) {
                    stack.pop_back();
                    popStalePoints(stack, p);
                }
            }
        }
    };

    // see http://geomalgorithms.com/a10-_hull-1.html
    template <typename T>
    typename vec<T,3>::List convexHull2D(const typename vec<T,3>::List& points) {
        const ConvexHull2D<T> hull(points);
        if (!hull.hasResult())
            return vec<T,3>::EmptyList;
        return hull.result();
    }
}

#endif
