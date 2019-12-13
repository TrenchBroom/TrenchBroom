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

#include "Model/Snapshot.h"
#include "View/MapDocumentCommandFacade.h"

#include <string>

namespace TrenchBroom {
    namespace View {
        SnapshotCommand::SnapshotCommand(Command::CommandType type, const std::string &name) :
        DocumentCommand(type, name) {}

        SnapshotCommand::~SnapshotCommand() = default;

        bool SnapshotCommand::performDo(MapDocumentCommandFacade *document) {
            takeSnapshot(document);
            if (DocumentCommand::performDo(document)) {
                return true;
            } else {
                deleteSnapshot();
                return false;
            }
        }

        bool SnapshotCommand::doPerformUndo(MapDocumentCommandFacade *document) {
            return restoreSnapshot(document);
        }

        void SnapshotCommand::takeSnapshot(MapDocumentCommandFacade *document) {
            assert(m_snapshot == nullptr);
            m_snapshot = doTakeSnapshot(document);
        }

        bool SnapshotCommand::restoreSnapshot(MapDocumentCommandFacade *document) {
            ensure(m_snapshot != nullptr, "snapshot is null");
            document->restoreSnapshot(m_snapshot.get());
            deleteSnapshot();
            return true;
        }

        void SnapshotCommand::deleteSnapshot() {
            assert(m_snapshot != nullptr);
            m_snapshot.reset();
        }

        std::unique_ptr<Model::Snapshot> SnapshotCommand::doTakeSnapshot(MapDocumentCommandFacade *document) const {
            const auto& nodes = document->selectedNodes().nodes();
            return std::make_unique<Model::Snapshot>(std::begin(nodes), std::end(nodes));
        }
    }
}
