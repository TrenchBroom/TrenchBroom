/*
 Copyright (C) 2019 Eric Wasylishen

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

#include "Macros.h"
#include "NotifierConnection.h"
#include "View/CommandProcessor.h"
#include "View/TransactionScope.h"
#include "View/UndoableCommand.h"

#include "kdl/vector_utils.h"

#include <chrono>
#include <memory>
#include <optional>
#include <thread>
#include <variant>

#include "Catch2.h"

namespace TrenchBroom
{
namespace View
{
enum class CommandNotif
{
  CommandDo,
  CommandDone,
  CommandDoFailed,
  CommandUndo,
  CommandUndone,
  CommandUndoFailed,
  TransactionDone,
  TransactionUndone
};

using NotificationTuple = std::tuple<CommandNotif, std::string>;

class TestObserver
{
private:
  std::vector<NotificationTuple> m_notifications;
  NotifierConnection m_notifierConnection;

public:
  explicit TestObserver(CommandProcessor& commandProcessor)
  {
    m_notifierConnection +=
      commandProcessor.commandDoNotifier.connect(this, &TestObserver::commandDo);
    m_notifierConnection +=
      commandProcessor.commandDoneNotifier.connect(this, &TestObserver::commandDone);
    m_notifierConnection += commandProcessor.commandDoFailedNotifier.connect(
      this, &TestObserver::commandDoFailed);
    m_notifierConnection +=
      commandProcessor.commandUndoNotifier.connect(this, &TestObserver::commandUndo);
    m_notifierConnection +=
      commandProcessor.commandUndoneNotifier.connect(this, &TestObserver::commandUndone);
    m_notifierConnection += commandProcessor.commandUndoFailedNotifier.connect(
      this, &TestObserver::commandUndoFailed);
    m_notifierConnection += commandProcessor.transactionDoneNotifier.connect(
      this, &TestObserver::transactionDone);
    m_notifierConnection += commandProcessor.transactionUndoneNotifier.connect(
      this, &TestObserver::transactionUndone);
  }

  // FIXME: should probably unregister from the notifications in the destructor

  /**
   * Returns the list of notifications that have been produced by the CommandProcessor
   * since the last call to popNotifications().
   */
  std::vector<NotificationTuple> popNotifications()
  {
    auto result = std::vector<NotificationTuple>{};

    using std::swap;
    swap(m_notifications, result);

    return result;
  }

private:
  // these would be tidier as lambdas in the TestObserver() constructor
  void commandDo(Command& command)
  {
    m_notifications.emplace_back(CommandNotif::CommandDo, command.name());
  }
  void commandDone(Command& command)
  {
    m_notifications.emplace_back(CommandNotif::CommandDone, command.name());
  }
  void commandDoFailed(Command& command)
  {
    m_notifications.emplace_back(CommandNotif::CommandDoFailed, command.name());
  }
  void commandUndo(UndoableCommand& command)
  {
    m_notifications.emplace_back(CommandNotif::CommandUndo, command.name());
  }
  void commandUndone(UndoableCommand& command)
  {
    m_notifications.emplace_back(CommandNotif::CommandUndone, command.name());
  }
  void commandUndoFailed(UndoableCommand& command)
  {
    m_notifications.emplace_back(CommandNotif::CommandUndoFailed, command.name());
  }
  void transactionDone(const std::string& transactionName)
  {
    m_notifications.emplace_back(CommandNotif::TransactionDone, transactionName);
  }
  void transactionUndone(const std::string& transactionName)
  {
    m_notifications.emplace_back(CommandNotif::TransactionUndone, transactionName);
  }
};

struct DoPerformDo
{
  bool returnSuccess;
};
struct DoPerformUndo
{
  bool returnSuccess;
};
struct DoCollateWith
{
  bool returnCanCollate;
  UndoableCommand* expectedOtherCommand;
};

using TestCommandCall = std::variant<DoPerformDo, DoPerformUndo, DoCollateWith>;

class TestCommand : public UndoableCommand
{
private:
  mutable std::vector<TestCommandCall> m_expectedCalls;

public:
  explicit TestCommand(const std::string& name)
    : UndoableCommand(name, false)
  {
  }

  ~TestCommand() { CHECK(m_expectedCalls.empty()); }

private:
  template <class T>
  T popCall() const
  {
    CHECK_FALSE(m_expectedCalls.empty());
    auto variant = kdl::vec_pop_front(m_expectedCalls);
    auto call = std::get<T>(std::move(variant));
    return call;
  }

  std::unique_ptr<CommandResult> doPerformDo(MapDocumentCommandFacade*) override
  {
    const auto expectedCall = popCall<DoPerformDo>();
    return std::make_unique<CommandResult>(expectedCall.returnSuccess);
  }

  std::unique_ptr<CommandResult> doPerformUndo(MapDocumentCommandFacade*) override
  {
    const auto expectedCall = popCall<DoPerformUndo>();
    return std::make_unique<CommandResult>(expectedCall.returnSuccess);
  }

  bool doCollateWith(UndoableCommand& otherCommand) override
  {
    const auto expectedCall = popCall<DoCollateWith>();

    REQUIRE(
      (expectedCall.expectedOtherCommand == nullptr
       || &otherCommand == expectedCall.expectedOtherCommand));

    return expectedCall.returnCanCollate;
  }

public:
  /**
   * Sets an expectation that doPerformDo() should be called.
   * When called, it will return the given `returnSuccess` value.
   */
  void expectDo(const bool returnSuccess)
  {
    m_expectedCalls.push_back(DoPerformDo{returnSuccess});
  }

  /**
   * Sets an expectation that doPerformUndo() should be called.
   * When called, it will return the given `returnSuccess` value.
   */
  void expectUndo(const bool returnSuccess)
  {
    m_expectedCalls.push_back(DoPerformUndo{returnSuccess});
  }

  /**
   * Sets an expectation that doCollateWith() should be called with the given
   * expectedOtherCommand. When called, doCollateWith() will return `returnCanCollate`.
   */
  void expectCollate(UndoableCommand* expectedOtherCommand, const bool returnCanCollate)
  {
    m_expectedCalls.push_back(DoCollateWith{returnCanCollate, expectedOtherCommand});
  }

  deleteCopyAndMove(TestCommand);
};

class NullCommand : public UndoableCommand
{
public:
  explicit NullCommand(std::string name)
    : UndoableCommand{std::move(name), true}
  {
  }

  std::unique_ptr<CommandResult> doPerformDo(MapDocumentCommandFacade*) override
  {
    return std::make_unique<CommandResult>(true);
  }

  std::unique_ptr<CommandResult> doPerformUndo(MapDocumentCommandFacade*) override
  {
    return std::make_unique<CommandResult>(true);
  }
};

TEST_CASE("CommandProcessorTest.doAndUndoSuccessfulCommand")
{
  /*
   * Execute a successful command, then undo it successfully.
   */

  auto commandProcessor = CommandProcessor{nullptr};
  auto observer = TestObserver{commandProcessor};

  const auto commandName = "test command";
  auto command = std::make_unique<TestCommand>(commandName);

  command->expectDo(true);
  command->expectUndo(true);

  const auto doResult = commandProcessor.executeAndStore(std::move(command));
  CHECK(doResult->success());
  CHECK(commandProcessor.canUndo());
  CHECK_FALSE(commandProcessor.canRedo());
  REQUIRE(commandProcessor.undoCommandName() == commandName);

  CHECK_THAT(
    observer.popNotifications(),
    Catch::Equals(std::vector<NotificationTuple>{
      {CommandNotif::CommandDo, commandName},
      {CommandNotif::CommandDone, commandName},
      {CommandNotif::TransactionDone, commandName},
    }));

  const auto undoResult = commandProcessor.undo();
  CHECK(undoResult->success());
  CHECK_FALSE(commandProcessor.canUndo());
  CHECK(commandProcessor.canRedo());

  REQUIRE(commandProcessor.redoCommandName() == commandName);

  CHECK_THAT(
    observer.popNotifications(),
    Catch::Equals(std::vector<NotificationTuple>{
      {CommandNotif::CommandUndo, commandName},
      {CommandNotif::CommandUndone, commandName},
      {CommandNotif::TransactionUndone, commandName},
    }));
}

TEST_CASE("CommandProcessorTest.doSuccessfulCommandAndFailAtUndo")
{
  /*
   * Execute a successful command, then undo fails.
   */

  auto commandProcessor = CommandProcessor{nullptr};
  auto observer = TestObserver{commandProcessor};

  const auto commandName = "test command";
  auto command = std::make_unique<TestCommand>(commandName);
  command->expectDo(true);
  command->expectUndo(false);

  const auto doResult = commandProcessor.executeAndStore(std::move(command));
  CHECK(doResult->success());
  CHECK(commandProcessor.canUndo());
  CHECK_FALSE(commandProcessor.canRedo());
  REQUIRE(commandProcessor.undoCommandName() == commandName);

  CHECK_THAT(
    observer.popNotifications(),
    Catch::Equals(std::vector<NotificationTuple>{
      {CommandNotif::CommandDo, commandName},
      {CommandNotif::CommandDone, commandName},
      {CommandNotif::TransactionDone, commandName},
    }));

  const auto undoResult = commandProcessor.undo();
  CHECK_FALSE(undoResult->success());
  CHECK_FALSE(commandProcessor.canUndo());
  CHECK_FALSE(commandProcessor.canRedo());

  CHECK_THAT(
    observer.popNotifications(),
    Catch::Equals(std::vector<NotificationTuple>{
      {CommandNotif::CommandUndo, commandName},
      {CommandNotif::CommandUndoFailed, commandName},
    }));
}

TEST_CASE("CommandProcessorTest.doFailingCommand")
{
  /*
   * Execute a failing command.
   */

  auto commandProcessor = CommandProcessor{nullptr};
  auto observer = TestObserver{commandProcessor};

  const auto commandName = "test command";
  auto command = std::make_unique<TestCommand>(commandName);
  command->expectDo(false);

  const auto doResult = commandProcessor.executeAndStore(std::move(command));
  CHECK_FALSE(doResult->success());

  CHECK_FALSE(commandProcessor.canUndo());
  CHECK_FALSE(commandProcessor.canRedo());

  CHECK_THAT(
    observer.popNotifications(),
    Catch::Equals(std::vector<NotificationTuple>{
      {CommandNotif::CommandDo, commandName},
      {CommandNotif::CommandDoFailed, commandName},
    }));
}

TEST_CASE("CommandProcessorTest.commitUndoRedoTransaction")
{
  /*
   * Execute two successful commands in a transaction, then undo the transaction
   * successfully. Finally, redo it, also with success.
   */

  auto commandProcessor = CommandProcessor{nullptr};
  auto observer = TestObserver{commandProcessor};

  const auto commandName1 = "test command 1";
  auto command1 = std::make_unique<TestCommand>(commandName1);

  const auto commandName2 = "test command 2";
  auto command2 = std::make_unique<TestCommand>(commandName2);

  command1->expectDo(true);
  command2->expectDo(true);
  command1->expectCollate(command2.get(), false);

  const auto transactionName = "transaction";

  // undo transaction
  command2->expectUndo(true);
  command1->expectUndo(true);

  // redo
  command1->expectDo(true);
  command2->expectDo(true);

  commandProcessor.startTransaction(transactionName, TransactionScope::Oneshot);
  CHECK(commandProcessor.executeAndStore(std::move(command1))->success());
  CHECK(commandProcessor.executeAndStore(std::move(command2))->success());
  commandProcessor.commitTransaction();

  CHECK_THAT(
    observer.popNotifications(),
    Catch::Equals(std::vector<NotificationTuple>{
      {CommandNotif::CommandDo, commandName1},
      {CommandNotif::CommandDone, commandName1},
      {CommandNotif::CommandDo, commandName2},
      {CommandNotif::CommandDone, commandName2},
      {CommandNotif::TransactionDone, transactionName},
    }));

  CHECK(commandProcessor.canUndo());
  CHECK_FALSE(commandProcessor.canRedo());
  REQUIRE(commandProcessor.undoCommandName() == transactionName);

  CHECK(commandProcessor.undo()->success());

  CHECK_FALSE(commandProcessor.canUndo());
  CHECK(commandProcessor.canRedo());
  REQUIRE(commandProcessor.redoCommandName() == transactionName);

  CHECK_THAT(
    observer.popNotifications(),
    Catch::Equals(std::vector<NotificationTuple>{
      {CommandNotif::CommandUndo, commandName2},
      {CommandNotif::CommandUndone, commandName2},
      {CommandNotif::CommandUndo, commandName1},
      {CommandNotif::CommandUndone, commandName1},
      {CommandNotif::TransactionUndone, transactionName},
    }));

  CHECK(commandProcessor.redo()->success());

  CHECK(commandProcessor.canUndo());
  CHECK_FALSE(commandProcessor.canRedo());
  REQUIRE(commandProcessor.undoCommandName() == transactionName);

  CHECK_THAT(
    observer.popNotifications(),
    Catch::Equals(std::vector<NotificationTuple>{
      {CommandNotif::CommandDo, commandName1},
      {CommandNotif::CommandDone, commandName1},
      {CommandNotif::CommandDo, commandName2},
      {CommandNotif::CommandDone, commandName2},
      {CommandNotif::TransactionDone, transactionName},
    }));
}

TEST_CASE("CommandProcessorTest.rollbackTransaction")
{
  /*
   * Execute two successful commands in a transaction, then rollback the transaction and
   * commit it.
   */

  auto commandProcessor = CommandProcessor{nullptr};
  auto observer = TestObserver{commandProcessor};

  const auto commandName1 = "test command 1";
  auto command1 = std::make_unique<TestCommand>(commandName1);

  const auto commandName2 = "test command 2";
  auto command2 = std::make_unique<TestCommand>(commandName2);

  command1->expectDo(true);
  command2->expectDo(true);
  command1->expectCollate(command2.get(), false);

  // rollback
  command2->expectUndo(true);
  command1->expectUndo(true);

  const auto transactionName = "transaction";
  commandProcessor.startTransaction(transactionName, TransactionScope::Oneshot);
  CHECK(commandProcessor.executeAndStore(std::move(command1))->success());
  CHECK_THAT(
    observer.popNotifications(),
    Catch::Equals(std::vector<NotificationTuple>{
      {CommandNotif::CommandDo, commandName1},
      {CommandNotif::CommandDone, commandName1},
    }));

  CHECK(commandProcessor.executeAndStore(std::move(command2))->success());
  CHECK_THAT(
    observer.popNotifications(),
    Catch::Equals(std::vector<NotificationTuple>{
      {CommandNotif::CommandDo, commandName2},
      {CommandNotif::CommandDone, commandName2},
    }));

  commandProcessor.rollbackTransaction();
  CHECK_THAT(
    observer.popNotifications(),
    Catch::Equals(std::vector<NotificationTuple>{
      {CommandNotif::CommandUndo, commandName2},
      {CommandNotif::CommandUndone, commandName2},
      {CommandNotif::CommandUndo, commandName1},
      {CommandNotif::CommandUndone, commandName1},
    }));

  CHECK_FALSE(commandProcessor.canUndo());
  CHECK_FALSE(commandProcessor.canRedo());

  // does nothing, but closes the transaction
  commandProcessor.commitTransaction();

  CHECK_FALSE(commandProcessor.canUndo());
  CHECK_FALSE(commandProcessor.canRedo());

  REQUIRE(observer.popNotifications().empty());
}

TEST_CASE("CommandProcessorTest.nestedTransactions")
{
  /*
   * Execute a command in a transaction, start a nested transaction, execute a command,
   * and commit both transactions. Then undo the outer transaction.
   */

  auto commandProcessor = CommandProcessor{nullptr};
  auto observer = TestObserver{commandProcessor};

  const auto outerCommandName = "outer command";
  auto outerCommand = std::make_unique<TestCommand>(outerCommandName);

  const auto innerCommandName = "inner command";
  auto innerCommand = std::make_unique<TestCommand>(innerCommandName);

  outerCommand->expectDo(true);
  innerCommand->expectDo(true);

  outerCommand->expectCollate(nullptr, false);

  const auto innerTransactionName = "inner transaction";
  const auto outerTransactionName = "outer transaction";

  // undo transaction
  innerCommand->expectUndo(true);
  outerCommand->expectUndo(true);

  commandProcessor.startTransaction(outerTransactionName, TransactionScope::Oneshot);
  CHECK(commandProcessor.executeAndStore(std::move(outerCommand))->success());
  CHECK_THAT(
    observer.popNotifications(),
    Catch::Equals(std::vector<NotificationTuple>{
      {CommandNotif::CommandDo, outerCommandName},
      {CommandNotif::CommandDone, outerCommandName},
    }));

  commandProcessor.startTransaction(innerTransactionName, TransactionScope::Oneshot);
  CHECK(commandProcessor.executeAndStore(std::move(innerCommand))->success());
  CHECK_THAT(
    observer.popNotifications(),
    Catch::Equals(std::vector<NotificationTuple>{
      {CommandNotif::CommandDo, innerCommandName},
      {CommandNotif::CommandDone, innerCommandName},
    }));

  commandProcessor.commitTransaction();
  CHECK_THAT(
    observer.popNotifications(),
    Catch::Equals(std::vector<NotificationTuple>{
      {CommandNotif::TransactionDone, innerTransactionName},
    }));

  commandProcessor.commitTransaction();
  CHECK_THAT(
    observer.popNotifications(),
    Catch::Equals(std::vector<NotificationTuple>{
      {CommandNotif::TransactionDone, outerTransactionName},
    }));

  CHECK(commandProcessor.canUndo());
  CHECK_FALSE(commandProcessor.canRedo());
  REQUIRE(commandProcessor.undoCommandName() == outerTransactionName);

  CHECK(commandProcessor.undo()->success());

  CHECK_FALSE(commandProcessor.canUndo());
  CHECK(commandProcessor.canRedo());
  REQUIRE(commandProcessor.redoCommandName() == outerTransactionName);

  CHECK_THAT(
    observer.popNotifications(),
    Catch::Equals(std::vector<NotificationTuple>{
      {CommandNotif::CommandUndo, innerCommandName},
      {CommandNotif::CommandUndone, innerCommandName},
      {CommandNotif::CommandUndo, outerCommandName},
      {CommandNotif::CommandUndone, outerCommandName},
      {CommandNotif::TransactionUndone, outerTransactionName},
    }));
}

TEST_CASE("CommandProceossor.isCurrentDocumentStateObservable")
{
  auto commandProcessor = CommandProcessor{nullptr};

  SECTION("No enclosing transaction")
  {
    CHECK(commandProcessor.isCurrentDocumentStateObservable());

    commandProcessor.executeAndStore(std::make_unique<NullCommand>("command"));
    CHECK(commandProcessor.isCurrentDocumentStateObservable());
  }

  SECTION("One enclosing one shot transaction")
  {
    commandProcessor.startTransaction("", TransactionScope::Oneshot);
    CHECK(commandProcessor.isCurrentDocumentStateObservable());

    commandProcessor.executeAndStore(std::make_unique<NullCommand>("command"));
    CHECK(commandProcessor.isCurrentDocumentStateObservable());

    commandProcessor.commitTransaction();
    CHECK(commandProcessor.isCurrentDocumentStateObservable());
  }

  SECTION("One enclosing long running transaction")
  {
    commandProcessor.startTransaction("", TransactionScope::LongRunning);
    CHECK(commandProcessor.isCurrentDocumentStateObservable());

    commandProcessor.executeAndStore(std::make_unique<NullCommand>("command"));
    CHECK(commandProcessor.isCurrentDocumentStateObservable());

    commandProcessor.commitTransaction();
    CHECK(commandProcessor.isCurrentDocumentStateObservable());
  }

  SECTION("Nested one shot transactions")
  {
    commandProcessor.startTransaction("outer", TransactionScope::Oneshot);
    CHECK(commandProcessor.isCurrentDocumentStateObservable());

    commandProcessor.startTransaction("inner", TransactionScope::Oneshot);
    CHECK_FALSE(commandProcessor.isCurrentDocumentStateObservable());

    commandProcessor.executeAndStore(std::make_unique<NullCommand>("command"));
    CHECK_FALSE(commandProcessor.isCurrentDocumentStateObservable());

    commandProcessor.commitTransaction();
    CHECK(commandProcessor.isCurrentDocumentStateObservable());

    commandProcessor.commitTransaction();
    CHECK(commandProcessor.isCurrentDocumentStateObservable());
  }

  SECTION("Enclosing long running transaction with nested one shot transactions")
  {
    commandProcessor.startTransaction("long running", TransactionScope::LongRunning);
    CHECK(commandProcessor.isCurrentDocumentStateObservable());

    commandProcessor.startTransaction("outer", TransactionScope::Oneshot);
    CHECK(commandProcessor.isCurrentDocumentStateObservable());

    commandProcessor.startTransaction("inner", TransactionScope::Oneshot);
    CHECK_FALSE(commandProcessor.isCurrentDocumentStateObservable());

    commandProcessor.executeAndStore(std::make_unique<NullCommand>("command"));
    CHECK_FALSE(commandProcessor.isCurrentDocumentStateObservable());

    commandProcessor.commitTransaction();
    CHECK(commandProcessor.isCurrentDocumentStateObservable());

    commandProcessor.commitTransaction();
    CHECK(commandProcessor.isCurrentDocumentStateObservable());

    commandProcessor.commitTransaction();
    CHECK(commandProcessor.isCurrentDocumentStateObservable());
  }
}

TEST_CASE("CommandProcessorTest.collateCommands")
{
  /*
   * Execute a command and collate the next command, then undo.
   */

  auto commandProcessor = CommandProcessor{nullptr};
  auto observer = TestObserver{commandProcessor};

  const auto commandName1 = "test command 1";
  auto command1 = std::make_unique<TestCommand>(commandName1);

  const auto commandName2 = "test command 2";
  auto command2 = std::make_unique<TestCommand>(commandName2);

  command1->expectDo(true);
  command2->expectDo(true);
  command1->expectCollate(command2.get(), true);
  command1->expectUndo(true);

  commandProcessor.executeAndStore(std::move(command1));
  CHECK_THAT(
    observer.popNotifications(),
    Catch::Equals(std::vector<NotificationTuple>{
      {CommandNotif::CommandDo, commandName1},
      {CommandNotif::CommandDone, commandName1},
      {CommandNotif::TransactionDone, commandName1},
    }));

  commandProcessor.executeAndStore(std::move(command2));
  CHECK_THAT(
    observer.popNotifications(),
    Catch::Equals(std::vector<NotificationTuple>{
      {CommandNotif::CommandDo, commandName2},
      {CommandNotif::CommandDone, commandName2},
      {CommandNotif::TransactionDone, commandName2},
    }));

  CHECK(commandProcessor.canUndo());
  CHECK_FALSE(commandProcessor.canRedo());
  REQUIRE(commandProcessor.undoCommandName() == commandName1);

  CHECK(commandProcessor.undo()->success());

  CHECK_FALSE(commandProcessor.canUndo());
  CHECK(commandProcessor.canRedo());
  REQUIRE(commandProcessor.redoCommandName() == commandName1);

  // NOTE: commandName2 is gone because it was coalesced into commandName1
  CHECK_THAT(
    observer.popNotifications(),
    Catch::Equals(std::vector<NotificationTuple>{
      {CommandNotif::CommandUndo, commandName1},
      {CommandNotif::CommandUndone, commandName1},
      {CommandNotif::TransactionUndone, commandName1},
    }));
}

TEST_CASE("CommandProcessorTest.collationInterval")
{
  /*
   * Execute two commands, with time passing between their execution exceeding the
   * collation interval. Then, undo the second command.
   */

  auto commandProcessor = CommandProcessor{nullptr, std::chrono::milliseconds(100)};
  auto observer = TestObserver{commandProcessor};

  const auto commandName1 = "test command 1";
  auto command1 = std::make_unique<TestCommand>(commandName1);

  const auto commandName2 = "test command 2";
  auto command2 = std::make_unique<TestCommand>(commandName2);

  command1->expectDo(true);
  command2->expectDo(true);
  command2->expectUndo(true);

  commandProcessor.executeAndStore(std::move(command1));

  CHECK_THAT(
    observer.popNotifications(),
    Catch::Equals(std::vector<NotificationTuple>{
      {CommandNotif::CommandDo, commandName1},
      {CommandNotif::CommandDone, commandName1},
      {CommandNotif::TransactionDone, commandName1},
    }));

  using namespace std::chrono_literals;
  std::this_thread::sleep_for(100ms);

  commandProcessor.executeAndStore(std::move(command2));

  CHECK_THAT(
    observer.popNotifications(),
    Catch::Equals(std::vector<NotificationTuple>{
      {CommandNotif::CommandDo, commandName2},
      {CommandNotif::CommandDone, commandName2},
      {CommandNotif::TransactionDone, commandName2},
    }));

  CHECK(commandProcessor.canUndo());
  CHECK_FALSE(commandProcessor.canRedo());
  REQUIRE(commandProcessor.undoCommandName() == commandName2);

  CHECK(commandProcessor.undo()->success());

  CHECK_THAT(
    observer.popNotifications(),
    Catch::Equals(std::vector<NotificationTuple>{
      {CommandNotif::CommandUndo, commandName2},
      {CommandNotif::CommandUndone, commandName2},
      {CommandNotif::TransactionUndone, commandName2},
    }));

  CHECK(commandProcessor.canUndo());
  CHECK(commandProcessor.canRedo());
  REQUIRE(commandProcessor.undoCommandName() == commandName1);
  REQUIRE(commandProcessor.redoCommandName() == commandName2);
}

TEST_CASE("CommandProcessorTest.collateTransactions")
{
  auto commandProcessor = CommandProcessor{nullptr};
  auto observer = TestObserver{commandProcessor};

  auto transaction1_command1 = std::make_unique<TestCommand>("cmd1");
  auto transaction1_command2 = std::make_unique<TestCommand>("cmd2");
  auto transaction2_command1 = std::make_unique<TestCommand>("cmd1");
  auto transaction2_command2 = std::make_unique<TestCommand>("cmd2");

  transaction1_command1->expectDo(true);
  transaction1_command2->expectDo(true);
  transaction1_command1->expectCollate(transaction1_command2.get(), false);

  transaction2_command1->expectDo(true);
  transaction2_command2->expectDo(true);
  transaction2_command1->expectCollate(transaction2_command2.get(), false);

  transaction1_command2->expectCollate(transaction2_command1.get(), true);

  transaction1_command1->expectUndo(true);
  transaction1_command2->expectUndo(true);
  transaction2_command2->expectUndo(true);

  commandProcessor.startTransaction("transaction 1", TransactionScope::Oneshot);
  commandProcessor.executeAndStore(std::move(transaction1_command1));
  commandProcessor.executeAndStore(std::move(transaction1_command2));
  commandProcessor.commitTransaction();

  commandProcessor.startTransaction("transaction 2", TransactionScope::Oneshot);
  commandProcessor.executeAndStore(std::move(transaction2_command1));
  commandProcessor.executeAndStore(std::move(transaction2_command2));
  commandProcessor.commitTransaction();

  commandProcessor.undo();
}
} // namespace View
} // namespace TrenchBroom
