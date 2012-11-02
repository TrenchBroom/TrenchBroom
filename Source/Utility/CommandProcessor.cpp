/*
 Copyright (C) 2010-2012 Kristian Duske
 
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

#include <algorithm>
#include <cassert>

CompoundCommand::CompoundCommand(const wxString& name) :
wxCommand(true, name) {}

CompoundCommand::~CompoundCommand() {
    CommandList::iterator it, end;
    for (it = m_commands.begin(), end = m_commands.end(); it != end; ++it) {
        wxCommand* command = *it;
        wxDELETE(command);
    }
    m_commands.clear();
}

void CompoundCommand::addCommand(wxCommand* command) {
    m_commands.push_back(command);
}

void CompoundCommand::removeCommand(wxCommand* command) {
    m_commands.erase(std::remove(m_commands.begin(), m_commands.end(), command), m_commands.end());
}

bool CompoundCommand::Do() {
    CommandList::iterator it, end;
    for (it = m_commands.begin(), end = m_commands.end(); it != end; ++it) {
        wxCommand* command = *it;
        command->Do();
    }
    
    return true;
}

bool CompoundCommand::Undo() {
    CommandList::reverse_iterator it, end;
    for (it = m_commands.rbegin(), end = m_commands.rend(); it != end; ++it) {
        wxCommand* command = *it;
        command->Undo();
    }
    
    return true;
}

CommandProcessor::CommandProcessor(int maxCommandLevel) :
wxCommandProcessor(maxCommandLevel) {}

void CommandProcessor::BeginGroup(wxCommandProcessor* wxCommandProc, const wxString& name) {
    CommandProcessor* commandProc = static_cast<CommandProcessor*>(wxCommandProc);
    commandProc->BeginGroup(name);
}

void CommandProcessor::EndGroup(wxCommandProcessor* wxCommandProc) {
    CommandProcessor* commandProc = static_cast<CommandProcessor*>(wxCommandProc);
    commandProc->EndGroup();
}

void CommandProcessor::RollbackGroup(wxCommandProcessor* wxCommandProc) {
    CommandProcessor* commandProc = static_cast<CommandProcessor*>(wxCommandProc);
    commandProc->RollbackGroup();
}

void CommandProcessor::DiscardGroup(wxCommandProcessor* wxCommandProc) {
    CommandProcessor* commandProc = static_cast<CommandProcessor*>(wxCommandProc);
    commandProc->DiscardGroup();
}

void CommandProcessor::BeginGroup(const wxString& name) {
    CompoundCommand* parentGroup = m_groupStack.empty() ? NULL : m_groupStack.top();
    CompoundCommand* newGroup = new CompoundCommand(name);
    m_groupStack.push(newGroup);
    if (parentGroup != NULL)
        parentGroup->addCommand(newGroup);
}

void CommandProcessor::EndGroup() {
    assert(!m_groupStack.empty());
    
    CompoundCommand* group = m_groupStack.top();
    m_groupStack.pop();
    if (m_groupStack.empty())
        wxCommandProcessor::Store(group);
}

void CommandProcessor::RollbackGroup() {
    assert(!m_groupStack.empty());

    CompoundCommand* group = m_groupStack.top();
    UndoCommand(*group);
}

void CommandProcessor::DiscardGroup() {
    assert(!m_groupStack.empty());
    
    CompoundCommand* group = m_groupStack.top();
    m_groupStack.pop();
    if (!m_groupStack.empty())
        m_groupStack.top()->removeCommand(group);
    
    delete group;
}

bool CommandProcessor::Submit(wxCommand* command, bool storeIt) {
    if (m_groupStack.empty())
        return wxCommandProcessor::Submit(command, storeIt);

    bool result = DoCommand(*command);
    if (result && storeIt)
        m_groupStack.top()->addCommand(command);
    return result;
}
