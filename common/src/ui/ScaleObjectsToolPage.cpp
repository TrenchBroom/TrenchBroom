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

#include "ScaleObjectsToolPage.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QPushButton>
#include <QStackedLayout>

#include "ui/MapDocument.h"
#include "ui/ScaleObjectsTool.h"
#include "ui/ViewConstants.h"

#include "kdl/memory_utils.h"

#include "vm/vec.h"
#include "vm/vec_io.h"

#include <fmt/format.h>
#include <fmt/ostream.h>

namespace tb::ui
{

ScaleObjectsToolPage::ScaleObjectsToolPage(
  std::weak_ptr<MapDocument> document, QWidget* parent)
  : QWidget{parent}
  , m_document{std::move(document)}
{
  createGui();
  connectObservers();
  updateGui();
}

void ScaleObjectsToolPage::connectObservers()
{
  auto document = kdl::mem_lock(m_document);
  m_notifierConnection += document->selectionDidChangeNotifier.connect(
    this, &ScaleObjectsToolPage::selectionDidChange);
}

void ScaleObjectsToolPage::activate()
{
  const auto document = kdl::mem_lock(m_document);
  const auto suggestedSize = document->hasSelectedNodes()
                               ? document->selectionBounds().size()
                               : vm::vec3d{0, 0, 0};

  m_sizeTextBox->setText(
    QString::fromStdString(fmt::format("{}", fmt::streamed(suggestedSize))));
  m_factorsTextBox->setText("1.0 1.0 1.0");
}

void ScaleObjectsToolPage::createGui()
{
  auto document = kdl::mem_lock(m_document);

  auto* text = new QLabel{tr("Scale objects")};

  m_book = new QStackedLayout{};
  m_sizeTextBox = new QLineEdit{};
  m_factorsTextBox = new QLineEdit{};
  m_book->addWidget(m_sizeTextBox);
  m_book->addWidget(m_factorsTextBox);

  connect(
    m_sizeTextBox, &QLineEdit::returnPressed, this, &ScaleObjectsToolPage::applyScale);
  connect(
    m_factorsTextBox, &QLineEdit::returnPressed, this, &ScaleObjectsToolPage::applyScale);

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
  connect(m_button, &QAbstractButton::clicked, this, &ScaleObjectsToolPage::applyScale);

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

void ScaleObjectsToolPage::updateGui()
{
  auto document = kdl::mem_lock(m_document);
  m_button->setEnabled(canScale());
}

bool ScaleObjectsToolPage::canScale() const
{
  return kdl::mem_lock(m_document)->hasSelectedNodes();
}

std::optional<vm::vec3d> ScaleObjectsToolPage::getScaleFactors() const
{
  switch (m_scaleFactorsOrSize->currentIndex())
  {
  case 0: {
    auto document = kdl::mem_lock(m_document);
    if (
      const auto desiredSize = vm::parse<double, 3>(m_sizeTextBox->text().toStdString()))
    {
      return *desiredSize / document->selectionBounds().size();
    }
    return std::nullopt;
  }
  default:
    return vm::parse<double, 3>(m_factorsTextBox->text().toStdString());
  }
}

void ScaleObjectsToolPage::selectionDidChange(const Selection&)
{
  updateGui();
}

void ScaleObjectsToolPage::applyScale()
{
  if (canScale())
  {
    if (const auto scaleFactors = getScaleFactors())
    {
      auto document = kdl::mem_lock(m_document);
      const auto box = document->selectionBounds();
      document->scaleObjects(box.center(), *scaleFactors);
    }
  }
}

} // namespace tb::ui
