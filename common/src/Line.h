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

#ifndef TrenchBroom_Line_h
#define TrenchBroom_Line_h

#include "vec_decl.h"

template <typename T, size_t S>
class line {
public:
    typedef std::vector<line<T,S> > List;
    static const List EmptyList;
    
    vec<T,S> point;
    vec<T,S> direction;

    line() :
    point(vec<T,S>::zero),
    direction(vec<T,S>::zero) {}
    
    // Copy and move constructors
    line(const line<T,S>& line) = default;
    line(line<T,S>&& other) = default;
    
    // Assignment operators
    line<T,S>& operator=(const line<T,S>& other) = default;
    line<T,S>& operator=(line<T,S>&& other) = default;

    /**
     * Converts the given line by converting its component type using static_cast.
     *
     * @tparam U the component type of the given line
     * @param line the line to convert
     */
    template <typename U>
    line(const line<U,S>& line) :
    point(line.point),
    direction(line.direction) {}

    /**
     * Creates a new line with the given point and direction.
     *
     * @param i_point the point
     * @param i_direction the direction
     */
    line(const vec<T,S>& i_point, const vec<T,S>& i_direction) :
    point(i_point),
    direction(i_direction) {}
public:
    /**
     * Projects the given point orthogonally onto this line and computes the distance from the line anchor point to
     * the projected point.
     *
     * @param i_point the point to project
     * @return the distance
     */
    T distance(const vec<T,S>& i_point) const {
        return dot(i_point - point, direction);
    }

    /**
     * Returns a point on this line at the given distance from its anchor point.
     *
     * @param distance the distance of the point (along the direction)
     * @return the point at the given distance
     */
    const vec<T,S> pointAtDistance(const T distance) const {
        return point + direction * distance;
    }
    

    /**
     * Orthogonally projects the given point onto this line.
     *
     * @param i_point the point to project
     * @return the projected point
     */
    const vec<T,S> project(const vec<T,S>& i_point) const {
        return pointAtDistance(distance(i_point));
    }
};

template <typename T, size_t S>
const typename line<T,S>::List line<T,S>::EmptyList = line<T,S>::List();

/**
 * Prints a textual representation of the given line to the given stream.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param stream the stream to print to
 * @param line the line to print
 * @return the given stream
 */
template <typename T, size_t S>
std::ostream& operator<<(std::ostream& stream, const line<T,S>& line) {
    stream << "{ point: (" << line.point << "), direction: (" << line.direction << ") }";
    return stream;
}

typedef line<float,3> line3f;
typedef line<double,3> line3d;

#endif
