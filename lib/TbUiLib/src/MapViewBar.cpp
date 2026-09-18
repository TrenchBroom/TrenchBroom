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

#include "ui/MapViewBar.h"

#include <QHBoxLayout>
#include <QResizeEvent>
#include <QStackedLayout>
#include <QtSystemDetection>

#include "base/PreferenceManager.h"
#include "prefs/Preferences.h"
#include "ui/MapDocument.h"
#include "ui/SearchPanel.h"
#include "ui/ViewConstants.h"
#include "ui/ViewEditor.h"

namespace tb::ui
{

namespace
{

/**
 * A stacked layout that is only as wide as its current page instead of its widest page,
 * so that the space not used by the current page is available to other widgets.
 */
class ToolPagesLayout : public QStackedLayout
{
public:
  QSize sizeHint() const override
  {
    auto size = QStackedLayout::sizeHint();
    if (const auto* item = itemAt(currentIndex()))
    {
      size.setWidth(item->sizeHint().width());
    }
    return size;
  }

  QSize minimumSize() const override
  {
    auto size = QStackedLayout::minimumSize();
    if (const auto* item = itemAt(currentIndex()))
    {
      size.setWidth(item->minimumSize().width());
    }
    return size;
  }
};

} // namespace

MapViewBar::MapViewBar(
  AppController& appController, MapDocument& document, QWidget* parent)
  : ContainerBar(Sides::BottomSide, parent)
{
  createGui(appController, document);
}

QStackedLayout* MapViewBar::toolPages()
{
  return m_toolPages;
}

void MapViewBar::createGui(AppController& appController, MapDocument& document)
{
  setAttribute(Qt::WA_MacSmallSize);

  m_toolPages = new ToolPagesLayout{};
  m_toolPages->setContentsMargins(0, 0, 0, 0);

  m_searchPanel = new SearchPanel{appController, document};
  m_viewEditor = new ViewPopupEditor{document};

#if defined(Q_OS_MACOS)
  const auto vMargin = pref(Preferences::Theme) == Preferences::DarkTheme
                         ? LayoutConstants::MediumVMargin
                         : 0;
#else
  const auto vMargin = LayoutConstants::MediumVMargin;
#endif

  auto* layout = new QHBoxLayout{};
  layout->setContentsMargins(
    LayoutConstants::WideHMargin, vMargin, LayoutConstants::WideHMargin, vMargin);
  layout->setSpacing(LayoutConstants::WideHMargin);
  layout->addLayout(m_toolPages, 1);
  layout->addWidget(m_searchPanel, 0, Qt::AlignVCenter);
  layout->addWidget(m_viewEditor, 0, Qt::AlignVCenter);

  setLayout(layout);

  // an expanded search panel takes all the space not used by the current tool page; the
  // layout is the context because it is deleted before the search panel when this bar is
  // destroyed, and the search panel can still collapse after that
  connect(
    m_searchPanel, &SearchPanel::expandedChanged, layout, [=, this](const auto expanded) {
      layout->setStretchFactor(m_toolPages, expanded ? 0 : 1);
      layout->setStretchFactor(m_searchPanel, expanded ? 1 : 0);
    });
}

} // namespace tb::ui
