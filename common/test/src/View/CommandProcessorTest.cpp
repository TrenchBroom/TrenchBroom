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

#include <catch2/catch.hpp>
#include <gmock/gmock.h>

#include "Macros.h"
#include "View/UndoableCommand.h"
#include "View/CommandProcessor.h"

#include <chrono>
#include <memory>
#include <thread>

namespace TrenchBroom {
    namespace View {
        class TestObserver {
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

            MOCK_METHOD1(commandDo, void(Command*));
            MOCK_METHOD1(commandDone, void(Command*));
            MOCK_METHOD1(commandDoFailed, void(Command*));
            MOCK_METHOD1(commandUndo, void(UndoableCommand*));
            MOCK_METHOD1(commandUndone, void(UndoableCommand*));
            MOCK_METHOD1(commandUndoFailed, void(UndoableCommand*));
            MOCK_METHOD1(transactionDone, void(const std::string&));
            MOCK_METHOD1(transactionUndone, void(const std::string&));

            void expectTransactionDone(const std::string& name) {
                EXPECT_CALL(*this, transactionDone(name)).RetiresOnSaturation();
            }

            void expectTransactionUndone(const std::string& name) {
                EXPECT_CALL(*this, transactionUndone(name)).RetiresOnSaturation();
            }
        };

        class TestCommand : public UndoableCommand {
        private:
            bool m_isRepeatDelimiter;
        public:
            static const CommandType Type;

            static std::unique_ptr<TestCommand> create(const std::string& name, const bool isRepeatDelimiter) {
                return std::make_unique<TestCommand>(name, isRepeatDelimiter);
            }

            explicit TestCommand(const std::string& name, const bool isRepeatDelimiter) :
            UndoableCommand(Type, name),
            m_isRepeatDelimiter(isRepeatDelimiter) {}
        private:
            std::unique_ptr<CommandResult> doPerformDo(MapDocumentCommandFacade* document) override {
                return std::make_unique<CommandResult>(doPerformDoProxy(document));
            }

            std::unique_ptr<CommandResult> doPerformUndo(MapDocumentCommandFacade* document) override {
                return std::make_unique<CommandResult>(doPerformUndoProxy(document));
            }

            bool doIsRepeatDelimiter() const override {
                return m_isRepeatDelimiter;
            }

            std::unique_ptr<UndoableCommand> doRepeat(MapDocumentCommandFacade* document) const override {
                return std::unique_ptr<UndoableCommand>(doRepeatProxy(document));
            }
        public:
            MOCK_METHOD1(doPerformDoProxy, bool(MapDocumentCommandFacade*));
            MOCK_METHOD1(doPerformUndoProxy, bool(MapDocumentCommandFacade*));

            MOCK_CONST_METHOD1(doIsRepeatable, bool(MapDocumentCommandFacade*));
            MOCK_CONST_METHOD1(doRepeatProxy, UndoableCommand*(MapDocumentCommandFacade*));

            MOCK_METHOD1(doCollateWith, bool(UndoableCommand*));

            void expectDo(const bool success, TestObserver& observer) {
                using namespace ::testing;

                EXPECT_CALL(observer, commandDo(this)).RetiresOnSaturation();
                EXPECT_CALL(*this, doPerformDoProxy(nullptr)).WillOnce(Return(success)).RetiresOnSaturation();

                if (success) {
                    EXPECT_CALL(observer, commandDone(this)).RetiresOnSaturation();
                } else {
                    EXPECT_CALL(observer, commandDoFailed(this)).RetiresOnSaturation();
                }
            }

            void expectUndo(const bool success, TestObserver& observer) {
                using namespace ::testing;

                EXPECT_CALL(observer, commandUndo(this)).RetiresOnSaturation();
                EXPECT_CALL(*this, doPerformUndoProxy(nullptr)).WillOnce(Return(success)).RetiresOnSaturation();

                if (success) {
                    EXPECT_CALL(observer, commandUndone(this)).RetiresOnSaturation();
                } else {
                    EXPECT_CALL(observer, commandUndoFailed(this)).RetiresOnSaturation();
                }
            }

            void expectCollate(UndoableCommand* command, const bool canCollate) {
                using namespace ::testing;

                EXPECT_CALL(*this, doCollateWith(command)).WillOnce(Return(canCollate)).RetiresOnSaturation();
            }

            TestCommand* expectRepeat(const bool repeatable, const std::string& repeatCommandName = "") {
                using namespace ::testing;

                EXPECT_CALL(*this, doIsRepeatable(nullptr)).WillOnce(Return(repeatable)).RetiresOnSaturation();

                if (repeatable) {
                    auto* repeatCommand = new TestCommand(repeatCommandName, false);
                    EXPECT_CALL(*this, doRepeatProxy(nullptr)).WillOnce(Return(repeatCommand)).RetiresOnSaturation();
                    return repeatCommand;
                } else {
                    return nullptr;
                }
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
            auto command = TestCommand::create(commandName, false);
            command->expectDo(true, observer);
            observer.expectTransactionDone(commandName);

            command->expectUndo(true, observer);
            observer.expectTransactionUndone(commandName);

            const auto doResult = commandProcessor.executeAndStore(std::move(command));
            ASSERT_TRUE(doResult->success());
            ASSERT_TRUE(commandProcessor.canUndo());
            ASSERT_FALSE(commandProcessor.canRedo());
            ASSERT_TRUE(commandProcessor.canRepeat());
            ASSERT_EQ(commandName, commandProcessor.undoCommandName());

            const auto undoResult = commandProcessor.undo();
            ASSERT_TRUE(undoResult->success());
            ASSERT_FALSE(commandProcessor.canUndo());
            ASSERT_TRUE(commandProcessor.canRedo());
            ASSERT_FALSE(commandProcessor.canRepeat());

            ASSERT_EQ(commandName, commandProcessor.redoCommandName());
        }

        TEST_CASE("CommandProcessorTest.doSuccessfulCommandAndFailAtUndo", "[CommandProcessorTest]") {
            /*
             * Execute a successful command, then undo fails.
             */

            CommandProcessor commandProcessor(nullptr);
            TestObserver observer(commandProcessor);

            const auto commandName = "test command";
            auto command = TestCommand::create(commandName, false);
            command->expectDo(true, observer);
            observer.expectTransactionDone(commandName);

            command->expectUndo(false, observer);

            const auto doResult = commandProcessor.executeAndStore(std::move(command));
            ASSERT_TRUE(doResult->success());
            ASSERT_TRUE(commandProcessor.canUndo());
            ASSERT_FALSE(commandProcessor.canRedo());
            ASSERT_TRUE(commandProcessor.canRepeat());
            ASSERT_EQ(commandName, commandProcessor.undoCommandName());

            const auto undoResult = commandProcessor.undo();
            ASSERT_FALSE(undoResult->success());
            ASSERT_FALSE(commandProcessor.canUndo());
            ASSERT_FALSE(commandProcessor.canRedo());
            ASSERT_FALSE(commandProcessor.canRepeat());
        }

        TEST_CASE("CommandProcessorTest.doFailingCommand", "[CommandProcessorTest]") {
            /*
             * Execute a failing command.
             */

            CommandProcessor commandProcessor(nullptr);
            TestObserver observer(commandProcessor);

            const auto commandName = "test command";
            auto command = TestCommand::create(commandName, false);
            command->expectDo(false, observer);

            const auto doResult = commandProcessor.executeAndStore(std::move(command));
            ASSERT_FALSE(doResult->success());

            ASSERT_FALSE(commandProcessor.canUndo());
            ASSERT_FALSE(commandProcessor.canRedo());
            ASSERT_FALSE(commandProcessor.canRepeat());
        }

        TEST_CASE("CommandProcessorTest.repeatAndUndoSingleCommand", "[CommandProcessorTest]") {
            /*
             * Execute a successful command, then repeat it successfully, and undo the repeated command
             * successfully, too.
             */

            CommandProcessor commandProcessor(nullptr);
            TestObserver observer(commandProcessor);

            const auto commandName = "test command";
            auto command = TestCommand::create(commandName, false);
            command->expectDo(true, observer);
            observer.expectTransactionDone(commandName);

            const auto repeatCommandName = "repeated command";
            auto* repeatCommand = command->expectRepeat(true, repeatCommandName);
            repeatCommand->expectDo(true, observer);
            observer.expectTransactionDone(repeatCommandName);

            repeatCommand->expectUndo(true, observer);
            observer.expectTransactionUndone(repeatCommandName);

            commandProcessor.executeAndStore(std::move(command));
            ASSERT_TRUE(commandProcessor.canRepeat());

            const auto repeatResult = commandProcessor.repeat();
            ASSERT_TRUE(repeatResult->success());

            ASSERT_TRUE(commandProcessor.canUndo());
            ASSERT_FALSE(commandProcessor.canRedo());
            ASSERT_TRUE(commandProcessor.canRepeat());

            ASSERT_EQ(repeatCommandName, commandProcessor.undoCommandName());

            const auto undoResult = commandProcessor.undo();
            ASSERT_TRUE(undoResult->success());

            ASSERT_TRUE(commandProcessor.canUndo());
            ASSERT_TRUE(commandProcessor.canRedo());
            ASSERT_TRUE(commandProcessor.canRepeat());

            ASSERT_EQ(commandName, commandProcessor.undoCommandName());
            ASSERT_EQ(repeatCommandName, commandProcessor.redoCommandName());
        }


        TEST_CASE("CommandProcessorTest.repeatSingleCommandTwice", "[CommandProcessorTest]") {
            /*
             * Execute a successful command, then repeat it successfully two times
             */

            CommandProcessor commandProcessor(nullptr);
            TestObserver observer(commandProcessor);

            const auto commandName = "test command";
            auto command = TestCommand::create(commandName, false);
            command->expectDo(true, observer);
            observer.expectTransactionDone(commandName);

            const auto repeatCommandName1 = "repeated command 1";
            auto* repeatCommand1 = command->expectRepeat(true, repeatCommandName1);
            repeatCommand1->expectDo(true, observer);
            observer.expectTransactionDone(repeatCommandName1);

            const auto repeatCommandName2 = "repeated command 2";
            auto* repeatCommand2 = command->expectRepeat(true, repeatCommandName2);
            repeatCommand2->expectDo(true, observer);
            observer.expectTransactionDone(repeatCommandName2);

            commandProcessor.executeAndStore(std::move(command));
            ASSERT_TRUE(commandProcessor.canRepeat());

            const auto repeatResult1 = commandProcessor.repeat();
            ASSERT_TRUE(repeatResult1->success());

            ASSERT_TRUE(commandProcessor.canUndo());
            ASSERT_FALSE(commandProcessor.canRedo());
            ASSERT_TRUE(commandProcessor.canRepeat());

            const auto repeatResult2 = commandProcessor.repeat();
            ASSERT_TRUE(repeatResult2->success());

            ASSERT_TRUE(commandProcessor.canUndo());
            ASSERT_FALSE(commandProcessor.canRedo());
            ASSERT_TRUE(commandProcessor.canRepeat());
        }

        TEST_CASE("CommandProcessorTest.repeatAndUndoMultipleCommands", "[CommandProcessorTest]") {
            /*
             * Execute two successful commands, then repeat them successfully, and undo the repeated commands
             * successfully, too.
             */

            CommandProcessor commandProcessor(nullptr);
            TestObserver observer(commandProcessor);

            const auto commandName1 = "test command 1";
            auto command1 = TestCommand::create(commandName1, false);
            command1->expectDo(true, observer);
            observer.expectTransactionDone(commandName1);

            const auto commandName2 = "test command 2";
            auto command2 = TestCommand::create(commandName2, false);
            command2->expectDo(true, observer);
            command1->expectCollate(command2.get(), false);
            observer.expectTransactionDone(commandName2);

            const auto repeatCommandName1 = "repeated command 1";
            const auto repeatCommandName2 = "repeated command 2";
            auto* repeatCommand1 = command1->expectRepeat(true, repeatCommandName1);
            auto* repeatCommand2 = command2->expectRepeat(true, repeatCommandName2);

            repeatCommand1->expectDo(true, observer);
            repeatCommand2->expectDo(true, observer);
            observer.expectTransactionDone("Repeat 2 Commands");


            commandProcessor.executeAndStore(std::move(command1));
            commandProcessor.executeAndStore(std::move(command2));

            const auto repeatResult = commandProcessor.repeat();
            ASSERT_TRUE(repeatResult->success());

            ASSERT_TRUE(commandProcessor.canUndo());
            ASSERT_FALSE(commandProcessor.canRedo());
            ASSERT_TRUE(commandProcessor.canRepeat());

            repeatCommand2->expectUndo(true, observer);
            repeatCommand1->expectUndo(true, observer);
            observer.expectTransactionUndone("Repeat 2 Commands");

            const auto undoResult = commandProcessor.undo();
            ASSERT_TRUE(undoResult->success());

            ASSERT_TRUE(commandProcessor.canUndo());
            ASSERT_TRUE(commandProcessor.canRedo());
            ASSERT_TRUE(commandProcessor.canRepeat());

            ASSERT_EQ(commandName2, commandProcessor.undoCommandName());
        }

        TEST_CASE("CommandProcessorTest.commitUndoRedoTransaction", "[CommandProcessorTest]") {
            /*
             * Execute two successful commands in a transaction, then undo the transaction successfully.
             * Finally, redo it, also with success.
             */

            CommandProcessor commandProcessor(nullptr);
            TestObserver observer(commandProcessor);

            const auto commandName1 = "test command 1";
            auto command1 = TestCommand::create(commandName1, false);

            const auto commandName2 = "test command 2";
            auto command2 = TestCommand::create(commandName2, false);

            command1->expectDo(true, observer);
            command2->expectDo(true, observer);
            command1->expectCollate(command2.get(), false);

            const auto transactionName = "transaction";
            observer.expectTransactionDone(transactionName);

            // undo transaction
            command2->expectUndo(true, observer);
            command1->expectUndo(true, observer);
            observer.expectTransactionUndone(transactionName);

            // redo
            command1->expectDo(true, observer);
            command2->expectDo(true, observer);
            observer.expectTransactionDone(transactionName);

            commandProcessor.startTransaction(transactionName);
            ASSERT_TRUE(commandProcessor.executeAndStore(std::move(command1))->success());
            ASSERT_TRUE(commandProcessor.executeAndStore(std::move(command2))->success());
            commandProcessor.commitTransaction();

            ASSERT_TRUE(commandProcessor.canUndo());
            ASSERT_FALSE(commandProcessor.canRedo());
            ASSERT_TRUE(commandProcessor.canRepeat());
            ASSERT_EQ(transactionName, commandProcessor.undoCommandName());

            ASSERT_TRUE(commandProcessor.undo()->success());

            ASSERT_FALSE(commandProcessor.canUndo());
            ASSERT_TRUE(commandProcessor.canRedo());
            ASSERT_FALSE(commandProcessor.canRepeat());
            ASSERT_EQ(transactionName, commandProcessor.redoCommandName());

            ASSERT_TRUE(commandProcessor.redo()->success());

            ASSERT_TRUE(commandProcessor.canUndo());
            ASSERT_FALSE(commandProcessor.canRedo());
            ASSERT_TRUE(commandProcessor.canRepeat());
            ASSERT_EQ(transactionName, commandProcessor.undoCommandName());
        }

        TEST_CASE("CommandProcessorTest.rollbackTransaction", "[CommandProcessorTest]") {
            /*
             * Execute two successful commands in a transaction, then rollback the transaction and commit it.
             */

            CommandProcessor commandProcessor(nullptr);
            TestObserver observer(commandProcessor);

            const auto commandName1 = "test command 1";
            auto command1 = TestCommand::create(commandName1, false);

            const auto commandName2 = "test command 2";
            auto command2 = TestCommand::create(commandName2, false);

            command1->expectDo(true, observer);
            command2->expectDo(true, observer);
            command1->expectCollate(command2.get(), false);

            // rollback
            command2->expectUndo(true, observer);
            command1->expectUndo(true, observer);

            const auto transactionName = "transaction";
            commandProcessor.startTransaction(transactionName);
            ASSERT_TRUE(commandProcessor.executeAndStore(std::move(command1))->success());
            ASSERT_TRUE(commandProcessor.executeAndStore(std::move(command2))->success());
            commandProcessor.rollbackTransaction();

            ASSERT_FALSE(commandProcessor.canUndo());
            ASSERT_FALSE(commandProcessor.canRedo());
            ASSERT_FALSE(commandProcessor.canRepeat());

            // does nothing, but closes the transaction
            commandProcessor.commitTransaction();

            ASSERT_FALSE(commandProcessor.canUndo());
            ASSERT_FALSE(commandProcessor.canRedo());
            ASSERT_FALSE(commandProcessor.canRepeat());
        }

        TEST_CASE("CommandProcessorTest.nestedTransactions", "[CommandProcessorTest]") {
            /*
             * Execute a command in a transaction, start a nested transaction, execute a command, and
             * commit both transactions. Then undo the outer transaction.
             */

            CommandProcessor commandProcessor(nullptr);
            TestObserver observer(commandProcessor);

            const auto outerCommandName = "outer command";
            auto outerCommand = TestCommand::create(outerCommandName, false);

            const auto innerCommandName = "inner command";
            auto innerCommand = TestCommand::create(innerCommandName, false);

            outerCommand->expectDo(true, observer);
            innerCommand->expectDo(true, observer);

            const auto innerTransactionName = "inner transaction";
            observer.expectTransactionDone(innerTransactionName);

            const auto outerTransactionName = "outer transaction";
            observer.expectTransactionDone(outerTransactionName);

            // undo transaction
            innerCommand->expectUndo(true, observer);
            outerCommand->expectUndo(true, observer);
            observer.expectTransactionUndone(outerTransactionName);

            commandProcessor.startTransaction(outerTransactionName);
            ASSERT_TRUE(commandProcessor.executeAndStore(std::move(outerCommand))->success());

            commandProcessor.startTransaction(innerTransactionName);
            ASSERT_TRUE(commandProcessor.executeAndStore(std::move(innerCommand))->success());

            commandProcessor.commitTransaction();
            commandProcessor.commitTransaction();

            ASSERT_TRUE(commandProcessor.canUndo());
            ASSERT_FALSE(commandProcessor.canRedo());
            ASSERT_TRUE(commandProcessor.canRepeat());
            ASSERT_EQ(outerTransactionName, commandProcessor.undoCommandName());

            ASSERT_TRUE(commandProcessor.undo()->success());

            ASSERT_FALSE(commandProcessor.canUndo());
            ASSERT_TRUE(commandProcessor.canRedo());
            ASSERT_FALSE(commandProcessor.canRepeat());
            ASSERT_EQ(outerTransactionName, commandProcessor.redoCommandName());
        }

        TEST_CASE("CommandProcessorTest.collateCommands", "[CommandProcessorTest]") {
            /*
             * Execute a command and collate the next command, then undo.
             */

            CommandProcessor commandProcessor(nullptr);
            TestObserver observer(commandProcessor);

            const auto commandName1 = "test command 1";
            auto command1 = TestCommand::create(commandName1, false);

            const auto commandName2 = "test command 2";
            auto command2 = TestCommand::create(commandName2, false);

            command1->expectDo(true, observer);
            observer.expectTransactionDone(commandName1);

            command2->expectDo(true, observer);
            observer.expectTransactionDone(commandName2);

            command1->expectCollate(command2.get(), true);

            command1->expectUndo(true, observer);
            observer.expectTransactionUndone(commandName1);


            commandProcessor.executeAndStore(std::move(command1));
            commandProcessor.executeAndStore(std::move(command2));

            ASSERT_TRUE(commandProcessor.canUndo());
            ASSERT_FALSE(commandProcessor.canRedo());
            ASSERT_TRUE(commandProcessor.canRepeat());
            ASSERT_EQ(commandName1, commandProcessor.undoCommandName());

            ASSERT_TRUE(commandProcessor.undo()->success());

            ASSERT_FALSE(commandProcessor.canUndo());
            ASSERT_TRUE(commandProcessor.canRedo());
            ASSERT_FALSE(commandProcessor.canRepeat());
            ASSERT_EQ(commandName1, commandProcessor.redoCommandName());
        }

        TEST_CASE("CommandProcessorTest.collationInterval", "[CommandProcessorTest]") {
            /*
             * Execute two commands, with time passing between their execution exceeding the collation interval.
             */

            CommandProcessor commandProcessor(nullptr, std::chrono::milliseconds(100));
            TestObserver observer(commandProcessor);

            const auto commandName1 = "test command 1";
            auto command1 = TestCommand::create(commandName1, false);

            const auto commandName2 = "test command 2";
            auto command2 = TestCommand::create(commandName2, false);

            command1->expectDo(true, observer);
            observer.expectTransactionDone(commandName1);

            command2->expectDo(true, observer);
            observer.expectTransactionDone(commandName2);

            command2->expectUndo(true, observer);
            observer.expectTransactionUndone(commandName2);


            commandProcessor.executeAndStore(std::move(command1));

            using namespace std::chrono_literals;
            std::this_thread::sleep_for(100ms);

            commandProcessor.executeAndStore(std::move(command2));

            ASSERT_TRUE(commandProcessor.canUndo());
            ASSERT_FALSE(commandProcessor.canRedo());
            ASSERT_TRUE(commandProcessor.canRepeat());
            ASSERT_EQ(commandName2, commandProcessor.undoCommandName());

            ASSERT_TRUE(commandProcessor.undo()->success());

            ASSERT_TRUE(commandProcessor.canUndo());
            ASSERT_TRUE(commandProcessor.canRedo());
            ASSERT_TRUE(commandProcessor.canRepeat());
            ASSERT_EQ(commandName1, commandProcessor.undoCommandName());
            ASSERT_EQ(commandName2, commandProcessor.redoCommandName());
        }
    }
}
