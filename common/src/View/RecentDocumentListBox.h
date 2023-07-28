/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include <QPixmap>

#include "View/ImageListBox.h"

#include <filesystem>

namespace TrenchBroom
{
namespace View
{
class RecentDocumentListBox : public ImageListBox
{
  Q_OBJECT
private:
  QPixmap m_documentIcon;
  std::vector<std::filesystem::path> m_existingRecentDocuments;

public:
  explicit RecentDocumentListBox(QWidget* parent = nullptr);
private slots:
  void recentDocumentsDidChange();

private:
  size_t itemCount() const override;
  QPixmap image(size_t index) const override;
  QString title(size_t index) const override;
  QString subtitle(size_t index) const override;

  void doubleClicked(size_t index) override;
signals:
  void loadRecentDocument(const std::filesystem::path& path);
};
} // namespace View
} // namespace TrenchBroom
