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

#ifndef TrenchBroom_CommandProcessor
#define TrenchBroom_CommandProcessor

#include "Notifier.h"
#include "View/Command.h"
#include "View/UndoableCommand.h"

#include <string>
#include <vector>

namespace TrenchBroom {
    namespace View {
        class MapDocumentCommandFacade;

        using CommandList = std::vector<UndoableCommand::Ptr>;

        class CommandGroup : public UndoableCommand {
        public:
            static const CommandType Type;
        private:
            CommandList m_commands;

            Notifier<Command::Ptr>& m_commandDoNotifier;
            Notifier<Command::Ptr>& m_commandDoneNotifier;
            Notifier<UndoableCommand::Ptr>& m_commandUndoNotifier;
            Notifier<UndoableCommand::Ptr>& m_commandUndoneNotifier;
        public:
            CommandGroup(const std::string& name, const CommandList& commands,
                         Notifier<Command::Ptr>& commandDoNotifier,
                         Notifier<Command::Ptr>& commandDoneNotifier,
                         Notifier<UndoableCommand::Ptr>& commandUndoNotifier,
                         Notifier<UndoableCommand::Ptr>& commandUndoneNotifier);
        private:
            bool doPerformDo(MapDocumentCommandFacade* document) override;
            bool doPerformUndo(MapDocumentCommandFacade* document) override;

            bool doIsRepeatDelimiter() const override;
            bool doIsRepeatable(MapDocumentCommandFacade* document) const override;
            UndoableCommand::Ptr doRepeat(MapDocumentCommandFacade* document) const override;

            bool doCollateWith(UndoableCommand::Ptr command) override;
        };

        class CommandProcessor {
        private:
            static const int64_t CollationInterval;

            MapDocumentCommandFacade* m_document;

            using CommandStack = CommandList;
            CommandStack m_lastCommandStack;
            CommandStack m_nextCommandStack;
            CommandStack m_repeatableCommandStack;
            bool m_clearRepeatableCommandStack;
            int64_t m_lastCommandTimestamp;

            std::string m_groupName;
            CommandStack m_groupedCommands;
            size_t m_groupLevel;

            struct SubmitAndStoreResult;
        public:
            explicit CommandProcessor(MapDocumentCommandFacade* document);

            Notifier<Command::Ptr> commandDoNotifier;
            Notifier<Command::Ptr> commandDoneNotifier;
            Notifier<Command::Ptr> commandDoFailedNotifier;
            Notifier<UndoableCommand::Ptr> commandUndoNotifier;
            Notifier<UndoableCommand::Ptr> commandUndoneNotifier;
            Notifier<UndoableCommand::Ptr> commandUndoFailedNotifier;

            /**
             * Fired when a transaction completes successfully.
             */
            Notifier<const std::string&> transactionDoneNotifier;
            /**
             * Fired when a transaction is undone successfully.
             */
            Notifier<const std::string&> transactionUndoneNotifier;

            bool hasLastCommand() const;
            bool hasNextCommand() const;

            const std::string& lastCommandName() const;
            const std::string& nextCommandName() const;

            void beginGroup(const std::string& name = "");
            void endGroup();
            void rollbackGroup();

            bool submitCommand(Command::Ptr command);
            bool submitAndStoreCommand(UndoableCommand::Ptr command);
            bool undoLastCommand();
            bool redoNextCommand();

            bool hasRepeatableCommands() const;
            bool repeatLastCommands();
            void clearRepeatableCommands();

            void clear();
        private:
            SubmitAndStoreResult submitAndStoreCommand(UndoableCommand::Ptr command, bool collate);
            bool doCommand(Command::Ptr command);
            bool undoCommand(UndoableCommand::Ptr command);
            bool storeCommand(UndoableCommand::Ptr command, bool collate);

            void beginGroup(const std::string& name, bool undoable);
            bool pushGroupedCommand(UndoableCommand::Ptr command, bool collate);
            UndoableCommand::Ptr popGroupedCommand();
            void createAndStoreCommandGroup();
            UndoableCommand::Ptr createCommandGroup(const std::string& name, const CommandList& commands);

            bool pushLastCommand(UndoableCommand::Ptr command, bool collate);
            bool collatable(bool collate, int64_t timestamp) const;

            void pushNextCommand(UndoableCommand::Ptr command);
            void pushRepeatableCommand(UndoableCommand::Ptr command);


            UndoableCommand::Ptr popLastCommand();
            UndoableCommand::Ptr popNextCommand();
            void popLastRepeatableCommand(UndoableCommand::Ptr command);
        };
    }
}

#endif /* defined(TrenchBroom_CommandProcessor) */
