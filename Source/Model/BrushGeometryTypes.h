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

#ifndef TrenchBroom_BrushGeometryTypes_h
#define TrenchBroom_BrushGeometryTypes_h

#include "Model/BrushTypes.h"
#include "Utility/VecMath.h"

#include <map>
#include <vector>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Model {
        class Vertex;
        class Edge;
        class Side;
        
        typedef std::vector<Vertex*> VertexList;
        typedef std::vector<Edge*> EdgeList;
        typedef std::vector<Side*> SideList;

        static const VertexList EmptyVertexList;
        static const EdgeList EmptyEdgeList;
        static const SideList EmptySideList;
        
        typedef std::map<Vec3f, Model::BrushList, Vec3f::LexicographicOrder> VertexToBrushesMap;
        typedef std::map<Vec3f, Model::EdgeList, Vec3f::LexicographicOrder> VertexToEdgesMap;
    }
}

#endif
