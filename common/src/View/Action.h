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
        class Action {
        public:
            typedef enum {
                Context_Default         = 1 << 1,
                Context_VertexTool      = 1 << 2,
                Context_ClipTool        = 1 << 3,
                Context_RotateTool      = 1 << 4,
                Context_FlyMode         = 1 << 5,
                Context_ObjectSelection = 1 << 6,
                Context_FaceSelection   = 1 << 7,
                Context_AnyTool         = Context_VertexTool | Context_ClipTool | Context_RotateTool,
                Context_Any             = Context_Default | Context_VertexTool | Context_ClipTool | Context_RotateTool | Context_ObjectSelection | Context_FaceSelection
            } Context;
            
            static String makeContextName(int context);
            
            typedef std::vector<Action> List;
        private:
            int m_id;
            int m_context;
            String m_name;
            Preference<KeyboardShortcut> m_preference;
            bool m_modifiable;
            bool m_requiresModifiers;
        public:
            Action(int id, int context, const String& name, const IO::Path& preferencePath, const KeyboardShortcut& defaultShortcut, bool modifiable, bool requiresModifiers);
            
            int id() const;
            const String& name() const;
            String displayName() const;
            String contextName() const;
            bool modifiable() const;
            bool requiresModifiers() const;

            wxAcceleratorEntry acceleratorEntry() const;
            bool appliesToContext(int context) const;
            
            wxString shortcutMenuString() const;
            wxString shortcutDisplayString() const;
            
            wxString menuItemString(const wxString& suffix = "") const;

            void updateShortcut(const KeyboardShortcut& shortcut);
            bool conflictsWith(const Action& action) const;
        private:
            const KeyboardShortcut& shortcut() const;
        };
    }
}

#endif /* defined(__TrenchBroom__Action__) */
