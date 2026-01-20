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
#include "mdl/CatchConfig.h"
#include "mdl/CommandProcessor.h"
#include "mdl/Map.h"
#include "mdl/MapFixture.h"
#include "mdl/TransactionScope.h"
#include "mdl/UndoableCommand.h"

#include "kd/k.h"
#include "kd/reflection_impl.h"
#include "kd/vector_utils.h"

#include <memory>
#include <ostream>
#include <thread>
#include <variant>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

namespace tb::mdl
{
using namespace Catch::Matchers;

namespace
{

struct CommandNotification
{
  enum class Type
  {
    Do,
    Done,
    DoFailed,
    Undo,
    Undone,
    UndoFailed,
  };

  Type type;
  const Command* command;

  [[maybe_unused]] friend std::ostream& operator<<(std::ostream& lhs, const Type rhs)
  {
    switch (rhs)
    {
    case Type::Do:
      lhs << "Do";
      break;
    case Type::Done:
      lhs << "Done";
      break;
    case Type::DoFailed:
      lhs << "DoFailed";
      break;
    case Type::Undo:
      lhs << "Undo";
      break;
    case Type::Undone:
      lhs << "Undone";
      break;
    case Type::UndoFailed:
      lhs << "UndoFailed";
      break;
    }
    return lhs;
  }

  kdl_reflect_inline(CommandNotification, type, command);
};

struct TransactionNotification
{

  enum class Type
  {
    Done,
    Undone,
  };

  Type type;
  std::string name;
  bool isObservable;
  bool isModification;

  [[maybe_unused]] friend std::ostream& operator<<(std::ostream& lhs, const Type rhs)
  {
    switch (rhs)
    {
    case Type::Done:
      lhs << "Done";
      break;
    case Type::Undone:
      lhs << "Undone";
      break;
    }
    return lhs;
  }

  kdl_reflect_inline(TransactionNotification, type, name, isObservable, isModification);
};

using Notification = std::variant<CommandNotification, TransactionNotification>;

[[maybe_unused]] std::ostream& operator<<(std::ostream& lhs, const Notification& rhs)
{
  std::visit([&](const auto& x) { lhs << x; }, rhs);
  return lhs;
}

auto makeCommandObserver(const auto type, auto& notifications)
{
  return [type, &notifications](auto& command) {
    notifications.emplace_back(CommandNotification{type, &command});
  };
}

auto makeTransactionObserver(const auto type, auto& notifications)
{
  return [type, &notifications](
           const auto& name, const auto isObservable, const auto isModification) {
    notifications.emplace_back(
      TransactionNotification{type, name, isObservable, isModification});
  };
}

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
  explicit TestCommand(std::string name, const bool updateModificationCount)
    : UndoableCommand{std::move(name), updateModificationCount}
  {
  }

  ~TestCommand() override { CHECK(m_expectedCalls.empty()); }

private:
  template <class T>
  T popCall() const
  {
    CHECK_FALSE(m_expectedCalls.empty());
    auto variant = kdl::vec_pop_front(m_expectedCalls);
    auto call = std::get<T>(std::move(variant));
    return call;
  }

  bool doPerformDo(Map&) override
  {
    const auto expectedCall = popCall<DoPerformDo>();
    return expectedCall.returnSuccess;
  }

  bool doPerformUndo(Map&) override
  {
    const auto expectedCall = popCall<DoPerformUndo>();
    return expectedCall.returnSuccess;
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
    m_expectedCalls.emplace_back(DoPerformDo{returnSuccess});
  }

  /**
   * Sets an expectation that doPerformUndo() should be called.
   * When called, it will return the given `returnSuccess` value.
   */
  void expectUndo(const bool returnSuccess)
  {
    m_expectedCalls.emplace_back(DoPerformUndo{returnSuccess});
  }

  /**
   * Sets an expectation that doCollateWith() should be called with the given
   * expectedOtherCommand. When called, doCollateWith() will return `returnCanCollate`.
   */
  void expectCollate(UndoableCommand* expectedOtherCommand, const bool returnCanCollate)
  {
    m_expectedCalls.emplace_back(DoCollateWith{returnCanCollate, expectedOtherCommand});
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

  bool doPerformDo(Map&) override { return true; }

  bool doPerformUndo(Map&) override { return true; }
};

} // namespace

TEST_CASE("CommandProcessor")
{
  using namespace std::chrono_literals;

  using CN = CommandNotification;
  using TN = TransactionNotification;

  auto fixture = MapFixture{};
  auto& map = fixture.create();

  constexpr auto collationInterval = 100ms;
  auto commandProcessor = CommandProcessor{map, collationInterval};

  auto notifierConnection = NotifierConnection{};
  auto notifications = std::vector<Notification>{};
  const auto getNotifications = [&notifications]() {
    return std::exchange(notifications, {});
  };

  notifierConnection += commandProcessor.commandDoNotifier.connect(
    makeCommandObserver(CN::Type::Do, notifications));
  notifierConnection += commandProcessor.commandDoneNotifier.connect(
    makeCommandObserver(CN::Type::Done, notifications));
  notifierConnection += commandProcessor.commandDoFailedNotifier.connect(
    makeCommandObserver(CN::Type::DoFailed, notifications));

  notifierConnection += commandProcessor.commandUndoNotifier.connect(
    makeCommandObserver(CN::Type::Undo, notifications));
  notifierConnection += commandProcessor.commandUndoneNotifier.connect(
    makeCommandObserver(CN::Type::Undone, notifications));
  notifierConnection += commandProcessor.commandUndoFailedNotifier.connect(
    makeCommandObserver(CN::Type::UndoFailed, notifications));

  notifierConnection += commandProcessor.transactionDoneNotifier.connect(
    makeTransactionObserver(TN::Type::Done, notifications));
  notifierConnection += commandProcessor.transactionUndoneNotifier.connect(
    makeTransactionObserver(TN::Type::Undone, notifications));

  SECTION("doAndUndoSuccessfulCommand")
  {
    /*
     * Execute a successful command, then undo it successfully.
     */

    const auto commandName = "test command";
    auto command =
      std::make_unique<TestCommand>(commandName, !K(updateModificationCount));
    auto* commandPtr = command.get();

    command->expectDo(true);
    command->expectUndo(true);

    CHECK(commandProcessor.executeAndStore(std::move(command)));
    CHECK_FALSE(commandProcessor.canRedo());
    REQUIRE(commandProcessor.canUndo());
    CHECK(*commandProcessor.undoCommandName() == commandName);

    CHECK(
      getNotifications()
      == std::vector<Notification>{
        CN{CN::Type::Do, commandPtr},
        CN{CN::Type::Done, commandPtr},
        TN{TN::Type::Done, commandName, K(isObservable), !K(isModification)},
      });

    CHECK(commandProcessor.undo());
    CHECK_FALSE(commandProcessor.canUndo());
    REQUIRE(commandProcessor.canRedo());
    CHECK(*commandProcessor.redoCommandName() == commandName);

    CHECK(
      getNotifications()
      == std::vector<Notification>{
        CN{CN::Type::Undo, commandPtr},
        CN{CN::Type::Undone, commandPtr},
        TN{TN::Type::Undone, commandName, K(isObservable), !K(isModification)},
      });
  }

  SECTION("doSuccessfulCommandAndFailAtUndo")
  {
    /*
     * Execute a successful command, then undo fails.
     */

    const auto commandName = "test command";
    auto command =
      std::make_unique<TestCommand>(commandName, !K(updateModificationCount));
    auto* commandPtr = command.get();

    command->expectDo(true);
    command->expectUndo(false);

    CHECK(commandProcessor.executeAndStore(std::move(command)));
    CHECK_FALSE(commandProcessor.canRedo());
    REQUIRE(commandProcessor.canUndo());
    CHECK(*commandProcessor.undoCommandName() == commandName);

    CHECK(
      getNotifications()
      == std::vector<Notification>{
        CN{CN::Type::Do, commandPtr},
        CN{CN::Type::Done, commandPtr},
        TN{TN::Type::Done, commandName, K(isObservable), !K(isModification)},
      });

    CHECK_FALSE(commandProcessor.undo());
    CHECK_FALSE(commandProcessor.canUndo());
    CHECK_FALSE(commandProcessor.canRedo());

    CHECK(
      getNotifications()
      == std::vector<Notification>{
        CN{CN::Type::Undo, commandPtr},
        CN{CN::Type::UndoFailed, commandPtr},
      });
  }

  SECTION("doFailingCommand")
  {
    /*
     * Execute a failing command.
     */

    const auto commandName = "test command";
    auto command =
      std::make_unique<TestCommand>(commandName, !K(updateModificationCount));
    auto* commandPtr = command.get();
    command->expectDo(false);

    CHECK_FALSE(commandProcessor.executeAndStore(std::move(command)));

    CHECK_FALSE(commandProcessor.canUndo());
    CHECK_FALSE(commandProcessor.canRedo());

    CHECK(
      getNotifications()
      == std::vector<Notification>{
        CN{CN::Type::Do, commandPtr},
        CN{CN::Type::DoFailed, commandPtr},
      });
  }

  SECTION("commitUndoRedoTransaction")
  {
    /*
     * Execute two successful commands in a transaction, then undo the transaction
     * successfully. Finally, redo it, also with success.
     */

    const auto commandName1 = "test command 1";
    auto command1 =
      std::make_unique<TestCommand>(commandName1, !K(updateModificationCount));
    auto* command1Ptr = command1.get();

    const auto commandName2 = "test command 2";
    auto command2 =
      std::make_unique<TestCommand>(commandName2, !K(updateModificationCount));
    auto* command2Ptr = command2.get();

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
    CHECK(commandProcessor.executeAndStore(std::move(command1)));
    CHECK(commandProcessor.executeAndStore(std::move(command2)));
    commandProcessor.commitTransaction();

    CHECK(
      getNotifications()
      == std::vector<Notification>{
        CN{CN::Type::Do, command1Ptr},
        CN{CN::Type::Done, command1Ptr},
        CN{CN::Type::Do, command2Ptr},
        CN{CN::Type::Done, command2Ptr},
        TN{TN::Type::Done, transactionName, K(isObservable), !K(isModification)},
      });

    CHECK_FALSE(commandProcessor.canRedo());
    REQUIRE(commandProcessor.canUndo());
    CHECK(*commandProcessor.undoCommandName() == transactionName);

    CHECK(commandProcessor.undo());

    CHECK_FALSE(commandProcessor.canUndo());
    REQUIRE(commandProcessor.canRedo());
    CHECK(*commandProcessor.redoCommandName() == transactionName);

    CHECK(
      getNotifications()
      == std::vector<Notification>{
        CN{CN::Type::Undo, command2Ptr},
        CN{CN::Type::Undone, command2Ptr},
        CN{CN::Type::Undo, command1Ptr},
        CN{CN::Type::Undone, command1Ptr},
        TN{TN::Type::Undone, transactionName, K(isObservable), !K(isModification)},
      });

    CHECK(commandProcessor.redo());

    CHECK_FALSE(commandProcessor.canRedo());
    REQUIRE(commandProcessor.canUndo());
    CHECK(*commandProcessor.undoCommandName() == transactionName);

    CHECK(
      getNotifications()
      == std::vector<Notification>{
        CN{CN::Type::Do, command1Ptr},
        CN{CN::Type::Done, command1Ptr},
        CN{CN::Type::Do, command2Ptr},
        CN{CN::Type::Done, command2Ptr},
        TN{TN::Type::Done, transactionName, K(isObservable), !K(isModification)},
      });
  }

  SECTION("rollbackTransaction")
  {
    /*
     * Execute two successful commands in a transaction, then rollback the transaction and
     * commit it.
     */

    const auto commandName1 = "test command 1";
    auto command1 =
      std::make_unique<TestCommand>(commandName1, !K(updateModificationCount));
    auto* command1Ptr = command1.get();

    const auto commandName2 = "test command 2";
    auto command2 =
      std::make_unique<TestCommand>(commandName2, !K(updateModificationCount));
    auto* command2Ptr = command2.get();

    command1->expectDo(true);
    command2->expectDo(true);
    command1->expectCollate(command2.get(), false);

    // rollback
    command2->expectUndo(true);
    command1->expectUndo(true);

    const auto transactionName = "transaction";
    commandProcessor.startTransaction(transactionName, TransactionScope::Oneshot);
    CHECK(commandProcessor.executeAndStore(std::move(command1)));
    CHECK(
      getNotifications()
      == std::vector<Notification>{
        CN{CN::Type::Do, command1Ptr},
        CN{CN::Type::Done, command1Ptr},
      });


    CHECK(commandProcessor.executeAndStore(std::move(command2)));
    CHECK(
      getNotifications()
      == std::vector<Notification>{
        CN{CN::Type::Do, command2Ptr},
        CN{CN::Type::Done, command2Ptr},
      });

    commandProcessor.rollbackTransaction();
    CHECK(
      getNotifications()
      == std::vector<Notification>{
        CN{CN::Type::Undo, command2Ptr},
        CN{CN::Type::Undone, command2Ptr},
        CN{CN::Type::Undo, command1Ptr},
        CN{CN::Type::Undone, command1Ptr},
      });

    CHECK_FALSE(commandProcessor.canUndo());
    CHECK_FALSE(commandProcessor.canRedo());

    // does nothing, but closes the transaction
    commandProcessor.commitTransaction();

    CHECK_FALSE(commandProcessor.canUndo());
    CHECK_FALSE(commandProcessor.canRedo());

    CHECK(notifications.empty());
  }

  SECTION("nestedTransactions")
  {
    /*
     * Execute a command in a transaction, start a nested transaction, execute a
     * command, and commit both transactions. Then undo the outer transaction.
     */

    const auto outerCommandName = "outer command";
    auto outerCommand =
      std::make_unique<TestCommand>(outerCommandName, !K(updateModificationCount));
    auto* outerCommandPtr = outerCommand.get();

    const auto innerCommandName = "inner command";
    auto innerCommand =
      std::make_unique<TestCommand>(innerCommandName, !K(updateModificationCount));
    auto* innerCommandPtr = innerCommand.get();

    outerCommand->expectDo(true);
    innerCommand->expectDo(true);

    outerCommand->expectCollate(nullptr, false);

    const auto innerTransactionName = "inner transaction";
    const auto outerTransactionName = "outer transaction";

    // undo transaction
    innerCommand->expectUndo(true);
    outerCommand->expectUndo(true);

    commandProcessor.startTransaction(outerTransactionName, TransactionScope::Oneshot);
    CHECK(commandProcessor.executeAndStore(std::move(outerCommand)));
    CHECK(
      getNotifications()
      == std::vector<Notification>{
        CN{CN::Type::Do, outerCommandPtr},
        CN{CN::Type::Done, outerCommandPtr},
      });

    commandProcessor.startTransaction(innerTransactionName, TransactionScope::Oneshot);
    CHECK(commandProcessor.executeAndStore(std::move(innerCommand)));
    CHECK(
      getNotifications()
      == std::vector<Notification>{
        CN{CN::Type::Do, innerCommandPtr},
        CN{CN::Type::Done, innerCommandPtr},
      });

    commandProcessor.commitTransaction();
    CHECK(
      getNotifications()
      == std::vector<Notification>{
        TN{TN::Type::Done, innerTransactionName, K(isObservable), !K(isModification)},
      });

    commandProcessor.commitTransaction();
    CHECK(
      getNotifications()
      == std::vector<Notification>{
        TN{TN::Type::Done, outerTransactionName, K(isObservable), !K(isModification)},
      });

    CHECK_FALSE(commandProcessor.canRedo());
    REQUIRE(commandProcessor.canUndo());
    CHECK(*commandProcessor.undoCommandName() == outerTransactionName);

    CHECK(commandProcessor.undo());

    CHECK_FALSE(commandProcessor.canUndo());
    REQUIRE(commandProcessor.canRedo());
    CHECK(*commandProcessor.redoCommandName() == outerTransactionName);

    CHECK(
      getNotifications()
      == std::vector<Notification>{
        CN{CN::Type::Undo, innerCommandPtr},
        CN{CN::Type::Undone, innerCommandPtr},
        CN{CN::Type::Undo, outerCommandPtr},
        CN{CN::Type::Undone, outerCommandPtr},
        TN{TN::Type::Undone, outerTransactionName, K(isObservable), !K(isModification)},
      });
  }

  SECTION("isCurrentDocumentStateObservable")
  {
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

  SECTION("isModification")
  {
    using T = std::tuple<bool, bool>;

    const auto [outerIsModification, innerIsModification] = GENERATE(values<T>({
      {false, false},
      {false, true},
      {true, false},
      {true, true},
    }));

    CAPTURE(outerIsModification, innerIsModification);

    const auto outerCommandName = "outer command";
    auto outerCommand =
      std::make_unique<TestCommand>(outerCommandName, outerIsModification);
    auto* outerCommandPtr = outerCommand.get();

    const auto innerCommandName = "inner command";
    auto innerCommand =
      std::make_unique<TestCommand>(innerCommandName, innerIsModification);
    auto* innerCommandPtr = innerCommand.get();

    outerCommand->expectDo(true);
    innerCommand->expectDo(true);

    outerCommand->expectCollate(nullptr, false);

    const auto innerTransactionName = "inner transaction";
    const auto outerTransactionName = "outer transaction";

    // undo transaction
    innerCommand->expectUndo(true);
    outerCommand->expectUndo(true);

    commandProcessor.startTransaction(outerTransactionName, TransactionScope::Oneshot);
    CHECK(commandProcessor.executeAndStore(std::move(outerCommand)));
    CHECK(
      getNotifications()
      == std::vector<Notification>{
        CN{CN::Type::Do, outerCommandPtr},
        CN{CN::Type::Done, outerCommandPtr},
      });

    commandProcessor.startTransaction(innerTransactionName, TransactionScope::Oneshot);
    CHECK(commandProcessor.executeAndStore(std::move(innerCommand)));
    CHECK(
      getNotifications()
      == std::vector<Notification>{
        CN{CN::Type::Do, innerCommandPtr},
        CN{CN::Type::Done, innerCommandPtr},
      });

    commandProcessor.commitTransaction();
    CHECK(
      getNotifications()
      == std::vector<Notification>{
        TN{TN::Type::Done, innerTransactionName, K(isObservable), innerIsModification},
      });

    commandProcessor.commitTransaction();
    CHECK(
      getNotifications()
      == std::vector<Notification>{
        TN{
          TN::Type::Done,
          outerTransactionName,
          K(isObservable),
          outerIsModification || innerIsModification},
      });

    CHECK_FALSE(commandProcessor.canRedo());
    REQUIRE(commandProcessor.canUndo());
    CHECK(*commandProcessor.undoCommandName() == outerTransactionName);

    CHECK(commandProcessor.undo());

    CHECK_FALSE(commandProcessor.canUndo());
    REQUIRE(commandProcessor.canRedo());
    CHECK(*commandProcessor.redoCommandName() == outerTransactionName);

    CHECK(
      getNotifications()
      == std::vector<Notification>{
        CN{CN::Type::Undo, innerCommandPtr},
        CN{CN::Type::Undone, innerCommandPtr},
        CN{CN::Type::Undo, outerCommandPtr},
        CN{CN::Type::Undone, outerCommandPtr},
        TN{
          TN::Type::Undone,
          outerTransactionName,
          K(isObservable),
          outerIsModification || innerIsModification},
      });
  }

  SECTION("collateCommands")
  {
    /*
     * Execute a command and collate the next command, then undo.
     */

    const auto commandName1 = "test command 1";
    auto command1 =
      std::make_unique<TestCommand>(commandName1, !K(updateModificationCount));
    auto* command1Ptr = command1.get();

    const auto commandName2 = "test command 2";
    auto command2 =
      std::make_unique<TestCommand>(commandName2, !K(updateModificationCount));
    auto* command2Ptr = command2.get();

    command1->expectDo(true);
    command2->expectDo(true);
    command1->expectCollate(command2.get(), true);
    command1->expectUndo(true);

    commandProcessor.executeAndStore(std::move(command1));
    CHECK(
      getNotifications()
      == std::vector<Notification>{
        CN{CN::Type::Do, command1Ptr},
        CN{CN::Type::Done, command1Ptr},
        TN{TN::Type::Done, commandName1, K(isObservable), !K(isModification)},
      });

    commandProcessor.executeAndStore(std::move(command2));
    CHECK(
      getNotifications()
      == std::vector<Notification>{
        CN{CN::Type::Do, command2Ptr},
        CN{CN::Type::Done, command2Ptr},
        TN{TN::Type::Done, commandName2, K(isObservable), !K(isModification)},
      });

    CHECK_FALSE(commandProcessor.canRedo());
    REQUIRE(commandProcessor.canUndo());
    CHECK(*commandProcessor.undoCommandName() == commandName1);

    CHECK(commandProcessor.undo());

    CHECK_FALSE(commandProcessor.canUndo());
    REQUIRE(commandProcessor.canRedo());
    CHECK(*commandProcessor.redoCommandName() == commandName1);

    // NOTE: command2 is gone because it was coalesced into commandName1
    CHECK(
      getNotifications()
      == std::vector<Notification>{
        CN{CN::Type::Undo, command1Ptr},
        CN{CN::Type::Undone, command1Ptr},
        TN{TN::Type::Undone, commandName1, K(isObservable), !K(isModification)},
      });
  }

  SECTION("collationInterval")
  {
    /*
     * Execute two commands, with time passing between their execution exceeding the
     * collation interval. Then, undo the second command.
     */

    const auto commandName1 = "test command 1";
    auto command1 =
      std::make_unique<TestCommand>(commandName1, !K(updateModificationCount));
    auto* command1Ptr = command1.get();

    const auto commandName2 = "test command 2";
    auto command2 =
      std::make_unique<TestCommand>(commandName2, !K(updateModificationCount));
    auto* command2Ptr = command2.get();

    command1->expectDo(true);
    command2->expectDo(true);
    command2->expectUndo(true);

    commandProcessor.executeAndStore(std::move(command1));

    CHECK(
      getNotifications()
      == std::vector<Notification>{
        CN{CN::Type::Do, command1Ptr},
        CN{CN::Type::Done, command1Ptr},
        TN{TN::Type::Done, commandName1, K(isObservable), !K(isModification)},
      });

    std::this_thread::sleep_for(collationInterval);

    commandProcessor.executeAndStore(std::move(command2));

    CHECK(
      getNotifications()
      == std::vector<Notification>{
        CN{CN::Type::Do, command2Ptr},
        CN{CN::Type::Done, command2Ptr},
        TN{TN::Type::Done, commandName2, K(isObservable), !K(isModification)},
      });

    CHECK_FALSE(commandProcessor.canRedo());
    REQUIRE(commandProcessor.canUndo());
    CHECK(*commandProcessor.undoCommandName() == commandName2);

    CHECK(commandProcessor.undo());

    CHECK(
      getNotifications()
      == std::vector<Notification>{
        CN{CN::Type::Undo, command2Ptr},
        CN{CN::Type::Undone, command2Ptr},
        TN{TN::Type::Undone, commandName2, K(isObservable), !K(isModification)},
      });

    REQUIRE(commandProcessor.canUndo());
    REQUIRE(commandProcessor.canRedo());
    CHECK(*commandProcessor.undoCommandName() == commandName1);
    CHECK(*commandProcessor.redoCommandName() == commandName2);
  }

  SECTION("collateTransactions")
  {
    auto transaction1_command1 =
      std::make_unique<TestCommand>("cmd1", !K(updateModificationCount));
    auto transaction1_command2 =
      std::make_unique<TestCommand>("cmd2", !K(updateModificationCount));
    auto transaction2_command1 =
      std::make_unique<TestCommand>("cmd1", !K(updateModificationCount));
    auto transaction2_command2 =
      std::make_unique<TestCommand>("cmd2", !K(updateModificationCount));

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
}

} // namespace tb::mdl
