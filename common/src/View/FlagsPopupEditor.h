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

class QWidget;

namespace TrenchBroom {
    namespace View {
        class ElidedLabel;
        class FlagsEditor;
        class PopupButton;

        /**
         * Button that opens up a flags editor popup
         */
        class FlagsPopupEditor : public QWidget {
            Q_OBJECT
        private:
            ElidedLabel* m_flagsTxt;
            PopupButton* m_button;
            FlagsEditor* m_editor;
        public:
            explicit FlagsPopupEditor(size_t numCols, QWidget* parent = nullptr, const QString& buttonLabel = "...", bool showFlagsText = true);

            void setFlags(const QStringList& labels, const QStringList& tooltips = QStringList());
            void setFlags(const QList<int>& values, const QStringList& labels, const QStringList& tooltips = QStringList());
            void setFlagValue(int set, int mixed = 0);
        private:
            void updateFlagsText();
        signals:
            void flagChanged(size_t index, int value, int setFlag, int mixedFlag);
        };
    }
}

#endif /* defined(TrenchBroom_FlagsPopupEditor) */
