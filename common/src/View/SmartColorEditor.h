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

#include "View/SmartAttributeEditor.h"

#include <memory>
#include <vector>

class QColor;
class QWidget;
class QPushButton;
class QRadioButton;

namespace TrenchBroom {
    namespace View {
        class ColorButton;
        class ColorTable;
        class MapDocument;

        class SmartColorEditor : public SmartAttributeEditor {
            Q_OBJECT
        private:
            static const size_t ColorHistoryCellSize = 15;
            using wxColorList = std::vector<QColor>;

            QRadioButton* m_floatRadio;
            QRadioButton* m_byteRadio;
            ColorButton* m_colorPicker;
            ColorTable* m_colorHistory;
        public:
            explicit SmartColorEditor(std::weak_ptr<MapDocument> document, QWidget* parent = nullptr);
        private:
            void createGui();
            void doUpdateVisual(const std::vector<Model::EntityNodeBase*>& attributables) override;

            void updateColorRange(const std::vector<Model::EntityNodeBase*>& attributables);
            void updateColorHistory();

            void setColor(const QColor& wxColor) const;

            void floatRangeRadioButtonClicked();
            void byteRangeRadioButtonClicked();
            void colorPickerChanged(const QColor& color);
            void colorTableSelected(QColor color);
        };
    }
}

