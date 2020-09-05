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

#ifndef TrenchBroom_SmartChoiceEditor
#define TrenchBroom_SmartChoiceEditor

#include "View/SmartAttributeEditor.h"

#include <memory>
#include <vector>

class QComboBox;
class QWidget;
class QLabel;
class QWidget;

namespace TrenchBroom {
    namespace Assets {
        class ChoicePropertyDefinition;
    }

    namespace View {
        class MapDocument;

        class SmartChoiceEditor : public SmartAttributeEditor {
            Q_OBJECT
        private:
            QComboBox* m_comboBox;
            bool m_ignoreEditTextChanged;
        public:
            explicit SmartChoiceEditor(std::weak_ptr<MapDocument> document, QWidget* parent = nullptr);

            void comboBoxActivated(int index);
            void comboBoxEditTextChanged(const QString& text);
        private:
            void createGui();
            void doUpdateVisual(const std::vector<Model::AttributableNode*>& attributables) override;
        };
    }
}

#endif /* defined(TrenchBroom_SmartChoiceEditor) */
