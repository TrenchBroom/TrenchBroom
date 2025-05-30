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

#include "ScaleToolPage.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QPushButton>
#include <QStackedLayout>

#include "ui/MapDocument.h"
#include "ui/QtUtils.h"
#include "ui/ScaleTool.h"
#include "ui/ViewConstants.h"

#include "kdl/memory_utils.h"

#include "vm/vec.h"

#include <fmt/format.h>
#include <fmt/ostream.h>

namespace tb::ui
{

ScaleToolPage::ScaleToolPage(std::weak_ptr<MapDocument> document, QWidget* parent)
  : QWidget{parent}
  , m_document{std::move(document)}
{
  createGui();
  connectObservers();
  updateGui();
}

void ScaleToolPage::connectObservers()
{
  auto document = kdl::mem_lock(m_document);
  m_notifierConnection += document->selectionDidChangeNotifier.connect(
    this, &ScaleToolPage::selectionDidChange);
}

void ScaleToolPage::activate()
{
  const auto document = kdl::mem_lock(m_document);
  const auto suggestedSize = document->hasSelectedNodes()
                               ? document->selectionBounds().size()
                               : vm::vec3d{0, 0, 0};

  m_sizeTextBox->setText(toString(suggestedSize));
  m_factorsTextBox->setText(toString(vm::vec3d{1, 1, 1}));
}

void ScaleToolPage::createGui()
{
  auto document = kdl::mem_lock(m_document);

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
  auto document = kdl::mem_lock(m_document);
  m_button->setEnabled(canScale());
}

bool ScaleToolPage::canScale() const
{
  return kdl::mem_lock(m_document)->hasSelectedNodes();
}

std::optional<vm::vec3d> ScaleToolPage::getScaleFactors() const
{
  switch (m_scaleFactorsOrSize->currentIndex())
  {
  case 0: {
    auto document = kdl::mem_lock(m_document);
    if (const auto desiredSize = parse<double, 3>(m_sizeTextBox->text()))
    {
      return *desiredSize / document->selectionBounds().size();
    }
    return std::nullopt;
  }
  default:
    return parse<double, 3>(m_factorsTextBox->text());
  }
}

void ScaleToolPage::selectionDidChange(const Selection&)
{
  updateGui();
}

void ScaleToolPage::applyScale()
{
  if (canScale())
  {
    if (const auto scaleFactors = getScaleFactors())
    {
      auto document = kdl::mem_lock(m_document);
      const auto box = document->selectionBounds();
      document->scale(box.center(), *scaleFactors);
    }
  }
}

} // namespace tb::ui
