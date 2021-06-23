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

#include "FloatType.h"
#include "NotifierConnection.h"

#include <vecmath/forward.h>

#include <memory>
#include <optional>

#include <QWidget>

class QComboBox;
class QStackedLayout;
class QLineEdit;
class QComboBox;
class QAbstractButton;

namespace TrenchBroom {
    namespace View {
        class MapDocument;
        class Selection;
        class ScaleObjectsToolPage : public QWidget {
            Q_OBJECT
        private:
            std::weak_ptr<MapDocument> m_document;

            QStackedLayout* m_book;

            QLineEdit* m_sizeTextBox;
            QLineEdit* m_factorsTextBox;

            QComboBox* m_scaleFactorsOrSize;
            QAbstractButton* m_button;

            NotifierConnection m_notifierConnection;
        public:
            explicit ScaleObjectsToolPage(std::weak_ptr<MapDocument> document, QWidget* parent = nullptr);
            void activate();
        private:
            void connectObservers();

            void createGui();
            void updateGui();

            bool canScale() const;
            std::optional<vm::vec3> getScaleFactors() const;

            void selectionDidChange(const Selection& selection);

            void applyScale();
        };
    }
}

