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

#pragma once

#include "Polyhedron.h"

#include <kdl/intrusive_circular_list.h>

#include <ostream>

namespace TrenchBroom
{
namespace Model
{
/**
 * Appends a textual representation of the given vertices' position to the given stream.
 */
template <typename T, typename FP, typename VP>
std::ostream& operator<<(std::ostream& stream, const Polyhedron_Vertex<T, FP, VP>& vertex)
{
  stream << vertex.position();
  return stream;
}

/**
 * Prints a textual description of the given edge to the given output stream.
 */
template <typename T, typename FP, typename VP>
std::ostream& operator<<(std::ostream& stream, const Polyhedron_Edge<T, FP, VP>& edge)
{
  if (edge.firstEdge() != nullptr)
  {
    stream << *edge.firstEdge()->origin();
  }
  else
  {
    stream << "NULL";
  }
  stream << " <--> ";
  if (edge.secondEdge() != nullptr)
  {
    stream << *edge.secondEdge()->origin();
  }
  else
  {
    stream << "NULL";
  }
  return stream;
}

/**
 * Prints a textual description of the given half edge to the given output stream.
 */
template <typename T, typename FP, typename VP>
std::ostream& operator<<(std::ostream& stream, const Polyhedron_HalfEdge<T, FP, VP>& edge)
{
  stream << *edge.origin() << " --> ";
  if (edge.destination() != nullptr)
  {
    stream << *edge.destination();
  }
  else
  {
    stream << "NULL";
  }
  return stream;
}

/**
 * Prints a textual description of the face to the given output stream.
 */
template <typename T, typename FP, typename VP>
std::ostream& operator<<(std::ostream& stream, const Polyhedron_Face<T, FP, VP>& face)
{
  for (const Polyhedron_HalfEdge<T, FP, VP>* edge : face.boundary())
  {
    stream << *edge << "\n";
  }
  return stream;
}
} // namespace Model
} // namespace TrenchBroom
