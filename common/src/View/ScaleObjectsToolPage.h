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

#ifndef TrenchBroom_ScaleObjectsToolPage
#define TrenchBroom_ScaleObjectsToolPage

#include "TrenchBroom.h"
#include "View/ViewTypes.h"

#include <QWidget>

class QComboBox;
class QStackedLayout;
class QLineEdit;
class QComboBox;
class QAbstractButton;

namespace TrenchBroom {
    namespace View {
        class Selection;
        class ScaleObjectsTool;

        class ScaleObjectsToolPage : public QWidget {
            Q_OBJECT
        private:
            MapDocumentWPtr m_document;

            QStackedLayout* m_book;

            QLineEdit* m_sizeTextBox;
            QLineEdit* m_factorsTextBox;

            QComboBox* m_scaleFactorsOrSize;
            QAbstractButton* m_button;
        public:
            ScaleObjectsToolPage(QWidget* parent, MapDocumentWPtr document);
            ~ScaleObjectsToolPage() override;
            void activate();
        private:
            void bindObservers();
            void unbindObservers();

            void createGui();
            void updateGui();

            void OnUpdateButton();
            void OnApply();

            bool canScale() const;
            vm::vec3 getScaleFactors() const;

            void selectionDidChange(const Selection& selection);
        };
    }
}

#endif /* defined(TrenchBroom_ScaleObjectsToolPage) */
