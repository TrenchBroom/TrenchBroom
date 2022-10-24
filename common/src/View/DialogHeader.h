/*
 Copyright (C) 2021 Kristian Duske

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

#include <QWidget>

class QLabel;
class QPixmap;
class QString;

namespace TrenchBroom
{
namespace View
{
class DialogHeader : public QWidget
{
  Q_OBJECT
private:
  QLabel* m_iconLabel;
  QLabel* m_textLabel;

public:
  explicit DialogHeader(QWidget* parent = nullptr);
  explicit DialogHeader(const QString& text, QWidget* parent = nullptr);
  DialogHeader(const QString& text, QPixmap icon, QWidget* parent = nullptr);

  void set(const QString& text);
  void set(const QString& text, QPixmap icon);

private:
  void createGui();
};
} // namespace View
} // namespace TrenchBroom
