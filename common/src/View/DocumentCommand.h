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

#include "View/UndoableCommand.h"

#include <string>

namespace TrenchBroom {
    namespace View {
        class DocumentCommand : public UndoableCommand {
        private:
            size_t m_modificationCount;
        public:
            DocumentCommand(CommandType type, const std::string& name);
            virtual ~DocumentCommand() override;
        public:
            std::unique_ptr<CommandResult> performDo(MapDocumentCommandFacade* document) override;
            std::unique_ptr<CommandResult> performUndo(MapDocumentCommandFacade* document) override;
            bool collateWith(UndoableCommand* command) override;
        private:
            size_t documentModificationCount() const override;
        private:
            DocumentCommand(const DocumentCommand& other);
            DocumentCommand& operator=(const DocumentCommand& other);
        };
    }
}

