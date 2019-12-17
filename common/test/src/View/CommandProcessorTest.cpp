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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "Macros.h"
#include "View/UndoableCommand.h"
#include "View/CommandProcessor.h"

#include <memory>

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
        };

        class TestCommand : public UndoableCommand {
        public:
            static const CommandType Type;

            static std::unique_ptr<TestCommand> create(const std::string& name) {
                return std::make_unique<TestCommand>(name);
            }

            TestCommand(const std::string& name) :
            UndoableCommand(Type, name) {}
        private:
            std::unique_ptr<CommandResult> doPerformDo(MapDocumentCommandFacade* document) override {
                return std::make_unique<CommandResult>(doPerformDoProxy(document));
            }

            std::unique_ptr<CommandResult> doPerformUndo(MapDocumentCommandFacade* document) override {
                return std::make_unique<CommandResult>(doPerformUndoProxy(document));
            }

            std::unique_ptr<UndoableCommand> doRepeat(MapDocumentCommandFacade* document) const override {
                return std::unique_ptr<UndoableCommand>(doRepeatProxy(document));
            }
        public:
            MOCK_METHOD1(doPerformDoProxy, bool(MapDocumentCommandFacade*));
            MOCK_METHOD1(doPerformUndoProxy, bool(MapDocumentCommandFacade*));

            MOCK_CONST_METHOD0(doIsRepeatDelimiter, bool());
            MOCK_CONST_METHOD1(doIsRepeatable, bool(MapDocumentCommandFacade*));
            MOCK_CONST_METHOD1(doRepeatProxy, UndoableCommand*(MapDocumentCommandFacade*));

            MOCK_METHOD1(doCollateWith, bool(UndoableCommand*));

            void expectDo(const bool success, TestObserver& observer) {
                using namespace ::testing;

                EXPECT_CALL(observer, commandDo(this)).Times(1);
                EXPECT_CALL(*this, doPerformDoProxy(nullptr)).Times(1).WillOnce(Return(success));

                if (success) {
                    EXPECT_CALL(observer, commandDone(this)).Times(1);
                } else {
                    EXPECT_CALL(observer, commandDoFailed(this)).Times(1);
                }
            }

            void expectUndo(const bool success, TestObserver& observer) {
                using namespace ::testing;

                EXPECT_CALL(observer, commandUndo(this)).Times(1);
                EXPECT_CALL(*this, doPerformUndoProxy(nullptr)).Times(1).WillOnce(Return(success));

                if (success) {
                    EXPECT_CALL(observer, commandUndone(this)).Times(1);
                } else {
                    EXPECT_CALL(observer, commandUndoFailed(this)).Times(1);
                }
            }

            void expectCollate(UndoableCommand* command, const bool canCollate) {
                using namespace ::testing;

                EXPECT_CALL(*this, doCollateWith(command)).Times(1).WillOnce(Return(canCollate));
            }

            TestCommand* expectRepeat(const bool repeatable, const std::string& repeatCommandName = "") {
                using namespace ::testing;

                EXPECT_CALL(*this, doIsRepeatable(nullptr)).Times(1).WillOnce(Return(repeatable));

                if (repeatable) {
                    auto* repeatCommand = new TestCommand(repeatCommandName);
                    EXPECT_CALL(*this, doRepeatProxy(nullptr)).Times(1).WillOnce(Return(repeatCommand));
                    return repeatCommand;
                } else {
                    return nullptr;
                }
            }

            void delimitRepeats(const bool delimit) {
                using namespace ::testing;
                EXPECT_CALL(*this, doIsRepeatDelimiter()).Times(1).WillOnce(Return(delimit));
            }

            deleteCopyAndMove(TestCommand)
        };

        const Command::CommandType TestCommand::Type = Command::freeType();

        TEST(CommandProcessorTest, doAndUndoSuccessfulCommand) {
            /*
             * Execute a successful command, then undo it successfully.
             */

            using namespace ::testing;
            InSequence s;

            CommandProcessor commandProcessor(nullptr);
            TestObserver observer(commandProcessor);

            const auto commandName = "test command";
            auto command = TestCommand::create(commandName);
            command->expectDo(true, observer);
            command->delimitRepeats(false);
            command->expectUndo(true, observer);

            const auto doResult = commandProcessor.executeAndStoreCommand(std::move(command));
            ASSERT_TRUE(doResult->success());
            ASSERT_TRUE(commandProcessor.canUndo());
            ASSERT_FALSE(commandProcessor.canRedo());
            ASSERT_TRUE(commandProcessor.hasRepeatableCommands());
            ASSERT_EQ(commandName, commandProcessor.undoCommandName());

            const auto undoResult = commandProcessor.undoLastCommand();
            ASSERT_TRUE(undoResult->success());
            ASSERT_FALSE(commandProcessor.canUndo());
            ASSERT_TRUE(commandProcessor.canRedo());
            ASSERT_FALSE(commandProcessor.hasRepeatableCommands());

            ASSERT_EQ(commandName, commandProcessor.redoCommandName());
        }

        TEST(CommandProcessorTest, doSuccessfulCommandAndFailAtUndo) {
            /*
             * Execute a successful command, then undo fails.
             */

            using namespace ::testing;
            InSequence s;

            CommandProcessor commandProcessor(nullptr);
            TestObserver observer(commandProcessor);

            const auto commandName = "test command";
            auto command = TestCommand::create(commandName);
            command->expectDo(true, observer);
            command->delimitRepeats(false);
            command->expectUndo(false, observer);

            const auto doResult = commandProcessor.executeAndStoreCommand(std::move(command));
            ASSERT_TRUE(doResult->success());
            ASSERT_TRUE(commandProcessor.canUndo());
            ASSERT_FALSE(commandProcessor.canRedo());
            ASSERT_TRUE(commandProcessor.hasRepeatableCommands());
            ASSERT_EQ(commandName, commandProcessor.undoCommandName());

            const auto undoResult = commandProcessor.undoLastCommand();
            ASSERT_FALSE(undoResult->success());
            ASSERT_FALSE(commandProcessor.canUndo());
            ASSERT_FALSE(commandProcessor.canRedo());
            ASSERT_FALSE(commandProcessor.hasRepeatableCommands());
        }

        TEST(CommandProcessorTest, doFailingCommand) {
            /*
             * Execute a failing command.
             */

            using namespace ::testing;
            InSequence s;

            CommandProcessor commandProcessor(nullptr);
            TestObserver observer(commandProcessor);

            const auto commandName = "test command";
            auto command = TestCommand::create(commandName);
            command->expectDo(false, observer);

            const auto doResult = commandProcessor.executeAndStoreCommand(std::move(command));
            ASSERT_FALSE(doResult->success());

            ASSERT_FALSE(commandProcessor.canUndo());
            ASSERT_FALSE(commandProcessor.canRedo());
            ASSERT_FALSE(commandProcessor.hasRepeatableCommands());
        }

        TEST(CommandProcessorTest, repeatAndUndoSingleCommand) {
            /*
             * Execute a successful command, then repeat it succesfully, and undo the repeated command
             * successfully, too.
             */

            using namespace ::testing;
            InSequence s;

            CommandProcessor commandProcessor(nullptr);
            TestObserver observer(commandProcessor);

            const auto commandName = "test command";
            auto command = TestCommand::create(commandName);
            command->expectDo(true, observer);
            command->delimitRepeats(false);

            const auto repeatCommandName = "repeated command";
            auto* repeatCommand = command->expectRepeat(true, repeatCommandName);
            repeatCommand->expectDo(true, observer);
            repeatCommand->delimitRepeats(false);
            repeatCommand->expectUndo(true, observer);

            commandProcessor.executeAndStoreCommand(std::move(command));
            ASSERT_TRUE(commandProcessor.hasRepeatableCommands());

            const auto repeatResult = commandProcessor.repeatLastCommands();
            ASSERT_TRUE(repeatResult->success());

            ASSERT_TRUE(commandProcessor.canUndo());
            ASSERT_FALSE(commandProcessor.canRedo());
            ASSERT_TRUE(commandProcessor.hasRepeatableCommands());

            ASSERT_EQ(repeatCommandName, commandProcessor.undoCommandName());

            const auto undoResult = commandProcessor.undoLastCommand();
            ASSERT_TRUE(undoResult->success());

            ASSERT_TRUE(commandProcessor.canUndo());
            ASSERT_TRUE(commandProcessor.canRedo());
            ASSERT_TRUE(commandProcessor.hasRepeatableCommands());

            ASSERT_EQ(commandName, commandProcessor.undoCommandName());
            ASSERT_EQ(repeatCommandName, commandProcessor.redoCommandName());
        }

        TEST(CommandProcessorTest, repeatAndUndoMultipleCommands) {
            /*
             * Execute two successful commands, then repeat them succesfully, and undo the repeated commands
             * successfully, too.
             */

            using namespace ::testing;
            InSequence s;

            CommandProcessor commandProcessor(nullptr);
            TestObserver observer(commandProcessor);

            const auto commandName1 = "test command 1";
            auto command1 = TestCommand::create(commandName1);
            command1->expectDo(true, observer);
            command1->delimitRepeats(false);

            const auto commandName2 = "test command 2";
            auto command2 = TestCommand::create(commandName2);
            command2->expectDo(true, observer);
            command1->expectCollate(command2.get(), false);
            command2->delimitRepeats(false);

            auto* rawCommand1 = command1.get();
            auto* rawCommand2 = command2.get();

            commandProcessor.executeAndStoreCommand(std::move(command1));
            commandProcessor.executeAndStoreCommand(std::move(command2));

            const auto repeatCommandName1 = "repeated command 1";
            const auto repeatCommandName2 = "repeated command 2";
            auto* repeatCommand1 = rawCommand1->expectRepeat(true, repeatCommandName1);
            auto* repeatCommand2 = rawCommand2->expectRepeat(true, repeatCommandName2);

            repeatCommand1->expectDo(true, observer);
            repeatCommand2->expectDo(true, observer);

            const auto repeatResult = commandProcessor.repeatLastCommands();
            ASSERT_TRUE(repeatResult->success());

            ASSERT_TRUE(commandProcessor.canUndo());
            ASSERT_FALSE(commandProcessor.canRedo());
            ASSERT_TRUE(commandProcessor.hasRepeatableCommands());

            repeatCommand2->expectUndo(true, observer);
            repeatCommand1->expectUndo(true, observer);

            const auto undoResult = commandProcessor.undoLastCommand();
            ASSERT_TRUE(undoResult->success());

            ASSERT_TRUE(commandProcessor.canUndo());
            ASSERT_TRUE(commandProcessor.canRedo());
            ASSERT_TRUE(commandProcessor.hasRepeatableCommands());

            ASSERT_EQ(commandName2, commandProcessor.undoCommandName());
        }

        TEST(CommandProcessorTest, transaction) {
            /*
             * Execute two successful commands in a transaction, then undo the transaction successfully.
             * Finally, redo it, also with success.
             */

            using namespace ::testing;
            InSequence s;

            CommandProcessor commandProcessor(nullptr);
            TestObserver observer(commandProcessor);

            const auto commandName1 = "test command 1";
            auto command1 = TestCommand::create(commandName1);

            const auto commandName2 = "test command 2";
            auto command2 = TestCommand::create(commandName2);

            // execute commands in transaction
            command1->expectDo(true, observer);
            command2->expectDo(true, observer);

            command1->delimitRepeats(false);
            command2->delimitRepeats(false);

            const auto transactionName = "transaction";
            EXPECT_CALL(observer, transactionDone(transactionName)).Times(1);

            // undo transaction
            command2->expectUndo(true, observer);
            command1->expectUndo(true, observer);
            EXPECT_CALL(observer, transactionUndone(transactionName)).Times(1);

            // redo transaction
            command1->expectDo(true, observer);
            command2->expectDo(true, observer);

            EXPECT_CALL(observer, transactionDone(transactionName)).Times(1);

            commandProcessor.startTransaction(transactionName);
            ASSERT_TRUE(commandProcessor.executeAndStoreCommand(std::move(command1))->success());
            ASSERT_TRUE(commandProcessor.executeAndStoreCommand(std::move(command2))->success());
            commandProcessor.commitTransaction();

            ASSERT_TRUE(commandProcessor.canUndo());
            ASSERT_FALSE(commandProcessor.canRedo());
            ASSERT_TRUE(commandProcessor.hasRepeatableCommands());
            ASSERT_EQ(transactionName, commandProcessor.undoCommandName());

            ASSERT_TRUE(commandProcessor.undoLastCommand()->success());

            ASSERT_FALSE(commandProcessor.canUndo());
            ASSERT_TRUE(commandProcessor.canRedo());
            ASSERT_FALSE(commandProcessor.hasRepeatableCommands());
            ASSERT_EQ(transactionName, commandProcessor.redoCommandName());

            ASSERT_TRUE(commandProcessor.redoNextCommand()->success());

            ASSERT_TRUE(commandProcessor.canUndo());
            ASSERT_FALSE(commandProcessor.canRedo());
            ASSERT_TRUE(commandProcessor.hasRepeatableCommands());
            ASSERT_EQ(transactionName, commandProcessor.undoCommandName());
        }
    }
}
