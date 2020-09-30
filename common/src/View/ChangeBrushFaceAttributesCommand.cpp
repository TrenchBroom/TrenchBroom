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

#include "ChangeBrushFaceAttributesCommand.h"

#include "Model/BrushFaceHandle.h"
#include "Model/BrushNode.h"
#include "Model/Snapshot.h"
#include "View/MapDocumentCommandFacade.h"

#include <vector>

namespace TrenchBroom {
    namespace View {
        const Command::CommandType ChangeBrushFaceAttributesCommand::Type = Command::freeType();

        std::unique_ptr<ChangeBrushFaceAttributesCommand> ChangeBrushFaceAttributesCommand::command(const Model::ChangeBrushFaceAttributesRequest& request) {
            return std::make_unique<ChangeBrushFaceAttributesCommand>(request);
        }

        ChangeBrushFaceAttributesCommand::ChangeBrushFaceAttributesCommand(const Model::ChangeBrushFaceAttributesRequest& request) :
        DocumentCommand(Type, request.name()),
        m_request(request) {}

        ChangeBrushFaceAttributesCommand::~ChangeBrushFaceAttributesCommand() = default;

        std::unique_ptr<CommandResult> ChangeBrushFaceAttributesCommand::doPerformDo(MapDocumentCommandFacade* document) {
            const auto faceHandles = document->allSelectedBrushFaces();
            assert(!faceHandles.empty());
            
            const auto nodes = Model::toNodes(faceHandles);

            assert(m_snapshot == nullptr);
            m_snapshot = std::make_unique<Model::Snapshot>(std::begin(nodes), std::end(nodes));

            document->performChangeBrushFaceAttributes(m_request);
            return std::make_unique<CommandResult>(true);
        }

        std::unique_ptr<CommandResult> ChangeBrushFaceAttributesCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            assert(m_snapshot != nullptr);

            document->restoreSnapshot(m_snapshot.get());
            m_snapshot.reset();

            return std::make_unique<CommandResult>(true);
        }

        bool ChangeBrushFaceAttributesCommand::doCollateWith(UndoableCommand* command) {
            ChangeBrushFaceAttributesCommand* other = static_cast<ChangeBrushFaceAttributesCommand*>(command);
            return m_request.collateWith(other->m_request);
        }
    }
}
