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

#include <QSignalSpy>
#include <QtTest/QTest>

#include "ui/SignalDelayer.h"

#include <chrono>

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{

using namespace std::chrono_literals;

TEST_CASE("SignalDelayer")
{
  auto delayer = SignalDelayer{10ms};
  auto spy = QSignalSpy{&delayer, &SignalDelayer::processSignal};

  SECTION("flushSignal")
  {
    SECTION("emits a pending signal immediately")
    {
      delayer.queueSignal();
      delayer.flushSignal();
      CHECK(spy.count() == 1);

      // the flushed signal is not emitted again once the delay has elapsed
      QTest::qWait(50);
      CHECK(spy.count() == 1);
    }

    SECTION("does nothing if no signal is pending")
    {
      delayer.flushSignal();
      CHECK(spy.count() == 0);
    }
  }
}

} // namespace tb::ui
