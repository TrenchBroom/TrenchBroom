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

#ifndef TrenchBroom_TextureCollectionCommand
#define TrenchBroom_TextureCollectionCommand

#include "Macros.h"
#include "View/DocumentCommand.h"

#include <memory>
#include <vector>

namespace TrenchBroom {
    namespace IO {
        class Path;
    }

    namespace View {
        class SetTextureCollectionsCommand : public DocumentCommand {
        public:
            static const CommandType Type;
        private:
            std::vector<IO::Path> m_paths;
            std::vector<IO::Path> m_oldPaths;
        public:
            static std::unique_ptr<SetTextureCollectionsCommand> set(const std::vector<IO::Path>& paths);

            SetTextureCollectionsCommand(const std::vector<IO::Path>& paths);
        private:
            std::unique_ptr<CommandResult> doPerformDo(MapDocumentCommandFacade* document) override;
            std::unique_ptr<CommandResult> doPerformUndo(MapDocumentCommandFacade* document) override;

            bool doIsRepeatable(MapDocumentCommandFacade* document) const override;
            bool doCollateWith(UndoableCommand* command) override;

            deleteCopyAndMove(SetTextureCollectionsCommand)
        };
    }
}

#endif /* defined(TrenchBroom_TextureCollectionCommand) */
