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

#include "ViewShortcut.h"

#include "View/ActionContext.h"

namespace TrenchBroom {
    namespace View {
        ViewShortcut::ViewShortcut() :
        m_shortcut(),
        m_context(ActionContext_Default) {
            for (size_t i = 0; i < NumActionViews; ++i)
                m_actions[i] = wxID_NONE;
        }

        ViewShortcut::ViewShortcut(const KeyboardShortcut& shortcut, const int context, const int action2D, const int action3D) :
        m_shortcut(shortcut),
        m_context(context) {
            m_actions[ActionView_Map2D] = action2D;
            m_actions[ActionView_Map3D] = action3D;
        }

        wxAcceleratorEntry ViewShortcut::acceleratorEntry(const ActionView view) const {
            return m_shortcut.acceleratorEntry(m_actions[view]);
        }
        
        bool ViewShortcut::appliesToContext(const int context) const {
            return (context & m_context) != 0;
        }
    }
}
