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

#ifndef TrenchBroom_ConvertEntityColorCommand
#define TrenchBroom_ConvertEntityColorCommand

#include "Macros.h"
#include "Model/EntityColor.h"
#include "Model/Model_Forward.h"
#include "View/DocumentCommand.h"
#include "View/View_Forward.h"

#include <map>
#include <memory>
#include <vector>

namespace TrenchBroom {
    namespace View {
        class ConvertEntityColorCommand : public DocumentCommand {
        public:
            static const CommandType Type;
        private:
            Model::AttributeName m_attributeName;
            Assets::ColorRange::Type m_colorRange;

            std::map<Model::AttributableNode*, std::vector<Model::EntityAttributeSnapshot>> m_snapshots;
        public:
            static std::unique_ptr<ConvertEntityColorCommand> convert(const Model::AttributeName& attributeName, Assets::ColorRange::Type colorRange);

            ConvertEntityColorCommand(const Model::AttributeName& attributeName, Assets::ColorRange::Type colorRange);
        private:
            bool doPerformDo(MapDocumentCommandFacade* document) override;
            bool doPerformUndo(MapDocumentCommandFacade* document) override;

            bool doIsRepeatable(MapDocumentCommandFacade* document) const override;

            bool doCollateWith(UndoableCommand* command) override;

            deleteCopyAndMove(ConvertEntityColorCommand)
        };
    }
}

#endif /* defined(TrenchBroom_ConvertEntityColorCommand) */
