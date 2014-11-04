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

#include "VertexCommand.h"

#include "Model/Brush.h"
#include "Model/Snapshot.h"
#include "View/MapDocumentCommandFacade.h"
#include "View/VertexHandleManager.h"

namespace TrenchBroom {
    namespace View {
        VertexCommand::VertexCommand(const CommandType type, const String& name, const Model::BrushList& brushes) :
        DocumentCommand(type, name),
        m_brushes(brushes),
        m_snapshot(NULL) {}

        VertexCommand::~VertexCommand() {
            if (m_snapshot != NULL)
                deleteSnapshot();
        }
        
        void VertexCommand::extractVertexMap(const Model::VertexToBrushesMap& vertices, Model::BrushList& brushes, Model::BrushVerticesMap& brushVertices, Vec3::List& vertexPositions) {

            typedef std::pair<Model::BrushVerticesMap::iterator, bool> BrushVerticesMapInsertResult;
            Model::VertexToBrushesMap::const_iterator vIt, vEnd;
            Model::BrushList::const_iterator bIt, bEnd;
            
            for (vIt = vertices.begin(), vEnd = vertices.end(); vIt != vEnd; ++vIt) {
                const Vec3& position = vIt->first;
                const Model::BrushList& vertexBrushes = vIt->second;
                for (bIt = vertexBrushes.begin(), bEnd = vertexBrushes.end(); bIt != bEnd; ++bIt) {
                    Model::Brush* brush = *bIt;
                    BrushVerticesMapInsertResult result = brushVertices.insert(std::make_pair(brush, Vec3::List()));
                    if (result.second)
                        brushes.push_back(brush);
                    result.first->second.push_back(position);
                }
                vertexPositions.push_back(position);
            }
        }

        bool VertexCommand::doPerformDo(MapDocumentCommandFacade* document) {
            if (!doCanDoVertexOperation(document))
                return false;
            
            takeSnapshot();
            return doVertexOperation(document);
        }
        
        bool VertexCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            assert(m_snapshot != NULL);
            document->restoreSnapshot(m_snapshot);
            deleteSnapshot();
            return true;
        }

        bool VertexCommand::doIsRepeatable(MapDocumentCommandFacade* document) const {
            return false;
        }

        void VertexCommand::takeSnapshot() {
            assert(m_snapshot == NULL);
            m_snapshot = new Model::Snapshot(m_brushes.begin(), m_brushes.end());
        }
        
        void VertexCommand::deleteSnapshot() {
            assert(m_snapshot != NULL);
            delete m_snapshot;
            m_snapshot = NULL;
        }

        void VertexCommand::removeBrushes(VertexHandleManager& manager) {
            manager.removeBrushes(m_brushes);
        }
        
        void VertexCommand::addBrushes(VertexHandleManager& manager) {
            manager.addBrushes(m_brushes);
        }
        
        void VertexCommand::selectNewHandlePositions(VertexHandleManager& manager) {
            doSelectNewHandlePositions(manager, m_brushes);
        }
        
        void VertexCommand::selectOldHandlePositions(VertexHandleManager& manager) {
            doSelectOldHandlePositions(manager, m_brushes);
        }
    }
}
