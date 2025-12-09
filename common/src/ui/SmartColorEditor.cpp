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

#include "SmartColorEditor.h"

#include <QButtonGroup>
#include <QColor>
#include <QHBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <QScrollArea>

#include "Color.h"
#include "mdl/ColorRange.h"
#include "mdl/EntityColorPropertyValue.h" // IWYU pragma: keep
#include "mdl/EntityDefinition.h"         // IWYU pragma: keep
#include "mdl/EntityDefinitionUtils.h"
#include "mdl/EntityNode.h"
#include "mdl/EntityNodeBase.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Entities.h"
#include "mdl/PropertyDefinition.h"
#include "mdl/WorldNode.h"
#include "ui/BorderLine.h"
#include "ui/ColorButton.h"
#include "ui/ColorTable.h"
#include "ui/MapDocument.h"
#include "ui/QtUtils.h"
#include "ui/ViewConstants.h"

#include "kd/contracts.h"
#include "kd/overload.h"
#include "kd/vector_set.h"

#include <fmt/ostream.h>

namespace tb::ui
{
namespace
{

template <typename T>
bool allHaveColorType(
  const std::vector<mdl::EntityNodeBase*>& nodes, const std::string& propertyKey)
{
  return std::ranges::all_of(nodes, [&](const auto* node) {
    const auto* propDef = mdl::propertyDefinition(node, propertyKey);
    return propDef
           && std::holds_alternative<mdl::PropertyValueTypes::Color<T>>(
             propDef->valueType);
  });
};

template <typename T>
bool anyHaveColorType(
  const std::vector<mdl::EntityNodeBase*>& nodes, const std::string& propertyKey)
{
  return std::ranges::any_of(nodes, [&](const auto* node) {
    const auto* propDef = mdl::propertyDefinition(node, propertyKey);
    return propDef
           && std::holds_alternative<mdl::PropertyValueTypes::Color<T>>(
             propDef->valueType);
  });
};

template <typename Node>
std::vector<QColor> collectColors(
  const std::vector<Node*>& nodes, const std::string& propertyKey)
{
  const auto toHsl = [](const auto& qColor) {
    return std::tuple{qColor.hslHue(), qColor.hslSaturation(), qColor.lightness()};
  };

  const auto cmp = [&](const auto& lhs, const auto& rhs) {
    return toHsl(lhs) < toHsl(rhs);
  };

  auto colors = kdl::vector_set<QColor, decltype(cmp)>{cmp};
  const auto visitEntityNode = [&](const auto* node) {
    if (const auto* propertyValue = node->entity().property(propertyKey))
    {
      parseEntityColorPropertyValue(
        node->entity().definition(), propertyKey, *propertyValue)
        | kdl::transform([&](const auto entityColorProperty) {
            colors.insert(toQColor(entityColorProperty.color));
          })
        | kdl::ignore();
    }
  };

  for (const auto* node : nodes)
  {
    node->accept(kdl::overload(
      [&](auto&& thisLambda, const mdl::WorldNode* world) {
        world->visitChildren(thisLambda);
        visitEntityNode(world);
      },
      [](auto&& thisLambda, const mdl::LayerNode* layer) {
        layer->visitChildren(thisLambda);
      },
      [](auto&& thisLambda, const mdl::GroupNode* group) {
        group->visitChildren(thisLambda);
      },
      [&](const mdl::EntityNode* entity) { visitEntityNode(entity); },
      [](const mdl::BrushNode*) {},
      [](const mdl::PatchNode*) {}));
  }

  return colors.get_data();
}

} // namespace

SmartColorEditor::SmartColorEditor(MapDocument& document, QWidget* parent)
  : SmartPropertyEditor{document, parent}
{
  createGui();
}

void SmartColorEditor::createGui()
{
  contract_pre(m_radioGroup == nullptr);
  contract_pre(m_floatRadio == nullptr);
  contract_pre(m_byteRadio == nullptr);
  contract_pre(m_colorPicker == nullptr);
  contract_pre(m_colorHistory == nullptr);

  auto* rangeTxt = new QLabel{tr("Color range")};
  makeEmphasized(rangeTxt);

  m_floatRadio = new QRadioButton{tr("Float [0,1]")};
  m_byteRadio = new QRadioButton{tr("Byte [0,255]")};
  m_colorPicker = new ColorButton{};
  m_colorHistory = new ColorTable{ColorHistoryCellSize};

  m_radioGroup = new QButtonGroup{this};
  m_radioGroup->addButton(m_floatRadio);
  m_radioGroup->addButton(m_byteRadio);

  auto* colorHistoryScroller = new QScrollArea{};
  colorHistoryScroller->setWidget(m_colorHistory);
  colorHistoryScroller->setWidgetResizable(true);
  colorHistoryScroller->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

  auto* leftLayout = new QVBoxLayout{};
  leftLayout->setContentsMargins(0, 0, 0, 0);
  leftLayout->setSpacing(LayoutConstants::NarrowVMargin);
  leftLayout->addWidget(rangeTxt);
  leftLayout->addWidget(m_floatRadio);
  leftLayout->addWidget(m_byteRadio);
  leftLayout->addWidget(m_colorPicker);
  leftLayout->addStretch(1);

  auto* outerLayout = new QHBoxLayout{};
  outerLayout->setContentsMargins(LayoutConstants::WideHMargin, 0, 0, 0);
  outerLayout->setSpacing(0);
  outerLayout->addLayout(leftLayout);
  outerLayout->addSpacing(LayoutConstants::WideHMargin);
  outerLayout->addWidget(new BorderLine{BorderLine::Direction::Vertical});
  outerLayout->addWidget(colorHistoryScroller, 1);
  setLayout(outerLayout);

  connect(
    m_floatRadio,
    &QAbstractButton::clicked,
    this,
    &SmartColorEditor::floatRangeRadioButtonClicked);
  connect(
    m_byteRadio,
    &QAbstractButton::clicked,
    this,
    &SmartColorEditor::byteRangeRadioButtonClicked);
  connect(
    m_colorPicker,
    &ColorButton::colorChangedByUser,
    this,
    &SmartColorEditor::colorPickerChanged);
  connect(
    m_colorHistory,
    &ColorTable::colorTableSelected,
    this,
    &SmartColorEditor::colorTableSelected);
}

void SmartColorEditor::doUpdateVisual(const std::vector<mdl::EntityNodeBase*>& nodes)
{
  contract_pre(m_radioGroup != nullptr);
  contract_pre(m_floatRadio != nullptr);
  contract_pre(m_byteRadio != nullptr);
  contract_pre(m_colorPicker != nullptr);
  contract_pre(m_colorHistory != nullptr);

  updateColorRange(nodes);
  updateColorHistory();
}

void SmartColorEditor::updateColorRange(const std::vector<mdl::EntityNodeBase*>& nodes)
{

  if (allHaveColorType<RgbF>(nodes, propertyKey()))
  {
    m_radioGroup->setExclusive(true);
    m_floatRadio->setChecked(true);
    m_byteRadio->setChecked(false);
    m_floatRadio->setEnabled(false);
    m_byteRadio->setEnabled(false);
  }
  else if (allHaveColorType<RgbB>(nodes, propertyKey()))
  {
    m_radioGroup->setExclusive(true);
    m_floatRadio->setChecked(false);
    m_byteRadio->setChecked(true);
    m_floatRadio->setEnabled(false);
    m_byteRadio->setEnabled(false);
  }
  else if (
    anyHaveColorType<RgbF>(nodes, propertyKey())
    || anyHaveColorType<RgbB>(nodes, propertyKey()))
  {
    m_radioGroup->setExclusive(false);
    m_floatRadio->setChecked(false);
    m_byteRadio->setChecked(false);
    m_floatRadio->setEnabled(false);
    m_byteRadio->setEnabled(false);
  }
  else
  {
    // either no property definition, or colors are convertible (type Color)
    m_floatRadio->setEnabled(true);
    m_byteRadio->setEnabled(true);

    const auto range = detectColorRange(propertyKey(), nodes);
    if (range == mdl::ColorRange::Float)
    {
      m_radioGroup->setExclusive(true);
      m_floatRadio->setChecked(true);
      m_byteRadio->setChecked(false);
    }
    else if (range == mdl::ColorRange::Byte)
    {
      m_radioGroup->setExclusive(true);
      m_floatRadio->setChecked(false);
      m_byteRadio->setChecked(true);
    }
    else
    {
      m_radioGroup->setExclusive(false);
      m_floatRadio->setChecked(false);
      m_byteRadio->setChecked(false);
    }
  }
}

void SmartColorEditor::updateColorHistory()
{
  auto& map = document().map();

  m_colorHistory->setColors(collectColors(std::vector{&map.world()}, propertyKey()));

  const auto selectedColors = collectColors(map.selection().allEntities(), propertyKey());
  m_colorHistory->setSelection(selectedColors);
  m_colorPicker->setColor(
    !selectedColors.empty() ? selectedColors.back() : QColor(Qt::black));
}

void SmartColorEditor::setColor(const QColor& qColor)
{
  const auto rawColor = fromQColor(qColor);
  const auto requestedColor =
    m_floatRadio->isChecked() ? Rgb{rawColor.to<RgbF>()} : Rgb{rawColor.to<RgbB>()};
  setEntityColorProperty(document().map(), propertyKey(), requestedColor);
}

void SmartColorEditor::floatRangeRadioButtonClicked()
{
  convertEntityColorRange(document().map(), propertyKey(), mdl::ColorRange::Float);
}

void SmartColorEditor::byteRangeRadioButtonClicked()
{
  convertEntityColorRange(document().map(), propertyKey(), mdl::ColorRange::Byte);
}

void SmartColorEditor::colorPickerChanged(const QColor& color)
{
  setColor(color);
}

void SmartColorEditor::colorTableSelected(QColor color)
{
  setColor(color);
}

} // namespace tb::ui
