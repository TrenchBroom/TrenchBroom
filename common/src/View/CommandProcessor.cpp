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
#include "View/TransactionScope.h"
#include "View/UndoableCommand.h"

#include <kdl/set_temp.h>
#include <kdl/string_utils.h>
#include <kdl/vector_utils.h>

#include <algorithm>

#include <QDateTime>

namespace TrenchBroom {
namespace View {
template <typename Ignore, typename T, typename C>
void notifyCommandIfNotType(T& notifier, C& command) {
  if (dynamic_cast<Ignore*>(&command) == nullptr) {
    notifier(command);
  }
}

struct CommandProcessor::TransactionState {
  std::string name;
  TransactionScope scope;
  std::vector<std::unique_ptr<UndoableCommand>> commands;

  TransactionState(std::string i_name, const TransactionScope i_scope)
    : name{std::move(i_name)}
    , scope{i_scope} {}
};

struct CommandProcessor::SubmitAndStoreResult {
  std::unique_ptr<CommandResult> commandResult;
  bool commandStored;

  SubmitAndStoreResult(std::unique_ptr<CommandResult> i_commandResult, const bool i_commandStored)
    : commandResult{std::move(i_commandResult)}
    , commandStored{i_commandStored} {}
};

class CommandProcessor::TransactionCommand : public UndoableCommand {
private:
  std::vector<std::unique_ptr<UndoableCommand>> m_commands;

  Notifier<Command&>& m_commandDoNotifier;
  Notifier<Command&>& m_commandDoneNotifier;
  Notifier<UndoableCommand&>& m_commandUndoNotifier;
  Notifier<UndoableCommand&>& m_commandUndoneNotifier;

public:
  TransactionCommand(
    std::string name, std::vector<std::unique_ptr<UndoableCommand>> commands,
    Notifier<Command&>& i_commandDoNotifier, Notifier<Command&>& i_commandDoneNotifier,
    Notifier<UndoableCommand&>& i_commandUndoNotifier,
    Notifier<UndoableCommand&>& i_commandUndoneNotifier)
    : UndoableCommand(std::move(name), false)
    , m_commands{std::move(commands)}
    , m_commandDoNotifier{i_commandDoNotifier}
    , m_commandDoneNotifier{i_commandDoneNotifier}
    , m_commandUndoNotifier{i_commandUndoNotifier}
    , m_commandUndoneNotifier{i_commandUndoneNotifier} {}

private:
  std::unique_ptr<CommandResult> doPerformDo(MapDocumentCommandFacade* document) override {
    for (auto& command : m_commands) {
      notifyCommandIfNotType<TransactionCommand>(m_commandDoNotifier, *command);
      if (!command->performDo(document)) {
        throw CommandProcessorException("Partial failure while executing transaction");
      }
      notifyCommandIfNotType<TransactionCommand>(m_commandDoneNotifier, *command);
    }
    return std::make_unique<CommandResult>(true);
  }

  std::unique_ptr<CommandResult> doPerformUndo(MapDocumentCommandFacade* document) override {
    for (auto it = m_commands.rbegin(), end = m_commands.rend(); it != end; ++it) {
      auto& command = *it;
      notifyCommandIfNotType<TransactionCommand>(m_commandUndoNotifier, *command);
      if (!command->performUndo(document)) {
        throw CommandProcessorException("Partial failure while undoing transaction");
      }
      notifyCommandIfNotType<TransactionCommand>(m_commandUndoneNotifier, *command);
    }
    return std::make_unique<CommandResult>(true);
  }

  bool doCollateWith(UndoableCommand& other) override {
    if (auto* transactionCommand = dynamic_cast<TransactionCommand*>(&other)) {
      if (m_commands.empty()) {
        m_commands = std::move(transactionCommand->m_commands);
        return true;
      }
      if (transactionCommand->m_commands.empty()) {
        return true;
      }

      auto it = std::begin(transactionCommand->m_commands);
      if (m_commands.back()->collateWith(**it)) {
        std::move(
          std::next(it), std::end(transactionCommand->m_commands), std::back_inserter(m_commands));
        return true;
      }
    }

    return false;
  }
};

CommandProcessor::CommandProcessor(
  MapDocumentCommandFacade* document, const std::chrono::milliseconds collationInterval)
  : m_document{document}
  , m_collationInterval{collationInterval}
  , m_lastCommandTimestamp{std::chrono::time_point<std::chrono::system_clock>{}} {}

CommandProcessor::~CommandProcessor() = default;

bool CommandProcessor::canUndo() const {
  return m_transactionStack.empty() && !m_undoStack.empty();
}

bool CommandProcessor::canRedo() const {
  return m_transactionStack.empty() && !m_redoStack.empty();
}

const std::string& CommandProcessor::undoCommandName() const {
  if (!canUndo()) {
    throw CommandProcessorException{"Command stack is empty"};
  } else {
    return m_undoStack.back()->name();
  }
}

const std::string& CommandProcessor::redoCommandName() const {
  if (!canRedo()) {
    throw CommandProcessorException{"Undo stack is empty"};
  } else {
    return m_redoStack.back()->name();
  }
}

void CommandProcessor::startTransaction(std::string name, const TransactionScope scope) {
  m_transactionStack.emplace_back(std::move(name), scope);
}

void CommandProcessor::commitTransaction() {
  if (m_transactionStack.empty()) {
    throw CommandProcessorException{"No transaction is currently executing"};
  } else {
    createAndStoreTransaction();
  }
}

void CommandProcessor::rollbackTransaction() {
  if (m_transactionStack.empty()) {
    throw CommandProcessorException{"No transaction is currently executing"};
  }

  auto& transaction = m_transactionStack.back();
  for (auto it = std::rbegin(transaction.commands), end = std::rend(transaction.commands);
       it != end; ++it) {
    undoCommand(**it);
  }
  transaction.commands.clear();
}

bool CommandProcessor::isCurrentDocumentStateObservable() const {
  return m_transactionStack.size() < 2 ||
         m_transactionStack[m_transactionStack.size() - 2].scope == TransactionScope::LongRunning;
}

std::unique_ptr<CommandResult> CommandProcessor::execute(std::unique_ptr<Command> command) {
  auto result = executeCommand(*command);
  if (result->success()) {
    m_undoStack.clear();
    m_redoStack.clear();
  }
  return result;
}

std::unique_ptr<CommandResult> CommandProcessor::executeAndStore(
  std::unique_ptr<UndoableCommand> command) {
  return executeAndStoreCommand(std::move(command), true).commandResult;
}

std::unique_ptr<CommandResult> CommandProcessor::undo() {
  if (!m_transactionStack.empty()) {
    throw CommandProcessorException("Cannot undo individual commands of a transaction");
  } else if (m_undoStack.empty()) {
    throw CommandProcessorException("Undo stack is empty");
  } else {
    auto command = popFromUndoStack();
    auto result = undoCommand(*command);
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
    auto result = executeCommand(*command);
    if (result->success()) {
      assertResult(pushToUndoStack(std::move(command), false));
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

CommandProcessor::SubmitAndStoreResult CommandProcessor::executeAndStoreCommand(
  std::unique_ptr<UndoableCommand> command, const bool collate) {
  auto commandResult = executeCommand(*command);
  if (!commandResult->success()) {
    return SubmitAndStoreResult(std::move(commandResult), false);
  }

  const auto commandStored = storeCommand(std::move(command), collate);
  m_redoStack.clear();
  return SubmitAndStoreResult(std::move(commandResult), commandStored);
}

std::unique_ptr<CommandResult> CommandProcessor::executeCommand(Command& command) {
  notifyCommandIfNotType<TransactionCommand>(commandDoNotifier, command);
  auto result = command.performDo(m_document);
  if (result->success()) {
    notifyCommandIfNotType<TransactionCommand>(commandDoneNotifier, command);
    if (m_transactionStack.empty()) {
      transactionDoneNotifier(command.name());
    }
  } else {
    notifyCommandIfNotType<TransactionCommand>(commandDoFailedNotifier, command);
  }
  return result;
}

std::unique_ptr<CommandResult> CommandProcessor::undoCommand(UndoableCommand& command) {
  notifyCommandIfNotType<TransactionCommand>(commandUndoNotifier, command);
  auto result = command.performUndo(m_document);
  if (result->success()) {
    notifyCommandIfNotType<TransactionCommand>(commandUndoneNotifier, command);
  } else {
    notifyCommandIfNotType<TransactionCommand>(commandUndoFailedNotifier, command);
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

bool CommandProcessor::pushTransactionCommand(
  std::unique_ptr<UndoableCommand> command, const bool collate) {
  assert(!m_transactionStack.empty());
  auto& transaction = m_transactionStack.back();
  if (!transaction.commands.empty()) {
    auto& lastCommand = transaction.commands.back();
    if (collate && lastCommand->collateWith(*command)) {
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
      pushToUndoStack(std::move(command), true);
    } else {
      pushTransactionCommand(std::move(command), true);
    }
    transactionDoneNotifier(transaction.name);
  }
}

std::unique_ptr<UndoableCommand> CommandProcessor::createTransaction(
  std::string name, std::vector<std::unique_ptr<UndoableCommand>> commands) {
  return std::make_unique<TransactionCommand>(
    std::move(name), std::move(commands), commandDoNotifier, commandDoneNotifier,
    commandUndoNotifier, commandUndoneNotifier);
}

bool CommandProcessor::pushToUndoStack(
  std::unique_ptr<UndoableCommand> command, const bool collate) {
  assert(m_transactionStack.empty());

  const auto timestamp = std::chrono::system_clock::now();
  const auto setLastCommandTimestamp = kdl::set_later{m_lastCommandTimestamp, timestamp};

  if (collatable(collate, timestamp)) {
    auto& lastCommand = m_undoStack.back();
    if (lastCommand->collateWith(*command)) {
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

bool CommandProcessor::collatable(
  const bool collate, const std::chrono::system_clock::time_point timestamp) const {
  return collate && !m_undoStack.empty() &&
         timestamp - m_lastCommandTimestamp <= m_collationInterval;
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
} // namespace View
} // namespace TrenchBroom
