/*
 Copyright (C) 2010-2013 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__CommandProcessor__
#define __TrenchBroom__CommandProcessor__

#include "SharedPointer.h"
#include "StringUtils.h"
#include "Controller/Command.h"

#include <vector>

namespace TrenchBroom {
    namespace Controller {
        class CommandGroup : public Command {
        private:
            List m_commands;
        public:
            CommandGroup(const String& name, const bool undoable, const Command::List& commands);
        private:
            bool doPerformDo();
            bool doPerformUndo();
        };
        
        class CommandProcessor {
        private:
            typedef Command::List CommandStack;
            CommandStack m_lastCommandStack;
            CommandStack m_nextCommandStack;

            String m_groupName;
            bool m_groupUndoable;
            CommandStack m_groupedCommands;
            size_t m_groupLevel;
        public:
            CommandProcessor();
            
            bool hasLastCommand() const;
            bool hasNextCommand() const;
            const String& lastCommandName() const;
            const String& nextCommandName() const;
            
            void beginGroup(const String& name, const bool undoable);
            void endGroup();
            
            bool submitCommand(Command::Ptr command);
            bool submitAndStoreCommand(Command::Ptr command);
            bool undoLastCommand();
            bool undoLastGroupedCommand();
            bool redoNextCommand();
        private:
            bool doCommand(Command::Ptr command);
            bool undoCommand(Command::Ptr command);
            void storeCommand(Command::Ptr command);
            
            void pushGroupedCommand(Command::Ptr command);
            Command::Ptr popGroupedCommand();
            Command::Ptr createCommandGroup();

            void pushLastCommand(Command::Ptr command);
            void pushNextCommand(Command::Ptr command);
            Command::Ptr popLastCommand();
            Command::Ptr popNextCommand();
        };
    }
}

#endif /* defined(__TrenchBroom__CommandProcessor__) */
