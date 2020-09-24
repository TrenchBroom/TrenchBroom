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

#ifndef TrenchBroom_ChangeBrushFaceAttributesCommand
#define TrenchBroom_ChangeBrushFaceAttributesCommand

#include "Macros.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "View/DocumentCommand.h"

#include <memory>

namespace TrenchBroom {
    namespace Model {
        class Snapshot;
    }

    namespace View {
        class ChangeBrushFaceAttributesCommand : public DocumentCommand {
        public:
            static const CommandType Type;
        private:
            Model::ChangeBrushFaceAttributesRequest m_request;
            std::unique_ptr<Model::Snapshot> m_snapshot;
        public:
            static std::unique_ptr<ChangeBrushFaceAttributesCommand> command(const Model::ChangeBrushFaceAttributesRequest& request);

            explicit ChangeBrushFaceAttributesCommand(const Model::ChangeBrushFaceAttributesRequest& request);
            ~ChangeBrushFaceAttributesCommand() override;
        private:
            std::unique_ptr<CommandResult> doPerformDo(MapDocumentCommandFacade* document) override;
            std::unique_ptr<CommandResult> doPerformUndo(MapDocumentCommandFacade* document) override;

            bool doIsRepeatable(MapDocumentCommandFacade* document) const override;

            bool doCollateWith(UndoableCommand* command) override;
        private:
            ChangeBrushFaceAttributesCommand(const ChangeBrushFaceAttributesCommand& other);
            ChangeBrushFaceAttributesCommand& operator=(const ChangeBrushFaceAttributesCommand& other);
        };
    }
}

#endif /* defined(TrenchBroom_ChangeBrushFaceAttributesCommand) */
