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

#include "MenuAction.h"
#include "ActionContext.h"

#include "PreferenceManager.h"

#include "Exceptions.h"

namespace TrenchBroom {
    namespace View {
        MenuAction::MenuAction(const int id, const String& name, const IO::Path& preferencePath, const KeyboardShortcut& defaultShortcut, const bool modifiable) :
        m_id(id),
        m_name(name),
        m_preference(preferencePath, defaultShortcut),
        m_modifiable(modifiable) {}

        int MenuAction::id() const {
            return m_id;
        }
        
        const String& MenuAction::name() const {
            return m_name;
        }
        
        String MenuAction::displayName() const {
            return m_preference.path().asString(" > ");
        }

        bool MenuAction::modifiable() const {
            return m_modifiable;
        }

        bool MenuAction::hasShortcut(const KeyboardShortcut& shortcut) const {
            return MenuAction::shortcut() == shortcut;
        }

        wxString MenuAction::shortcutMenuString() const {
            return shortcut().shortcutMenuString();
        }
        
        wxString MenuAction::shortcutDisplayString() const {
            return shortcut().shortcutDisplayString();
        }
        
        wxString MenuAction::menuItemString(const wxString& suffix) const {
            wxString caption;
            caption << m_name;
            if (!suffix.empty())
                caption << " " << suffix;
            return shortcut().shortcutMenuItemString(caption);
        }
        
        void MenuAction::updateShortcut(const KeyboardShortcut& shortcut) {
            assert(m_modifiable);
            if (!m_modifiable)
                throw Exception(m_name + " is not modifiable");
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.set(m_preference, shortcut);
        }
        
        void MenuAction::resetShortcut() {
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.resetToDefault(m_preference);
        }

        bool MenuAction::conflictsWith(const MenuAction& action) const {
            if (m_id == action.m_id)
                return false;
            
            const KeyboardShortcut& mine = shortcut();
            const KeyboardShortcut& theirs = action.shortcut();

            return (mine.hasKey() && theirs.hasKey() &&
                    shortcut() == action.shortcut());
        }

        const KeyboardShortcut& MenuAction::shortcut() const {
            PreferenceManager& prefs = PreferenceManager::instance();
            return prefs.get(m_preference);
        }
    }
}
