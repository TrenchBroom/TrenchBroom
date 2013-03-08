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

#ifndef TrenchBroom_CreateBrushFromFacesStrategy_h
#define TrenchBroom_CreateBrushFromFacesStrategy_h

#include "IO/MapParser.h"
#include "Model/Brush.h"
#include "Model/Face.h"
#include "Utility/VecMath.h"

#include <algorithm>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace IO {
        class CreateBrushFromFacesStrategy : public MapParser::CreateBrushStrategy {
        public:
            Model::Brush* operator()(const BBox& worldBounds, const Model::FaceList& faces) {
                Model::Brush* brush = new Model::Brush(worldBounds);
                
                // sort the faces by the weight of their plane normals like QBSP does
                Model::FaceList sortedFaces = faces;
                std::sort(sortedFaces.begin(), sortedFaces.end(), Model::Face::WeightOrder(Plane::WeightOrder(true)));
                std::sort(sortedFaces.begin(), sortedFaces.end(), Model::Face::WeightOrder(Plane::WeightOrder(false)));
                
                Model::FaceList::iterator faceIt, faceEnd;
                for (faceIt = sortedFaces.begin(), faceEnd = sortedFaces.end(); faceIt != faceEnd; ++faceIt) {
                    Model::Face* face = *faceIt;
                    if (!brush->addFace(face)) {
                        brush->setFaces(Model::EmptyFaceList); // make sure the faces added so far don't get deleted
                        delete brush;
                        brush = NULL;
                        break;
                    }
                }
                
                // try to correct the vertices just like QBSP does
                if (brush != NULL) {
                    const Model::VertexList& vertices = brush->vertices();
                    Model::VertexList::const_iterator vertexIt, vertexEnd;
                    for (vertexIt = vertices.begin(), vertexEnd = vertices.end(); vertexIt != vertexEnd; ++vertexIt) {
                        Model::Vertex& vertex = **vertexIt;
                        vertex.position.correct();
                    }
                }
                
                return brush;
            }
        };
    }
}

#endif
