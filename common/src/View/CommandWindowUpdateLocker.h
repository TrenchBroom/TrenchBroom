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

#ifndef CommandWindowUpdateLocker_h
#define CommandWindowUpdateLocker_h

#include "View/Command.h"
#include "View/NestedWindowUpdateLocker.h"
#include "View/UndoableCommand.h"
#include "View/ViewTypes.h"

class QWidget;

namespace TrenchBroom {
    namespace View {
        class CommandWindowUpdateLocker {
        private:
            NestedWindowUpdateLocker m_locker;
            MapDocumentWPtr m_document;
            bool m_bound;
        public:
            CommandWindowUpdateLocker(QWidget* window, MapDocumentWPtr document);
            ~CommandWindowUpdateLocker();
            
            void Start();
            void Stop();
        private:
            void bindObservers();
            void unbindObservers();
            
            void commandDo(Command::Ptr command);
            void commandDone(Command::Ptr command);
            void commandDoFailed(Command::Ptr command);
            
            void commandUndo(UndoableCommand::Ptr command);
            void commandUndone(UndoableCommand::Ptr command);
            void commandUndoFailed(UndoableCommand::Ptr command);
        };
    }
}

#endif /* CommandWindowUpdateLocker_h */
