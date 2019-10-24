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

#ifndef TrenchBroom_MoveTexturesCommand
#define TrenchBroom_MoveTexturesCommand

#include "TrenchBroom.h"
#include "View/DocumentCommand.h"

#include <vecmath/vec.h>

namespace TrenchBroom {
    namespace View {
        class MoveTexturesCommand : public DocumentCommand {
        public:
            static const CommandType Type;
            using Ptr = std::shared_ptr<MoveTexturesCommand>;
        private:
            vm::vec3f m_cameraUp;
            vm::vec3f m_cameraRight;
            vm::vec2f m_delta;
        public:
            static Ptr move(const vm::vec3f& cameraUp, const vm::vec3f& cameraRight, const vm::vec2f& delta);
        private:
            MoveTexturesCommand(const vm::vec3f& cameraUp, const vm::vec3f& cameraRight, const vm::vec2f& delta);

            bool doPerformDo(MapDocumentCommandFacade* document) override;
            bool doPerformUndo(MapDocumentCommandFacade* document) override;

            void moveTextures(MapDocumentCommandFacade* document, const vm::vec2f& delta) const;

            bool doIsRepeatable(MapDocumentCommandFacade* document) const override;
            UndoableCommand::Ptr doRepeat(MapDocumentCommandFacade* document) const override;

            bool doCollateWith(UndoableCommand::Ptr command) override;
        private:
            MoveTexturesCommand(const MoveTexturesCommand& other);
            MoveTexturesCommand& operator=(const MoveTexturesCommand& other);
        };
    }
}

#endif /* defined(TrenchBroom_MoveTexturesCommand) */
