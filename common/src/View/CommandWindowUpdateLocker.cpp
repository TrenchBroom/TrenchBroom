/*
 Copyright (C) 2010-2017 Kristian Duske
 
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

#include "CommandWindowUpdateLocker.h"

#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace View {
        CommandWindowUpdateLocker::CommandWindowUpdateLocker(wxWindow* window, MapDocumentWPtr document) :
        m_locker(window),
        m_document(document),
        m_bound(false) {}

        CommandWindowUpdateLocker::~CommandWindowUpdateLocker() {
            if (m_bound)
                Stop();
        }

        void CommandWindowUpdateLocker::Start() {
            bindObservers();
        }
        
        void CommandWindowUpdateLocker::Stop() {
            unbindObservers();
        }
        
        void CommandWindowUpdateLocker::bindObservers() {
            assert(!m_bound);
            
            MapDocumentSPtr document = lock(m_document);

            document->commandDoNotifier.addObserver(this, &CommandWindowUpdateLocker::commandDo);
            document->commandDoneNotifier.addObserver(this, &CommandWindowUpdateLocker::commandDone);
            document->commandDoFailedNotifier.addObserver(this, &CommandWindowUpdateLocker::commandDoFailed);
            
            document->commandUndoNotifier.addObserver(this, &CommandWindowUpdateLocker::commandUndo);
            document->commandUndoneNotifier.addObserver(this, &CommandWindowUpdateLocker::commandUndone);
            document->commandUndoFailedNotifier.addObserver(this, &CommandWindowUpdateLocker::commandUndoFailed);
            
            m_bound = true;
        }
        
        void CommandWindowUpdateLocker::unbindObservers() {
            assert(m_bound);
            
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);

                document->commandDoNotifier.removeObserver(this, &CommandWindowUpdateLocker::commandDo);
                document->commandDoneNotifier.removeObserver(this, &CommandWindowUpdateLocker::commandDone);
                document->commandDoFailedNotifier.removeObserver(this, &CommandWindowUpdateLocker::commandDoFailed);
                
                document->commandUndoNotifier.removeObserver(this, &CommandWindowUpdateLocker::commandUndo);
                document->commandUndoneNotifier.removeObserver(this, &CommandWindowUpdateLocker::commandUndone);
                document->commandUndoFailedNotifier.removeObserver(this, &CommandWindowUpdateLocker::commandUndoFailed);
            }
            
            m_bound = false;
        }

        void CommandWindowUpdateLocker::commandDo(Command::Ptr command) {
            m_locker.Freeze();
        }
        
        void CommandWindowUpdateLocker::commandDone(Command::Ptr command) {
            m_locker.Thaw();
        }
        
        void CommandWindowUpdateLocker::commandDoFailed(Command::Ptr command) {
            m_locker.Reset();
        }
        
        void CommandWindowUpdateLocker::commandUndo(UndoableCommand::Ptr command) {
            m_locker.Freeze();
        }
        
        void CommandWindowUpdateLocker::commandUndone(UndoableCommand::Ptr command) {
            m_locker.Thaw();
        }
        
        void CommandWindowUpdateLocker::commandUndoFailed(UndoableCommand::Ptr command) {
            m_locker.Reset();
        }
    }
}
