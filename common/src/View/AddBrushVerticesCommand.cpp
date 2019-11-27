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

#include "AddBrushVerticesCommand.h"

#include "CollectionUtils.h"
#include "StringUtils.h"
#include "View/MapDocument.h"
#include "View/MapDocumentCommandFacade.h"

#include <map>
#include <set>
#include <vector>

namespace TrenchBroom {
    namespace View {
        const Command::CommandType AddBrushVerticesCommand::Type = Command::freeType();

        AddBrushVerticesCommand::Ptr AddBrushVerticesCommand::add(const VertexToBrushesMap& vertices) {
            std::set<Model::Brush*> allBrushSet;
            for (const auto& entry : vertices) {
                const std::set<Model::Brush*>& brushes = entry.second;
                SetUtils::merge(allBrushSet, brushes);
            }

            const std::vector<Model::Brush*> allBrushList(std::begin(allBrushSet), std::end(allBrushSet));
            const String actionName = StringUtils::safePlural(vertices.size(), "Add Vertex", "Add Vertices");
            return Ptr(new AddBrushVerticesCommand(Type, actionName, allBrushList, vertices));
        }

        AddBrushVerticesCommand::AddBrushVerticesCommand(CommandType type, const String& name, const std::vector<Model::Brush*>& brushes, const VertexToBrushesMap& vertices) :
        VertexCommand(type, name, brushes),
        m_vertices(vertices) {}

        bool AddBrushVerticesCommand::doCanDoVertexOperation(const MapDocument* document) const {
            const vm::bbox3& worldBounds = document->worldBounds();
            for (const auto& entry : m_vertices) {
                const vm::vec3& position = entry.first;
                const std::set<Model::Brush*>& brushes = entry.second;
                for (const Model::Brush* brush : brushes) {
                    if (!brush->canAddVertex(worldBounds, position))
                        return false;
                }
            }
            return true;
        }

        bool AddBrushVerticesCommand::doVertexOperation(MapDocumentCommandFacade* document) {
            document->performAddVertices(m_vertices);
            return true;
        }

        bool AddBrushVerticesCommand::doCollateWith(UndoableCommand::Ptr) {
            return false;
        }
    }
}
