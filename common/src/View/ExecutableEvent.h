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

#ifndef TrenchBroom_ExecutableEvent
#define TrenchBroom_ExecutableEvent

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
            Executable::Ptr m_executable;
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

typedef void (wxEvtHandler::*ExecutableEventFunction)(TrenchBroom::View::ExecutableEvent &);

wxDECLARE_EVENT(EXECUTABLE_EVENT, TrenchBroom::View::ExecutableEvent);
#define ExecutableEventHandler(func) wxEVENT_HANDLER_CAST(ExecutableEventFunction, func)

#endif /* defined(TrenchBroom_ExecutableEvent) */
