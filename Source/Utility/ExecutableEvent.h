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

#ifndef TrenchBroom_ExecutableEvent_h
#define TrenchBroom_ExecutableEvent_h

#include <wx/event.h>

#include "Utility/SharedPointer.h"

namespace TrenchBroom {
    class ExecutableEvent : public wxEvent {
    public:
        class Executable {
        public:
            typedef std::tr1::shared_ptr<Executable> Ptr;
        protected:
            virtual void execute() = 0;
        public:
            virtual ~Executable() {}
            
            inline void operator()() {
                execute();
            }
        };
    private:
        Executable::Ptr m_sharedExecutable;
        Executable* m_executable;
        DECLARE_DYNAMIC_CLASS(ExecutableEvent);
    public:
        ExecutableEvent() {}
        ExecutableEvent(Executable* executable);
        ExecutableEvent(Executable::Ptr executable);
        
        inline void execute() {
            if (m_executable != NULL)
                (*m_executable)();
            else
                (*m_sharedExecutable)();
        }

        virtual wxEvent* Clone() const;
    };
}


#define WXDLLIMPEXP_CUSTOM_EVENT

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_CUSTOM_EVENT, EVT_EXECUTABLE_EVENT, 1)
END_DECLARE_EVENT_TYPES()

typedef void (wxEvtHandler::*executableEventFunction)(TrenchBroom::ExecutableEvent&);

#define EVT_EXECUTABLE(func) \
    DECLARE_EVENT_TABLE_ENTRY( EVT_EXECUTABLE_EVENT, \
        wxID_ANY, \
        wxID_ANY, \
        (wxObjectEventFunction) \
        (executableEventFunction) & func, \
        (wxObject *) NULL),

#endif
