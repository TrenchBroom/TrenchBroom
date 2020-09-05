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

#include "ConvertEntityColorCommand.h"

#include "Model/EntityAttributeSnapshot.h"
#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType ConvertEntityColorCommand::Type = Command::freeType();

        std::unique_ptr<ConvertEntityColorCommand> ConvertEntityColorCommand::convert(const std::string& attributeName, const Assets::ColorRange::Type colorRange) {
            return std::make_unique<ConvertEntityColorCommand>(attributeName, colorRange);
        }

        ConvertEntityColorCommand::ConvertEntityColorCommand(const std::string& attributeName, Assets::ColorRange::Type colorRange) :
        DocumentCommand(Type, "Convert Color"),
        m_attributeName(attributeName),
        m_colorRange(colorRange) {}

        ConvertEntityColorCommand::~ConvertEntityColorCommand() = default;

        std::unique_ptr<CommandResult> ConvertEntityColorCommand::doPerformDo(MapDocumentCommandFacade* document) {
            m_snapshots = document->performConvertColorRange(m_attributeName, m_colorRange);
            return std::make_unique<CommandResult>(true);
        }

        std::unique_ptr<CommandResult> ConvertEntityColorCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            document->restoreAttributes(m_snapshots);
            return std::make_unique<CommandResult>(true);
        }

        bool ConvertEntityColorCommand::doIsRepeatable(MapDocumentCommandFacade*) const {
            return false;
        }

        bool ConvertEntityColorCommand::doCollateWith(UndoableCommand*) {
            return false;
        }
    }
}
