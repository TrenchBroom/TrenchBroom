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
#include "TemporarilySetAny.h"
#include "View/MapDocumentCommandFacade.h"

#include <kdl/string_utils.h>

#include <string>

#include <QDateTime>

namespace TrenchBroom {
    namespace View {
        const Command::CommandType CommandGroup::Type = Command::freeType();

        CommandGroup::CommandGroup(const std::string& name, std::vector<std::unique_ptr<UndoableCommand>>&& commands,
                                   Notifier<Command*>& commandDoNotifier,
                                   Notifier<Command*>& commandDoneNotifier,
                                   Notifier<UndoableCommand*>& commandUndoNotifier,
                                   Notifier<UndoableCommand*>& commandUndoneNotifier) :
        UndoableCommand(Type, name),
        m_commands(std::move(commands)),
        m_commandDoNotifier(commandDoNotifier),
        m_commandDoneNotifier(commandDoneNotifier),
        m_commandUndoNotifier(commandUndoNotifier),
        m_commandUndoneNotifier(commandUndoneNotifier) {}

        bool CommandGroup::doPerformDo(MapDocumentCommandFacade* document) {
            for (auto it = std::begin(m_commands), end = std::end(m_commands); it != end; ++it) {
                auto& command = *it;
                m_commandDoNotifier(command.get());
                if (!command->performDo(document)) {
                    throw CommandProcessorException("Partial failure while executing command group");
                }
                m_commandDoneNotifier(command.get());
            }
            return true;
        }

        bool CommandGroup::doPerformUndo(MapDocumentCommandFacade* document) {
            for (auto it = m_commands.rbegin(), end = m_commands.rend(); it != end; ++it) {
                auto& command = *it;
                m_commandUndoNotifier(command.get());
                if (!command->performUndo(document)) {
                    throw CommandProcessorException("Partial failure while undoing command group");
                }
                m_commandUndoneNotifier(command.get());
            }
            return true;
        }

        bool CommandGroup::doIsRepeatDelimiter() const {
            for (auto it = std::begin(m_commands), end = std::end(m_commands); it != end; ++it) {
                auto& command = *it;
                if (command->isRepeatDelimiter()) {
                    return true;
                }
            }
            return false;
        }

        bool CommandGroup::doIsRepeatable(MapDocumentCommandFacade* document) const {
            for (auto it = std::begin(m_commands), end = std::end(m_commands); it != end; ++it) {
                auto& command = *it;
                if (!command->isRepeatable(document)) {
                    return false;
                }
            }
            return true;
        }

        std::unique_ptr<UndoableCommand> CommandGroup::doRepeat(MapDocumentCommandFacade* document) const {
            std::vector<std::unique_ptr<UndoableCommand>> clones;
            for (auto it = std::begin(m_commands), end = std::end(m_commands); it != end; ++it) {
                auto& command = *it;
                assert(command->isRepeatable(document));
                clones.push_back(command->repeat(document));
            }
            return std::make_unique<CommandGroup>(name(), std::move(clones), m_commandDoNotifier, m_commandDoneNotifier, m_commandUndoNotifier, m_commandUndoneNotifier);
        }

        bool CommandGroup::doCollateWith(UndoableCommand*) {
            return false;
        }

        const int64_t CommandProcessor::CollationInterval = 1000;

        struct CommandProcessor::SubmitAndStoreResult {
            bool submitted;
            bool stored;

            SubmitAndStoreResult() :
            submitted(false),
            stored(false) {}
        };

        CommandProcessor::CommandProcessor(MapDocumentCommandFacade* document) :
        m_document(document),
        m_clearRepeatableCommandStack(false),
        m_lastCommandTimestamp(0),
        m_groupLevel(0) {
            ensure(m_document != nullptr, "document is null");
        }

        bool CommandProcessor::hasLastCommand() const {
            return !m_lastCommandStack.empty();
        }

        bool CommandProcessor::hasNextCommand() const {
            return !m_nextCommandStack.empty();
        }

        const std::string& CommandProcessor::lastCommandName() const {
            if (!hasLastCommand()) {
                throw CommandProcessorException("Command stack is empty");
            } else {
                return m_lastCommandStack.back()->name();
            }
        }

        const std::string& CommandProcessor::nextCommandName() const {
            if (!hasNextCommand()) {
                throw CommandProcessorException("Undo stack is empty");
            } else {
                return m_nextCommandStack.back()->name();
            }
        }

        void CommandProcessor::beginGroup(const std::string& name) {
            if (m_groupLevel == 0) {
                m_groupName = name;
            }
            ++m_groupLevel;
        }

        void CommandProcessor::endGroup() {
            if (m_groupLevel == 0) {
                throw CommandProcessorException("Group stack is empty");
            } else {
                --m_groupLevel;
                if (m_groupLevel == 0) {
                    createAndStoreCommandGroup();
                }
            }
        }

        void CommandProcessor::rollbackGroup() {
            while (!m_groupedCommands.empty()) {
                undoCommand(popGroupedCommand().get());
            }
        }

        bool CommandProcessor::submitCommand(std::unique_ptr<Command>&& command) {
            const auto success = doCommand(command.get());
            if (!success) {
                return false;
            } else {
                m_lastCommandStack.clear();
                m_nextCommandStack.clear();
                return true;
            }
        }

        bool CommandProcessor::submitAndStoreCommand(std::unique_ptr<UndoableCommand>&& command) {
            return submitAndStoreCommand(std::move(command), true).submitted;
        }

        bool CommandProcessor::undoLastCommand() {
            if (m_groupLevel > 0) {
                throw CommandProcessorException("Cannot undo individual commands of a command group");
            } else {
                auto command = popLastCommand();
                if (undoCommand(command.get())) {
                    const auto commandName = command->name();
                    pushNextCommand(std::move(command));
                    transactionUndoneNotifier(commandName);
                    return true;
                } else {
                    return false;
                }
            }
        }

        bool CommandProcessor::redoNextCommand() {
            if (m_groupLevel > 0) {
                throw CommandProcessorException("Cannot redo while in a command group");
            } else {
                auto command = popNextCommand();
                if (doCommand(command.get())) {
                    return pushLastCommand(std::move(command), false);
                } else {
                    return false;
                }
            }
        }

        bool CommandProcessor::hasRepeatableCommands() const {
            return !m_repeatableCommandStack.empty();
        }

        bool CommandProcessor::repeatLastCommands() {
            std::vector<std::unique_ptr<UndoableCommand>> commands;
            for (auto it = std::begin(m_repeatableCommandStack), end = std::end(m_repeatableCommandStack); it != end; ++it) {
                auto command = *it;
                if (command->isRepeatable(m_document)) {
                    commands.push_back(command->repeat(m_document));
                }
            }

            if (commands.empty()) {
                return false;
            }

            const auto name = kdl::str_to_string("Repeat ", commands.size(), " Commands");
            auto repeatableCommand = createCommandGroup(name, std::move(commands));
            return submitAndStoreCommand(std::move(repeatableCommand), false).submitted;
        }

        void CommandProcessor::clearRepeatableCommands() {
            m_repeatableCommandStack.clear();
            m_clearRepeatableCommandStack = false;
        }

        void CommandProcessor::clear() {
            assert(m_groupLevel == 0);

            clearRepeatableCommands();
            m_lastCommandStack.clear();
            m_nextCommandStack.clear();
            m_lastCommandTimestamp = 0;
        }

        CommandProcessor::SubmitAndStoreResult CommandProcessor::submitAndStoreCommand(std::unique_ptr<UndoableCommand>&& command, const bool collate) {
            SubmitAndStoreResult result;
            result.submitted = doCommand(command.get());
            if (!result.submitted) {
                return result;
            }

            result.stored = storeCommand(std::move(command), collate);
            if (!m_nextCommandStack.empty()) {
                m_nextCommandStack.clear();
            }
            return result;
        }

        bool CommandProcessor::doCommand(Command* command) {
            commandDoNotifier(command);
            if (command->performDo(m_document)) {
                commandDoneNotifier(command);
                if (m_groupLevel == 0) {
                    transactionDoneNotifier(command->name());
                }
                return true;
            } else {
                commandDoFailedNotifier(command);
                return false;
            }
        }

        bool CommandProcessor::undoCommand(UndoableCommand* command) {
            commandUndoNotifier(command);
            if (command->performUndo(m_document)) {
                commandUndoneNotifier(command);
                return true;
            } else {
                commandUndoFailedNotifier(command);
            return false;
        }
        }

        bool CommandProcessor::storeCommand(std::unique_ptr<UndoableCommand>&& command, const bool collate) {
            if (m_groupLevel == 0) {
                return pushLastCommand(std::move(command), collate);
            } else {
                return pushGroupedCommand(std::move(command), collate);
            }
        }

        bool CommandProcessor::pushGroupedCommand(std::unique_ptr<UndoableCommand>&& command, const bool collate) {
            assert(m_groupLevel > 0);
            if (!m_groupedCommands.empty()) {
                auto& lastCommand = m_groupedCommands.back();
                if (collate && lastCommand->collateWith(command.get())) {
                    // the command is not stored because it was collated with its predecessor
                    return false;
                }
            }
            m_groupedCommands.push_back(std::move(command));
            return true;
        }

        std::unique_ptr<UndoableCommand> CommandProcessor::popGroupedCommand() {
            assert(m_groupLevel > 0);
            if (m_groupedCommands.empty()) {
                throw CommandProcessorException("Group command stack is empty");
            } else {
                auto groupedCommand = std::move(m_groupedCommands.back());
                m_groupedCommands.pop_back();
                return groupedCommand;
            }
        }

        void CommandProcessor::createAndStoreCommandGroup() {
            if (!m_groupedCommands.empty()) {
                if (m_groupName.empty()) {
                    m_groupName = m_groupedCommands.front()->name();
                }
                auto command = createCommandGroup(m_groupName, std::move(m_groupedCommands));
                pushLastCommand(std::move(command), false);
                transactionDoneNotifier(m_groupName);
            }
            m_groupName = "";
        }

        std::unique_ptr<UndoableCommand> CommandProcessor::createCommandGroup(const std::string& name, std::vector<std::unique_ptr<UndoableCommand>>&& commands) {
            return std::make_unique<CommandGroup>(
                name, std::move(commands),
                commandDoNotifier,
                commandDoneNotifier,
                commandUndoNotifier,
                commandUndoneNotifier);
        }

        bool CommandProcessor::pushLastCommand(std::unique_ptr<UndoableCommand>&& command, const bool collate) {
            assert(m_groupLevel == 0);

            const int64_t timestamp = QDateTime::currentMSecsSinceEpoch();
            const SetLate<int64_t> setLastCommandTimestamp(m_lastCommandTimestamp, timestamp);

            if (collatable(collate, timestamp)) {
                auto& lastCommand = m_lastCommandStack.back();
                if (lastCommand->collateWith(command.get())) {
                    return false;
                }
            }

            pushRepeatableCommand(command.get());
            m_lastCommandStack.push_back(std::move(command));
            return true;
        }

        std::unique_ptr<UndoableCommand> CommandProcessor::popLastCommand() {
            assert(m_groupLevel == 0);
            if (m_lastCommandStack.empty()) {
                throw CommandProcessorException("Command stack is empty");
            } else {
                auto lastCommand = std::move(m_lastCommandStack.back());
                m_lastCommandStack.pop_back();
                popLastRepeatableCommand(lastCommand.get());
                return lastCommand;
            }
        }

        bool CommandProcessor::collatable(const bool collate, const int64_t timestamp) const {
            return collate && !m_lastCommandStack.empty() && timestamp - m_lastCommandTimestamp <= CollationInterval;
        }

        void CommandProcessor::pushNextCommand(std::unique_ptr<UndoableCommand>&& command) {
            assert(m_groupLevel == 0);
            m_nextCommandStack.push_back(std::move(command));
        }

        std::unique_ptr<UndoableCommand> CommandProcessor::popNextCommand() {
            assert(m_groupLevel == 0);
            if (m_nextCommandStack.empty()) {
                throw CommandProcessorException("Command stack is empty");
            } else {
                auto nextCommand = std::move(m_nextCommandStack.back());
                m_nextCommandStack.pop_back();
                return nextCommand;
            }
        }

        void CommandProcessor::pushRepeatableCommand(UndoableCommand* command) {
            if (command->isRepeatDelimiter()) {
                m_clearRepeatableCommandStack = true;
            } else {
                if (m_clearRepeatableCommandStack) {
                    m_repeatableCommandStack.clear();
                    m_clearRepeatableCommandStack = false;
                }
                m_repeatableCommandStack.push_back(command);
            }
        }

        void CommandProcessor::popLastRepeatableCommand(UndoableCommand* command) {
            if (!m_repeatableCommandStack.empty() && m_repeatableCommandStack.back() == command) {
                m_repeatableCommandStack.pop_back();
            }
        }
    }
}
