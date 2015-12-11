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

#ifndef TrenchBroom_Line_h
#define TrenchBroom_Line_h

#include "Vec.h"

template <typename T, size_t S>
class Line {
public:
    typedef std::vector<Line<T,S> > List;
    static const List EmptyList;
    
    Vec<T,S> point;
    Vec<T,S> direction;

    Line() :
    point(Vec<T,S>::Null),
    direction(Vec<T,S>::Null) {}
    
    Line(const Vec<T,S>& i_point, const Vec<T,S>& i_direction) :
    point(i_point),
    direction(i_direction) {}
    
    template <typename U>
    Line(const Line<U,S>& other) :
    point(other.point),
    direction(other.direction) {}
    
    Line<T,S> makeCanonical() const {
        // choose the point such that its support vector is orthogonal to
        // the direction of this line
        const T d = point.dot(direction);
        const Vec<T,S> newPoint(point - d * direction);
        
        // make sure the first nonzero component of the direction is positive
        Vec<T,S> newDirection(direction);
        for (size_t i = 0; i < S; ++i) {
            if (direction[i] != 0.0) {
                if (direction[i] < 0.0)
                    newDirection = -newDirection;
                break;
            }
        }
        
        return Line<T,S>(newPoint, newDirection);
    }
    
    int compare(const Line<T,S>& other, const T epsilon = static_cast<T>(0.0)) const {
        assert(isCanonical() && other.isCanonical());

        const int pointCmp = point.compare(other.point, epsilon);
        if (pointCmp < 0)
            return -1;
        if (pointCmp > 0)
            return 1;
        return direction.compare(other.direction, epsilon);
    }
    
    bool operator==(const Line<T,S>& other) const {
        return compare(other) == 0;
    }
    
    bool operator!= (const Line<T,S>& other) const {
        return compare(other) != 0;
    }
    
    bool operator<(const Line<T,S>& other) const {
        return compare(other) < 0;
    }
    
    bool operator<= (const Line<T,S>& other) const {
        return compare(other) <= 0;
    }
    
    bool operator>(const Line<T,S>& other) const {
        return compare(other) > 0;
    }
    
    bool operator>= (const Line<T,S>& other) const {
        return compare(other) >= 0;
    }
    
    const Vec<T,S> pointAtDistance(const T distance) const {
        return point + direction * distance;
    }
private:
    bool isCanonical() const {
        const T d = point.dot(direction);
        if (!Math::zero(d))
            return false;

        for (size_t i = 0; i < S; ++i) {
            if (direction[i] != 0.0) {
                if (direction[i] < 0.0)
                    return false;
                break;
            }
        }
        
        return true;
    }
};

template <typename TT>
const TT intersectLineWithTriangle(const Line<TT,3>& L, const Vec<TT,3>& V0, const Vec<TT,3>& V1, const Vec<TT,3>& V2) {
    // see http://www.cs.virginia.edu/~gfx/Courses/2003/ImageSynthesis/papers/Acceleration/Fast%20MinimumStorage%20RayTriangle%20Intersection.pdf
    
    const Vec<TT,3>& O  = L.point;
    const Vec<TT,3>& D  = L.direction;
    const Vec<TT,3>  E1 = V1 - V0;
    const Vec<TT,3>  E2 = V2 - V0;
    const Vec<TT,3>  P  = crossed(D, E2);
    const TT         a  = P.dot(E1);
    if (Math::zero(a))
        return Math::nan<TT>();
    
    const Vec<TT,3>  T  = O - V0;
    const Vec<TT,3>  Q  = crossed(T, E1);
    
    const TT t = Q.dot(E2) / a;
    const TT u = P.dot(T) / a;
    if (Math::neg(u))
        return Math::nan<TT>();
    
    const TT v = Q.dot(D) / a;
    if (Math::neg(v))
        return Math::nan<TT>();
    
    if (Math::gt(u+v, static_cast<TT>(1.0)))
        return Math::nan<TT>();
    
    return t;
}

template <typename T, size_t S>
const typename Line<T,S>::List Line<T,S>::EmptyList = Line<T,S>::List();

typedef Line<float,3> Line3f;
typedef Line<double,3> Line3d;

#endif
