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

#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace View {
        class MapDocumentCommandFacade;

        class CommandGroup : public UndoableCommand {
        public:
            static const CommandType Type;
        private:
            std::vector<std::unique_ptr<UndoableCommand>> m_commands;

            Notifier<Command*>& m_commandDoNotifier;
            Notifier<Command*>& m_commandDoneNotifier;
            Notifier<UndoableCommand*>& m_commandUndoNotifier;
            Notifier<UndoableCommand*>& m_commandUndoneNotifier;
        public:
            CommandGroup(
                const std::string& name, std::vector<std::unique_ptr<UndoableCommand>>&& commands,
                Notifier<Command*>& commandDoNotifier,
                Notifier<Command*>& commandDoneNotifier,
                Notifier<UndoableCommand*>& commandUndoNotifier,
                Notifier<UndoableCommand*>& commandUndoneNotifier);
        private:
            bool doPerformDo(MapDocumentCommandFacade* document) override;
            bool doPerformUndo(MapDocumentCommandFacade* document) override;

            bool doIsRepeatDelimiter() const override;
            bool doIsRepeatable(MapDocumentCommandFacade* document) const override;
            std::unique_ptr<UndoableCommand> doRepeat(MapDocumentCommandFacade* document) const override;

            bool doCollateWith(UndoableCommand* command) override;
        };

        class CommandProcessor {
        private:
            static const int64_t CollationInterval;

            MapDocumentCommandFacade* m_document;

            std::vector<std::unique_ptr<UndoableCommand>> m_lastCommandStack;
            std::vector<std::unique_ptr<UndoableCommand>> m_nextCommandStack;
            std::vector<UndoableCommand*> m_repeatableCommandStack;
            bool m_clearRepeatableCommandStack;
            int64_t m_lastCommandTimestamp;

            std::string m_groupName;
            std::vector<std::unique_ptr<UndoableCommand>> m_groupedCommands;
            size_t m_groupLevel;

            struct SubmitAndStoreResult;
        public:
            explicit CommandProcessor(MapDocumentCommandFacade* document);

            Notifier<Command*> commandDoNotifier;
            Notifier<Command*> commandDoneNotifier;
            Notifier<Command*> commandDoFailedNotifier;
            Notifier<UndoableCommand*> commandUndoNotifier;
            Notifier<UndoableCommand*> commandUndoneNotifier;
            Notifier<UndoableCommand*> commandUndoFailedNotifier;

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

            bool submitCommand(std::unique_ptr<Command>&& command);
            bool submitAndStoreCommand(std::unique_ptr<UndoableCommand>&& command);
            bool undoLastCommand();
            bool redoNextCommand();

            bool hasRepeatableCommands() const;
            bool repeatLastCommands();
            void clearRepeatableCommands();

            void clear();
        private:
            SubmitAndStoreResult submitAndStoreCommand(std::unique_ptr<UndoableCommand>&& command, bool collate);
            bool doCommand(Command* command);
            bool undoCommand(UndoableCommand* command);
            bool storeCommand(std::unique_ptr<UndoableCommand>&& command, bool collate);

            void beginGroup(const std::string& name, bool undoable);
            bool pushGroupedCommand(std::unique_ptr<UndoableCommand>&& command, bool collate);
            std::unique_ptr<UndoableCommand> popGroupedCommand();
            void createAndStoreCommandGroup();
            std::unique_ptr<UndoableCommand> createCommandGroup(const std::string& name, std::vector<std::unique_ptr<UndoableCommand>>&& commands);

            bool pushLastCommand(std::unique_ptr<UndoableCommand>&& command, bool collate);
            std::unique_ptr<UndoableCommand> popLastCommand();

            bool collatable(bool collate, int64_t timestamp) const;

            void pushNextCommand(std::unique_ptr<UndoableCommand>&& command);
            std::unique_ptr<UndoableCommand> popNextCommand();

            void pushRepeatableCommand(UndoableCommand* command);
            void popLastRepeatableCommand(UndoableCommand* command);
        };
    }
}

#endif /* defined(TrenchBroom_CommandProcessor) */
