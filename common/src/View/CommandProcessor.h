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
#include "View/Command.h"
#include "View/UndoableCommand.h"
#include "View/ViewTypes.h"

// unfortunately we must depend on wx Widgets for time stamps here
#include <wx/longlong.h>

#include <vector>

namespace TrenchBroom {
    namespace View {
        class MapDocumentCommandFacade;
        
        typedef std::tr1::shared_ptr<UndoableCommand> CommandPtr;
        typedef std::vector<CommandPtr> CommandList;
        
        class CommandGroup : public UndoableCommand {
        public:
            static const CommandType Type;
        private:
            CommandList m_commands;

            Notifier1<Command*>& m_commandDoNotifier;
            Notifier1<Command*>& m_commandDoneNotifier;
            Notifier1<Command*>& m_commandUndoNotifier;
            Notifier1<Command*>& m_commandUndoneNotifier;
        public:
            CommandGroup(const String& name, const CommandList& commands,
                         Notifier1<Command*>& commandDoNotifier,
                         Notifier1<Command*>& commandDoneNotifier,
                         Notifier1<Command*>& commandUndoNotifier,
                         Notifier1<Command*>& commandUndoneNotifier);
        private:
            bool doPerformDo(MapDocumentCommandFacade* document);
            bool doPerformUndo(MapDocumentCommandFacade* document);

            bool doIsRepeatDelimiter() const;
            bool doIsRepeatable(MapDocumentCommandFacade* document) const;
            UndoableCommand* doRepeat(MapDocumentCommandFacade* document) const;

            bool doCollateWith(UndoableCommand* command);
        };
        
        class CommandProcessor {
        private:
            static const wxLongLong CollationInterval;
            
            MapDocumentCommandFacade* m_document;
            
            typedef CommandList CommandStack;
            CommandStack m_lastCommandStack;
            CommandStack m_nextCommandStack;
            CommandStack m_repeatableCommandStack;
            bool m_clearRepeatableCommandStack;
            wxLongLong m_lastCommandTimestamp;
            
            String m_groupName;
            CommandStack m_groupedCommands;
            size_t m_groupLevel;
        public:
            CommandProcessor(MapDocumentCommandFacade* document);
            
            Notifier1<Command*> commandDoNotifier;
            Notifier1<Command*> commandDoneNotifier;
            Notifier1<Command*> commandDoFailedNotifier;
            Notifier1<Command*> commandUndoNotifier;
            Notifier1<Command*> commandUndoneNotifier;
            Notifier1<Command*> commandUndoFailedNotifier;
            
            bool hasLastCommand() const;
            bool hasNextCommand() const;

            const String& lastCommandName() const;
            const String& nextCommandName() const;
            
            void beginGroup(const String& name = "");
            void endGroup();
            void rollbackGroup();
            
            bool submitCommand(Command* command);
            bool submitAndStoreCommand(UndoableCommand* command);
            bool undoLastCommand();
            bool redoNextCommand();
            
            bool repeatLastCommands();
            void clearRepeatableCommands();
        private:
            bool submitAndStoreCommand(CommandPtr command, bool collate);
            bool doCommand(Command* command);
            bool undoCommand(CommandPtr command);
            void storeCommand(CommandPtr command, bool collate);
            
            void beginGroup(const String& name, const bool undoable);
            void pushGroupedCommand(CommandPtr command);
            CommandPtr popGroupedCommand();
            void createAndStoreCommandGroup();
            UndoableCommand* createCommandGroup(const String& name, const CommandList& commands);

            void pushLastCommand(CommandPtr command, bool collate);
            void pushNextCommand(CommandPtr command);
            void pushRepeatableCommand(CommandPtr command);
            
            CommandPtr popLastCommand();
            CommandPtr popNextCommand();
            void popLastRepeatableCommand(CommandPtr command);
        };
    }
}

#endif /* defined(__TrenchBroom__CommandProcessor__) */
