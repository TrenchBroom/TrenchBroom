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

#pragma once

#include "View/PreferencePane.h"

class QWidget;
class QTableView;
class QSortFilterProxyModel;

namespace TrenchBroom {
    namespace View {
        class KeyboardShortcutModel;
        class MapDocument;

        class KeyboardPreferencePane : public PreferencePane {
            Q_OBJECT
        private:
            QTableView* m_table;
            KeyboardShortcutModel* m_model;
            QSortFilterProxyModel* m_proxy;
        public:
            explicit KeyboardPreferencePane(MapDocument* document, QWidget* parent = nullptr);
        private:
            bool doCanResetToDefaults() override;
            void doResetToDefaults() override;
            void doUpdateControls() override;
            bool doValidate() override;
        };
    }
}

