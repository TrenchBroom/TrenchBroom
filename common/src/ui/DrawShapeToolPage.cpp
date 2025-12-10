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

#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QStackedLayout>
#include <QToolButton>

#include "io/ResourceUtils.h"
#include "ui/DrawShapeToolExtension.h"
#include "ui/QtUtils.h"
#include "ui/ViewConstants.h"

namespace tb::ui
{

DrawShapeToolPage::DrawShapeToolPage(
  DrawShapeToolExtensionManager& extensionManager, QWidget* parent)
  : QWidget{parent}
  , m_extensionManager{extensionManager}
{
  createGui();
  m_notifierConnection += m_extensionManager.currentExtensionDidChangeNotifier.connect(
    this, &DrawShapeToolPage::currentExtensionDidChange);
}

void DrawShapeToolPage::createGui()
{
  auto* label = new QLabel{tr("Shape")};
  m_extensionButton = createBitmapButton(
    m_extensionManager.currentExtension().iconPath(), tr("Click to select a shape"));
  m_extensionButton->setObjectName("toolButton_withBorder");

  m_extensionPages = new QStackedLayout{};
  for (auto* extensionPage : m_extensionManager.createToolPages())
  {
    m_extensionPages->addWidget(extensionPage);
  }

  auto* layout = new QHBoxLayout();
  layout->setContentsMargins(QMargins{});
  layout->setSpacing(LayoutConstants::MediumHMargin);

  layout->addWidget(label, 0, Qt::AlignVCenter);
  layout->addWidget(m_extensionButton, 0, Qt::AlignVCenter);
  layout->addLayout(m_extensionPages);
  layout->addStretch(2);

  setLayout(layout);

  connect(m_extensionButton, &QAbstractButton::clicked, [&]() {
    auto menu = QMenu{};

    const auto extensions = m_extensionManager.extensions();
    for (size_t i = 0; i < extensions.size(); ++i)
    {
      auto* extension = extensions[i];
      auto icon = io::loadSVGIcon(extension->iconPath());

      auto* action =
        menu.addAction(icon, QString::fromStdString(extension->name()), [&, i]() {
          m_extensionManager.setCurrentExtensionIndex(i);
        });
      action->setIconVisibleInMenu(true);
    }

    menu.exec(QCursor::pos());
  });
}

void DrawShapeToolPage::currentExtensionDidChange(const size_t index)
{
  auto icon = io::loadSVGIcon(m_extensionManager.currentExtension().iconPath());
  m_extensionButton->setIcon(icon);
  m_extensionPages->setCurrentIndex(int(index));
}

} // namespace tb::ui
