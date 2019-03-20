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

#include "KeyboardShortcutEntry.h"

#include "PreferenceManager.h"
#include "View/ActionContext.h"
#include "View/KeyboardShortcut.h"

namespace TrenchBroom {
    namespace View {
        KeyboardShortcutEntry::KeyboardShortcutEntry() : m_hasConflicts(false) {}

        KeyboardShortcutEntry::KeyboardShortcutEntry(const KeyboardShortcutEntry& other) :
        m_hasConflicts(other.m_hasConflicts) {}

        KeyboardShortcutEntry& KeyboardShortcutEntry::operator=(const KeyboardShortcutEntry& other) {
            m_hasConflicts = other.m_hasConflicts;
            return *this;
        }

        KeyboardShortcutEntry::~KeyboardShortcutEntry() = default;

        bool KeyboardShortcutEntry::modifiable() const {
            return doGetModifiable();
        }

        bool KeyboardShortcutEntry::hasConflicts() const {
            return m_hasConflicts;
        }

        void KeyboardShortcutEntry::resetConflicts() {
            m_hasConflicts = false;
        }

        void KeyboardShortcutEntry::setHasConflicts() {
            m_hasConflicts = true;
        }

        int KeyboardShortcutEntry::actionContext() const {
            return doGetActionContext();
        }

        bool KeyboardShortcutEntry::appliesToContext(const int context) const {
            return (actionContext() & context) != 0;
        }

        QString KeyboardShortcutEntry::actionContextDescription() const {
            return actionContextName(actionContext());
        }

        QString KeyboardShortcutEntry::actionDescription() const {
            return doGetActionDescription();
        }

        QString KeyboardShortcutEntry::shortcutDescription() const {
            return shortcut().shortcutDisplayString();
        }

        QString KeyboardShortcutEntry::asJsonString() const {
            return doGetJsonString();
        }

        const IO::Path& KeyboardShortcutEntry::preferencePath() const {
            return doGetPreference().path();
        }

        const KeyboardShortcut& KeyboardShortcutEntry::shortcut() const {
            PreferenceManager& prefs = PreferenceManager::instance();
            return prefs.get(doGetPreference());
        }

        const KeyboardShortcut& KeyboardShortcutEntry::defaultShortcut() const {
            PreferenceManager& prefs = PreferenceManager::instance();
            return prefs.getDefault(doGetPreference());
        }

        bool KeyboardShortcutEntry::equals(const KeyboardShortcut& i_shortcut) const {
            return shortcut() == i_shortcut;
        }

        void KeyboardShortcutEntry::updateShortcut(const KeyboardShortcut& shortcut) {
            assert(modifiable());
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.set(doGetPreference(), shortcut);
        }

        wxAcceleratorEntry KeyboardShortcutEntry::acceleratorEntry(const ActionView view) const {
            return doGetAcceleratorEntry(view);
        }
    }
}
