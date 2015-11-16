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

#include "StringUtils.h"
#include "Preference.h"
#include "PreferenceManager.h"
#include "View/ActionContext.h"

namespace TrenchBroom {
    namespace View {
        ViewShortcut::ViewShortcut(const KeyboardShortcut& shortcut, const int context, const Action& action2D, const Action& action3D) :
        m_preference(path(action2D, action3D), shortcut),
        m_context(context) {
            m_actions[ActionView_Map2D] = action2D;
            m_actions[ActionView_Map3D] = action3D;
        }

        ViewShortcut::ViewShortcut(const KeyboardShortcut& shortcut, const int context, const Action& action) :
        m_preference(path(action, action), shortcut),
        m_context(context) {
            m_actions[ActionView_Map2D] = action;
            m_actions[ActionView_Map3D] = action;
        }

        void ViewShortcut::resetShortcut() {
        }

        int ViewShortcut::doGetActionContext() const {
            return m_context;
        }

        bool ViewShortcut::doGetModifiable() const {
            return true;
        }

        wxString ViewShortcut::doGetActionDescription() const {
            return buildDescription(m_actions[ActionView_Map2D], m_actions[ActionView_Map3D]);
        }
        
        wxString ViewShortcut::doGetJsonString() const {
            return shortcut().asJsonString();
        }

        const Preference<KeyboardShortcut>& ViewShortcut::doGetPreference() const {
            return m_preference;
        }
        

        const KeyboardShortcut& ViewShortcut::doGetShortcut() const {
            PreferenceManager& prefs = PreferenceManager::instance();
            return prefs.get(m_preference);
        }
        
        void ViewShortcut::doUpdateShortcut(const KeyboardShortcut& shortcut) {
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.set(m_preference, shortcut);
        }

        wxAcceleratorEntry ViewShortcut::doGetAcceleratorEntry(const ActionView view) const {
            const Action& action = m_actions[view];
            return shortcut().acceleratorEntry(action.id());
        }
        
        IO::Path ViewShortcut::path(const Action& action2D, const Action& action3D) const {
            return IO::Path("Controls/Map view") + IO::Path(buildDescription(action2D, action3D));
        }

        String ViewShortcut::buildDescription(const Action& action2D, const Action& action3D) const {
            if (action2D.id() == action3D.id() ||
                action3D.id() == wxID_NONE)
                return action2D.name();
            if (action2D.id() == wxID_NONE)
                return action3D.name();
            
            StringStream result;
            result << action2D.name() << "; " << action3D.name();
            return result.str();
        }
    }
}
