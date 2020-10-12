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

#ifndef TrenchBroom_SetCurrentLayerCommand
#define TrenchBroom_SetCurrentLayerCommand

#include "Macros.h"
#include "View/UndoableCommand.h"

#include <memory>

namespace TrenchBroom {
    namespace Model {
        class LayerNode;
    }

    namespace View {
        class SetCurrentLayerCommand : public UndoableCommand {
        public:
            static const CommandType Type;
        private:
            Model::LayerNode* m_currentLayer;
            Model::LayerNode* m_oldCurrentLayer;
        public:
            static std::unique_ptr<SetCurrentLayerCommand> set(Model::LayerNode* layer);

            SetCurrentLayerCommand(Model::LayerNode* layer);
        private:
            std::unique_ptr<CommandResult> doPerformDo(MapDocumentCommandFacade* document) override;
            std::unique_ptr<CommandResult> doPerformUndo(MapDocumentCommandFacade* document) override;

            bool doCollateWith(UndoableCommand* command) override;

            deleteCopyAndMove(SetCurrentLayerCommand)
        };
    }
}

#endif /* defined(TrenchBroom_SetCurrentLayerCommand) */
