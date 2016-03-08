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

#include "ExecutableEvent.h"

wxDEFINE_EVENT(EXECUTABLE_EVENT, TrenchBroom::View::ExecutableEvent);

namespace TrenchBroom {
    namespace View {
        IMPLEMENT_DYNAMIC_CLASS(ExecutableEvent, wxEvent)

        ExecutableEvent::Executable::~Executable() {}

        void ExecutableEvent::Executable::operator()() {
            execute();
        }

        ExecutableEvent::ExecutableEvent() {}

        ExecutableEvent::ExecutableEvent(Executable* executable) :
        wxEvent(wxID_ANY, EXECUTABLE_EVENT),
        m_executable(executable) {}

        ExecutableEvent::ExecutableEvent(Executable::Ptr sharedExecutable) :
        wxEvent(wxID_ANY, EXECUTABLE_EVENT),
        m_executable(sharedExecutable) {}

        wxEvent* ExecutableEvent::Clone() const {
            return new ExecutableEvent(*this);
        }

        void ExecutableEvent::execute() {
            if (m_executable != NULL)
                (*m_executable)();
        }
    }
}
