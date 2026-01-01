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

#include "ui/AutoSizeTableRows.h"

#include <QEvent>
#include <QHeaderView>
#include <QTableView>

namespace tb::ui
{

AutoSizeTableRowsEventFilter::AutoSizeTableRowsEventFilter(QTableView* tableView)
  : QObject{tableView}
  , m_tableView{tableView}
{
  m_tableView->installEventFilter(this);
}

bool AutoSizeTableRowsEventFilter::eventFilter(QObject* watched, QEvent* event)
{
  if (watched == m_tableView && event->type() == QEvent::Show)
  {
    m_tableView->resizeRowsToContents();
    m_tableView->removeEventFilter(this);
  }
  return QObject::eventFilter(watched, event);
}

void autoSizeTableRows(QTableView* tableView)
{
  tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  tableView->installEventFilter(new AutoSizeTableRowsEventFilter{tableView});
  tableView->resizeRowsToContents();
}

} // namespace tb::ui
