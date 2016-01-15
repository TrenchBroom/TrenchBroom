/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
#include "SetAny.h"

#include <wx/time.h>

#include <algorithm>

namespace TrenchBroom {
    namespace View {
        const Command::CommandType CommandGroup::Type = Command::freeType();
        
        CommandGroup::CommandGroup(const String& name, const CommandList& commands,
                                   Notifier1<Command::Ptr>& commandDoNotifier,
                                   Notifier1<Command::Ptr>& commandDoneNotifier,
                                   Notifier1<UndoableCommand::Ptr>& commandUndoNotifier,
                                   Notifier1<UndoableCommand::Ptr>& commandUndoneNotifier) :
        UndoableCommand(Type, name),
        m_commands(commands),
        m_commandDoNotifier(commandDoNotifier),
        m_commandDoneNotifier(commandDoneNotifier),
        m_commandUndoNotifier(commandUndoNotifier),
        m_commandUndoneNotifier(commandUndoneNotifier) {}
        
        bool CommandGroup::doPerformDo(MapDocumentCommandFacade* document) {
            CommandList::iterator it, end;
            for (it = m_commands.begin(), end = m_commands.end(); it != end; ++it) {
                UndoableCommand::Ptr command = *it;
                m_commandDoNotifier(command);
                if (!command->performDo(document))
                    throw CommandProcessorException("Partial failure while executing command group");
                m_commandDoneNotifier(command);
            }
            return true;
        }
        
        bool CommandGroup::doPerformUndo(MapDocumentCommandFacade* document) {
            CommandList::reverse_iterator it, end;
            for (it = m_commands.rbegin(), end = m_commands.rend(); it != end; ++it) {
                UndoableCommand::Ptr command = *it;
                m_commandUndoNotifier(command);
                if (!command->performUndo(document))
                    throw CommandProcessorException("Partial failure while undoing command group");
                m_commandUndoneNotifier(command);
            }
            return true;
        }
        
        bool CommandGroup::doIsRepeatDelimiter() const {
            CommandList::const_iterator it, end;
            for (it = m_commands.begin(), end = m_commands.end(); it != end; ++it) {
                UndoableCommand::Ptr command = *it;
                if (command->isRepeatDelimiter())
                    return true;
            }
            return false;
        }
        
        bool CommandGroup::doIsRepeatable(MapDocumentCommandFacade* document) const {
            CommandList::const_iterator it, end;
            for (it = m_commands.begin(), end = m_commands.end(); it != end; ++it) {
                UndoableCommand::Ptr command = *it;
                if (!command->isRepeatable(document))
                    return false;
            }
            return true;
        }
        
        UndoableCommand::Ptr CommandGroup::doRepeat(MapDocumentCommandFacade* document) const {
            CommandList clones;
            CommandList::const_iterator it, end;
            for (it = m_commands.begin(), end = m_commands.end(); it != end; ++it) {
                UndoableCommand::Ptr command = *it;
                assert(command->isRepeatable(document));
                UndoableCommand::Ptr clone = command->repeat(document);
                clones.push_back(clone);
            }
            return UndoableCommand::Ptr(new CommandGroup(name(), clones, m_commandDoNotifier, m_commandDoneNotifier, m_commandUndoNotifier, m_commandUndoneNotifier));
        }
        
        bool CommandGroup::doCollateWith(UndoableCommand::Ptr command) {
            return false;
        }
        
        const wxLongLong CommandProcessor::CollationInterval(1000);
        
        struct CommandProcessor::SubmitAndStoreResult {
            bool submitted;
            bool stored;
            
            SubmitAndStoreResult() :
            submitted(false),
            stored(false) {}
        };
        
        CommandProcessor::CommandProcessor(MapDocumentCommandFacade* document) :
        m_document(document),
        m_clearRepeatableCommandStack(false),
        m_lastCommandTimestamp(0),
        m_groupLevel(0) {
            assert(m_document != NULL);
        }
        
        bool CommandProcessor::hasLastCommand() const {
            return !m_lastCommandStack.empty();
        }
        
        bool CommandProcessor::hasNextCommand() const {
            return !m_nextCommandStack.empty();
        }
        
        const String& CommandProcessor::lastCommandName() const {
            if (!hasLastCommand())
                throw CommandProcessorException("Command stack is empty");
            return m_lastCommandStack.back()->name();
        }
        
        const String& CommandProcessor::nextCommandName() const {
            if (!hasNextCommand())
                throw CommandProcessorException("Undo stack is empty");
            return m_nextCommandStack.back()->name();
        }
        
        void CommandProcessor::beginGroup(const String& name) {
            if (m_groupLevel == 0)
                m_groupName = name;
            ++m_groupLevel;
        }
        
        void CommandProcessor::endGroup() {
            if (m_groupLevel == 0)
                throw CommandProcessorException("Group stack is empty");
            --m_groupLevel;
            if (m_groupLevel == 0)
                createAndStoreCommandGroup();
        }
        
        void CommandProcessor::rollbackGroup() {
            while (!m_groupedCommands.empty())
                popGroupedCommand()->performUndo(m_document);
        }
        
        bool CommandProcessor::submitCommand(Command::Ptr command) {
            const bool success = doCommand(command);
            if (!success)
                return false;
            
            m_lastCommandStack.clear();
            m_nextCommandStack.clear();
            return true;
        }
        
        bool CommandProcessor::submitAndStoreCommand(UndoableCommand::Ptr command) {
            const SubmitAndStoreResult result = submitAndStoreCommand(command, true);
            if (result.submitted) {
                if (result.stored && m_groupLevel == 0)
                    pushRepeatableCommand(command);
                return true;
            }
            return false;
        }
        
        bool CommandProcessor::undoLastCommand() {
            if (m_groupLevel > 0)
                throw CommandProcessorException("Cannot undo individual commands of a command group");
            
            UndoableCommand::Ptr command = popLastCommand();
            if (undoCommand(command)) {
                pushNextCommand(command);
                popLastRepeatableCommand(command);
                return true;
            }
            return false;
        }
        
        bool CommandProcessor::redoNextCommand() {
            if (m_groupLevel > 0)
                throw CommandProcessorException("Cannot redo while in a command group");
            
            UndoableCommand::Ptr command = popNextCommand();
            if (doCommand(command)) {
                if (pushLastCommand(command, false) && m_groupLevel == 0)
                    pushRepeatableCommand(command);
                return true;
            }
            return false;
        }
        
        bool CommandProcessor::repeatLastCommands() {
            CommandList commands;
            CommandStack::iterator it, end;
            for (it = m_repeatableCommandStack.begin(), end = m_repeatableCommandStack.end(); it != end; ++it) {
                UndoableCommand::Ptr command = *it;
                if (command->isRepeatable(m_document))
                    commands.push_back(UndoableCommand::Ptr(command->repeat(m_document)));
            }
            
            if (commands.empty())
                return false;
            
            StringStream name;
            name << "Repeat " << commands.size() << " Commands";
            
            UndoableCommand::Ptr repeatableCommand = UndoableCommand::Ptr(createCommandGroup(name.str(), commands));
            return submitAndStoreCommand(repeatableCommand, false).submitted;
        }
        
        void CommandProcessor::clearRepeatableCommands() {
            m_repeatableCommandStack.clear();
            m_clearRepeatableCommandStack = false;
        }
        
        CommandProcessor::SubmitAndStoreResult CommandProcessor::submitAndStoreCommand(UndoableCommand::Ptr command, const bool collate) {
            SubmitAndStoreResult result;
            result.submitted = doCommand(command);
            if (!result.submitted)
                return result;
            
            result.stored = storeCommand(command, collate);
            if (!m_nextCommandStack.empty())
                m_nextCommandStack.clear();
            return result;
        }
        
        bool CommandProcessor::doCommand(Command::Ptr command) {
            if (command->type() != CommandGroup::Type)
                commandDoNotifier(command);
            if (command->performDo(m_document)) {
                if (command->type() != CommandGroup::Type)
                    commandDoneNotifier(command);
                return true;
            }
            if (command->type() != CommandGroup::Type)
                commandDoFailedNotifier(command);
            return false;
        }
        
        bool CommandProcessor::undoCommand(UndoableCommand::Ptr command) {
            if (command->type() != CommandGroup::Type)
                commandUndoNotifier(command);
            if (command->performUndo(m_document)) {
                if (command->type() != CommandGroup::Type)
                    commandUndoneNotifier(command);
                return true;
            }
            if (command->type() != CommandGroup::Type)
                commandUndoFailedNotifier(command);
            return false;
        }
        
        bool CommandProcessor::storeCommand(UndoableCommand::Ptr command, const bool collate) {
            if (m_groupLevel == 0)
                return pushLastCommand(command, collate);
            return pushGroupedCommand(command, collate);
        }
        
        bool CommandProcessor::pushGroupedCommand(UndoableCommand::Ptr command, const bool collate) {
            assert(m_groupLevel > 0);
            if (!m_groupedCommands.empty()) {
                UndoableCommand::Ptr lastCommand = m_groupedCommands.back();
                if (collate && !lastCommand->collateWith(command)) {
                    m_groupedCommands.push_back(command);
                    return false;
                }
            } else {
                m_groupedCommands.push_back(command);
            }
            return true;
        }
        
        UndoableCommand::Ptr CommandProcessor::popGroupedCommand() {
            assert(m_groupLevel > 0);
            if (m_groupedCommands.empty())
                throw CommandProcessorException("Group command stack is empty");
            UndoableCommand::Ptr groupedCommand = m_groupedCommands.back();
            m_groupedCommands.pop_back();
            return groupedCommand;
        }
        
        void CommandProcessor::createAndStoreCommandGroup() {
            if (!m_groupedCommands.empty()) {
                if (m_groupName.empty())
                    m_groupName = m_groupedCommands.front()->name();
                UndoableCommand::Ptr group(createCommandGroup(m_groupName, m_groupedCommands));
                m_groupedCommands.clear();
                pushLastCommand(group, false);
                pushRepeatableCommand(group);
            }
            m_groupName = "";
        }
        
        UndoableCommand::Ptr CommandProcessor::createCommandGroup(const String& name, const CommandList& commands) {
            return UndoableCommand::Ptr(new CommandGroup(name, commands,
                                               commandDoNotifier,
                                               commandDoneNotifier,
                                               commandUndoNotifier,
                                               commandUndoneNotifier));
        }
        
        bool CommandProcessor::pushLastCommand(UndoableCommand::Ptr command, const bool collate) {
            assert(m_groupLevel == 0);
            
            const wxLongLong timestamp = ::wxGetLocalTimeMillis();
            const SetLate<wxLongLong> setLastCommandTimestamp(m_lastCommandTimestamp, timestamp);
            
            if (collatable(collate, timestamp)) {
                UndoableCommand::Ptr lastCommand = m_lastCommandStack.back();
                if (lastCommand->collateWith(command))
                    return false;
            }
            m_lastCommandStack.push_back(command);
            return true;
        }
        
        bool CommandProcessor::collatable(const bool collate, const wxLongLong timestamp) const {
            return collate && !m_lastCommandStack.empty() && timestamp - m_lastCommandTimestamp <= CollationInterval;
        }
        
        void CommandProcessor::pushNextCommand(UndoableCommand::Ptr command) {
            assert(m_groupLevel == 0);
            m_nextCommandStack.push_back(command);
        }
        
        void CommandProcessor::pushRepeatableCommand(UndoableCommand::Ptr command) {
            if (command->isRepeatDelimiter()) {
                m_clearRepeatableCommandStack = true;
            } else {
                if (m_clearRepeatableCommandStack) {
                    m_repeatableCommandStack.clear();
                    m_clearRepeatableCommandStack = false;
                }
                m_repeatableCommandStack.push_back(command);
            }
        }
        
        UndoableCommand::Ptr CommandProcessor::popLastCommand() {
            assert(m_groupLevel == 0);
            if (m_lastCommandStack.empty())
                throw CommandProcessorException("Command stack is empty");
            UndoableCommand::Ptr lastCommand = m_lastCommandStack.back();
            m_lastCommandStack.pop_back();
            return lastCommand;
        }
        
        UndoableCommand::Ptr CommandProcessor::popNextCommand() {
            assert(m_groupLevel == 0);
            if (m_nextCommandStack.empty())
                throw CommandProcessorException("Command stack is empty");
            UndoableCommand::Ptr nextCommand = m_nextCommandStack.back();
            m_nextCommandStack.pop_back();
            return nextCommand;
        }
        
        void CommandProcessor::popLastRepeatableCommand(UndoableCommand::Ptr command) {
            if (!m_repeatableCommandStack.empty() && m_repeatableCommandStack.back() == command)
                m_repeatableCommandStack.pop_back();
        }
    }
}
