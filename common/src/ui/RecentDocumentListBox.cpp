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

#include "RecentDocumentListBox.h"

#include "ui/ImageUtils.h"
#include "ui/QPathUtils.h"
#include "ui/RecentDocuments.h"

#include "kd/contracts.h"

#include <cassert>

namespace tb::ui
{
RecentDocumentListBox::RecentDocumentListBox(
  RecentDocuments& recentDocuments, QWidget* parent)
  : ImageListBox{"No Recent Documents", true, parent}
  , m_recentDocuments{recentDocuments}
  , m_documentIcon{loadPixmap("DocIcon.png")}
{
  connect(
    &m_recentDocuments,
    &RecentDocuments::didChange,
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
  return m_recentDocuments.recentDocuments().size();
}

QPixmap RecentDocumentListBox::image(const size_t /* index */) const
{
  return m_documentIcon;
}

QString RecentDocumentListBox::title(const size_t index) const
{
  const auto& recentDocuments = m_recentDocuments.recentDocuments();
  contract_assert(index < recentDocuments.size());

  return pathAsQString(recentDocuments[index].filename());
}

QString RecentDocumentListBox::subtitle(const size_t index) const
{
  const auto& recentDocuments = m_recentDocuments.recentDocuments();
  contract_assert(index < recentDocuments.size());

  return pathAsQString(recentDocuments[index]);
}

void RecentDocumentListBox::doubleClicked(const size_t index)
{
  const auto& recentDocuments = m_recentDocuments.recentDocuments();
  if (index < recentDocuments.size())
  {
    const auto& documentPath = recentDocuments[index];
    emit loadRecentDocument(documentPath);
  }
}

} // namespace tb::ui
