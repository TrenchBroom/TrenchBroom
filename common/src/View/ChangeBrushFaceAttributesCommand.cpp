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

        ChangeBrushFaceAttributesCommand::Ptr ChangeBrushFaceAttributesCommand::command(const Model::ChangeBrushFaceAttributesRequest& request) {
            return Ptr(new ChangeBrushFaceAttributesCommand(request));
        }

        ChangeBrushFaceAttributesCommand::ChangeBrushFaceAttributesCommand(const Model::ChangeBrushFaceAttributesRequest& request) :
        DocumentCommand(Type, request.name()),
        m_request(request),
        m_snapshot(nullptr) {}

        ChangeBrushFaceAttributesCommand::~ChangeBrushFaceAttributesCommand() {
            delete m_snapshot;
            m_snapshot = nullptr;
        }

        bool ChangeBrushFaceAttributesCommand::doPerformDo(MapDocumentCommandFacade* document) {
            const std::vector<Model::BrushFace*> faces = document->allSelectedBrushFaces();
            assert(!faces.empty());

            assert(m_snapshot == nullptr);
            m_snapshot = new Model::Snapshot(std::begin(faces), std::end(faces));

            document->performChangeBrushFaceAttributes(m_request);
            return true;
        }

        bool ChangeBrushFaceAttributesCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            document->restoreSnapshot(m_snapshot);
            delete m_snapshot;
            m_snapshot = nullptr;
            return true;
        }

        bool ChangeBrushFaceAttributesCommand::doIsRepeatable(MapDocumentCommandFacade* document) const {
            return document->hasSelectedBrushFaces();
        }

        UndoableCommand::Ptr ChangeBrushFaceAttributesCommand::doRepeat(MapDocumentCommandFacade*) const {
            return UndoableCommand::Ptr(new ChangeBrushFaceAttributesCommand(m_request));
        }

        bool ChangeBrushFaceAttributesCommand::doCollateWith(UndoableCommand::Ptr command) {
            ChangeBrushFaceAttributesCommand* other = static_cast<ChangeBrushFaceAttributesCommand*>(command.get());
            return m_request.collateWith(other->m_request);
        }
    }
}
