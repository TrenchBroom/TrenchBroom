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

#include "RecentDocumentListBox.h"

#include "Ensure.h"
#include "IO/PathQt.h"
#include "IO/ResourceUtils.h"
#include "TrenchBroomApp.h"

#include <cassert>

namespace TrenchBroom
{
namespace View
{
RecentDocumentListBox::RecentDocumentListBox(QWidget* parent)
  : ImageListBox("No Recent Documents", true, parent)
  , m_documentIcon(IO::loadPixmapResource("DocIcon.png"))
{
  TrenchBroomApp& app = View::TrenchBroomApp::instance();
  connect(
    &app,
    &TrenchBroomApp::recentDocumentsDidChange,
    this,
    &RecentDocumentListBox::recentDocumentsDidChange);
  reload();
}

void RecentDocumentListBox::recentDocumentsDidChange()
{
  reload();
}

size_t RecentDocumentListBox::itemCount() const
{
  const TrenchBroomApp& app = View::TrenchBroomApp::instance();
  const auto& recentDocuments = app.recentDocuments();
  return recentDocuments.size();
}

QPixmap RecentDocumentListBox::image(const size_t /* index */) const
{
  return m_documentIcon;
}

QString RecentDocumentListBox::title(const size_t index) const
{
  const auto& app = View::TrenchBroomApp::instance();
  const auto& recentDocuments = app.recentDocuments();
  ensure(index < recentDocuments.size(), "index out of range");
  return IO::pathAsQString(recentDocuments[index].filename());
}

QString RecentDocumentListBox::subtitle(const size_t index) const
{
  const auto& app = View::TrenchBroomApp::instance();
  const auto& recentDocuments = app.recentDocuments();
  ensure(index < recentDocuments.size(), "index out of range");
  return IO::pathAsQString(recentDocuments[index]);
}

void RecentDocumentListBox::doubleClicked(const size_t index)
{
  auto& app = View::TrenchBroomApp::instance();
  const auto& recentDocuments = app.recentDocuments();

  if (index < recentDocuments.size())
  {
    const auto& documentPath = recentDocuments[index];
    emit loadRecentDocument(documentPath);
  }
}
} // namespace View
} // namespace TrenchBroom
