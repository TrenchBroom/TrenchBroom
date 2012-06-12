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

#include "Utilities/SharedPointer.h"
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class Vertex;
        class Edge;
        class Side;
        
        typedef std::tr1::shared_ptr<Vertex> VertexPtr;
        typedef std::vector<VertexPtr> VertexList;
        typedef std::tr1::weak_ptr<Vertex> WeakVertexPtr;
        typedef std::vector<WeakVertexPtr> WeakVertexList;
        
        typedef std::tr1::shared_ptr<Edge> EdgePtr;
        typedef std::vector<EdgePtr> EdgeList;
        typedef std::tr1::weak_ptr<Edge> WeakEdgePtr;
        typedef std::vector<WeakEdgePtr> WeakEdgeList;
        
        typedef std::tr1::shared_ptr<Side> SidePtr;
        typedef std::vector<SidePtr> SideList;
        typedef std::tr1::weak_ptr<Side> WeakSidePtr;
        typedef std::vector<WeakSidePtr> WeakSideList;
    }
}

#endif
