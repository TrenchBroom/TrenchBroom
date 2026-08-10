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

#include "ui/DrawShapeToolExtensionPage.h"

#include <QHBoxLayout>
#include <QPushButton>

#include "mdl/Map.h"
#include "ui/MapDocument.h"
#include "ui/ViewConstants.h"

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

void DrawShapeToolExtensionPage::addApplyButton(MapDocument& document)
{
  auto* applyButton = new QPushButton{tr("Apply")};
  applyButton->setEnabled(false);
  connect(applyButton, &QPushButton::clicked, this, [&]() { applyParametersNotifier(); });

  addWidget(applyButton);

  const auto enableApplyButton = [&document, applyButton](const auto&...) {
    applyButton->setEnabled(document.map().selection().hasNodes());
  };

  m_notifierConnection += document.documentWasLoadedNotifier.connect(enableApplyButton);
  m_notifierConnection += document.documentDidChangeNotifier.connect(enableApplyButton);
  m_notifierConnection += document.selectionDidChangeNotifier.connect(enableApplyButton);
}

} // namespace tb::ui
