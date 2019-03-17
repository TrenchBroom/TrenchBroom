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

#ifndef TrenchBroom_CopyTexCoordSystemFromFaceCommand
#define TrenchBroom_CopyTexCoordSystemFromFaceCommand

#include "TrenchBroom.h"
#include "SharedPointer.h"
#include "View/DocumentCommand.h"
#include "Model/TexCoordSystem.h"
#include "Model/BrushFaceAttributes.h"

#include <vecmath/plane.h>

#include <memory>

namespace TrenchBroom {
    namespace Model {
        class Snapshot;
    }

    namespace View {
        class CopyTexCoordSystemFromFaceCommand : public DocumentCommand {
        public:
            static const CommandType Type;
            using Ptr = std::shared_ptr<CopyTexCoordSystemFromFaceCommand>;
        private:

            Model::Snapshot* m_snapshot;
            std::unique_ptr<Model::TexCoordSystemSnapshot> m_coordSystemSanpshot;
            const vm::plane3 m_sourceFacePlane;
            const Model::WrapStyle m_wrapStyle;
            const Model::BrushFaceAttributes m_attribs;
        public:
            static Ptr command(const Model::TexCoordSystemSnapshot& coordSystemSanpshot, const Model::BrushFaceAttributes& attribs, const vm::plane3& sourceFacePlane, const Model::WrapStyle wrapStyle);
        private:
            CopyTexCoordSystemFromFaceCommand(const Model::TexCoordSystemSnapshot& coordSystemSanpshot, const Model::BrushFaceAttributes& attribs, const vm::plane3& sourceFacePlane, const Model::WrapStyle wrapStyle);
        public:
            ~CopyTexCoordSystemFromFaceCommand() override;
        private:
            bool doPerformDo(MapDocumentCommandFacade* document) override;
            bool doPerformUndo(MapDocumentCommandFacade* document) override;

            bool doIsRepeatable(MapDocumentCommandFacade* document) const override;
            UndoableCommand::Ptr doRepeat(MapDocumentCommandFacade* document) const override;

            bool doCollateWith(UndoableCommand::Ptr command) override;
        private:
            CopyTexCoordSystemFromFaceCommand(const CopyTexCoordSystemFromFaceCommand& other);
            CopyTexCoordSystemFromFaceCommand& operator=(const CopyTexCoordSystemFromFaceCommand& other);
        };
    }
}

#endif /* defined(TrenchBroom_CopyTexCoordSystemFromFaceCommand) */
