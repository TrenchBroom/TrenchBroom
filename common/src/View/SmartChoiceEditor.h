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

#include "Model/ModelTypes.h"
#include "View/SmartAttributeEditor.h"
#include "View/ViewTypes.h"

class QComboBox;
class QWidget;
class QLabel;
class QWidget;

namespace TrenchBroom {
    namespace Assets {
        class ChoicePropertyDefinition;
    }
    
    namespace View {
        class SmartChoiceEditor : public SmartAttributeEditor {
            Q_OBJECT
        private:
            QWidget* m_panel;
            QComboBox* m_comboBox;
        public:
            SmartChoiceEditor(QObject* parent, View::MapDocumentWPtr document);
            
            void OnComboBox(int index);
            void OnTextEnter(const QString &text);
        private:
            QWidget* doCreateVisual(QWidget* parent) override;
            void doDestroyVisual() override;
            void doUpdateVisual(const Model::AttributableNodeList& attributables) override;
        };
    }
}

#endif /* defined(TrenchBroom_SmartChoiceEditor) */
