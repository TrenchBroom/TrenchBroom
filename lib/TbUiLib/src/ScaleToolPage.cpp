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

#include "ui/ScaleToolPage.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QPushButton>
#include <QStackedLayout>

#include "mdl/Map.h"
#include "mdl/Map_Geometry.h"
#include "ui/MapDocument.h"
#include "ui/QVecUtils.h"
#include "ui/ScaleTool.h"
#include "ui/ViewConstants.h"

#include "vm/vec.h"

#include <fmt/format.h>
#include <fmt/ostream.h>

namespace tb::ui
{

ScaleToolPage::ScaleToolPage(MapDocument& document, QWidget* parent)
  : QWidget{parent}
  , m_document{document}
{
  createGui();
  connectObservers();
  updateGui();
}

void ScaleToolPage::connectObservers()
{
  m_notifierConnection +=
    m_document.documentWasLoadedNotifier.connect(this, &ScaleToolPage::documentDidChange);
  m_notifierConnection +=
    m_document.documentDidChangeNotifier.connect(this, &ScaleToolPage::documentDidChange);
}

void ScaleToolPage::activate()
{
  const auto suggestedSize =
    m_document.map().selectionBounds().value_or(vm::bbox3d{}).size();

  m_sizeTextBox->setText(toString(suggestedSize));
  m_factorsTextBox->setText(toString(vm::vec3d{1, 1, 1}));
}

void ScaleToolPage::createGui()
{
  auto* text = new QLabel{tr("Scale objects")};

  m_book = new QStackedLayout{};
  m_sizeTextBox = new QLineEdit{};
  m_factorsTextBox = new QLineEdit{};
  m_book->addWidget(m_sizeTextBox);
  m_book->addWidget(m_factorsTextBox);

  connect(m_sizeTextBox, &QLineEdit::returnPressed, this, &ScaleToolPage::applyScale);
  connect(m_factorsTextBox, &QLineEdit::returnPressed, this, &ScaleToolPage::applyScale);

  m_scaleFactorsOrSize = new QComboBox{};
  m_scaleFactorsOrSize->addItem(tr("to size"));
  m_scaleFactorsOrSize->addItem(tr("by factors"));

  m_scaleFactorsOrSize->setCurrentIndex(0);
  connect(
    m_scaleFactorsOrSize,
    static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
    m_book,
    &QStackedLayout::setCurrentIndex);

  m_button = new QPushButton(tr("Apply"));
  connect(m_button, &QAbstractButton::clicked, this, &ScaleToolPage::applyScale);

  auto* layout = new QHBoxLayout{};
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(LayoutConstants::MediumHMargin);

  layout->addWidget(text, 0, Qt::AlignVCenter);
  layout->addWidget(m_scaleFactorsOrSize, 0, Qt::AlignVCenter);
  layout->addLayout(m_book, 0);
  layout->addWidget(m_button, 0, Qt::AlignVCenter);
  layout->addStretch(1);

  setLayout(layout);
}

void ScaleToolPage::updateGui()
{
  m_button->setEnabled(canScale());
}

bool ScaleToolPage::canScale() const
{
  return m_document.map().selection().hasNodes();
}

std::optional<vm::vec3d> ScaleToolPage::getScaleFactors() const
{
  switch (m_scaleFactorsOrSize->currentIndex())
  {
  case 0:
    if (const auto& selectionBounds = m_document.map().selectionBounds())
    {
      if (const auto desiredSize = parse<double, 3>(m_sizeTextBox->text()))
      {
        return *desiredSize / selectionBounds->size();
      }
    }
    return std::nullopt;
  default:
    return parse<double, 3>(m_factorsTextBox->text());
  }
}

void ScaleToolPage::documentDidChange()
{
  updateGui();
}

void ScaleToolPage::applyScale()
{
  if (canScale())
  {
    if (const auto scaleFactors = getScaleFactors())
    {
      auto& map = m_document.map();

      if (const auto& selectionBounds = map.selectionBounds())
      {
        scaleSelection(map, selectionBounds->center(), *scaleFactors);
      }
    }
  }
}

} // namespace tb::ui
