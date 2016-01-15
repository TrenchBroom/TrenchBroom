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

#ifndef TrenchBroom_CommandProcessor
#define TrenchBroom_CommandProcessor

#include "Notifier.h"
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
        
        typedef std::vector<UndoableCommand::Ptr> CommandList;
        
        class CommandGroup : public UndoableCommand {
        public:
            static const CommandType Type;
        private:
            CommandList m_commands;

            Notifier1<Command::Ptr>& m_commandDoNotifier;
            Notifier1<Command::Ptr>& m_commandDoneNotifier;
            Notifier1<UndoableCommand::Ptr>& m_commandUndoNotifier;
            Notifier1<UndoableCommand::Ptr>& m_commandUndoneNotifier;
        public:
            CommandGroup(const String& name, const CommandList& commands,
                         Notifier1<Command::Ptr>& commandDoNotifier,
                         Notifier1<Command::Ptr>& commandDoneNotifier,
                         Notifier1<UndoableCommand::Ptr>& commandUndoNotifier,
                         Notifier1<UndoableCommand::Ptr>& commandUndoneNotifier);
        private:
            bool doPerformDo(MapDocumentCommandFacade* document);
            bool doPerformUndo(MapDocumentCommandFacade* document);

            bool doIsRepeatDelimiter() const;
            bool doIsRepeatable(MapDocumentCommandFacade* document) const;
            UndoableCommand::Ptr doRepeat(MapDocumentCommandFacade* document) const;

            bool doCollateWith(UndoableCommand::Ptr command);
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

            struct SubmitAndStoreResult;
        public:
            CommandProcessor(MapDocumentCommandFacade* document);
            
            Notifier1<Command::Ptr> commandDoNotifier;
            Notifier1<Command::Ptr> commandDoneNotifier;
            Notifier1<Command::Ptr> commandDoFailedNotifier;
            Notifier1<UndoableCommand::Ptr> commandUndoNotifier;
            Notifier1<UndoableCommand::Ptr> commandUndoneNotifier;
            Notifier1<UndoableCommand::Ptr> commandUndoFailedNotifier;
            
            bool hasLastCommand() const;
            bool hasNextCommand() const;

            const String& lastCommandName() const;
            const String& nextCommandName() const;
            
            void beginGroup(const String& name = "");
            void endGroup();
            void rollbackGroup();
            
            bool submitCommand(Command::Ptr command);
            bool submitAndStoreCommand(UndoableCommand::Ptr command);
            bool undoLastCommand();
            bool redoNextCommand();
            
            bool repeatLastCommands();
            void clearRepeatableCommands();
        private:
            SubmitAndStoreResult submitAndStoreCommand(UndoableCommand::Ptr command, bool collate);
            bool doCommand(Command::Ptr command);
            bool undoCommand(UndoableCommand::Ptr command);
            bool storeCommand(UndoableCommand::Ptr command, bool collate);
            
            void beginGroup(const String& name, bool undoable);
            bool pushGroupedCommand(UndoableCommand::Ptr command, bool collate);
            UndoableCommand::Ptr popGroupedCommand();
            void createAndStoreCommandGroup();
            UndoableCommand::Ptr createCommandGroup(const String& name, const CommandList& commands);

            bool pushLastCommand(UndoableCommand::Ptr command, bool collate);
            bool collatable(bool collate, wxLongLong timestamp) const;
            
            void pushNextCommand(UndoableCommand::Ptr command);
            void pushRepeatableCommand(UndoableCommand::Ptr command);
            
            
            UndoableCommand::Ptr popLastCommand();
            UndoableCommand::Ptr popNextCommand();
            void popLastRepeatableCommand(UndoableCommand::Ptr command);
        };
    }
}

#endif /* defined(TrenchBroom_CommandProcessor) */
