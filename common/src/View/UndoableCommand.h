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

#ifndef TrenchBroom_UndoableCommand
#define TrenchBroom_UndoableCommand

#include "Macros.h"
#include "View/Command.h"

#include <memory>
#include <string>

namespace TrenchBroom {
    namespace View {
        class MapDocumentCommandFacade;

        class UndoableCommand : public Command {
        protected:
            UndoableCommand(CommandType type, const std::string& name);
        public:
            virtual ~UndoableCommand();

            virtual std::unique_ptr<CommandResult> performUndo(MapDocumentCommandFacade* document);

            bool isRepeatDelimiter() const;
            bool isRepeatable(MapDocumentCommandFacade* document) const;
            std::unique_ptr<UndoableCommand> repeat(MapDocumentCommandFacade* document) const;

            virtual bool collateWith(UndoableCommand* command);
        private:
            virtual std::unique_ptr<CommandResult> doPerformUndo(MapDocumentCommandFacade* document) = 0;

            virtual bool doIsRepeatDelimiter() const;
            virtual bool doIsRepeatable(MapDocumentCommandFacade* document) const = 0;
            virtual std::unique_ptr<UndoableCommand> doRepeat(MapDocumentCommandFacade* document) const;

            virtual bool doCollateWith(UndoableCommand* command) = 0;
        public: // this method is just a service for DocumentCommand and should never be called from anywhere else
            virtual size_t documentModificationCount() const;

            deleteCopyAndMove(UndoableCommand)
        };
    }
}

#endif /* defined(TrenchBroom_UndoableCommand) */
