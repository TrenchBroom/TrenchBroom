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

#include "SmartDefaultAttributeEditor.h"

#include "Assets/EntityDefinition.h"
#include "Model/AttributableNode.h"

#include <QTextEdit>

namespace TrenchBroom {
    namespace View {
        SmartDefaultAttributeEditor::SmartDefaultAttributeEditor(QObject* parent, View::MapDocumentWPtr document) :
        SmartAttributeEditor(parent, document),
        m_descriptionTxt(nullptr),
        m_currentDefinition(nullptr) {}

        QWidget* SmartDefaultAttributeEditor::doCreateVisual(QWidget* parent) {
            m_descriptionTxt = new QTextEdit();
            m_descriptionTxt->setReadOnly(true);
            return m_descriptionTxt;
        }
        
        void SmartDefaultAttributeEditor::doDestroyVisual() {
            delete m_descriptionTxt;
            m_descriptionTxt = nullptr;
            m_currentDefinition = nullptr;
        }

        void SmartDefaultAttributeEditor::doUpdateVisual(const Model::AttributableNodeList& attributables) {
            const Assets::EntityDefinition* entityDefinition = Model::AttributableNode::selectEntityDefinition(attributables);
            if (entityDefinition != m_currentDefinition) {
                m_currentDefinition = entityDefinition;
                
                m_descriptionTxt->clear();
                if (m_currentDefinition != nullptr)
                    m_descriptionTxt->append(QString::fromStdString(m_currentDefinition->description()));
            }
        }
    }
}
