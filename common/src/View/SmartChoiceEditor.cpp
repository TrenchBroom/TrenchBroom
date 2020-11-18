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
#include "View/QtUtils.h"
#include "View/ViewConstants.h"

#include <kdl/set_temp.h>

#include <QtGlobal>
#include <QLabel>
#include <QVBoxLayout>
#include <QComboBox>
#include <QDebug>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        SmartChoiceEditor::SmartChoiceEditor(std::weak_ptr<MapDocument> document, QWidget* parent) :
        SmartAttributeEditor(std::move(document), parent),
        m_comboBox(nullptr),
        m_ignoreEditTextChanged(false) {
            createGui();
        }

        void SmartChoiceEditor::comboBoxActivated(const int /* index */) {
            const kdl::set_temp ignoreTextChanged(m_ignoreEditTextChanged);

            const auto valueDescStr = mapStringFromUnicode(document()->encoding(), m_comboBox->currentText());
            const auto valueStr = valueDescStr.substr(0, valueDescStr.find_first_of(':') - 1);
            document()->setAttribute(name(), valueStr);
        }

        void SmartChoiceEditor::comboBoxEditTextChanged(const QString& text) {
            if (!m_ignoreEditTextChanged) {
                document()->setAttribute(name(), mapStringFromUnicode(document()->encoding(), text));
            }
        }

        void SmartChoiceEditor::createGui() {
            assert(m_comboBox == nullptr);

            auto* infoText = new QLabel(tr("Select a choice option:"));

            m_comboBox = new QComboBox();
            m_comboBox->setEditable(true);
            connect(m_comboBox, QOverload<int>::of(&QComboBox::activated), this, &SmartChoiceEditor::comboBoxActivated);
            connect(m_comboBox, &QComboBox::editTextChanged, this, &SmartChoiceEditor::comboBoxEditTextChanged);

            auto* layout = new QVBoxLayout();
            layout->setContentsMargins(
                LayoutConstants::WideHMargin,
                LayoutConstants::WideVMargin,
                LayoutConstants::WideHMargin,
                LayoutConstants::WideVMargin);
            layout->setSpacing(LayoutConstants::NarrowVMargin);
            layout->addWidget(infoText);
            layout->addWidget(m_comboBox);
            layout->addStretch(1);

            setLayout(layout);
        }

        void SmartChoiceEditor::doUpdateVisual(const std::vector<Model::AttributableNode*>& attributables) {
            ensure(m_comboBox != nullptr, "comboBox is null");

            const kdl::set_temp ignoreTextChanged(m_ignoreEditTextChanged);
            m_comboBox->clear();

            const auto* attrDef = Model::selectAttributeDefinition(name(), attributables);
            if (attrDef == nullptr || attrDef->type() != Assets::AttributeDefinitionType::ChoiceAttribute) {
                m_comboBox->setDisabled(true);
            } else {
                m_comboBox->setDisabled(false);
                const auto* choiceDef = static_cast<const Assets::ChoiceAttributeDefinition*>(attrDef);
                const auto& options = choiceDef->options();

                for (const Assets::ChoiceAttributeOption& option : options) {
                    m_comboBox->addItem(mapStringToUnicode(document()->encoding(), option.value() + " : " + option.description()));
                }

                const auto value = Model::selectAttributeValue(name(), attributables);
                m_comboBox->setCurrentText(mapStringToUnicode(document()->encoding(), value));
            }
        }
    }
}
