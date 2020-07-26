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

#include "SnapshotCommand.h"

#include "Ensure.h"
#include "Model/Snapshot.h"
#include "View/MapDocumentCommandFacade.h"

#include <string>

namespace TrenchBroom {
    namespace View {
        SnapshotCommand::SnapshotCommand(Command::CommandType type, const std::string &name) :
        DocumentCommand(type, name) {}

        SnapshotCommand::~SnapshotCommand() = default;

        std::unique_ptr<CommandResult> SnapshotCommand::performDo(MapDocumentCommandFacade *document) {
            takeSnapshot(document);
            auto result = DocumentCommand::performDo(document);
            if (!result->success()) {
                deleteSnapshot();
            }
            return result;
        }

        std::unique_ptr<CommandResult> SnapshotCommand::doPerformUndo(MapDocumentCommandFacade *document) {
            restoreSnapshot(document);
            deleteSnapshot();
            return std::make_unique<CommandResult>(true);
        }

        void SnapshotCommand::takeSnapshot(MapDocumentCommandFacade *document) {
            assert(m_snapshot == nullptr);
            m_snapshot = doTakeSnapshot(document);
        }

        void SnapshotCommand::deleteSnapshot() {
            assert(m_snapshot != nullptr);
            m_snapshot.reset();
        }

        void SnapshotCommand::restoreSnapshot(MapDocumentCommandFacade *document) {
            ensure(m_snapshot != nullptr, "snapshot is null");
            document->restoreSnapshot(m_snapshot.get());
        }

        std::unique_ptr<Model::Snapshot> SnapshotCommand::doTakeSnapshot(MapDocumentCommandFacade *document) const {
            const auto& nodes = document->selectedNodes().nodes();
            return std::make_unique<Model::Snapshot>(std::begin(nodes), std::end(nodes));
        }
    }
}
