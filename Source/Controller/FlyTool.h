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

#ifndef __TrenchBroom__FlyTool__
#define __TrenchBroom__FlyTool__

#include "Controller/Tool.h"
#include "Utility/ExecutableEvent.h"

#include <wx/timer.h>

namespace TrenchBroom {
    namespace View {
        class EditorView;
    }
    
    namespace Controller {
        namespace MoveKey {
            typedef size_t Type;
            static const Type MKForward     = 0;
            static const Type MKBackward    = 1;
            static const Type MKLeft        = 2;
            static const Type MKRight       = 3;
        }

        class FlyTool : public Tool, public wxTimer, public ExecutableEvent::Executable {
        private:
            MoveKey::Type m_moveKeys[4];
            wxLongLong m_lastUpdateTime;
        protected:
            void execute();
        public:
            FlyTool(View::DocumentViewHolder& documentViewHolder, InputController& inputController);
            ~FlyTool();
            
            void Notify();
        };
    }
}

#endif /* defined(__TrenchBroom__FlyTool__) */
