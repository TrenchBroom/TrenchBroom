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

#ifndef TrenchBroom_ViewShortcut
#define TrenchBroom_ViewShortcut

#include "Preference.h"
#include "View/Action.h"
#include "View/ActionContext.h"
#include "View/KeyboardShortcut.h"
#include "View/KeyboardShortcutEntry.h"

#include <wx/accel.h>

#include <memory>
#include <vector>

namespace TrenchBroom {
    namespace View {
        class ViewShortcut {
        public:
            using List = std::vector<ViewShortcut>;

            class ViewKeyboardShortcutEntry : public KeyboardShortcutEntry {
            private:
                ViewShortcut& m_viewShortcut;
            public:
                explicit ViewKeyboardShortcutEntry(ViewShortcut& viewShortcut);
            private: // implement KeyboardShortcutEntry interface
                int doGetActionContext() const override;
                bool doGetModifiable() const override;
                wxString doGetActionDescription() const override;
                wxString doGetJsonString() const override;
                const Preference<KeyboardShortcut>& doGetPreference() const override;
                Preference<KeyboardShortcut>& doGetPreference() override;
                wxAcceleratorEntry doGetAcceleratorEntry(ActionView view) const override;
            };
        private:
            Preference<KeyboardShortcut> m_preference;
            int m_context;
            Action m_actions[NumActionViews];
        public:
            ViewShortcut(const KeyboardShortcut& shortcut, int context, const Action& action2D, const Action& action3D);
            ViewShortcut(const KeyboardShortcut& shortcut, int context, const Action& action);

            bool hasShortcut() const;
            bool appliesToContext(int context) const;
            wxAcceleratorEntry acceleratorEntry(ActionView view) const;

            void resetShortcut();
            std::unique_ptr<ViewKeyboardShortcutEntry> shortcutEntry();
        private:
            IO::Path path(const Action& action2D, const Action& action3D) const;
            String buildDescription(const Action& action2D, const Action& action3D) const;
        };
    }
}

#endif /* defined(TrenchBroom_ViewShortcut) */
