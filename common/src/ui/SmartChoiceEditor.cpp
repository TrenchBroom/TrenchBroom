/*
 Copyright (C) 2010 Kristian Duske

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

#include <QComboBox>
#include <QDebug>
#include <QLabel>
#include <QVBoxLayout>
#include <QtGlobal>

#include "mdl/EntityNodeBase.h"
#include "mdl/Map.h"
#include "mdl/Map_Entities.h"
#include "mdl/PropertyDefinition.h"
#include "ui/MapDocument.h"
#include "ui/QtUtils.h"
#include "ui/ViewConstants.h"

#include "kd/contracts.h"
#include "kd/set_temp.h"

namespace tb::ui
{

SmartChoiceEditor::SmartChoiceEditor(MapDocument& document, QWidget* parent)
  : SmartPropertyEditor{document, parent}
{
  createGui();
}

void SmartChoiceEditor::comboBoxActivated(const int /* index */)
{
  const auto ignoreTextChanged = kdl::set_temp{m_ignoreEditTextChanged};

  auto& map = document().map();
  const auto valueDescStr =
    mapStringFromUnicode(map.encoding(), m_comboBox->currentText());
  const auto valueStr = valueDescStr.substr(0, valueDescStr.find_first_of(':') - 1);
  setEntityProperty(map, propertyKey(), valueStr);
}

void SmartChoiceEditor::comboBoxEditTextChanged(const QString& text)
{
  if (!m_ignoreEditTextChanged)
  {
    auto& map = document().map();
    setEntityProperty(map, propertyKey(), mapStringFromUnicode(map.encoding(), text));
  }
}

void SmartChoiceEditor::createGui()
{
  contract_pre(m_comboBox == nullptr);

  auto* infoText = new QLabel{tr("Select a choice option:")};

  m_comboBox = new QComboBox{};
  m_comboBox->setEditable(true);
  connect(
    m_comboBox,
    QOverload<int>::of(&QComboBox::activated),
    this,
    &SmartChoiceEditor::comboBoxActivated);
  connect(
    m_comboBox,
    &QComboBox::editTextChanged,
    this,
    &SmartChoiceEditor::comboBoxEditTextChanged);

  auto* layout = new QVBoxLayout{};
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

void SmartChoiceEditor::doUpdateVisual(const std::vector<mdl::EntityNodeBase*>& nodes)
{
  contract_pre(m_comboBox != nullptr);

  const auto ignoreTextChanged = kdl::set_temp{m_ignoreEditTextChanged};
  m_comboBox->clear();
  m_comboBox->setDisabled(true);

  if (const auto* propertyDef = mdl::selectPropertyDefinition(propertyKey(), nodes))
  {
    auto& map = document().map();

    if (
      const auto* choiceType =
        std::get_if<mdl::PropertyValueTypes::Choice>(&propertyDef->valueType))
    {
      m_comboBox->setDisabled(false);
      const auto& options = choiceType->options;

      for (const auto& option : options)
      {
        m_comboBox->addItem(
          mapStringToUnicode(map.encoding(), option.value + " : " + option.description));
      }

      const auto value = mdl::selectPropertyValue(propertyKey(), nodes);
      m_comboBox->setCurrentText(mapStringToUnicode(map.encoding(), value));
    }
  }
}

} // namespace tb::ui
