/*
 Copyright (C) 2023 Kristian Duske

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

#include "DrawShapeToolPage.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QStackedLayout>

#include "View/DrawShapeToolExtension.h"
#include "View/MapDocument.h"
#include "View/ViewConstants.h"

#include "kdl/memory_utils.h"

#include "vm/vec.h"
#include "vm/vec_io.h"

namespace TrenchBroom::View
{

DrawShapeToolPage::DrawShapeToolPage(
  std::weak_ptr<MapDocument> document,
  DrawShapeToolExtensionManager& extensionManager,
  QWidget* parent)
  : QWidget{parent}
  , m_document{std::move(document)}
{
  createGui(extensionManager);
  m_notifierConnection += extensionManager.currentExtensionDidChangeNotifier.connect(
    this, &DrawShapeToolPage::currentExtensionDidChange);
}

void DrawShapeToolPage::createGui(DrawShapeToolExtensionManager& extensionManager)
{
  auto* label = new QLabel{tr("Shape")};
  m_extensions = new QComboBox{};
  m_extensionPages = new QStackedLayout{};

  for (auto* extension : extensionManager.extensions())
  {
    m_extensions->addItem(QString::fromStdString(extension->name()));
    m_extensionPages->addWidget(extension->createToolPage());
  }

  auto* layout = new QHBoxLayout();
  layout->setContentsMargins(QMargins{});
  layout->setSpacing(LayoutConstants::MediumHMargin);

  layout->addWidget(label, 0, Qt::AlignVCenter);
  layout->addWidget(m_extensions, 0, Qt::AlignVCenter);
  layout->addLayout(m_extensionPages);
  layout->addStretch(2);

  setLayout(layout);

  connect(
    m_extensions, QOverload<int>::of(&QComboBox::activated), this, [&](const auto index) {
      extensionManager.setCurrentExtensionIndex(size_t(index));
    });
}

void DrawShapeToolPage::currentExtensionDidChange(size_t index)
{
  m_extensions->setCurrentIndex(int(index));
  m_extensionPages->setCurrentIndex(m_extensions->currentIndex());
}

} // namespace TrenchBroom::View
