/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#ifndef __TrenchBroom__CommandProcessor__
#define __TrenchBroom__CommandProcessor__

#include "Notifier.h"
#include "SharedPointer.h"
#include "StringUtils.h"
#include "Controller/Command.h"
#include "View/ViewTypes.h"

// unfortunately we must depend on wx Widgets for time stamps here
#include <wx/longlong.h>

#include <vector>

namespace TrenchBroom {
    namespace Controller {
        class CommandGroup : public Command {
        public:
            static const CommandType Type;
        private:
            List m_commands;

            Notifier1<Command::Ptr>& m_commandDoNotifier;
            Notifier1<Command::Ptr>& m_commandDoneNotifier;
            Notifier1<Command::Ptr>& m_commandUndoNotifier;
            Notifier1<Command::Ptr>& m_commandUndoneNotifier;
        public:
            CommandGroup(const String& name, const bool undoable, const Command::List& commands,
                         Notifier1<Command::Ptr>& commandDoNotifier,
                         Notifier1<Command::Ptr>& commandDoneNotifier,
                         Notifier1<Command::Ptr>& commandUndoNotifier,
                         Notifier1<Command::Ptr>& commandUndoneNotifier);
        private:
            bool doPerformDo();
            bool doPerformUndo();
            bool doIsRepeatable() const;
            Command* doRepeat(View::MapDocumentSPtr document) const;
            bool doCollateWith(Ptr command);
        };
        
        class CommandProcessor {
        private:
            static const wxLongLong CollationInterval;
            
            typedef Command::List CommandStack;
            CommandStack m_lastCommandStack;
            CommandStack m_nextCommandStack;
            wxLongLong m_lastCommandTimestamp;
            size_t m_nextRepeatableCommand;
            
            String m_groupName;
            bool m_groupUndoable;
            CommandStack m_groupedCommands;
            size_t m_groupLevel;
        public:
            CommandProcessor();
            
            Notifier1<Command::Ptr> commandDoNotifier;
            Notifier1<Command::Ptr> commandDoneNotifier;
            Notifier1<Command::Ptr> commandDoFailedNotifier;
            Notifier1<Command::Ptr> commandUndoNotifier;
            Notifier1<Command::Ptr> commandUndoneNotifier;
            Notifier1<Command::Ptr> commandUndoFailedNotifier;
            
            bool hasLastCommand() const;
            bool hasNextCommand() const;
            bool hasRepeatableCommand() const;

            const String& lastCommandName() const;
            const String& nextCommandName() const;
            const String& nextRepeatableCommandName() const;
            
            void beginUndoableGroup(const String& name = "");
            void beginOneShotGroup(const String& name = "");
            void closeGroup();
            void undoGroup();
            
            bool submitCommand(Command::Ptr command);
            bool submitAndStoreCommand(Command::Ptr command);
            bool undoLastCommand();
            bool redoNextCommand();
            
            bool repeatLastCommand(View::MapDocumentWPtr document);
        private:
            bool submitAndStoreCommand(Command::Ptr command, bool collate);
            bool doCommand(Command::Ptr command);
            bool undoCommand(Command::Ptr command);
            void storeCommand(Command::Ptr command, bool collate);
            
            void beginGroup(const String& name, const bool undoable);
            void pushGroupedCommand(Command::Ptr command);
            Command::Ptr popGroupedCommand();
            void createAndStoreCommandGroup();

            void pushLastCommand(Command::Ptr command, bool collate);
            void pushNextCommand(Command::Ptr command);
            
            Command::Ptr popLastCommand();
            Command::Ptr popNextCommand();

            size_t findFirstRepeatableCommand() const;
            size_t findNextRepeatableCommand(size_t from) const;
        };
    }
}

#endif /* defined(__TrenchBroom__CommandProcessor__) */
