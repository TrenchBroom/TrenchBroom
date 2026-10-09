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

#include <QAbstractItemView>
#include <QCompleter>
#include <QRegularExpression>
#include <QStringList>
#include <QtTest/QSignalSpy>
#include <QtTest/QTest>

#include "ui/CatchConfig.h"
#include "ui/MultiCompletionLineEdit.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb::ui
{

TEST_CASE("MultiCompletionLineEdit")
{
  auto lineEdit = MultiCompletionLineEdit{};
  lineEdit.setWordDelimiters(QRegularExpression{"\\$"}, QRegularExpression{"\\}"});

  auto* completer = new QCompleter{QStringList{"${MAP_BASE_NAME}", "${MAP_DIR_PATH}"}};
  lineEdit.setMultiCompleter(completer);
  lineEdit.show();

  auto returnPressedSpy = QSignalSpy{&lineEdit, &QLineEdit::returnPressed};
  auto editingFinishedSpy = QSignalSpy{&lineEdit, &QLineEdit::editingFinished};

  SECTION("keyPressEvent")
  {
    const auto key = GENERATE(Qt::Key_Return, Qt::Key_Enter);
    CAPTURE(key);

    // While the completion popup is visible, it receives all key events, so the tests
    // send them to the popup as well.

    SECTION("Return accepts the selected completion")
    {
      QTest::keyClicks(&lineEdit, "-map ${MAP_");
      REQUIRE(completer->popup()->isVisible());

      QTest::keyClick(completer->popup(), Qt::Key_Down);
      QTest::keyClick(completer->popup(), key);

      CHECK(!completer->popup()->isVisible());
      CHECK(lineEdit.text() == "-map ${MAP_BASE_NAME}");
      CHECK(returnPressedSpy.count() == 0);
      CHECK(editingFinishedSpy.count() == 0);
    }

    SECTION("Return closes the popup if no completion is selected")
    {
      QTest::keyClicks(&lineEdit, "-map ${MAP_");
      REQUIRE(completer->popup()->isVisible());

      QTest::keyClick(completer->popup(), key);

      CHECK(!completer->popup()->isVisible());
      CHECK(lineEdit.text() == "-map ${MAP_");
      CHECK(returnPressedSpy.count() == 0);
      CHECK(editingFinishedSpy.count() == 0);
    }

    SECTION("Return is passed on if the popup is not visible")
    {
      QTest::keyClicks(&lineEdit, "-map");
      REQUIRE(!completer->popup()->isVisible());

      QTest::keyClick(&lineEdit, key);

      CHECK(lineEdit.text() == "-map");
      CHECK(returnPressedSpy.count() == 1);
      CHECK(editingFinishedSpy.count() == 1);
    }
  }
}

} // namespace tb::ui
