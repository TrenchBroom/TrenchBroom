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

#include <QColor>
#include <QHBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <QScrollArea>

#include "Color.h"
#include "asset/ColorRange.h"
#include "mdl/EntityColor.h"
#include "mdl/EntityNode.h"
#include "mdl/EntityNodeBase.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/WorldNode.h"
#include "ui/BorderLine.h"
#include "ui/ColorButton.h"
#include "ui/ColorTable.h"
#include "ui/MapDocument.h"
#include "ui/QtUtils.h"
#include "ui/ViewConstants.h"

#include "kdl/overload.h"
#include "kdl/vector_set.h"

namespace tb::ui
{
namespace
{

template <typename Node>
std::vector<QColor> collectColors(
  const std::vector<Node*>& nodes, const std::string& propertyKey)
{
  const auto cmp = [](const auto& lhs, const auto& rhs) {
    const auto lr = float(lhs.red()) / 255.0f;
    const auto lg = float(lhs.green()) / 255.0f;
    const auto lb = float(lhs.blue()) / 255.0f;
    const auto rr = float(rhs.red()) / 255.0f;
    const auto rg = float(rhs.green()) / 255.0f;
    const auto rb = float(rhs.blue()) / 255.0f;

    float lh, ls, lbr, rh, rs, rbr;
    Color::rgbToHSB(lr, lg, lb, lh, ls, lbr);
    Color::rgbToHSB(rr, rg, rb, rh, rs, rbr);

    return lh < rh     ? true
           : lh > rh   ? false
           : ls < rs   ? true
           : ls > rs   ? false
           : lbr < rbr ? true
                       : false;
  };

  auto colors = kdl::vector_set<QColor, decltype(cmp)>{cmp};

  const auto visitEntityNode = [&](const auto* node) {
    if (const auto* value = node->entity().property(propertyKey))
    {
      colors.insert(toQColor(mdl::parseEntityColor(*value)));
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

SmartColorEditor::SmartColorEditor(std::weak_ptr<MapDocument> document, QWidget* parent)
  : SmartPropertyEditor{std::move(document), parent}
{
  createGui();
}

void SmartColorEditor::createGui()
{
  assert(m_floatRadio == nullptr);
  assert(m_byteRadio == nullptr);
  assert(m_colorPicker == nullptr);
  assert(m_colorHistory == nullptr);

  auto* rangeTxt = new QLabel{tr("Color range")};
  makeEmphasized(rangeTxt);

  m_floatRadio = new QRadioButton{tr("Float [0,1]")};
  m_byteRadio = new QRadioButton{tr("Byte [0,255]")};
  m_colorPicker = new ColorButton{};
  m_colorHistory = new ColorTable{ColorHistoryCellSize};

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
  ensure(m_floatRadio != nullptr, "floatRadio is null");
  ensure(m_byteRadio != nullptr, "byteRadio is null");
  ensure(m_colorPicker != nullptr, "colorPicker is null");
  ensure(m_colorHistory != nullptr, "colorHistory is null");

  updateColorRange(nodes);
  updateColorHistory();
}

void SmartColorEditor::updateColorRange(const std::vector<mdl::EntityNodeBase*>& nodes)
{
  const auto range = detectColorRange(propertyKey(), nodes);
  if (range == asset::ColorRange::Float)
  {
    m_floatRadio->setChecked(true);
    m_byteRadio->setChecked(false);
  }
  else if (range == asset::ColorRange::Byte)
  {
    m_floatRadio->setChecked(false);
    m_byteRadio->setChecked(true);
  }
  else
  {
    m_floatRadio->setChecked(false);
    m_byteRadio->setChecked(false);
  }
}

void SmartColorEditor::updateColorHistory()
{
  m_colorHistory->setColors(
    collectColors(std::vector{document()->world()}, propertyKey()));

  const auto selectedColors =
    collectColors(document()->allSelectedEntityNodes(), propertyKey());
  m_colorHistory->setSelection(selectedColors);
  m_colorPicker->setColor(
    !selectedColors.empty() ? selectedColors.back() : QColor(Qt::black));
}

void SmartColorEditor::setColor(const QColor& color) const
{
  const auto colorRange =
    m_floatRadio->isChecked() ? asset::ColorRange::Float : asset::ColorRange::Byte;
  const auto value = mdl::entityColorAsString(fromQColor(color), colorRange);
  document()->setProperty(propertyKey(), value);
}

void SmartColorEditor::floatRangeRadioButtonClicked()
{
  document()->convertEntityColorRange(propertyKey(), asset::ColorRange::Float);
}

void SmartColorEditor::byteRangeRadioButtonClicked()
{
  document()->convertEntityColorRange(propertyKey(), asset::ColorRange::Byte);
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
