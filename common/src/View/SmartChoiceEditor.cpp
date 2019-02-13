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

#include "SmartChoiceEditor.h"
#include "Assets/AttributeDefinition.h"
#include "Model/AttributableNode.h"
#include "View/MapDocument.h"
#include "View/ViewConstants.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QComboBox>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        SmartChoiceEditor::SmartChoiceEditor(QObject* parent, View::MapDocumentWPtr document) :
        SmartAttributeEditor(parent, document),
        m_panel(nullptr),
        m_comboBox(nullptr) {}

        void SmartChoiceEditor::OnComboBox(int index) {
            const String valueDescStr = m_comboBox->currentText().toStdString();
            const String valueStr = valueDescStr.substr(0, valueDescStr.find_first_of(':') - 1);
            document()->setAttribute(name(), valueStr);
        }
        
        void SmartChoiceEditor::OnTextEnter(const QString &text) {
            document()->setAttribute(name(), text.toStdString());
        }

        QWidget* SmartChoiceEditor::doCreateVisual(QWidget* parent) {
            assert(m_panel == nullptr);
            assert(m_comboBox == nullptr);
            
            m_panel = new QWidget(parent);
            QLabel* infoText = new QLabel(tr("Select a choice option:"));

            m_comboBox = new QComboBox();
            m_comboBox->setEditable(true);
            connect(m_comboBox, QOverload<int>::of(&QComboBox::activated), this, &SmartChoiceEditor::OnComboBox);
            connect(m_comboBox, &QComboBox::currentTextChanged, this, &SmartChoiceEditor::OnTextEnter);
            
            auto* sizer = new QVBoxLayout();
            sizer->addSpacing(LayoutConstants::WideVMargin);
            sizer->addWidget(infoText, 0);
            sizer->addSpacing(LayoutConstants::WideVMargin);
            sizer->addWidget(m_comboBox, 0);
            sizer->addStretch(1);
            
            m_panel->setLayout(sizer);
            return m_panel;
        }
        
        void SmartChoiceEditor::doDestroyVisual() {
            ensure(m_panel != nullptr, "panel is null");
            ensure(m_comboBox != nullptr, "comboBox is null");
            
            delete m_panel;
            m_panel = nullptr;
            m_comboBox= nullptr;
        }
        
        void SmartChoiceEditor::doUpdateVisual(const Model::AttributableNodeList& attributables) {
            ensure(m_panel != nullptr, "panel is null");
            ensure(m_comboBox != nullptr, "comboBox is null");
            
            m_comboBox->clear();

            const Assets::AttributeDefinition* attrDef = Model::AttributableNode::selectAttributeDefinition(name(), attributables);
            if (attrDef == nullptr || attrDef->type() != Assets::AttributeDefinition::Type_ChoiceAttribute) {
                m_comboBox->setDisabled(true);
            } else {
                m_comboBox->setDisabled(false);
                const Assets::ChoiceAttributeDefinition* choiceDef = static_cast<const Assets::ChoiceAttributeDefinition*>(attrDef);
                const Assets::ChoiceAttributeOption::List& options = choiceDef->options();
                
                for (const Assets::ChoiceAttributeOption& option : options)
                    m_comboBox->addItem(QString::fromStdString(option.value() + " : " + option.description()));
                
                const Model::AttributeValue value = Model::AttributableNode::selectAttributeValue(name(), attributables);
                m_comboBox->setCurrentText(QString::fromStdString(value));
            }
        }
    }
}
