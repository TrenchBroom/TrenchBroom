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

#ifndef __TrenchBroom__ExecutableEvent__
#define __TrenchBroom__ExecutableEvent__

#include "SharedPointer.h"
#include <wx/event.h>

namespace TrenchBroom {
    namespace View {
        class ExecutableEvent : public wxEvent {
        public:
            class Executable {
            public:
                typedef std::tr1::shared_ptr<Executable> Ptr;
            public:
                virtual ~Executable();
                void operator()();
            private:
                virtual void execute() = 0;
            };
        private:
            Executable::Ptr m_sharedExecutable;
            Executable* m_executable;
            DECLARE_DYNAMIC_CLASS(ExecutableEvent)
        public:
            ExecutableEvent();
            ExecutableEvent(Executable* executable);
            ExecutableEvent(Executable::Ptr executable);
            
            void execute();
            
            virtual wxEvent* Clone() const;
        };
    }
}


#define WXDLLIMPEXP_CUSTOM_EVENT

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_CUSTOM_EVENT, EVT_EXECUTABLE_EVENT, 1)
END_DECLARE_EVENT_TYPES()

typedef void (wxEvtHandler::*executableEventFunction)(TrenchBroom::View::ExecutableEvent&);

#define EVT_EXECUTABLE_EVENT_HANDLER(func) \
    (wxObjectEventFunction) \
    (executableEventFunction) & func

#define EVT_EXECUTABLE(func) \
    DECLARE_EVENT_TABLE_ENTRY( EVT_EXECUTABLE_EVENT, \
        id, \
        wxID_ANY, \
        (wxObjectEventFunction) \
        (executableEventFunction) & func, \
        (wxObject *) NULL),

#endif /* defined(__TrenchBroom__ExecutableEvent__) */
