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

#include "Exceptions.h"
#include "Controller/Command.h"
#include "Controller/CommandProcessor.h"
#include "TestUtils.h"

namespace TrenchBroom {
    namespace Controller {
        class TestCommand : public Command {
        private:
            bool m_doFailure;
            bool m_undoFailure;
            
            inline bool doPerformDo() {
                return !m_doFailure;
            }
            
            inline bool doPerformUndo() {
                return !m_undoFailure;
            }
        public:
            TestCommand(const String& name, const bool undoable = true, const bool doFailure = false, const bool undoFailure = false) :
            Command(name, undoable),
            m_doFailure(doFailure),
            m_undoFailure(undoFailure) {}
        };
        
        TEST(CommandProcessorTest, submitAndDontStoreCommand) {
            CommandProcessor proc;
            Command::Ptr cmd = Command::Ptr(new TestCommand("test"));
            
            ASSERT_TRUE(proc.submitCommand(cmd));
            ASSERT_FALSE(proc.hasLastCommand());
            ASSERT_FALSE(proc.hasNextCommand());
            ASSERT_THROW(proc.lastCommandName(), CommandProcessorException);
            ASSERT_THROW(proc.nextCommandName(), CommandProcessorException);
            ASSERT_THROW(proc.undoLastCommand(), CommandProcessorException);
            ASSERT_THROW(proc.redoNextCommand(), CommandProcessorException);
        }
        
        TEST(CommandProcessorTest, submitAndStoreCommand) {
            CommandProcessor proc;
            Command::Ptr cmd = Command::Ptr(new TestCommand("test"));
            
            ASSERT_TRUE(proc.submitAndStoreCommand(cmd));
            ASSERT_TRUE(proc.hasLastCommand());
            ASSERT_FALSE(proc.hasNextCommand());
            ASSERT_THROW(proc.nextCommandName(), CommandProcessorException);
            ASSERT_THROW(proc.redoNextCommand(), CommandProcessorException);
            ASSERT_EQ(cmd->name(), proc.lastCommandName());
        }

        TEST(CommandProcessorTest, submitAndStore2Commands) {
            CommandProcessor proc;
            Command::Ptr cmd1 = Command::Ptr(new TestCommand("test1"));
            Command::Ptr cmd2 = Command::Ptr(new TestCommand("test2"));
            
            ASSERT_TRUE(proc.submitAndStoreCommand(cmd1));
            ASSERT_TRUE(proc.submitAndStoreCommand(cmd2));
            ASSERT_TRUE(proc.hasLastCommand());
            ASSERT_FALSE(proc.hasNextCommand());
            ASSERT_THROW(proc.nextCommandName(), CommandProcessorException);
            ASSERT_THROW(proc.redoNextCommand(), CommandProcessorException);
            ASSERT_EQ(cmd2->name(), proc.lastCommandName());
        }
        
        TEST(CommandProcessorTest, undoLastCommand) {
            CommandProcessor proc;
            Command::Ptr cmd1 = Command::Ptr(new TestCommand("test1"));
            Command::Ptr cmd2 = Command::Ptr(new TestCommand("test2"));
            
            ASSERT_TRUE(proc.submitAndStoreCommand(cmd1));
            ASSERT_TRUE(proc.submitAndStoreCommand(cmd2));
            ASSERT_TRUE(proc.undoLastCommand());
            ASSERT_TRUE(proc.hasLastCommand());
            ASSERT_TRUE(proc.hasNextCommand());
            ASSERT_EQ(cmd1->name(), proc.lastCommandName());
            ASSERT_EQ(cmd2->name(), proc.nextCommandName());
        }
        
        TEST(CommandProcessorTest, undoTooManyCommands) {
            CommandProcessor proc;
            Command::Ptr cmd1 = Command::Ptr(new TestCommand("test1"));
            Command::Ptr cmd2 = Command::Ptr(new TestCommand("test2"));
            
            ASSERT_TRUE(proc.submitAndStoreCommand(cmd1));
            ASSERT_TRUE(proc.submitAndStoreCommand(cmd2));
            ASSERT_TRUE(proc.undoLastCommand());
            ASSERT_TRUE(proc.undoLastCommand());
            ASSERT_THROW(proc.undoLastCommand(), CommandProcessorException);
        }
        
        TEST(CommandProcessorTest, redoNextCommand) {
            CommandProcessor proc;
            Command::Ptr cmd1 = Command::Ptr(new TestCommand("test1"));
            Command::Ptr cmd2 = Command::Ptr(new TestCommand("test2"));
            
            ASSERT_TRUE(proc.submitAndStoreCommand(cmd1));
            ASSERT_TRUE(proc.submitAndStoreCommand(cmd2));
            ASSERT_TRUE(proc.undoLastCommand());
            ASSERT_TRUE(proc.redoNextCommand());
            ASSERT_TRUE(proc.hasLastCommand());
            ASSERT_FALSE(proc.hasNextCommand());
            ASSERT_EQ(cmd2->name(), proc.lastCommandName());
            ASSERT_THROW(proc.redoNextCommand(), CommandProcessorException);
        }
        
        TEST(CommandProcessorTest, redoTooManyCommands) {
            CommandProcessor proc;
            Command::Ptr cmd1 = Command::Ptr(new TestCommand("test1"));
            Command::Ptr cmd2 = Command::Ptr(new TestCommand("test2"));
            
            ASSERT_TRUE(proc.submitAndStoreCommand(cmd1));
            ASSERT_TRUE(proc.submitAndStoreCommand(cmd2));
            ASSERT_TRUE(proc.undoLastCommand());
            ASSERT_TRUE(proc.redoNextCommand());
            ASSERT_THROW(proc.redoNextCommand(), CommandProcessorException);
        }
        
        TEST(CommandProcessorTest, undoLastAndSubmitNewCommand) {
            CommandProcessor proc;
            Command::Ptr cmd1 = Command::Ptr(new TestCommand("test1"));
            Command::Ptr cmd2 = Command::Ptr(new TestCommand("test2"));
            Command::Ptr cmd3 = Command::Ptr(new TestCommand("test3"));
            
            ASSERT_TRUE(proc.submitAndStoreCommand(cmd1));
            ASSERT_TRUE(proc.submitAndStoreCommand(cmd2));
            ASSERT_TRUE(proc.undoLastCommand());
            ASSERT_TRUE(proc.submitAndStoreCommand(cmd3));
            ASSERT_EQ(cmd3->name(), proc.lastCommandName());
            ASSERT_EQ(1u, cmd2.use_count());
            ASSERT_TRUE(proc.hasLastCommand());
            ASSERT_FALSE(proc.hasNextCommand());
            ASSERT_THROW(proc.redoNextCommand(), CommandProcessorException);
        }

        TEST(CommandProcessorTest, submitOneShotCommand) {
            CommandProcessor proc;
            Command::Ptr cmd1 = Command::Ptr(new TestCommand("test1"));
            Command::Ptr cmd2 = Command::Ptr(new TestCommand("test2"));
            Command::Ptr cmd3 = Command::Ptr(new TestCommand("test3", false));
            
            ASSERT_TRUE(proc.submitAndStoreCommand(cmd1));
            ASSERT_TRUE(proc.submitAndStoreCommand(cmd2));
            ASSERT_TRUE(proc.undoLastCommand());
            ASSERT_TRUE(proc.submitAndStoreCommand(cmd3));

            ASSERT_FALSE(proc.hasLastCommand());
            ASSERT_FALSE(proc.hasNextCommand());
            ASSERT_THROW(proc.lastCommandName(), CommandProcessorException);
            ASSERT_THROW(proc.nextCommandName(), CommandProcessorException);
            ASSERT_THROW(proc.undoLastCommand(), CommandProcessorException);
            ASSERT_THROW(proc.redoNextCommand(), CommandProcessorException);
        }
        
        TEST(CommandProcessorTest, createCommandGroup) {
            CommandProcessor proc;
            Command::Ptr cmd1 = Command::Ptr(new TestCommand("test1"));
            Command::Ptr cmd2 = Command::Ptr(new TestCommand("test2"));
            Command::Ptr cmd3 = Command::Ptr(new TestCommand("test3"));
            
        }
    }
}
