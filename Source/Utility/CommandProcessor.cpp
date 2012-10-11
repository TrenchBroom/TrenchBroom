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
wxCommandProcessor(maxCommandLevel),
m_groupLevel(0),
m_compoundCommand(NULL) {}

void CommandProcessor::BeginGroup(wxCommandProcessor* wxCommandProc, const wxString& name) {
    CommandProcessor* commandProc = static_cast<CommandProcessor*>(wxCommandProc);
    commandProc->BeginGroup(name);
}

void CommandProcessor::EndGroup(wxCommandProcessor* wxCommandProc) {
    CommandProcessor* commandProc = static_cast<CommandProcessor*>(wxCommandProc);
    commandProc->EndGroup();
}

void CommandProcessor::BeginGroup(const wxString& name) {
    if (m_groupLevel == 0) {
        assert(m_compoundCommand == NULL);
        m_compoundCommand = new CompoundCommand(name);
    }
    m_groupLevel++;
}

void CommandProcessor::EndGroup() {
    m_groupLevel--;
    if (m_groupLevel == 0) {
        assert(m_compoundCommand != NULL);
        wxCommandProcessor::Store(m_compoundCommand);
        m_compoundCommand = NULL;
    }
}

bool CommandProcessor::Submit(wxCommand* command, bool storeIt) {
    if (m_groupLevel == 0)
        return wxCommandProcessor::Submit(command, storeIt);

    assert(m_compoundCommand != NULL);
    bool result = DoCommand(*command);
    if (result && storeIt)
        m_compoundCommand->addCommand(command);
    return result;
}
