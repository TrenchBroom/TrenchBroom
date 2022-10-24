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

#pragma once

#include "Notifier.h"

#include <chrono>
#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom
{
namespace View
{
class Command;
class CommandResult;
class MapDocumentCommandFacade;
class UndoableCommand;
enum class TransactionScope;

/**
 * The command processor is responsible for executing and undoing commands and for
 * maintining the command history in the form of a stack of undo commands and a stack of
 * redo commands.
 *
 * Successive commands can be collated if they meet certain conditions. For two commands
 * to be collated, the following conditions must apply:
 *
 * - the commands are not executed as part of an undo or redo
 * - the previous command's `collateWith` method returns true when passed the succeeding
 * command
 * - the time passed between the execution of the commands does not exceed the collation
 * interval.
 *
 * The command processor supports nested transactions. Each transaction can be committed
 * or rolled back individually. Committing a nested transaction adds it as a command to
 * the containing transaction.
 */
class CommandProcessor
{
private:
  /**
   * The document to pass on to commands when they are executed or undone.
   */
  MapDocumentCommandFacade* m_document;

  /**
   * Limits the time after which to succeeding commands can be collated.
   */
  std::chrono::milliseconds m_collationInterval;

  /**
   * Holds the commands that were executed so far, with the most recently executed command
   * at the end of the vector.
   */
  std::vector<std::unique_ptr<UndoableCommand>> m_undoStack;

  /**
   * Holds the commands that were undone, with the most recently undone command at the
   * beginning of the vector.
   */
  std::vector<std::unique_ptr<UndoableCommand>> m_redoStack;

  /**
   * The time stamp of when the last command was executed.
   */
  std::chrono::system_clock::time_point m_lastCommandTimestamp;

  struct TransactionState;

  /**
   * Holds the states of the currently executing transitions.
   */
  std::vector<TransactionState> m_transactionStack;

  struct SubmitAndStoreResult;
  class TransactionCommand;

public:
  /**
   * Creates a new command processor which will pass the given document to commands when
   * they are executed or undone.
   *
   * @param document the document to pass to commands, may be null
   */
  explicit CommandProcessor(
    MapDocumentCommandFacade* document,
    std::chrono::milliseconds collationInterval = std::chrono::milliseconds{1000});

  ~CommandProcessor();

  /**
   * Notifies observers when a command is going to be executed.
   */
  Notifier<Command&> commandDoNotifier;

  /**
   * Notifies observers when a command was successfully executed.
   */
  Notifier<Command&> commandDoneNotifier;

  /**
   * Notifies observers when a command failed to execute.
   */
  Notifier<Command&> commandDoFailedNotifier;

  /**
   * Notifies observers when an undoable command is going to be undone.
   */
  Notifier<UndoableCommand&> commandUndoNotifier;

  /**
   * Notifies observers when an undoable command was successfully undone.
   */
  Notifier<UndoableCommand&> commandUndoneNotifier;

  /**
   * Notifies observers when an undoable command failed to undo.
   */
  Notifier<UndoableCommand&> commandUndoFailedNotifier;

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
   * Starts a new transaction. If a transaction is currently executing, then the newly
   * started transaction becomes a nested transaction and will be added as a command to
   * its parent transaction upon commit.
   *
   * @param name the name of the transaction to start
   * @param scope the scope of the transaction to start
   */
  void startTransaction(std::string name, TransactionScope scope);

  /**
   * Commits the currently executing transaction. If it is a nested transaction, then its
   * commands will be added to the containing transaction as a single command.
   *
   * If the current transaction does not contain any commands, then the transaction ends,
   * but nothing will be stored in the command processor.
   *
   * @throws CommandProcessorException if no transaction is currently executing
   */
  void commitTransaction();

  /**
   * Rolls the currently executing transaction back by undoing all commands that belong to
   * the transaction. The transaction does not end when it is rolled back, rather, it
   * remains executing. To end a transaction after it was rolled back, call
   * `commitTransaction`. Since the transaction will be empty, committing it will just do
   * nothing but remove the transaction itself.
   *
   * @throws CommandProcessorException if no transaction is currently executing
   */
  void rollbackTransaction();

  /**
   * Indicates whether the current document state is observable.
   *
   * If no transaction is active, the state is observable.
   * If a transaction is active, the state is observable unless the transaction is nested
   * inside a one shot transaction.
   */
  bool isCurrentDocumentStateObservable() const;

  /**
   * Executes the given command by calling its `performDo` method without storing it for
   * later undo. If the command is executed successfully, both the undo and the redo
   * stacks are cleared since the application's state is likely to have become
   * inconsistent with the state expected by the commands on these stacks.
   *
   * The command processor takes ownership of the given command, and since it is not
   * stored, it will be deleted immediately after execution.
   *
   * @param command the command to execute
   * @return the result of executing the given command
   */
  std::unique_ptr<CommandResult> execute(std::unique_ptr<Command> command);

  /**
   * Executes the given command by calling its `performDo` method and stores it for later
   * undo if it is successfully executed. In that case, the redo stack is cleared since
   * the application's state is likely to have become inconsistent with the state expected
   * by the commands on the redo stack.
   *
   * If the given command is executed successfully, the command processor will attempt to
   * collate it with the command at the top of the undo stack by calling its `collateWith`
   * method. If the collation takes place, then the given command will not be stored on
   * the undo stack.
   *
   * The command processor takes ownership of the given command, so unless it is stored on
   * the undo stack under the previously described conditions, the command is deleted
   * immediately after it is executed.
   *
   * @param command the command to execute
   * @return the result of executing the given command
   */
  std::unique_ptr<CommandResult> executeAndStore(
    std::unique_ptr<UndoableCommand> command);

  /**
   * Undoes the most recently executed command by calling its `performUndo` method and
   * stores the command on the redo stack if it could be undone successfully, i.e. its
   * `performUndo` method returned a successful command result.
   *
   * @return the result of undoing the command
   *
   * @throws CommandProcessorException if a transaction is currently being executed or if
   * the undo stack is empty
   */
  std::unique_ptr<CommandResult> undo();

  /**
   * Redoes the most recently undone comment by calling its `performDo` method and stores
   * the command on the undo stack if it could be executed successfully, i.e. its
   * `performDo` method returned a successful command result.
   *
   * @return the result of executing the command
   *
   * @throws CommandProcessorException if a transaction is currently being executed or if
   * the redo stack is empty
   */
  std::unique_ptr<CommandResult> redo();

  /**
   * Clears this command processor. Both the undo and the redo stack are cleared, and
   * therefore all stored commands are deleted as well.
   */
  void clear();

private:
  /**
   * Executes and stores the given command. The command will only be stored if it was
   * executed successfully and if it wasn't collated with the topmost command on the undo
   * stack.
   *
   * @param command the command to execute and store
   * @param collate whether or not the given command should be collated with the topmost
   * command on the undo stack
   * @return a struct containing the result of executing the given command and a boolean
   * indicating whether the given command was stored on the undo stack
   */
  SubmitAndStoreResult executeAndStoreCommand(
    std::unique_ptr<UndoableCommand> command, bool collate);

  /**
   * Executes the given command by calling its `performDo` method and triggers the
   * corresponding notifications.
   *
   * @param command the command to execute
   * @return the result of executing the given command
   */
  std::unique_ptr<CommandResult> executeCommand(Command& command);

  /**
   * Undoes the given command by calling its `performUndo` method and triggers the
   * corresponding notifications.
   *
   * @param command the command to undo
   * @return the result of undoing the given command
   */
  std::unique_ptr<CommandResult> undoCommand(UndoableCommand& command);

  /**
   * Stores the given command or collates it with the topmost command on the undo stack.
   *
   * @param command the command to store
   * @param collate whether not to attempt to collate the given command with the topmost
   * command on the undo stack
   * @return true if the command was stored, and false if it was not stored
   */
  bool storeCommand(std::unique_ptr<UndoableCommand> command, bool collate);

  /**
   * Pushes the given command to the back of the list of commands belonging to the
   * currently executing transaction. If `collate` is `true`, then it is attempted to
   * collate the given command with the last command executed in the currently executing
   * transaction.
   *
   * Precondition: a transaction is currently executing
   *
   * @param command the command to push
   * @param collate whether or not to attempt to collate the given command with the last
   * command executed in the currently executing transaction
   * @return true if the given command was stored, and false if it was not stored
   */
  bool pushTransactionCommand(std::unique_ptr<UndoableCommand> command, bool collate);

  /**
   * Creates a new transaction containing all commands which were stored in the scope of
   * the current transaction. If no command was stored, then nothing happens.
   *
   * Triggers a `transactionDone` notification. Afterwards, the transaction is stored as a
   * single command on the undo stack.
   */
  void createAndStoreTransaction();

  /**
   * Creates and returns a command that, when executed, executes all commands which were
   * stored in the scope of the current transaction. When this command is undone, it also
   * undoes all of these commands in reverse order.
   *
   * @param name the name of the command to create
   * @param commands the commands to store in the newly created transaction command
   * @return the newly created command
   */
  std::unique_ptr<UndoableCommand> createTransaction(
    std::string name, std::vector<std::unique_ptr<UndoableCommand>> commands);

  /**
   * Pushes the given command onto the undo stack, unless it can be collated with the
   * topmost command on the undo stack. Takes ownership of the given command, so if it
   * isn't stored on the undo stack, the command is deleted.
   *
   * @param command the command to push
   * @param collate whether or not it should be attempted to collate the given command
   * with the topmost command on the undo stack
   * @return true if the given command was stored on the undo stack and false otherwise
   */
  bool pushToUndoStack(std::unique_ptr<UndoableCommand> command, bool collate);

  /**
   * Pops the topmost command from the undo stack and returns it.
   *
   * Precondition: the undo stack is not empty, and no transaction is currently executing
   *
   * @return the topmost command of the undo stack
   */
  std::unique_ptr<UndoableCommand> popFromUndoStack();

  bool collatable(bool collate, std::chrono::system_clock::time_point timestamp) const;

  /**
   * Pushes the given command onto the redo stack. Takes ownership of the given command.
   *
   * @param command the command to push
   */
  void pushToRedoStack(std::unique_ptr<UndoableCommand> command);

  /**
   * Pops the topmost command from the redo stack and returns it.
   *
   * Precondition: the redo stack is not empty, and no transaction is currently executing
   *
   * @return the topmost command of the redo stack
   */
  std::unique_ptr<UndoableCommand> popFromRedoStack();
};
} // namespace View
} // namespace TrenchBroom
