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

#include "CommandProcessor.h"

#include "Exceptions.h"

namespace TrenchBroom {
    namespace Controller {
        bool CommandProcessor::hasLastCommand() const {
            return !m_lastCommandStack.empty();
        }
        
        bool CommandProcessor::hasNextCommand() const {
            return !m_nextCommandStack.empty();
        }

        const String& CommandProcessor::lastCommandName() const {
            if (m_lastCommandStack.empty())
                throw CommandProcessorException("Command stack is empty");
            return m_lastCommandStack.back()->name();
        }
        
        const String& CommandProcessor::nextCommandName() const {
            if (m_nextCommandStack.empty())
                throw CommandProcessorException("Undo stack is empty");
            return m_nextCommandStack.back()->name();
        }

        bool CommandProcessor::submitCommand(Command::Ptr command) {
            if (command->execute()) {
                if (!command->canRollback()) {
                    m_lastCommandStack.clear();
                    m_nextCommandStack.clear();
                }
                return true;
            }
            return false;
        }
        
        bool CommandProcessor::submitAndStoreCommand(Command::Ptr command) {
            if (!submitCommand(command))
                return false;
            if (command->canRollback())
                m_lastCommandStack.push_back(command);
            if (!m_nextCommandStack.empty())
                m_nextCommandStack.clear();
            return true;
        }

        bool CommandProcessor::undoLastCommand() {
            if (m_lastCommandStack.empty())
                throw CommandProcessorException("Command stack is empty");
        
            Command::Ptr command = m_lastCommandStack.back();
            m_lastCommandStack.pop_back();
            
            if (command->rollback()) {
                m_nextCommandStack.push_back(command);
                return true;
            }
            return false;
        }
        
        bool CommandProcessor::redoNextCommand() {
            if (m_nextCommandStack.empty())
                throw CommandProcessorException("Undo stack is empty");
            
            Command::Ptr command = m_nextCommandStack.back();
            m_nextCommandStack.pop_back();
            
            if (command->execute()) {
                m_lastCommandStack.push_back(command);
                return true;
            }
            return false;
        }
    }
}
