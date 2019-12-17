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
#include "View/View_Forward.h"

#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace View {
        class CommandProcessor {
        private:
            static const int64_t CollationInterval;

            /**
             * The document to pass on to commands when they are executed, undone, or repeated.
             */
            MapDocumentCommandFacade* m_document;

            /**
             * Holds the commands that were executed so far, with the most recently executed command at the
             * end of the vector.
             */
            std::vector<std::unique_ptr<UndoableCommand>> m_undoStack;

            /**
             * Holds the commands that were undone, with the most recently undone command at the beginning of
             * the vector.
             */
            std::vector<std::unique_ptr<UndoableCommand>> m_redoStack;

            /**
             * Holds all commands that are potentially repeatable, in the order in which they should be repeated.
             */
            std::vector<UndoableCommand*> m_repeatStack;

            /**
             * Indicates whether to clear the stack of repeatable commands when the next command is stored.
             */
            bool m_clearRepeatStack;

            /**
             * The time stamp of when the last command was executed.
             */
            int64_t m_lastCommandTimestamp;

            /**
             * The name of the currently executed transaction.
             */
            std::string m_transactionName;

            /**
             * The commands which belong to the currently executed transaction.
             */
            std::vector<std::unique_ptr<UndoableCommand>> m_transactionCommands;

            /**
             * The nesting depth of the currently executed transactions.
             */
            size_t m_transactionLevel;

            struct SubmitAndStoreResult;
            class Transaction;
        public:
            /**
             * Creates a new command processor which will pass the given document to commands when they are
             * executed, undone or repeated.
             *
             * @param document the document to pass to commands, may be null
             */
            explicit CommandProcessor(MapDocumentCommandFacade* document);

            /**
             * Notifies observers when a command is going to be executed.
             */
            Notifier<Command*> commandDoNotifier;

            /**
             * Notifies observers when a command was successfully executed.
             */
            Notifier<Command*> commandDoneNotifier;

            /**
             * Notifies observers when a command failed to execute.
             */
            Notifier<Command*> commandDoFailedNotifier;

            /**
             * Notifies observers when an undoable command is going to be undone.
             */
            Notifier<UndoableCommand*> commandUndoNotifier;

            /**
             * Notifies observers when an undoable command was successfully undone.
             */
            Notifier<UndoableCommand*> commandUndoneNotifier;

            /**
             * Notifies observers when an undoable command failed to undo.
             */
            Notifier<UndoableCommand*> commandUndoFailedNotifier;

            /**
             * Notifies observers when a transaction completed successfully.
             */
            Notifier<const std::string&> transactionDoneNotifier;
            /**
             * Notifies observers when a transaction was undone successfully.
             */
            Notifier<const std::string&> transactionUndoneNotifier;

            /**
             * Indicates whether there is any command on the undo stack.
             */
            bool canUndo() const;

            /**
             * Indicates whether there is any command on the redo stack.
             */
            bool canRedo() const;

            /**
             * Returns the name of the command that will be undone when calling `undo`.
             *
             * Precondition: canUndo() == true
             */
            const std::string& undoCommandName() const;

            /**
             * Returns the name of the command that will be executed when callind `redo`.
             */
            const std::string& redoCommandName() const;

            /**
             * Starts a new transaction if none is currently executing. If a transaction is already executing,
             * then nothing happens, but it is required to call `commitTransaction` the same number of times that
             * `startTransaction` was called in order to commit the currently executing transaction.
             *
             * @param name the name of the transaction to start
             */
            void startTransaction(const std::string& name = "");

            /**
             * Commits the currently executing transaction if all nested transactions have been committed. Let `S` be
             * the number of times that `startTransaction` has been called, and let `C` be the number of times that
             * `commitTransaction` has been called. Then the current transaction is committed if and only if `S`-`C` = 1
             * before `commitTransaction` is called.
             *
             * If the current transaction does not contain any commands, then the transaction ends, but nothing
             * will be stored in the command processor.
             *
             * @throws CommandProcessorException If `S` = `C` before `commitTransaction` is called
             */
            void commitTransaction();

            /**
             * Rolls the currently executing transaction back by undoing all commands that belong to the transaction.
             * The transaction does not end when it is rolled back, rather, it remains executing.
             */
            void rollbackTransaction();

            std::unique_ptr<CommandResult> executeCommand(std::unique_ptr<Command> command);
            std::unique_ptr<CommandResult> executeAndStoreCommand(std::unique_ptr<UndoableCommand> command);
            std::unique_ptr<CommandResult> undoLastCommand();
            std::unique_ptr<CommandResult> redoNextCommand();

            bool hasRepeatableCommands() const;
            std::unique_ptr<CommandResult> repeatLastCommands();
            void clearRepeatableCommands();

            void clear();
        private:
            SubmitAndStoreResult executeAndStoreCommand(std::unique_ptr<UndoableCommand> command, const bool collate);
            std::unique_ptr<CommandResult> doCommand(Command* command);
            std::unique_ptr<CommandResult> undoCommand(UndoableCommand* command);
            bool storeCommand(std::unique_ptr<UndoableCommand> command, bool collate);

            void beginTransaction(const std::string& name, bool undoable);
            bool pushTransactionCommand(std::unique_ptr<UndoableCommand> command, bool collate);
            std::unique_ptr<UndoableCommand> popTransactionCommand();
            void createAndStoreTransaction();

            std::unique_ptr<UndoableCommand> createTransaction(const std::string& name, std::vector<std::unique_ptr<UndoableCommand>> commands);

            bool pushLastCommand(std::unique_ptr<UndoableCommand> command, bool collate);
            std::unique_ptr<UndoableCommand> popLastCommand();

            bool collatable(bool collate, int64_t timestamp) const;

            void pushNextCommand(std::unique_ptr<UndoableCommand> command);
            std::unique_ptr<UndoableCommand> popNextCommand();

            void pushRepeatableCommand(UndoableCommand* command);
            void popLastRepeatableCommand(UndoableCommand* command);
        };
    }
}

#endif /* defined(TrenchBroom_CommandProcessor) */
