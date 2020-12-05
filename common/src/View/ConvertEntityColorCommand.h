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

#pragma once

#include "Macros.h"
#include "Model/EntityColor.h"
#include "View/DocumentCommand.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class EntityAttributeSnapshot;
    }

    namespace View {
        class ConvertEntityColorCommand : public DocumentCommand {
        public:
            static const CommandType Type;
        private:
            std::string m_attributeName;
            Assets::ColorRange::Type m_colorRange;

            std::map<Model::AttributableNode*, std::vector<Model::EntityAttributeSnapshot>> m_snapshots;
        public:
            static std::unique_ptr<ConvertEntityColorCommand> convert(const std::string& attributeName, Assets::ColorRange::Type colorRange);

            ConvertEntityColorCommand(const std::string& attributeName, Assets::ColorRange::Type colorRange);
            ~ConvertEntityColorCommand() override;
        private:
            std::unique_ptr<CommandResult> doPerformDo(MapDocumentCommandFacade* document) override;
            std::unique_ptr<CommandResult> doPerformUndo(MapDocumentCommandFacade* document) override;

            bool doCollateWith(UndoableCommand* command) override;

            deleteCopyAndMove(ConvertEntityColorCommand)
        };
    }
}

#endif /* defined(TrenchBroom_ConvertEntityColorCommand) */
