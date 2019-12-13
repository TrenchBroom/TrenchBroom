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

#include "Model/BrushFace.h"
#include "Model/Snapshot.h"
#include "View/MapDocumentCommandFacade.h"

#include <vector>

namespace TrenchBroom {
    namespace View {
        const Command::CommandType ChangeBrushFaceAttributesCommand::Type = Command::freeType();

        std::shared_ptr<ChangeBrushFaceAttributesCommand> ChangeBrushFaceAttributesCommand::command(const Model::ChangeBrushFaceAttributesRequest& request) {
            return std::make_shared<ChangeBrushFaceAttributesCommand>(request);
        }

        ChangeBrushFaceAttributesCommand::ChangeBrushFaceAttributesCommand(const Model::ChangeBrushFaceAttributesRequest& request) :
        DocumentCommand(Type, request.name()),
        m_request(request) {}

        bool ChangeBrushFaceAttributesCommand::doPerformDo(MapDocumentCommandFacade* document) {
            const std::vector<Model::BrushFace*> faces = document->allSelectedBrushFaces();
            assert(!faces.empty());

            assert(m_snapshot == nullptr);
            m_snapshot = std::make_unique<Model::Snapshot>(std::begin(faces), std::end(faces));

            document->performChangeBrushFaceAttributes(m_request);
            return true;
        }

        bool ChangeBrushFaceAttributesCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            assert(m_snapshot != nullptr);

            document->restoreSnapshot(m_snapshot.get());
            m_snapshot.reset();

            return true;
        }

        bool ChangeBrushFaceAttributesCommand::doIsRepeatable(MapDocumentCommandFacade* document) const {
            return document->hasSelectedBrushFaces();
        }

        std::shared_ptr<UndoableCommand> ChangeBrushFaceAttributesCommand::doRepeat(MapDocumentCommandFacade*) const {
            return std::make_shared<ChangeBrushFaceAttributesCommand>(m_request);
        }

        bool ChangeBrushFaceAttributesCommand::doCollateWith(std::shared_ptr<UndoableCommand> command) {
            ChangeBrushFaceAttributesCommand* other = static_cast<ChangeBrushFaceAttributesCommand*>(command.get());
            return m_request.collateWith(other->m_request);
        }
    }
}
