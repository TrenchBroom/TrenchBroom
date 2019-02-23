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

#include "ViewShortcut.h"

#include "PreferenceManager.h"

namespace TrenchBroom {
    namespace View {
        ViewShortcut::ViewKeyboardShortcutEntry::ViewKeyboardShortcutEntry(ViewShortcut& viewShortcut) :
        m_viewShortcut(viewShortcut) {}

        int ViewShortcut::ViewKeyboardShortcutEntry::doGetActionContext() const {
            return m_viewShortcut.m_context;
        }

        bool ViewShortcut::ViewKeyboardShortcutEntry::doGetModifiable() const {
            return true;
        }

        wxString ViewShortcut::ViewKeyboardShortcutEntry::doGetActionDescription() const {
            return m_viewShortcut.buildDescription(
                m_viewShortcut.m_actions[ActionView_Map2D],
                m_viewShortcut.m_actions[ActionView_Map3D]);
        }

        wxString ViewShortcut::ViewKeyboardShortcutEntry::doGetJsonString() const {
            return defaultShortcut().asJsonString();
        }

        const Preference<KeyboardShortcut>& ViewShortcut::ViewKeyboardShortcutEntry::doGetPreference() const {
            return m_viewShortcut.m_preference;
        }

        Preference<KeyboardShortcut>& ViewShortcut::ViewKeyboardShortcutEntry::doGetPreference() {
            return m_viewShortcut.m_preference;
        }

        wxAcceleratorEntry ViewShortcut::ViewKeyboardShortcutEntry::doGetAcceleratorEntry(ActionView view) const {
            const Action& action = m_viewShortcut.m_actions[view];
            return shortcut().acceleratorEntry(action.id());
        }

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
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.resetToDefault(m_preference);
        }

        std::unique_ptr<ViewShortcut::ViewKeyboardShortcutEntry> ViewShortcut::shortcutEntry() {
            return std::make_unique<ViewKeyboardShortcutEntry>(*this);
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
