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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TrenchBroom_MousePreferencePane
#define TrenchBroom_MousePreferencePane

#include "Preferences.h"
#include "View/PreferencePane.h"

class QCheckBox;
class QSlider;

namespace TrenchBroom {
    namespace View {
        class KeyboardShortcut;
        class KeyboardShortcutEditor;
        class KeyboardShortcutEvent;

        class MousePreferencePane : public PreferencePane {
        private:
            QSlider* m_lookSpeedSlider;
            QCheckBox* m_invertLookHAxisCheckBox;
            QCheckBox* m_invertLookVAxisCheckBox;
            QSlider* m_panSpeedSlider;
            QCheckBox* m_invertPanHAxisCheckBox;
            QCheckBox* m_invertPanVAxisCheckBox;
            QSlider* m_moveSpeedSlider;
            QCheckBox* m_invertMouseWheelCheckBox;
            QCheckBox* m_enableAltMoveCheckBox;
            QCheckBox* m_invertAltMoveAxisCheckBox;
            QCheckBox* m_moveInCursorDirCheckBox;

            KeyboardShortcutEditor* m_forwardKeyEditor;
            KeyboardShortcutEditor* m_backwardKeyEditor;
            KeyboardShortcutEditor* m_leftKeyEditor;
            KeyboardShortcutEditor* m_rightKeyEditor;
            KeyboardShortcutEditor* m_upKeyEditor;
            KeyboardShortcutEditor* m_downKeyEditor;
            QSlider* m_flyMoveSpeedSlider;
        public:
            explicit MousePreferencePane(QWidget* parent = nullptr);

        private:
            void createGui();

            void bindEvents();

            bool doCanResetToDefaults() override;
            void doResetToDefaults() override;
            void doUpdateControls() override;
            bool doValidate() override;
        private slots:
            void lookSpeedChanged(int value);
            void invertLookHAxisChanged(int state);
            void invertLookVAxisChanged(int state);

            void panSpeedChanged(int value);
            void invertPanHAxisChanged(int state);
            void invertPanVAxisChanged(int state);

            void moveSpeedChanged(int value);
            void invertMouseWheelChanged(int state);
            void enableAltMoveChanged(int state);
            void invertAltMoveAxisChanged(int state);
            void moveInCursorDirChanged(int state);

            /* FIXME: keyboard shorcuts
            void forwardKeyChanged(KeyboardShortcutEvent& event);
            void backwardKeyChanged(KeyboardShortcutEvent& event);
            void leftKeyChanged(KeyboardShortcutEvent& event);
            void rightKeyChanged(KeyboardShortcutEvent& event);
            void upKeyChanged(KeyboardShortcutEvent& event);
            void downKeyChanged(KeyboardShortcutEvent& event);
             */

            void flyMoveSpeedChanged(int value);
        private:
            bool setShortcut(const KeyboardShortcut& shortcut, Preference<KeyboardShortcut>& preference);
            bool hasConflict(const KeyboardShortcut& shortcut, const Preference<KeyboardShortcut>& preference) const;
        };
    }
}

#endif /* defined(TrenchBroom_MousePreferencePane) */
