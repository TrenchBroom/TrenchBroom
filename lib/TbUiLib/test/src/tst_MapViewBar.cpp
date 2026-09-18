/*
 Copyright (C) 2026 Kristian Duske

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

#include <QApplication>
#include <QLineEdit>
#include <QStackedLayout>
#include <QWidget>

#include "ui/AppControllerFixture.h"
#include "ui/MapDocument.h"
#include "ui/MapDocumentFixture.h"
#include "ui/MapViewBar.h"
#include "ui/SearchPanel.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{

TEST_CASE("MapViewBar")
{
  auto appControllerFixture = AppControllerFixture{};
  auto& appController = appControllerFixture.appController();

  auto fixture = MapDocumentFixture{};
  auto& document = fixture.create();

  auto bar = MapViewBar{appController, document};

  auto* emptyPage = new QWidget{};
  bar.toolPages()->addWidget(emptyPage);

  auto* widePage = new QWidget{};
  widePage->setMinimumWidth(300);
  bar.toolPages()->addWidget(widePage);

  auto* searchPanel = bar.findChild<SearchPanel*>();
  REQUIRE(searchPanel != nullptr);

  auto* searchBox = searchPanel->findChild<QLineEdit*>();
  REQUIRE(searchBox != nullptr);

  // whether and when the test window becomes active depends on the platform, so the
  // search box must not get focus from that, which would expand the search panel
  searchBox->setFocusPolicy(Qt::NoFocus);

  bar.resize(1000, 40);
  bar.show();
  QApplication::processEvents();
  REQUIRE(!searchPanel->expanded());

  const auto collapsedWidth = searchPanel->width();
  REQUIRE(collapsedWidth < 500);

  SECTION("an expanded search panel fills the space not used by the current tool page")
  {
    searchBox->setText("Combat");
    QApplication::processEvents();

    const auto expandedWidth = searchPanel->width();
    CHECK(expandedWidth > 500);

    bar.toolPages()->setCurrentWidget(widePage);
    QApplication::processEvents();

    CHECK(widePage->width() == 300);
    CHECK(searchPanel->width() == expandedWidth - 300);

    searchBox->clear();
    QApplication::processEvents();

    CHECK(searchPanel->width() == collapsedWidth);
  }

  SECTION("the search panel can collapse while the bar is being destroyed")
  {
    // emitted directly because the search box only has focus if the test window is active
    emit qApp->focusChanged(nullptr, searchBox);
    REQUIRE(searchPanel->expanded());

    // a widget deletes its layout before it closes its window, and closing the window
    // takes the focus away from the search box
    delete bar.layout();
    emit qApp->focusChanged(searchBox, nullptr);

    CHECK(!searchPanel->expanded());
  }
}

} // namespace tb::ui
