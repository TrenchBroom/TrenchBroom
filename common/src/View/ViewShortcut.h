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

#ifndef __TrenchBroom__ViewShortcut__
#define __TrenchBroom__ViewShortcut__

#include "View/ActionContext.h"
#include "View/KeyboardShortcut.h"

#include <wx/accel.h>

#include <vector>

namespace TrenchBroom {
    namespace View {
        class ViewShortcut {
        public:
            typedef std::vector<ViewShortcut> List;
        private:
            KeyboardShortcut m_shortcut;
            int m_context;
            int m_actions[NumActionViews];
        public:
            ViewShortcut();
            ViewShortcut(const KeyboardShortcut& shortcut, int context, int action2D, int action3D);
            
            wxAcceleratorEntry acceleratorEntry(ActionView view) const;
            bool appliesToContext(int context) const;
        };
    }
}

#endif /* defined(__TrenchBroom__ViewShortcut__) */
