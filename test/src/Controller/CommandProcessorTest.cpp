/*
 Copyright (C) 2010-2013 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "Exceptions.h"
#include "Controller/Command.h"
#include "Controller/CommandProcessor.h"
#include "TestUtils.h"

namespace TrenchBroom {
    namespace Controller {
        class MockCommand : public Command {
        public: // changing visibility to public for gmock
            MOCK_METHOD0(doPerformDo, bool());
            MOCK_METHOD0(doPerformUndo, bool());
        public:
            MockCommand(const String& name, const bool undoable = true) :
            Command(freeType(), name, undoable) {}
        };
        
        TEST(CommandProcessorTest, submitAndDontStoreCommand) {
            using namespace testing;
            InSequence forceInSequenceMockCalls;

            CommandProcessor proc;
            MockCommand* cmd = new MockCommand("test");
            
            EXPECT_CALL(*cmd, doPerformDo()).WillOnce(Return(true));
            
            ASSERT_TRUE(proc.submitCommand(Command::Ptr(cmd)));
            ASSERT_FALSE(proc.hasLastCommand());
            ASSERT_FALSE(proc.hasNextCommand());
            ASSERT_THROW(proc.lastCommandName(), CommandProcessorException);
            ASSERT_THROW(proc.nextCommandName(), CommandProcessorException);
            ASSERT_THROW(proc.undoLastCommand(), CommandProcessorException);
            ASSERT_THROW(proc.redoNextCommand(), CommandProcessorException);
        }
        
        TEST(CommandProcessorTest, submitAndStoreCommand) {
            using namespace testing;
            InSequence forceInSequenceMockCalls;
            
            CommandProcessor proc;
            MockCommand* cmd = new MockCommand("test");
            
            EXPECT_CALL(*cmd, doPerformDo()).WillOnce(Return(true));

            ASSERT_TRUE(proc.submitAndStoreCommand(Command::Ptr(cmd)));
            ASSERT_TRUE(proc.hasLastCommand());
            ASSERT_FALSE(proc.hasNextCommand());
            ASSERT_THROW(proc.nextCommandName(), CommandProcessorException);
            ASSERT_THROW(proc.redoNextCommand(), CommandProcessorException);
            ASSERT_EQ(cmd->name(), proc.lastCommandName());
        }

        TEST(CommandProcessorTest, submitAndStore2Commands) {
            using namespace testing;
            InSequence forceInSequenceMockCalls;
            
            CommandProcessor proc;
            MockCommand* cmd1 = new MockCommand("test1");
            MockCommand* cmd2 = new MockCommand("test2");
            
            EXPECT_CALL(*cmd1, doPerformDo()).WillOnce(Return(true));
            EXPECT_CALL(*cmd2, doPerformDo()).WillOnce(Return(true));

            ASSERT_TRUE(proc.submitAndStoreCommand(Command::Ptr(cmd1)));
            ASSERT_TRUE(proc.submitAndStoreCommand(Command::Ptr(cmd2)));
            ASSERT_TRUE(proc.hasLastCommand());
            ASSERT_FALSE(proc.hasNextCommand());
            ASSERT_THROW(proc.nextCommandName(), CommandProcessorException);
            ASSERT_THROW(proc.redoNextCommand(), CommandProcessorException);
            ASSERT_EQ(cmd2->name(), proc.lastCommandName());
        }
        
        TEST(CommandProcessorTest, undoLastCommand) {
            using namespace testing;
            InSequence forceInSequenceMockCalls;
            
            CommandProcessor proc;
            MockCommand* cmd1 = new MockCommand("test1");
            MockCommand* cmd2 = new MockCommand("test2");
            
            EXPECT_CALL(*cmd1, doPerformDo()).WillOnce(Return(true));
            EXPECT_CALL(*cmd2, doPerformDo()).WillOnce(Return(true));
            EXPECT_CALL(*cmd2, doPerformUndo()).WillOnce(Return(true));

            ASSERT_TRUE(proc.submitAndStoreCommand(Command::Ptr(cmd1)));
            ASSERT_TRUE(proc.submitAndStoreCommand(Command::Ptr(cmd2)));
            ASSERT_TRUE(proc.undoLastCommand());
            ASSERT_TRUE(proc.hasLastCommand());
            ASSERT_TRUE(proc.hasNextCommand());
            ASSERT_EQ(cmd1->name(), proc.lastCommandName());
            ASSERT_EQ(cmd2->name(), proc.nextCommandName());
        }
        
        TEST(CommandProcessorTest, undoTooManyCommands) {
            using namespace testing;
            InSequence forceInSequenceMockCalls;
            
            CommandProcessor proc;
            MockCommand* cmd1 = new MockCommand("test1");
            MockCommand* cmd2 = new MockCommand("test2");
            
            EXPECT_CALL(*cmd1, doPerformDo()).WillOnce(Return(true));
            EXPECT_CALL(*cmd2, doPerformDo()).WillOnce(Return(true));
            EXPECT_CALL(*cmd2, doPerformUndo()).WillOnce(Return(true));
            EXPECT_CALL(*cmd1, doPerformUndo()).WillOnce(Return(true));
            
            ASSERT_TRUE(proc.submitAndStoreCommand(Command::Ptr(cmd1)));
            ASSERT_TRUE(proc.submitAndStoreCommand(Command::Ptr(cmd2)));
            ASSERT_TRUE(proc.undoLastCommand());
            ASSERT_TRUE(proc.undoLastCommand());
            ASSERT_THROW(proc.undoLastCommand(), CommandProcessorException);
        }
        
        TEST(CommandProcessorTest, redoNextCommand) {
            using namespace testing;
            InSequence forceInSequenceMockCalls;
            
            CommandProcessor proc;
            MockCommand* cmd1 = new MockCommand("test1");
            MockCommand* cmd2 = new MockCommand("test2");
            
            EXPECT_CALL(*cmd1, doPerformDo()).WillOnce(Return(true));
            EXPECT_CALL(*cmd2, doPerformDo()).WillOnce(Return(true));
            EXPECT_CALL(*cmd2, doPerformUndo()).WillOnce(Return(true));
            EXPECT_CALL(*cmd2, doPerformDo()).WillOnce(Return(true));
            
            ASSERT_TRUE(proc.submitAndStoreCommand(Command::Ptr(cmd1)));
            ASSERT_TRUE(proc.submitAndStoreCommand(Command::Ptr(cmd2)));
            ASSERT_TRUE(proc.undoLastCommand());
            ASSERT_TRUE(proc.redoNextCommand());
            ASSERT_TRUE(proc.hasLastCommand());
            ASSERT_FALSE(proc.hasNextCommand());
            ASSERT_EQ(cmd2->name(), proc.lastCommandName());
            ASSERT_THROW(proc.redoNextCommand(), CommandProcessorException);
        }
        
        TEST(CommandProcessorTest, redoTooManyCommands) {
            using namespace testing;
            InSequence forceInSequenceMockCalls;
            
            CommandProcessor proc;
            MockCommand* cmd1 = new MockCommand("test1");
            MockCommand* cmd2 = new MockCommand("test2");
            
            EXPECT_CALL(*cmd1, doPerformDo()).WillOnce(Return(true));
            EXPECT_CALL(*cmd2, doPerformDo()).WillOnce(Return(true));
            EXPECT_CALL(*cmd2, doPerformUndo()).WillOnce(Return(true));
            EXPECT_CALL(*cmd2, doPerformDo()).WillOnce(Return(true));
            
            ASSERT_TRUE(proc.submitAndStoreCommand(Command::Ptr(cmd1)));
            ASSERT_TRUE(proc.submitAndStoreCommand(Command::Ptr(cmd2)));
            ASSERT_TRUE(proc.undoLastCommand());
            ASSERT_TRUE(proc.redoNextCommand());
            ASSERT_THROW(proc.redoNextCommand(), CommandProcessorException);
        }
        
        TEST(CommandProcessorTest, undoLastAndSubmitNewCommand) {
            using namespace testing;
            InSequence forceInSequenceMockCalls;
            
            CommandProcessor proc;
            MockCommand* cmd1 = new MockCommand("test1");
            MockCommand* cmd2 = new MockCommand("test2");
            MockCommand* cmd3 = new MockCommand("test3");
            
            EXPECT_CALL(*cmd1, doPerformDo()).WillOnce(Return(true));
            EXPECT_CALL(*cmd2, doPerformDo()).WillOnce(Return(true));
            EXPECT_CALL(*cmd2, doPerformUndo()).WillOnce(Return(true));
            EXPECT_CALL(*cmd3, doPerformDo()).WillOnce(Return(true));
            
            ASSERT_TRUE(proc.submitAndStoreCommand(Command::Ptr(cmd1)));
            ASSERT_TRUE(proc.submitAndStoreCommand(Command::Ptr(cmd2)));
            ASSERT_TRUE(proc.undoLastCommand());
            ASSERT_TRUE(proc.submitAndStoreCommand(Command::Ptr(cmd3)));
            ASSERT_EQ(cmd3->name(), proc.lastCommandName());
            ASSERT_TRUE(proc.hasLastCommand());
            ASSERT_FALSE(proc.hasNextCommand());
            ASSERT_THROW(proc.redoNextCommand(), CommandProcessorException);
        }

        TEST(CommandProcessorTest, submitOneShotCommand) {
            using namespace testing;
            InSequence forceInSequenceMockCalls;
            
            CommandProcessor proc;
            MockCommand* cmd1 = new MockCommand("test1");
            MockCommand* cmd2 = new MockCommand("test2");
            MockCommand* cmd3 = new MockCommand("test3", false);
            
            EXPECT_CALL(*cmd1, doPerformDo()).WillOnce(Return(true));
            EXPECT_CALL(*cmd2, doPerformDo()).WillOnce(Return(true));
            EXPECT_CALL(*cmd2, doPerformUndo()).WillOnce(Return(true));
            EXPECT_CALL(*cmd3, doPerformDo()).WillOnce(Return(true));
            
            ASSERT_TRUE(proc.submitAndStoreCommand(Command::Ptr(cmd1)));
            ASSERT_TRUE(proc.submitAndStoreCommand(Command::Ptr(cmd2)));
            ASSERT_TRUE(proc.undoLastCommand());
            ASSERT_TRUE(proc.submitAndStoreCommand(Command::Ptr(cmd3)));

            ASSERT_FALSE(proc.hasLastCommand());
            ASSERT_FALSE(proc.hasNextCommand());
            ASSERT_THROW(proc.lastCommandName(), CommandProcessorException);
            ASSERT_THROW(proc.nextCommandName(), CommandProcessorException);
            ASSERT_THROW(proc.undoLastCommand(), CommandProcessorException);
            ASSERT_THROW(proc.redoNextCommand(), CommandProcessorException);
        }
        
        TEST(CommandProcessorTest, createAndCloseCommandGroup) {
            using namespace testing;
            InSequence forceInSequenceMockCalls;
            
            CommandProcessor proc;
            MockCommand* cmd1 = new MockCommand("test1");
            MockCommand* cmd2 = new MockCommand("test2");
            MockCommand* cmd3 = new MockCommand("test3");
            
            EXPECT_CALL(*cmd1, doPerformDo()).WillOnce(Return(true));
            EXPECT_CALL(*cmd2, doPerformDo()).WillOnce(Return(true));
            EXPECT_CALL(*cmd3, doPerformDo()).WillOnce(Return(true));
            
            ASSERT_TRUE(proc.submitAndStoreCommand(Command::Ptr(cmd1)));
            proc.beginUndoableGroup("group");
            ASSERT_TRUE(proc.submitAndStoreCommand(Command::Ptr(cmd2)));
            ASSERT_TRUE(proc.submitAndStoreCommand(Command::Ptr(cmd3)));
            proc.closeGroup();
            
            ASSERT_TRUE(proc.hasLastCommand());
            ASSERT_FALSE(proc.hasNextCommand());
            ASSERT_EQ(String("group"), proc.lastCommandName());
        }
        
        TEST(CommandProcessorTest, createAndCloseEmptyCommandGroup) {
            using namespace testing;
            InSequence forceInSequenceMockCalls;
            
            CommandProcessor proc;
            MockCommand* cmd1 = new MockCommand("test1");
            
            EXPECT_CALL(*cmd1, doPerformDo()).WillOnce(Return(true));
            
            ASSERT_TRUE(proc.submitAndStoreCommand(Command::Ptr(cmd1)));
            proc.beginUndoableGroup("group");
            proc.closeGroup();
            
            ASSERT_TRUE(proc.hasLastCommand());
            ASSERT_FALSE(proc.hasNextCommand());
            ASSERT_EQ(cmd1->name(), proc.lastCommandName());
        }
        
        TEST(CommandProcessorTest, createCommandGroupAndUndoGroupedCommand) {
            using namespace testing;
            InSequence forceInSequenceMockCalls;
            
            CommandProcessor proc;
            MockCommand* cmd1 = new MockCommand("test1");
            MockCommand* cmd2 = new MockCommand("test2");
            
            EXPECT_CALL(*cmd1, doPerformDo()).WillOnce(Return(true));
            EXPECT_CALL(*cmd2, doPerformDo()).WillOnce(Return(true));
            
            ASSERT_TRUE(proc.submitAndStoreCommand(Command::Ptr(cmd1)));
            proc.beginUndoableGroup("group");
            ASSERT_TRUE(proc.submitAndStoreCommand(Command::Ptr(cmd2)));
            ASSERT_THROW(proc.undoLastCommand(), CommandProcessorException);
            proc.closeGroup();
            
            ASSERT_TRUE(proc.hasLastCommand());
            ASSERT_FALSE(proc.hasNextCommand());
            ASSERT_EQ(String("group"), proc.lastCommandName());
        }
        
        TEST(CommandProcessorTest, createCommandGroupAndRedo) {
            using namespace testing;
            InSequence forceInSequenceMockCalls;
            
            CommandProcessor proc;
            MockCommand* cmd1 = new MockCommand("test1");
            MockCommand* cmd2 = new MockCommand("test2");
            
            EXPECT_CALL(*cmd1, doPerformDo()).WillOnce(Return(true));
            EXPECT_CALL(*cmd1, doPerformUndo()).WillOnce(Return(true));
            EXPECT_CALL(*cmd2, doPerformDo()).WillOnce(Return(true));
            
            ASSERT_TRUE(proc.submitAndStoreCommand(Command::Ptr(cmd1)));
            ASSERT_TRUE(proc.undoLastCommand());
            proc.beginUndoableGroup("group");
            ASSERT_THROW(proc.redoNextCommand(), CommandProcessorException);
            ASSERT_TRUE(proc.submitAndStoreCommand(Command::Ptr(cmd2)));
            proc.closeGroup();
            
            ASSERT_TRUE(proc.hasLastCommand());
            ASSERT_FALSE(proc.hasNextCommand());
            ASSERT_EQ(String("group"), proc.lastCommandName());
        }
        
        TEST(CommandProcessorTest, createAndCloseCommandGroupTooOften) {
            using namespace testing;
            InSequence forceInSequenceMockCalls;
            
            CommandProcessor proc;
            MockCommand* cmd1 = new MockCommand("test1");
            MockCommand* cmd2 = new MockCommand("test2");
            MockCommand* cmd3 = new MockCommand("test3");
            
            EXPECT_CALL(*cmd1, doPerformDo()).WillOnce(Return(true));
            EXPECT_CALL(*cmd2, doPerformDo()).WillOnce(Return(true));
            EXPECT_CALL(*cmd3, doPerformDo()).WillOnce(Return(true));
            
            ASSERT_TRUE(proc.submitAndStoreCommand(Command::Ptr(cmd1)));
            proc.beginUndoableGroup("group");
            ASSERT_TRUE(proc.submitAndStoreCommand(Command::Ptr(cmd2)));
            ASSERT_TRUE(proc.submitAndStoreCommand(Command::Ptr(cmd3)));
            proc.closeGroup();
            ASSERT_THROW(proc.closeGroup(), CommandProcessorException);
            
            ASSERT_TRUE(proc.hasLastCommand());
            ASSERT_FALSE(proc.hasNextCommand());
            ASSERT_EQ(String("group"), proc.lastCommandName());
        }
        
        TEST(CommandProcessorTest, createCloseAndUndoCommandGroup) {
            using namespace testing;
            InSequence forceInSequenceMockCalls;
            
            CommandProcessor proc;
            MockCommand* cmd1 = new MockCommand("test1");
            MockCommand* cmd2 = new MockCommand("test2");
            MockCommand* cmd3 = new MockCommand("test3");

            EXPECT_CALL(*cmd1, doPerformDo()).WillOnce(Return(true));
            EXPECT_CALL(*cmd2, doPerformDo()).WillOnce(Return(true));
            EXPECT_CALL(*cmd3, doPerformDo()).WillOnce(Return(true));
            EXPECT_CALL(*cmd3, doPerformUndo()).WillOnce(Return(true));
            EXPECT_CALL(*cmd2, doPerformUndo()).WillOnce(Return(true));
            
            ASSERT_TRUE(proc.submitAndStoreCommand(Command::Ptr(cmd1)));
            proc.beginUndoableGroup("group");
            ASSERT_TRUE(proc.submitAndStoreCommand(Command::Ptr(cmd2)));
            ASSERT_TRUE(proc.submitAndStoreCommand(Command::Ptr(cmd3)));
            proc.closeGroup();
            ASSERT_TRUE(proc.undoLastCommand());
            
            ASSERT_TRUE(proc.hasLastCommand());
            ASSERT_TRUE(proc.hasNextCommand());
            ASSERT_EQ(cmd1->name(), proc.lastCommandName());
            ASSERT_EQ(String("group"), proc.nextCommandName());
        }

        TEST(CommandProcessorTest, createNestedCommandGroups) {
            using namespace testing;
            InSequence forceInSequenceMockCalls;
            
            CommandProcessor proc;
            MockCommand* cmd1 = new MockCommand("test1");
            MockCommand* cmd2 = new MockCommand("test2");
            MockCommand* cmd3 = new MockCommand("test3");
            MockCommand* cmd4 = new MockCommand("test4");
            
            EXPECT_CALL(*cmd1, doPerformDo()).WillOnce(Return(true));
            EXPECT_CALL(*cmd2, doPerformDo()).WillOnce(Return(true));
            EXPECT_CALL(*cmd3, doPerformDo()).WillOnce(Return(true));
            EXPECT_CALL(*cmd4, doPerformDo()).WillOnce(Return(true));
            EXPECT_CALL(*cmd4, doPerformUndo()).WillOnce(Return(true));
            EXPECT_CALL(*cmd3, doPerformUndo()).WillOnce(Return(true));
            EXPECT_CALL(*cmd2, doPerformUndo()).WillOnce(Return(true));
            
            ASSERT_TRUE(proc.submitAndStoreCommand(Command::Ptr(cmd1)));
            proc.beginUndoableGroup("outer");
            ASSERT_TRUE(proc.submitAndStoreCommand(Command::Ptr(cmd2)));
            proc.beginUndoableGroup("inner");
            ASSERT_TRUE(proc.submitAndStoreCommand(Command::Ptr(cmd3)));
            proc.closeGroup();
            ASSERT_TRUE(proc.submitAndStoreCommand(Command::Ptr(cmd4)));
            proc.closeGroup();
            
            ASSERT_EQ(String("outer"), proc.lastCommandName());
            ASSERT_TRUE(proc.undoLastCommand());
            ASSERT_TRUE(proc.hasLastCommand());
            ASSERT_TRUE(proc.hasNextCommand());
            ASSERT_EQ(cmd1->name(), proc.lastCommandName());
            ASSERT_EQ(String("outer"), proc.nextCommandName());
        }
    }
}
