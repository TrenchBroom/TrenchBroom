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
#include "Model/FaceTypes.h"
#include "Utility/VecMath.h"

#include <map>
#include <vector>

using namespace TrenchBroom::VecMath;

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

        struct EdgeInfo {
            Vec3f start;
            Vec3f end;

            EdgeInfo(const Vec3f& i_start, const Vec3f& i_end) :
            start(i_start),
            end(i_end) {}

            inline bool operator==(const EdgeInfo& rhs) const {
                return (start == rhs.start && end == rhs.end) || (start == rhs.end && end == rhs.start);
            }
            
            inline Vec3f center() const {
                return (start + end) / 2.0f;
            }
        };

        struct FaceInfo {
            Vec3f::List vertices;

            inline bool operator==(const FaceInfo& rhs) const {
                if (vertices.size() != rhs.vertices.size())
                    return false;

                size_t count = vertices.size();
                for (size_t i = 0; i < count; i++) {
                    bool equal = true;
                    for (size_t j = 0; j < count && equal; j++) {
                        equal = vertices[(i + j) % count] == rhs.vertices[j];
                    }
                    if (equal)
                        return true;
                }
                return false;
            }

            inline Vec3f center() const {
                Vec3f center = vertices[0];
                for (size_t i = 1; i < vertices.size(); i++)
                    center += vertices[i];
                center /= static_cast<float>(vertices.size());
                return center;
            }
            
            inline FaceInfo& translate(const Vec3f& delta) {
                for (size_t i = 0; i < vertices.size(); i++)
                    vertices[i] += delta;
                return *this;
            }

            inline FaceInfo translated(const Vec3f& delta) const {
                FaceInfo result = *this;
                result.translate(delta);
                return result;
            }
        };

        typedef std::vector<EdgeInfo> EdgeInfoList;
        typedef std::vector<FaceInfo> FaceInfoList;

        typedef std::map<Vec3f, Model::BrushList, Vec3f::LexicographicOrder> VertexToBrushesMap;
        typedef std::map<Vec3f, Model::EdgeList, Vec3f::LexicographicOrder> VertexToEdgesMap;
        typedef std::map<Vec3f, Model::FaceList, Vec3f::LexicographicOrder> VertexToFacesMap;

        typedef std::map<Model::Brush*, Model::EdgeInfoList> BrushEdgesMap;
        typedef std::pair<Model::Brush*, Model::EdgeInfoList> BrushEdgesMapEntry;
        typedef std::pair<BrushEdgesMap::iterator, bool> BrushEdgesMapInsertResult;

        typedef std::map<Model::Brush*, Model::FaceInfoList> BrushFacesMap;
        typedef std::pair<Model::Brush*, Model::FaceInfoList> BrushFacesMapEntry;
        typedef std::pair<BrushFacesMap::iterator, bool> BrushFacesMapInsertResult;
    }
}

#endif
