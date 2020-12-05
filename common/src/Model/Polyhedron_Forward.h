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

#include <kdl/intrusive_circular_list_forward.h>

namespace TrenchBroom {
    namespace Model {
        template<typename T, typename FP, typename VP> class Polyhedron;
        template<typename T, typename FP, typename VP> class Polyhedron_Vertex;
        template<typename T, typename FP, typename VP> class Polyhedron_Edge;
        template<typename T, typename FP, typename VP> class Polyhedron_HalfEdge;
        template<typename T, typename FP, typename VP> class Polyhedron_Face;

        template<typename T, typename FP, typename VP> struct Polyhedron_GetVertexLink;
        template<typename T, typename FP, typename VP> struct Polyhedron_GetEdgeLink;
        template<typename T, typename FP, typename VP> struct Polyhedron_GetHalfEdgeLink;
        template<typename T, typename FP, typename VP> struct Polyhedron_GetFaceLink;

        template <typename T, typename FP, typename VP>
        using Polyhedron_VertexList = kdl::intrusive_circular_list<Polyhedron_Vertex<T,FP,VP>, Polyhedron_GetVertexLink<T,FP,VP>>;

        template <typename T, typename FP, typename VP>
        using Polyhedron_EdgeList = kdl::intrusive_circular_list<Polyhedron_Edge<T,FP,VP>, Polyhedron_GetEdgeLink<T,FP,VP>>;

        template <typename T, typename FP, typename VP>
        using Polyhedron_HalfEdgeList = kdl::intrusive_circular_list<Polyhedron_HalfEdge<T,FP,VP>, Polyhedron_GetHalfEdgeLink<T,FP,VP>>;

        template <typename T, typename FP, typename VP>
        using Polyhedron_FaceList = kdl::intrusive_circular_list<Polyhedron_Face<T,FP,VP>, Polyhedron_GetFaceLink<T,FP,VP>>;
    }
}

#endif //TRENCHBROOM_POLYHEDRON_FORWARD_H
