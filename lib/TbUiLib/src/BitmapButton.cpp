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

#include "ui/BitmapButton.h"

#include <QIcon>
#include <QString>
#include <QToolButton>

#include "ui/ImageUtils.h"

#include "kd/contracts.h"

namespace tb::ui
{

QToolButton* createBitmapButton(
  const std::filesystem::path& imagePath, const QString& tooltip, QWidget* parent)
{
  return createBitmapButton(loadSVGIcon(imagePath), tooltip, parent);
}

QToolButton* createBitmapButton(
  const QIcon& icon, const QString& tooltip, QWidget* parent)
{
  // NOTE: QIcon::availableSizes() is not high-dpi friendly, it returns pixels when we
  // want logical sizes. We rely on the fact that loadIconResourceQt inserts pixmaps in
  // the order 1x then 2x, so the first pixmap has the logical size.
  contract_pre(!icon.availableSizes().empty());

  auto* button = new QToolButton{parent};
  button->setMinimumSize(icon.availableSizes().front());
  // button->setAutoDefault(false);
  button->setToolTip(tooltip);
  button->setIcon(icon);
  // button->setFlat(true);
  button->setObjectName("toolButton_borderless");

  return button;
}

QToolButton* createBitmapToggleButton(
  const std::filesystem::path& imagePath, const QString& tooltip, QWidget* parent)
{
  auto* button = createBitmapButton(imagePath, tooltip, parent);
  button->setCheckable(true);
  return button;
}

} // namespace tb::ui
