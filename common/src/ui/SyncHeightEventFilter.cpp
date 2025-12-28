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

#include "SyncHeightEventFilter.h"

#include <QEvent>
#include <QWidget>

#include "kd/contracts.h"

namespace tb::ui
{

SyncHeightEventFilter::SyncHeightEventFilter(
  QWidget* primary, QWidget* secondary, QObject* parent)
  : QObject{parent}
  , m_primary{primary}
  , m_secondary{secondary}
{
  contract_pre(m_primary != nullptr);
  contract_pre(m_secondary != nullptr);

  m_primary->installEventFilter(this);
}

SyncHeightEventFilter::~SyncHeightEventFilter()
{
  if (m_primary)
  {
    m_primary->removeEventFilter(this);
  }
}

bool SyncHeightEventFilter::eventFilter(QObject* target, QEvent* event)
{
  if (target == m_primary && event->type() == QEvent::Resize)
  {
    const auto* sizeEvent = static_cast<QResizeEvent*>(event);
    const auto height = sizeEvent->size().height();
    if (m_secondary->height() != height)
    {
      m_secondary->setFixedHeight(height);
    }
    return false;
  }
  else
  {
    return QObject::eventFilter(target, event);
  }
}

} // namespace tb::ui
