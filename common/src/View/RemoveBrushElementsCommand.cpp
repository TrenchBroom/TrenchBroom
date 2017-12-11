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

#include "RemoveBrushElementsCommand.h"

#include "Model/Brush.h"
#include "Model/Snapshot.h"
#include "View/MapDocument.h"
#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        RemoveBrushElementsCommand::RemoveBrushElementsCommand(const CommandType type, const String& name, const Model::BrushList& brushes, const Model::BrushVerticesMap& vertices) :
        VertexCommand(type, name, brushes),
        m_vertices(vertices) {}

        bool RemoveBrushElementsCommand::doCanDoVertexOperation(const MapDocument* document) const {
            const BBox3& worldBounds = document->worldBounds();
            for (const auto& entry : m_vertices) {
                Model::Brush* brush = entry.first;
                const Vec3::List& vertices = entry.second;
                if (!brush->canRemoveVertices(worldBounds, vertices))
                    return false;
            }
            return true;
        }

        bool RemoveBrushElementsCommand::doVertexOperation(MapDocumentCommandFacade* document) {
            document->performRemoveVertices(m_vertices);
            return true;
        }

        bool RemoveBrushElementsCommand::doCollateWith(UndoableCommand::Ptr command) {
            return false;
        }
    }
}
