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

#pragma once

#include "ui/ControlListBox.h"

class QLabel;
class QPixmap;

namespace tb::ui
{
class ElidedLabel;

class ImageListBoxItemRenderer : public ControlListBoxItemRenderer
{
  Q_OBJECT
private:
  ElidedLabel* m_titleLabel = nullptr;
  ElidedLabel* m_subtitleLabel = nullptr;
  QLabel* m_imageLabel = nullptr;

public:
  ImageListBoxItemRenderer(
    const QString& title, const QString& subtitle, const QPixmap& image, QWidget* parent);
  void updateItem() override;
};

class ImageListBox : public ControlListBox
{
  Q_OBJECT
public:
  explicit ImageListBox(
    const QString& emptyText, bool showSeparator, QWidget* parent = nullptr);

private:
  ControlListBoxItemRenderer* createItemRenderer(QWidget* parent, size_t index) override;

private:
  friend class ImageListBoxItemRenderer;

  virtual QPixmap image(size_t index) const;
  virtual QString title(size_t index) const = 0;
  virtual QString subtitle(size_t index) const = 0;
};

} // namespace tb::ui
