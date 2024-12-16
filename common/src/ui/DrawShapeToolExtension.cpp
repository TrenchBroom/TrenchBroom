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

#include "DrawShapeToolExtension.h"

#include <QHBoxLayout>
#include <QPushButton>

#include "Ensure.h"
#include "ui/MapDocument.h"
#include "ui/ViewConstants.h"

#include "kdl/memory_utils.h"
#include "kdl/vector_utils.h"

namespace tb::ui
{

DrawShapeToolExtensionPage::DrawShapeToolExtensionPage(QWidget* parent)
  : QWidget{parent}
{
  auto* layout = new QHBoxLayout{};
  layout->setContentsMargins(QMargins{});
  layout->setSpacing(LayoutConstants::MediumHMargin);
  layout->addStretch(1);
  setLayout(layout);
}

void DrawShapeToolExtensionPage::addWidget(QWidget* widget)
{
  auto* boxLayout = qobject_cast<QHBoxLayout*>(layout());
  boxLayout->insertWidget(boxLayout->count() - 1, widget, 0, Qt::AlignVCenter);
}

void DrawShapeToolExtensionPage::addApplyButton(std::weak_ptr<MapDocument> document)
{
  auto* applyButton = new QPushButton{tr("Apply")};
  applyButton->setEnabled(false);
  connect(
    applyButton, &QPushButton::clicked, this, [&]() { settingsDidChangeNotifier(); });

  addWidget(applyButton);

  auto doc = kdl::mem_lock(document);
  m_notifierConnection += doc->selectionDidChangeNotifier.connect(
    [=](const auto&) { applyButton->setEnabled(doc->hasSelectedNodes()); });
}

DrawShapeToolExtension::DrawShapeToolExtension(std::weak_ptr<MapDocument> document)
  : m_document{std::move(document)}
{
}

DrawShapeToolExtension::~DrawShapeToolExtension() = default;

DrawShapeToolExtensionManager::DrawShapeToolExtensionManager(
  std::vector<std::unique_ptr<DrawShapeToolExtension>> extensions)
  : m_extensions{std::move(extensions)}
{
  ensure(!m_extensions.empty(), "extensions must not be empty");
}

const std::vector<DrawShapeToolExtension*> DrawShapeToolExtensionManager::extensions()
  const
{
  return kdl::vec_transform(
    m_extensions, [](const auto& extension) { return extension.get(); });
}

DrawShapeToolExtension& DrawShapeToolExtensionManager::currentExtension()
{
  return *m_extensions[m_currentExtensionIndex];
}

bool DrawShapeToolExtensionManager::setCurrentExtensionIndex(size_t currentExtensionIndex)
{
  if (currentExtensionIndex != m_currentExtensionIndex)
  {
    m_currentExtensionIndex = currentExtensionIndex;
    currentExtensionDidChangeNotifier(m_currentExtensionIndex);
    return true;
  }

  return false;
}

} // namespace tb::ui
