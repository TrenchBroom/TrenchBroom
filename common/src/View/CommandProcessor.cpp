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

#include "CommandProcessor.h"

#include "Exceptions.h"
#include "Notifier.h"
#include "View/Command.h"
#include "View/UndoableCommand.h"

#include <kdl/set_temp.h>
#include <kdl/string_utils.h>
#include <kdl/vector_utils.h>

#include <algorithm>

#include <QDateTime>

namespace TrenchBroom {
    namespace View {
        template <typename T, typename C>
        void notifyCommandIfNotType(T& notifier, const Command::CommandType ignore, C* command) {
            if (command->type() != ignore) {
                notifier(command);
            }
        }

        struct CommandProcessor::TransactionState {
            std::string name;
            std::vector<std::unique_ptr<UndoableCommand>> commands;

            explicit TransactionState(const std::string& i_name) :
            name(i_name) {}
        };

        struct CommandProcessor::SubmitAndStoreResult {
            std::unique_ptr<CommandResult> commandResult;
            bool commandStored;

            SubmitAndStoreResult(std::unique_ptr<CommandResult> i_commandResult, const bool i_commandStored) :
            commandResult(std::move(i_commandResult)),
            commandStored(i_commandStored) {}
        };

        class CommandProcessor::TransactionCommand : public UndoableCommand {
        public:
            static const CommandType Type;
        private:
            std::vector<std::unique_ptr<UndoableCommand>> m_commands;

            Notifier<Command*>& m_commandDoNotifier;
            Notifier<Command*>& m_commandDoneNotifier;
            Notifier<UndoableCommand*>& m_commandUndoNotifier;
            Notifier<UndoableCommand*>& m_commandUndoneNotifier;
        public:
            TransactionCommand(
                const std::string& name, std::vector<std::unique_ptr<UndoableCommand>>&& commands,
                Notifier<Command*>& i_commandDoNotifier,
                Notifier<Command*>& i_commandDoneNotifier,
                Notifier<UndoableCommand*>& i_commandUndoNotifier,
                Notifier<UndoableCommand*>& i_commandUndoneNotifier) :
                UndoableCommand(Type, name),
                m_commands(std::move(commands)),
                m_commandDoNotifier(i_commandDoNotifier),
                m_commandDoneNotifier(i_commandDoneNotifier),
                m_commandUndoNotifier(i_commandUndoNotifier),
                m_commandUndoneNotifier(i_commandUndoneNotifier) {}
        private:
            std::unique_ptr<CommandResult> doPerformDo(MapDocumentCommandFacade* document) override {
                for (auto& command : m_commands) {
                    notifyCommandIfNotType(m_commandDoNotifier, TransactionCommand::Type, command.get());
                    if (!command->performDo(document)) {
                        throw CommandProcessorException("Partial failure while executing transaction");
                    }
                    notifyCommandIfNotType(m_commandDoneNotifier, TransactionCommand::Type, command.get());
                }
                return std::make_unique<CommandResult>(true);
            }

            std::unique_ptr<CommandResult> doPerformUndo(MapDocumentCommandFacade* document) override {
                for (auto it = m_commands.rbegin(), end = m_commands.rend(); it != end; ++it) {
                    auto& command = *it;
                    notifyCommandIfNotType(m_commandUndoNotifier, TransactionCommand::Type, command.get());
                    if (!command->performUndo(document)) {
                        throw CommandProcessorException("Partial failure while undoing transaction");
                    }
                    notifyCommandIfNotType(m_commandUndoneNotifier, TransactionCommand::Type, command.get());
                }
                return std::make_unique<CommandResult>(true);
            }

            bool doCollateWith(UndoableCommand*) override {
                return false;
            }
        };

        const Command::CommandType CommandProcessor::TransactionCommand::Type = Command::freeType();

        CommandProcessor::CommandProcessor(MapDocumentCommandFacade* document, const std::chrono::milliseconds collationInterval) :
        m_document(document),
        m_collationInterval(collationInterval),
        m_lastCommandTimestamp(std::chrono::time_point<std::chrono::system_clock>()) {}

        CommandProcessor::~CommandProcessor() = default;

        bool CommandProcessor::canUndo() const {
            return m_transactionStack.empty() && !m_undoStack.empty();
        }

        bool CommandProcessor::canRedo() const {
            return m_transactionStack.empty() && !m_redoStack.empty();
        }

        const std::string& CommandProcessor::undoCommandName() const {
            if (!canUndo()) {
                throw CommandProcessorException("Command stack is empty");
            } else {
                return m_undoStack.back()->name();
            }
        }

        const std::string& CommandProcessor::redoCommandName() const {
            if (!canRedo()) {
                throw CommandProcessorException("Undo stack is empty");
            } else {
                return m_redoStack.back()->name();
            }
        }

        void CommandProcessor::startTransaction(const std::string& name) {
            m_transactionStack.push_back(TransactionState(name));
        }

        void CommandProcessor::commitTransaction() {
            if (m_transactionStack.empty()) {
                throw CommandProcessorException("No transaction is currently executing");
            } else {
                createAndStoreTransaction();
            }
        }

        void CommandProcessor::rollbackTransaction() {
            if (m_transactionStack.empty()) {
                throw CommandProcessorException("No transaction is currently executing");
            }

            auto& transaction = m_transactionStack.back();
            for (auto it = std::rbegin(transaction.commands), end = std::rend(transaction.commands); it != end; ++it) {
                undoCommand(it->get());
            }
            transaction.commands.clear();
        }

        std::unique_ptr<CommandResult> CommandProcessor::execute(std::unique_ptr<Command> command) {
            auto result = executeCommand(command.get());
            if (result->success()) {
                m_undoStack.clear();
                m_redoStack.clear();
            }
            return result;
        }

        std::unique_ptr<CommandResult> CommandProcessor::executeAndStore(std::unique_ptr<UndoableCommand> command) {
            return executeAndStoreCommand(std::move(command), true).commandResult;
        }

        std::unique_ptr<CommandResult> CommandProcessor::undo() {
            if (!m_transactionStack.empty()) {
                throw CommandProcessorException("Cannot undo individual commands of a transaction");
            } else if (m_undoStack.empty()) {
                throw CommandProcessorException("Undo stack is empty");
            } else {
                auto command = popFromUndoStack();
                auto result = undoCommand(command.get());
                if (result->success()) {
                    const auto commandName = command->name();
                    pushToRedoStack(std::move(command));
                    transactionUndoneNotifier(commandName);
                }
                return result;
            }
        }

        std::unique_ptr<CommandResult> CommandProcessor::redo() {
            if (!m_transactionStack.empty()) {
                throw CommandProcessorException("Cannot redo while in a transaction");
            } else if (m_redoStack.empty()) {
                throw CommandProcessorException("Redo stack is empty");
            } else {
                auto command = popFromRedoStack();
                auto result = executeCommand(command.get());
                if (result->success()) {
                    assertResult(pushToUndoStack(std::move(command), false))
                }
                return result;
            }
        }

        void CommandProcessor::clear() {
            assert(m_transactionStack.empty());

            m_undoStack.clear();
            m_redoStack.clear();
            m_lastCommandTimestamp = std::chrono::time_point<std::chrono::system_clock>();
        }

        CommandProcessor::SubmitAndStoreResult CommandProcessor::executeAndStoreCommand(std::unique_ptr<UndoableCommand> command, const bool collate) {
            auto commandResult = executeCommand(command.get());
            if (!commandResult->success()) {
                return SubmitAndStoreResult(std::move(commandResult), false);
            }

            const auto commandStored = storeCommand(std::move(command), collate);
            m_redoStack.clear();
            return SubmitAndStoreResult(std::move(commandResult), commandStored);
        }

        std::unique_ptr<CommandResult> CommandProcessor::executeCommand(Command* command) {
            notifyCommandIfNotType(commandDoNotifier, TransactionCommand::Type, command);
            auto result = command->performDo(m_document);
            if (result->success()) {
                notifyCommandIfNotType(commandDoneNotifier, TransactionCommand::Type, command);
                if (m_transactionStack.empty()) {
                    transactionDoneNotifier(command->name());
                }
            } else {
                notifyCommandIfNotType(commandDoFailedNotifier, TransactionCommand::Type, command);
            }
            return result;
        }

        std::unique_ptr<CommandResult> CommandProcessor::undoCommand(UndoableCommand* command) {
            notifyCommandIfNotType(commandUndoNotifier, TransactionCommand::Type, command);
            auto result = command->performUndo(m_document);
            if (result->success()) {
                notifyCommandIfNotType(commandUndoneNotifier, TransactionCommand::Type, command);
            } else {
                notifyCommandIfNotType(commandUndoFailedNotifier, TransactionCommand::Type, command);
            }
            return result;
        }

        bool CommandProcessor::storeCommand(std::unique_ptr<UndoableCommand> command, const bool collate) {
            if (m_transactionStack.empty()) {
                return pushToUndoStack(std::move(command), collate);
            } else {
                return pushTransactionCommand(std::move(command), collate);
            }
        }

        bool CommandProcessor::pushTransactionCommand(std::unique_ptr<UndoableCommand> command, const bool collate) {
            assert(!m_transactionStack.empty());
            auto& transaction = m_transactionStack.back();
            if (!transaction.commands.empty()) {
                auto& lastCommand = transaction.commands.back();
                if (collate && lastCommand->collateWith(command.get())) {
                    // the command is not stored because it was collated with its predecessor
                    return false;
                }
            }
            transaction.commands.push_back(std::move(command));
            return true;
        }

        void CommandProcessor::createAndStoreTransaction() {
            assert(!m_transactionStack.empty());

            auto transaction = kdl::vec_pop_back(m_transactionStack);
            if (!transaction.commands.empty()) {
                if (transaction.name.empty()) {
                    transaction.name = transaction.commands.front()->name();
                }
                auto command = createTransaction(transaction.name, std::move(transaction.commands));

                if (m_transactionStack.empty()) {
                    pushToUndoStack(std::move(command), false);
                } else {
                    pushTransactionCommand(std::move(command), false);
                }
                transactionDoneNotifier(transaction.name);
            }
        }

        std::unique_ptr<UndoableCommand> CommandProcessor::createTransaction(const std::string& name, std::vector<std::unique_ptr<UndoableCommand>> commands) {
            return std::make_unique<TransactionCommand>(
                name, std::move(commands),
                commandDoNotifier,
                commandDoneNotifier,
                commandUndoNotifier,
                commandUndoneNotifier);
        }

        bool CommandProcessor::pushToUndoStack(std::unique_ptr<UndoableCommand> command, const bool collate) {
            assert(m_transactionStack.empty());

            const auto timestamp = std::chrono::system_clock::now();
            const kdl::set_later setLastCommandTimestamp(m_lastCommandTimestamp, timestamp);

            if (collatable(collate, timestamp)) {
                auto& lastCommand = m_undoStack.back();
                if (lastCommand->collateWith(command.get())) {
                    return false;
                }
            }

            m_undoStack.push_back(std::move(command));
            return true;
        }

        std::unique_ptr<UndoableCommand> CommandProcessor::popFromUndoStack() {
            assert(m_transactionStack.empty());
            assert(!m_undoStack.empty());

            return kdl::vec_pop_back(m_undoStack);
        }

        bool CommandProcessor::collatable(const bool collate, const std::chrono::system_clock::time_point timestamp) const {
            return collate && !m_undoStack.empty() && timestamp - m_lastCommandTimestamp <= m_collationInterval;
        }

        void CommandProcessor::pushToRedoStack(std::unique_ptr<UndoableCommand> command) {
            assert(m_transactionStack.empty());
            m_redoStack.push_back(std::move(command));
        }

        std::unique_ptr<UndoableCommand> CommandProcessor::popFromRedoStack() {
            assert(m_transactionStack.empty());
            assert(!m_redoStack.empty());

            return kdl::vec_pop_back(m_redoStack);
        }
    }
}
