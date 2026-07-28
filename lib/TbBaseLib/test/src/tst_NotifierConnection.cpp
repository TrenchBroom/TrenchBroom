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

#include "base/Notifier.h"
#include "base/NotifierConnection.h"

#include <utility>

#include <catch2/catch_test_macros.hpp>

namespace tb
{
namespace
{

class Observed
{
public:
  Notifier<> noArgNotifier;

  void notify0() { noArgNotifier.notify(); }
};

class Observer
{
public:
  int notify0Calls = 0;

  void notify0() { ++notify0Calls; }
};

} // namespace

TEST_CASE("NotifierConnection")
{
  auto o1 = Observer{};
  auto o2 = Observer{};

  auto obs = Observed{};

  SECTION("move constructor")
  {
    auto con1 = obs.noArgNotifier.connect(&o1, &Observer::notify0);

    {
      auto con2 = NotifierConnection{std::move(con1)};

      obs.notify0();
      CHECK(o1.notify0Calls == 1);
    }

    // con2 took over the connection and disconnected it when it was destroyed
    obs.notify0();
    CHECK(o1.notify0Calls == 1);
  }

  SECTION("move assignment")
  {
    SECTION("takes over the connections of the moved from instance")
    {
      auto con1 = obs.noArgNotifier.connect(&o1, &Observer::notify0);

      {
        auto con2 = NotifierConnection{};
        con2 = std::move(con1);

        obs.notify0();
        CHECK(o1.notify0Calls == 1);
      }

      obs.notify0();
      CHECK(o1.notify0Calls == 1);
    }

    SECTION("disconnects the connections it held before")
    {
      auto con = obs.noArgNotifier.connect(&o1, &Observer::notify0);
      con = obs.noArgNotifier.connect(&o2, &Observer::notify0);

      obs.notify0();
      CHECK(o1.notify0Calls == 0);
      CHECK(o2.notify0Calls == 1);
    }

    SECTION("keeps its connections when moved onto itself")
    {
      auto con = obs.noArgNotifier.connect(&o1, &Observer::notify0);

      // assign via a reference to avoid a self move warning
      auto& conRef = con;
      con = std::move(conRef);

      obs.notify0();
      CHECK(o1.notify0Calls == 1);
    }
  }

  SECTION("disconnect")
  {
    SECTION("disconnects all connections")
    {
      auto con = NotifierConnection{};
      con += obs.noArgNotifier.connect(&o1, &Observer::notify0);
      con += obs.noArgNotifier.connect(&o2, &Observer::notify0);

      con.disconnect();

      obs.notify0();
      CHECK(o1.notify0Calls == 0);
      CHECK(o2.notify0Calls == 0);
    }

    SECTION("can be called more than once")
    {
      auto con = obs.noArgNotifier.connect(&o1, &Observer::notify0);

      con.disconnect();
      CHECK_NOTHROW(con.disconnect());

      obs.notify0();
      CHECK(o1.notify0Calls == 0);
    }

    SECTION("does nothing if the notifier was destroyed already")
    {
      auto con = NotifierConnection{};

      {
        auto shortLivedObs = Observed{};
        con = shortLivedObs.noArgNotifier.connect(&o1, &Observer::notify0);
      }

      CHECK_NOTHROW(con.disconnect());
    }
  }
}

} // namespace tb
