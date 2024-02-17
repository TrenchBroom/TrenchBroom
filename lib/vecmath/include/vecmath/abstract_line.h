/*
 Copyright 2010-2019 Kristian Duske
 Copyright 2015-2019 Eric Wasylishen

 Permission is hereby granted, free of charge, to any person obtaining a copy of this
 software and associated documentation files (the "Software"), to deal in the Software
 without restriction, including without limitation the rights to use, copy, modify, merge,
 publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include "vecmath/vec.h"

#include <cstddef>

namespace vm
{
/*
 * An abstract line is a generalization of lines in space. Thereby, a line can be bounded
 * or unbounded in either direction. The following concepts arise:
 *
 * - If the line is unbounded in both directions, it is just that, a line.
 * - If the line is bounded in one direction, it is a ray.
 * - If the line is bounded in both directions, it is a segment.
 *
 * In any case, the concept of an abstract line requires that the concrete line type has
 * the following members:
 *
 * - Line::component_type, the component type of the origin and direction
 * - Line::size, the number of dimensions
 * - vec<component_type, size> Line::get_origin() const, returns the line's origin
 * - vec<component_type, size> Line::get_direction() const, returns the line's direction
 *
 * The functions in this file operate on this concept.
 */

/**
 * Computes the distance from the origin to the orthogonal projection of the given point
 * onto the direction of the given abstract line.
 *
 * @param abstract_line the line to project onto
 * @param point the point to project
 * @return the distance from the origin to the orthogonal projection of the given point
 */
template <typename AL>
constexpr typename AL::component_type distance_to_projected_point(
  const AL& abstract_line, const vec<typename AL::component_type, AL::size>& point)
{
  return dot(point - abstract_line.get_origin(), abstract_line.get_direction());
}

/**
 * Computes the point on the given abstract line at the given distance from the line's
 * origin.
 *
 * @param abstract_line the line to compute the point on
 * @param distance the distance of the point
 * @return the point
 */
template <typename AL>
constexpr vec<typename AL::component_type, AL::size> point_at_distance(
  const AL& abstract_line, const typename AL::component_type distance)
{
  return abstract_line.get_origin() + abstract_line.get_direction() * distance;
}

/**
 * Orthogonally projects the given point onto the given abstract line.
 *
 * @param abstract_line the line to project the given point onto
 * @param point the point to project
 * @return the projected point
 */
template <typename AL>
constexpr vec<typename AL::component_type, AL::size> project_point(
  const AL& abstract_line, const vec<typename AL::component_type, AL::size>& point)
{
  return point_at_distance(
    abstract_line, distance_to_projected_point(abstract_line, point));
}
} // namespace vm
