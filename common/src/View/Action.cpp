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

#include "Action.h"

#include "PreferenceManager.h"

#include "Exceptions.h"

namespace TrenchBroom {
    namespace View {
        String Action::makeContextName(const int context) {
            if (context == Context_Any)
                return "Any";
            
            StringList contexts;
            if (context & Context_VertexTool)
                contexts.push_back("Vertex Tool");
            if (context & Context_ClipTool)
                contexts.push_back("Clip Tool");
            if (context & Context_RotateTool)
                contexts.push_back("Rotate Tool");
            if (context & Context_ObjectSelection)
                contexts.push_back("Objects");
            if (context & Context_FaceSelection)
                contexts.push_back("Textures");
            return StringUtils::join(contexts, ", ");
        }
        
        Action::Action(int id, int context, const String& name, const IO::Path& preferencePath, const KeyboardShortcut& defaultShortcut, bool modifiable) :
        m_id(id),
        m_context(context),
        m_name(name),
        m_preference(preferencePath, defaultShortcut),
        m_modifiable(modifiable) {}

        int Action::id() const {
            return m_id;
        }
        
        const String& Action::name() const {
            return m_name;
        }
        
        String Action::displayName() const {
            return m_preference.path().asString(" > ");
        }
        
        String Action::contextName() const {
            return makeContextName(m_context);
        }

        bool Action::modifiable() const {
            return m_modifiable;
        }

        wxAcceleratorEntry Action::acceleratorEntry() const {
            return shortcut().acceleratorEntry(m_id);
        }

        bool Action::appliesToContext(const int context) const {
            return (context & m_context) != 0;
        }

        wxString Action::shortcutMenuString() const {
            return shortcut().shortcutMenuString();
        }
        
        wxString Action::shortcutDisplayString() const {
            return shortcut().shortcutDisplayString();
        }
        
        wxString Action::menuItemString(const wxString& suffix) const {
            wxString caption;
            caption << m_name;
            if (!suffix.empty())
                caption << " " << suffix;
            return shortcut().shortcutMenuItemString(caption);
        }
        
        void Action::updateShortcut(const KeyboardShortcut& shortcut) {
            assert(m_modifiable);
            if (!m_modifiable)
                throw Exception(m_name + " is not modifiable");
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.set(m_preference, shortcut);
        }
        
        bool Action::conflictsWith(const Action& action) const {
            if (m_id == action.m_id)
                return false;
            if ((m_context & action.m_context) == 0)
                return false;
            
            const KeyboardShortcut& mine = shortcut();
            const KeyboardShortcut& theirs = action.shortcut();

            return (mine.hasKey() && theirs.hasKey() &&
                    shortcut() == action.shortcut());
        }

        const KeyboardShortcut& Action::shortcut() const {
            PreferenceManager& prefs = PreferenceManager::instance();
            return prefs.get(m_preference);
        }
    }
}
