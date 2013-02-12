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

#ifndef __TrenchBroom__CommandProcessor__
#define __TrenchBroom__CommandProcessor__

#include <wx/cmdproc.h>

#include <stack>
#include <vector>

typedef std::vector<wxCommand*> CommandList;

class CompoundCommand : public wxCommand {
protected:
    CommandList m_commands;
public:
    CompoundCommand(const wxString& name);
    ~CompoundCommand();
    
    void addCommand(wxCommand* command);
    void removeCommand(wxCommand* command);
    bool empty() const;
    void clear();
    
    bool Do();
    bool Undo();
};

class CommandProcessor : public wxCommandProcessor {
protected:
    typedef std::stack<CompoundCommand*> GroupStack;

    GroupStack m_groupStack;
    wxCommand* m_block;
public:
    CommandProcessor(int maxCommandLevel = -1);

    static void BeginGroup(wxCommandProcessor* wxCommandProc, const wxString& name);
    static void EndGroup(wxCommandProcessor* wxCommandProc);
    static void RollbackGroup(wxCommandProcessor* wxCommandProc);
    static void DiscardGroup(wxCommandProcessor* wxCommandProc);
    static void Block(wxCommandProcessor* wxCommandProc);
    static void Unblock(wxCommandProcessor* wxCommandProc);
    
    void Block();
    void Unblock();
    bool CanUndo() const;
    
    void BeginGroup(const wxString& name);
    void EndGroup();
    void RollbackGroup();
    void DiscardGroup();
    bool Submit(wxCommand* command, bool storeIt = true);
};

#endif /* defined(__TrenchBroom__CommandProcessor__) */
