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

#include "SnapBrushVerticesCommand.h"

#include "Model/Brush.h"
#include "Model/BrushGeometry.h"
#include "Model/Snapshot.h"
#include "View/MapDocument.h"
#include "View/MapDocumentCommandFacade.h"
#include "View/VertexHandleManager.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType SnapBrushVerticesCommand::Type = Command::freeType();

        SnapBrushVerticesCommand::Ptr SnapBrushVerticesCommand::snap(const Model::VertexToBrushesMap& vertices, const size_t snapTo) {
            Model::BrushList brushes;
            Model::BrushVerticesMap brushVertices;
            Vec3::List vertexPositions;
            extractVertexMap(vertices, brushes, brushVertices, vertexPositions);
            
            return Ptr(new SnapBrushVerticesCommand(brushes, brushVertices, vertexPositions, snapTo));
        }
        
        SnapBrushVerticesCommand::Ptr SnapBrushVerticesCommand::snap(const Model::BrushList& brushes, const size_t snapTo) {
            Model::BrushVerticesMap brushVertices;
            Vec3::List vertexPositions;
            
            Model::BrushList::const_iterator bIt, bEnd;
            Model::Brush::VertexList::const_iterator vIt, vEnd;
            
            for (bIt = brushes.begin(), bEnd = brushes.end(); bIt != bEnd; ++bIt) {
                Model::Brush* brush = *bIt;
                const Model::Brush::VertexList vertices = brush->vertices();
                for (vIt = vertices.begin(), vEnd = vertices.end(); vIt != vEnd; ++vIt) {
                    const Model::BrushVertex* vertex = *vIt;
                    brushVertices[brush].push_back(vertex->position());
                    vertexPositions.push_back(vertex->position());
                }
            }
            
            return Ptr(new SnapBrushVerticesCommand(brushes, brushVertices, vertexPositions, snapTo));
        }

        SnapBrushVerticesCommand::SnapBrushVerticesCommand(const Model::BrushList& brushes, const Model::BrushVerticesMap& vertices, const Vec3::List& vertexPositions, const size_t snapTo) :
        VertexCommand(Type, "Snap vertices", brushes),
        m_vertices(vertices),
        m_oldVertexPositions(vertexPositions),
        m_snapTo(snapTo) {}
        
        bool SnapBrushVerticesCommand::doCanDoVertexOperation(const MapDocument* document) const {
            return true;
        }
        
        bool SnapBrushVerticesCommand::doVertexOperation(MapDocumentCommandFacade* document) {
            m_newVertexPositions = document->performSnapVertices(m_vertices, m_snapTo);
            return true;
        }

        void SnapBrushVerticesCommand::doSelectNewHandlePositions(VertexHandleManager& manager, const Model::BrushList& brushes) {
            const Model::BrushSet brushSet(brushes.begin(), brushes.end());
            manager.reselectVertexHandles(brushSet, m_newVertexPositions, 0.01);
        }
        
        void SnapBrushVerticesCommand::doSelectOldHandlePositions(VertexHandleManager& manager, const Model::BrushList& brushes) {
            manager.selectVertexHandles(m_oldVertexPositions);
        }

        bool SnapBrushVerticesCommand::doCollateWith(UndoableCommand::Ptr command) {
            return false;
        }
    }
}
