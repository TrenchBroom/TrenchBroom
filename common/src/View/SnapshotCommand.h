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

#ifndef TRENCHBROOM_SNAPSHOTCOMMAND_H
#define TRENCHBROOM_SNAPSHOTCOMMAND_H

#include "View/DocumentCommand.h"

namespace TrenchBroom {
    namespace Model {
        class Snapshot;
    }

    namespace View {
        class SnapshotCommand : public DocumentCommand {
        private:
            Model::Snapshot* m_snapshot;
        protected:
            SnapshotCommand(CommandType type, const String& name);
            virtual ~SnapshotCommand() override;
        public:
            bool performDo(MapDocumentCommandFacade* document) override;
            bool doPerformUndo(MapDocumentCommandFacade* document) override;
        private:
            void takeSnapshot(MapDocumentCommandFacade* document);
            bool restoreSnapshot(MapDocumentCommandFacade* document);
            void deleteSnapshot();
        private:
            virtual Model::Snapshot* doTakeSnapshot(MapDocumentCommandFacade* document) const;
        };
    }
}

#endif //TRENCHBROOM_SNAPSHOTCOMMAND_H
