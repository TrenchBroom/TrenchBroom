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

#ifndef __TrenchBroom__Action__
#define __TrenchBroom__Action__

#include "Preference.h"
#include "StringUtils.h"
#include "View/KeyboardShortcut.h"

#include <wx/accel.h>
#include <wx/string.h>

#include <vector>

namespace TrenchBroom {
    namespace IO {
        class Path;
    }
    
    namespace View {
        class MenuAction {
        public:
            typedef std::vector<MenuAction> List;
        private:
            int m_id;
            int m_context;
            String m_name;
            Preference<KeyboardShortcut> m_preference;
            bool m_modifiable;
        public:
            MenuAction(int id, int context, const String& name, const IO::Path& preferencePath, const KeyboardShortcut& defaultShortcut, bool modifiable);
            
            int id() const;
            const String& name() const;
            String displayName() const;
            String contextName() const;
            
            bool modifiable() const;
            bool hasShortcut(const KeyboardShortcut& shortcut) const;
            
            wxAcceleratorEntry acceleratorEntry() const;
            bool appliesToContext(int context) const;
            
            wxString shortcutMenuString() const;
            wxString shortcutDisplayString() const;
            
            wxString menuItemString(const wxString& suffix = "") const;

            void updateShortcut(const KeyboardShortcut& shortcut);
            void resetShortcut();
            bool conflictsWith(const MenuAction& action) const;
        private:
            const KeyboardShortcut& shortcut() const;
        };
    }
}

#endif /* defined(__TrenchBroom__Action__) */
