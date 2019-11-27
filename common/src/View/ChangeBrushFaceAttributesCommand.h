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

#include "View/DocumentCommand.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"

namespace TrenchBroom {
    namespace Model {
        class Snapshot;
    }

    namespace View {
        class ChangeBrushFaceAttributesCommand : public DocumentCommand {
        public:
            static const CommandType Type;
            using Ptr = std::shared_ptr<ChangeBrushFaceAttributesCommand>;
        private:

            Model::ChangeBrushFaceAttributesRequest m_request;
            Model::Snapshot* m_snapshot;
        public:
            static Ptr command(const Model::ChangeBrushFaceAttributesRequest& request);
        private:
            explicit ChangeBrushFaceAttributesCommand(const Model::ChangeBrushFaceAttributesRequest& request);
        public:
            ~ChangeBrushFaceAttributesCommand() override;
        private:
            bool doPerformDo(MapDocumentCommandFacade* document) override;
            bool doPerformUndo(MapDocumentCommandFacade* document) override;

            bool doIsRepeatable(MapDocumentCommandFacade* document) const override;
            UndoableCommand::Ptr doRepeat(MapDocumentCommandFacade* document) const override;

            bool doCollateWith(UndoableCommand::Ptr command) override;
        private:
            ChangeBrushFaceAttributesCommand(const ChangeBrushFaceAttributesCommand& other);
            ChangeBrushFaceAttributesCommand& operator=(const ChangeBrushFaceAttributesCommand& other);
        };
    }
}

#endif /* defined(TrenchBroom_ChangeBrushFaceAttributesCommand) */
