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

#pragma once

#include <QWidget>

namespace TrenchBroom {
    namespace View {
        class PreferencePane : public QWidget {
        public:
            explicit PreferencePane(QWidget* parent = nullptr);
            ~PreferencePane() override;

            bool canResetToDefaults();
            void resetToDefaults();
            void updateControls();
            /**
             * Returns whether the settings in the preference pane are valid to save.
             * If the aren't, it also displays an error dialog box asking the user to correct the issues.
             */
            bool validate();
        private:
            virtual bool doCanResetToDefaults() = 0;
            virtual void doResetToDefaults() = 0;
            virtual void doUpdateControls() = 0;
            virtual bool doValidate() = 0;
        };
    }
}

