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
#include "View/UndoableCommand.h"
#include "View/CommandProcessor.h"

#include <kdl/vector_utils.h>

#include <chrono>
#include <memory>
#include <thread>
#include <optional>
#include <variant>

#include "Catch2.h"
#include "GTestCompat.h"

namespace TrenchBroom {
    namespace View {
        enum class CommandNotif {
            CommandDo, CommandDone, CommandDoFailed, CommandUndo, CommandUndone, CommandUndoFailed,
            TransactionDone, TransactionUndone
        };

        using NotificationTuple = std::tuple<CommandNotif, std::string>;

        class TestObserver {
        private:
            std::vector<NotificationTuple> m_notifications;
        public:
            explicit TestObserver(CommandProcessor& commandProcessor) {                
                commandProcessor.commandDoNotifier.addObserver(this, &TestObserver::commandDo);
                commandProcessor.commandDoneNotifier.addObserver(this, &TestObserver::commandDone);
                commandProcessor.commandDoFailedNotifier.addObserver(this, &TestObserver::commandDoFailed);
                commandProcessor.commandUndoNotifier.addObserver(this, &TestObserver::commandUndo);
                commandProcessor.commandUndoneNotifier.addObserver(this, &TestObserver::commandUndone);
                commandProcessor.commandUndoFailedNotifier.addObserver(this, &TestObserver::commandUndoFailed);
                commandProcessor.transactionDoneNotifier.addObserver(this, &TestObserver::transactionDone);
                commandProcessor.transactionUndoneNotifier.addObserver(this, &TestObserver::transactionUndone);
            }

            // FIXME: should probably unregister from the notifications in the destructor

            /**
             * Returns the list of notifications that have been produced by the CommandProcessor
             * since the last call to popNotifications().
             */
            std::vector<NotificationTuple> popNotifications() {
                std::vector<NotificationTuple> result;

                using std::swap;
                swap(m_notifications, result);

                return result;
            }
        private:
            // these would be tidier as lambdas in the TestObserver() constructor
            void commandDo(Command* command) {
                m_notifications.push_back(std::make_tuple(CommandNotif::CommandDo, command->name()));
            }
            void commandDone(Command* command) {
                m_notifications.push_back(std::make_tuple(CommandNotif::CommandDone, command->name()));
            }
            void commandDoFailed(Command* command) {
                m_notifications.push_back(std::make_tuple(CommandNotif::CommandDoFailed, command->name()));
            }
            void commandUndo(UndoableCommand* command) {
                m_notifications.push_back(std::make_tuple(CommandNotif::CommandUndo, command->name()));
            }
            void commandUndone(UndoableCommand* command) {
                m_notifications.push_back(std::make_tuple(CommandNotif::CommandUndone, command->name()));
            }
            void commandUndoFailed(UndoableCommand* command) {
                m_notifications.push_back(std::make_tuple(CommandNotif::CommandUndoFailed, command->name()));
            }
            void transactionDone(const std::string& transactionName) {
                m_notifications.push_back(std::make_tuple(CommandNotif::TransactionDone, transactionName));
            }
            void transactionUndone(const std::string& transactionName) {
                m_notifications.push_back(std::make_tuple(CommandNotif::TransactionUndone, transactionName));
            }
        };

        struct DoPerformDo   { bool returnSuccess; };
        struct DoPerformUndo { bool returnSuccess; };
        struct DoCollateWith { bool returnCanCollate; UndoableCommand* expectedOtherCommand; };

        using TestCommandCall = std::variant<DoPerformDo, DoPerformUndo, DoCollateWith>;

        class TestCommand : public UndoableCommand {
        private:
            mutable std::vector<TestCommandCall> m_expectedCalls;
        public:
            static const CommandType Type;

            static std::unique_ptr<TestCommand> create(const std::string& name) {
                return std::make_unique<TestCommand>(name);
            }

            explicit TestCommand(const std::string& name) :
            UndoableCommand(Type, name) {}

            ~TestCommand() {
                ASSERT_TRUE(m_expectedCalls.empty());
            }
        private:
            template <class T>
            T popCall() const {
                ASSERT_FALSE(m_expectedCalls.empty());
                TestCommandCall variant = kdl::vec_pop_front(m_expectedCalls);
                T call = std::get<T>(std::move(variant));
                return call;
            }

            std::unique_ptr<CommandResult> doPerformDo(MapDocumentCommandFacade*) override {
                const auto expectedCall = popCall<DoPerformDo>();
                return std::make_unique<CommandResult>(expectedCall.returnSuccess);
            }

            std::unique_ptr<CommandResult> doPerformUndo(MapDocumentCommandFacade*) override {
                const auto expectedCall = popCall<DoPerformUndo>();
                return std::make_unique<CommandResult>(expectedCall.returnSuccess);
            }

            bool doCollateWith(UndoableCommand* otherCommand) override {
                const auto expectedCall = popCall<DoCollateWith>();

                ASSERT_EQ(expectedCall.expectedOtherCommand, otherCommand);

                return expectedCall.returnCanCollate;
            }

        public:
            /**
             * Sets an expectation that doPerformDo() should be called.
             * When called, it will return the given `returnSuccess` value.
             */
            void expectDo(const bool returnSuccess) {
                m_expectedCalls.emplace_back(DoPerformDo{returnSuccess});
            }

            /**
             * Sets an expectation that doPerformUndo() should be called.
             * When called, it will return the given `returnSuccess` value.
             */
            void expectUndo(const bool returnSuccess) {
                m_expectedCalls.emplace_back(DoPerformUndo{returnSuccess});
            }

            /**
             * Sets an expectation that doCollateWith() should be called with the given expectedOtherCommand.
             * When called, doCollateWith() will return `returnCanCollate`.
             */
            void expectCollate(UndoableCommand* expectedOtherCommand, const bool returnCanCollate) {
                m_expectedCalls.emplace_back(DoCollateWith{returnCanCollate, expectedOtherCommand});
            }

            deleteCopyAndMove(TestCommand)
        };

        const Command::CommandType TestCommand::Type = Command::freeType();

        TEST_CASE("CommandProcessorTest.doAndUndoSuccessfulCommand", "[CommandProcessorTest]") {
            /*
             * Execute a successful command, then undo it successfully.
             */

            CommandProcessor commandProcessor(nullptr);
            TestObserver observer(commandProcessor);

            const auto commandName = "test command";
            auto command = TestCommand::create(commandName);

            command->expectDo(true);
            command->expectUndo(true);
            
            const auto doResult = commandProcessor.executeAndStore(std::move(command));
            ASSERT_TRUE(doResult->success());
            ASSERT_TRUE(commandProcessor.canUndo());
            ASSERT_FALSE(commandProcessor.canRedo());
            ASSERT_EQ(commandName, commandProcessor.undoCommandName());

            ASSERT_EQ((std::vector<NotificationTuple>{
                    {CommandNotif::CommandDo, commandName},
                    {CommandNotif::CommandDone, commandName},
                    {CommandNotif::TransactionDone, commandName}
                }), observer.popNotifications());

            const auto undoResult = commandProcessor.undo();
            ASSERT_TRUE(undoResult->success());
            ASSERT_FALSE(commandProcessor.canUndo());
            ASSERT_TRUE(commandProcessor.canRedo());

            ASSERT_EQ(commandName, commandProcessor.redoCommandName());

            ASSERT_EQ((std::vector<NotificationTuple>{
                    {CommandNotif::CommandUndo, commandName},
                    {CommandNotif::CommandUndone, commandName},
                    {CommandNotif::TransactionUndone, commandName}
                }), observer.popNotifications());
        }

        TEST_CASE("CommandProcessorTest.doSuccessfulCommandAndFailAtUndo", "[CommandProcessorTest]") {
            /*
             * Execute a successful command, then undo fails.
             */

            CommandProcessor commandProcessor(nullptr);
            TestObserver observer(commandProcessor);

            const auto commandName = "test command";
            auto command = TestCommand::create(commandName);
            command->expectDo(true);
            command->expectUndo(false);

            const auto doResult = commandProcessor.executeAndStore(std::move(command));
            ASSERT_TRUE(doResult->success());
            ASSERT_TRUE(commandProcessor.canUndo());
            ASSERT_FALSE(commandProcessor.canRedo());
            ASSERT_EQ(commandName, commandProcessor.undoCommandName());

            ASSERT_EQ((std::vector<NotificationTuple>{
                {CommandNotif::CommandDo, commandName},
                {CommandNotif::CommandDone, commandName},
                {CommandNotif::TransactionDone, commandName}
            }), observer.popNotifications());

            const auto undoResult = commandProcessor.undo();
            ASSERT_FALSE(undoResult->success());
            ASSERT_FALSE(commandProcessor.canUndo());
            ASSERT_FALSE(commandProcessor.canRedo());

            ASSERT_EQ((std::vector<NotificationTuple>{
                {CommandNotif::CommandUndo, commandName},
                {CommandNotif::CommandUndoFailed, commandName}
            }), observer.popNotifications());
        }

        TEST_CASE("CommandProcessorTest.doFailingCommand", "[CommandProcessorTest]") {
            /*
             * Execute a failing command.
             */

            CommandProcessor commandProcessor(nullptr);
            TestObserver observer(commandProcessor);

            const auto commandName = "test command";
            auto command = TestCommand::create(commandName);
            command->expectDo(false);

            const auto doResult = commandProcessor.executeAndStore(std::move(command));
            ASSERT_FALSE(doResult->success());

            ASSERT_FALSE(commandProcessor.canUndo());
            ASSERT_FALSE(commandProcessor.canRedo());

            ASSERT_EQ((std::vector<NotificationTuple>{
                {CommandNotif::CommandDo, commandName},
                {CommandNotif::CommandDoFailed, commandName}
            }), observer.popNotifications());
        }

        TEST_CASE("CommandProcessorTest.commitUndoRedoTransaction", "[CommandProcessorTest]") {
            /*
             * Execute two successful commands in a transaction, then undo the transaction successfully.
             * Finally, redo it, also with success.
             */

            CommandProcessor commandProcessor(nullptr);
            TestObserver observer(commandProcessor);

            const auto commandName1 = "test command 1";
            auto command1 = TestCommand::create(commandName1);

            const auto commandName2 = "test command 2";
            auto command2 = TestCommand::create(commandName2);

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

            commandProcessor.startTransaction(transactionName);
            ASSERT_TRUE(commandProcessor.executeAndStore(std::move(command1))->success());
            ASSERT_TRUE(commandProcessor.executeAndStore(std::move(command2))->success());
            commandProcessor.commitTransaction();

            ASSERT_EQ((std::vector<NotificationTuple>{
                {CommandNotif::CommandDo, commandName1},
                {CommandNotif::CommandDone, commandName1},
                {CommandNotif::CommandDo, commandName2},
                {CommandNotif::CommandDone, commandName2},
                {CommandNotif::TransactionDone, transactionName}
            }), observer.popNotifications());

            ASSERT_TRUE(commandProcessor.canUndo());
            ASSERT_FALSE(commandProcessor.canRedo());
            ASSERT_EQ(transactionName, commandProcessor.undoCommandName());

            ASSERT_TRUE(commandProcessor.undo()->success());

            ASSERT_FALSE(commandProcessor.canUndo());
            ASSERT_TRUE(commandProcessor.canRedo());
            ASSERT_EQ(transactionName, commandProcessor.redoCommandName());

            ASSERT_EQ((std::vector<NotificationTuple>{
                {CommandNotif::CommandUndo, commandName2},
                {CommandNotif::CommandUndone, commandName2},
                {CommandNotif::CommandUndo, commandName1},
                {CommandNotif::CommandUndone, commandName1},
                {CommandNotif::TransactionUndone, transactionName}
            }), observer.popNotifications());

            ASSERT_TRUE(commandProcessor.redo()->success());

            ASSERT_TRUE(commandProcessor.canUndo());
            ASSERT_FALSE(commandProcessor.canRedo());
            ASSERT_EQ(transactionName, commandProcessor.undoCommandName());

            ASSERT_EQ((std::vector<NotificationTuple>{
                {CommandNotif::CommandDo, commandName1},
                {CommandNotif::CommandDone, commandName1},
                {CommandNotif::CommandDo, commandName2},
                {CommandNotif::CommandDone, commandName2},
                {CommandNotif::TransactionDone, transactionName}
            }), observer.popNotifications());
        }

        TEST_CASE("CommandProcessorTest.rollbackTransaction", "[CommandProcessorTest]") {
            /*
             * Execute two successful commands in a transaction, then rollback the transaction and commit it.
             */

            CommandProcessor commandProcessor(nullptr);
            TestObserver observer(commandProcessor);

            const auto commandName1 = "test command 1";
            auto command1 = TestCommand::create(commandName1);

            const auto commandName2 = "test command 2";
            auto command2 = TestCommand::create(commandName2);

            command1->expectDo(true);
            command2->expectDo(true);
            command1->expectCollate(command2.get(), false);

            // rollback
            command2->expectUndo(true);
            command1->expectUndo(true);

            const auto transactionName = "transaction";
            commandProcessor.startTransaction(transactionName);
            ASSERT_TRUE(commandProcessor.executeAndStore(std::move(command1))->success());
            ASSERT_EQ((std::vector<NotificationTuple>{
                {CommandNotif::CommandDo, commandName1},
                {CommandNotif::CommandDone, commandName1}
            }), observer.popNotifications());

            ASSERT_TRUE(commandProcessor.executeAndStore(std::move(command2))->success());
            ASSERT_EQ((std::vector<NotificationTuple>{
                {CommandNotif::CommandDo, commandName2},
                {CommandNotif::CommandDone, commandName2}
            }), observer.popNotifications());

            commandProcessor.rollbackTransaction();
            ASSERT_EQ((std::vector<NotificationTuple>{
                {CommandNotif::CommandUndo, commandName2},
                {CommandNotif::CommandUndone, commandName2},
                {CommandNotif::CommandUndo, commandName1},
                {CommandNotif::CommandUndone, commandName1}
            }), observer.popNotifications());

            ASSERT_FALSE(commandProcessor.canUndo());
            ASSERT_FALSE(commandProcessor.canRedo());

            // does nothing, but closes the transaction
            commandProcessor.commitTransaction();

            ASSERT_FALSE(commandProcessor.canUndo());
            ASSERT_FALSE(commandProcessor.canRedo());

            ASSERT_EQ((std::vector<NotificationTuple>{}), observer.popNotifications());
        }

        TEST_CASE("CommandProcessorTest.nestedTransactions", "[CommandProcessorTest]") {
            /*
             * Execute a command in a transaction, start a nested transaction, execute a command, and
             * commit both transactions. Then undo the outer transaction.
             */

            CommandProcessor commandProcessor(nullptr);
            TestObserver observer(commandProcessor);

            const auto outerCommandName = "outer command";
            auto outerCommand = TestCommand::create(outerCommandName);

            const auto innerCommandName = "inner command";
            auto innerCommand = TestCommand::create(innerCommandName);

            outerCommand->expectDo(true);
            innerCommand->expectDo(true);

            const auto innerTransactionName = "inner transaction";
            const auto outerTransactionName = "outer transaction";

            // undo transaction
            innerCommand->expectUndo(true);
            outerCommand->expectUndo(true);

            commandProcessor.startTransaction(outerTransactionName);
            ASSERT_TRUE(commandProcessor.executeAndStore(std::move(outerCommand))->success());
            ASSERT_EQ((std::vector<NotificationTuple>{
                {CommandNotif::CommandDo, outerCommandName},
                {CommandNotif::CommandDone, outerCommandName}
            }), observer.popNotifications());


            commandProcessor.startTransaction(innerTransactionName);
            ASSERT_TRUE(commandProcessor.executeAndStore(std::move(innerCommand))->success());
            ASSERT_EQ((std::vector<NotificationTuple>{
                {CommandNotif::CommandDo, innerCommandName},
                {CommandNotif::CommandDone, innerCommandName}
            }), observer.popNotifications());

            commandProcessor.commitTransaction();
            ASSERT_EQ((std::vector<NotificationTuple>{
                {CommandNotif::TransactionDone, innerTransactionName}
            }), observer.popNotifications());

            commandProcessor.commitTransaction();
            ASSERT_EQ((std::vector<NotificationTuple>{
                {CommandNotif::TransactionDone, outerTransactionName}
            }), observer.popNotifications());

            ASSERT_TRUE(commandProcessor.canUndo());
            ASSERT_FALSE(commandProcessor.canRedo());
            ASSERT_EQ(outerTransactionName, commandProcessor.undoCommandName());

            ASSERT_TRUE(commandProcessor.undo()->success());

            ASSERT_FALSE(commandProcessor.canUndo());
            ASSERT_TRUE(commandProcessor.canRedo());
            ASSERT_EQ(outerTransactionName, commandProcessor.redoCommandName());

            ASSERT_EQ((std::vector<NotificationTuple>{
                {CommandNotif::CommandUndo, innerCommandName},
                {CommandNotif::CommandUndone, innerCommandName},
                {CommandNotif::CommandUndo, outerCommandName},
                {CommandNotif::CommandUndone, outerCommandName},
                {CommandNotif::TransactionUndone, outerTransactionName}
            }), observer.popNotifications());
        }

        TEST_CASE("CommandProcessorTest.collateCommands", "[CommandProcessorTest]") {
            /*
             * Execute a command and collate the next command, then undo.
             */

            CommandProcessor commandProcessor(nullptr);
            TestObserver observer(commandProcessor);

            const auto commandName1 = "test command 1";
            auto command1 = TestCommand::create(commandName1);

            const auto commandName2 = "test command 2";
            auto command2 = TestCommand::create(commandName2);

            command1->expectDo(true);
            command2->expectDo(true);
            command1->expectCollate(command2.get(), true);
            command1->expectUndo(true);

            commandProcessor.executeAndStore(std::move(command1));
            ASSERT_EQ((std::vector<NotificationTuple>{
                {CommandNotif::CommandDo, commandName1},
                {CommandNotif::CommandDone, commandName1},
                {CommandNotif::TransactionDone, commandName1}
            }), observer.popNotifications());

            commandProcessor.executeAndStore(std::move(command2));
            ASSERT_EQ((std::vector<NotificationTuple>{
                {CommandNotif::CommandDo, commandName2},
                {CommandNotif::CommandDone, commandName2},
                {CommandNotif::TransactionDone, commandName2}
            }), observer.popNotifications());

            ASSERT_TRUE(commandProcessor.canUndo());
            ASSERT_FALSE(commandProcessor.canRedo());
            ASSERT_EQ(commandName1, commandProcessor.undoCommandName());

            ASSERT_TRUE(commandProcessor.undo()->success());

            ASSERT_FALSE(commandProcessor.canUndo());
            ASSERT_TRUE(commandProcessor.canRedo());
            ASSERT_EQ(commandName1, commandProcessor.redoCommandName());

            // NOTE: commandName2 is gone because it was coalesced into commandName1
            ASSERT_EQ((std::vector<NotificationTuple>{
                {CommandNotif::CommandUndo, commandName1},
                {CommandNotif::CommandUndone, commandName1},
                {CommandNotif::TransactionUndone, commandName1}
            }), observer.popNotifications());
        }

        TEST_CASE("CommandProcessorTest.collationInterval", "[CommandProcessorTest]") {
            /*
             * Execute two commands, with time passing between their execution exceeding the collation interval.
             * Then, undo the second command.
             */

            CommandProcessor commandProcessor(nullptr, std::chrono::milliseconds(100));
            TestObserver observer(commandProcessor);

            const auto commandName1 = "test command 1";
            auto command1 = TestCommand::create(commandName1);

            const auto commandName2 = "test command 2";
            auto command2 = TestCommand::create(commandName2);

            command1->expectDo(true);
            command2->expectDo(true);
            command2->expectUndo(true);

            commandProcessor.executeAndStore(std::move(command1));
            
            ASSERT_EQ((std::vector<NotificationTuple>{
                    {CommandNotif::CommandDo, commandName1},
                    {CommandNotif::CommandDone, commandName1},
                    {CommandNotif::TransactionDone, commandName1}
                }), observer.popNotifications());

            using namespace std::chrono_literals;
            std::this_thread::sleep_for(100ms);

            commandProcessor.executeAndStore(std::move(command2));

            ASSERT_EQ((std::vector<NotificationTuple>{
                    {CommandNotif::CommandDo, commandName2},
                    {CommandNotif::CommandDone, commandName2},
                    {CommandNotif::TransactionDone, commandName2}
                }), observer.popNotifications());

            ASSERT_TRUE(commandProcessor.canUndo());
            ASSERT_FALSE(commandProcessor.canRedo());
            ASSERT_EQ(commandName2, commandProcessor.undoCommandName());

            ASSERT_TRUE(commandProcessor.undo()->success());

            ASSERT_EQ((std::vector<NotificationTuple>{
                    {CommandNotif::CommandUndo, commandName2},
                    {CommandNotif::CommandUndone, commandName2},
                    {CommandNotif::TransactionUndone, commandName2}
                }), observer.popNotifications());

            ASSERT_TRUE(commandProcessor.canUndo());
            ASSERT_TRUE(commandProcessor.canRedo());
            ASSERT_EQ(commandName1, commandProcessor.undoCommandName());
            ASSERT_EQ(commandName2, commandProcessor.redoCommandName());
        }
    }
}
