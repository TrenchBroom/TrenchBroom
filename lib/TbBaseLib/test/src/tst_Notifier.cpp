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

#include "base/Notifier.h"

#include <tuple>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb
{
namespace
{

class Observed
{
public:
  Notifier<> noArgNotifier;
  Notifier<const int&> oneArgNotifier;
  Notifier<const int&, const int&> twoArgNotifier;

  void notify0() { noArgNotifier.notify(); }

  void notify1(const int& a1) { oneArgNotifier.notify(a1); }

  void notify2(const int& a1, const int& a2) { twoArgNotifier.notify(a1, a2); }
};

class Observer
{
public:
  int notify0Calls;
  std::vector<int> notify1Calls;
  std::vector<std::tuple<int, int>> notify2Calls;

  Observer()
    : notify0Calls(0)
  {
  }

  void notify0() { ++notify0Calls; }

  void notify1(const int& a1) { notify1Calls.push_back(a1); }

  void notify2(const int& a1, const int& a2) { notify2Calls.emplace_back(a1, a2); }
};

struct Param
{
  size_t& copyCount;
  size_t& moveCount;

  Param(size_t& i_copyCount, size_t& i_moveCount)
    : copyCount{i_copyCount}
    , moveCount{i_moveCount}
  {
  }

  Param(const Param& other)
    : copyCount{other.copyCount}
    , moveCount{other.moveCount}
  {
    ++copyCount;
  }

  Param(Param&& other) noexcept
    : copyCount{other.copyCount}
    , moveCount{other.moveCount}
  {
    ++moveCount;
  }
};

} // namespace

TEST_CASE("Notifier")
{
  auto o1 = Observer{};
  auto o2 = Observer{};

  auto obs = Observed{};

  SECTION("add remove observers")
  {
    {
      auto con = NotifierConnection{};
      con += obs.noArgNotifier.connect(&o1, &Observer::notify0);
      con += obs.noArgNotifier.connect(&o1, &Observer::notify0);
      con += obs.noArgNotifier.connect(&o2, &Observer::notify0);

      obs.notify0();
      CHECK(o1.notify0Calls == 2);
      CHECK(o2.notify0Calls == 1);
    }

    obs.notify0();
    CHECK(o1.notify0Calls == 2);
    CHECK(o2.notify0Calls == 1);
  }

  SECTION("notify observers")
  {
    auto con = NotifierConnection{};

    con += obs.noArgNotifier.connect(&o1, &Observer::notify0);
    con += obs.noArgNotifier.connect(&o2, &Observer::notify0);
    con += obs.oneArgNotifier.connect(&o1, &Observer::notify1);
    con += obs.oneArgNotifier.connect(&o2, &Observer::notify1);
    con += obs.twoArgNotifier.connect(&o1, &Observer::notify2);
    con += obs.twoArgNotifier.connect(&o2, &Observer::notify2);

    CHECK(0 == o1.notify0Calls);
    CHECK(o1.notify1Calls.empty());
    CHECK(o1.notify2Calls.empty());

    CHECK(0 == o2.notify0Calls);
    CHECK(o2.notify1Calls.empty());
    CHECK(o2.notify2Calls.empty());

    obs.notify0();
    obs.notify1(1);
    obs.notify1(2);
    obs.notify2(1, 2);

    CHECK(1 == o1.notify0Calls);
    CHECK(std::vector<int>{1, 2} == o1.notify1Calls);
    CHECK(std::vector<std::tuple<int, int>>{{1, 2}} == o1.notify2Calls);

    CHECK(1 == o2.notify0Calls);
    CHECK(std::vector<int>{1, 2} == o2.notify1Calls);
    CHECK(std::vector<std::tuple<int, int>>{{1, 2}} == o2.notify2Calls);
  }

  SECTION("notify observers with different type qualifiers and value categories")
  {
    // clang-format off
    /*
                                 observer takes argument
     notifier takes argument   | by value | by lvalue reference | by const lvalue reference | by rvalue reference
     ==========================|==========|=====================|===========================|====================
     by value                  |    X     |                     |              X            |         X
     by lvalue reference       |    X     |          X          |              X            |
     by const lvalue reference |    X     |                     |              X            |
     by rvalue reference       |    X     |                     |              X            |         X

     Cells marked with an 'X' should be supported since the type system allows the
     observer's argument to bind to the notifier's argument.
     */
    // clang-format on

    size_t copyCount = 0;
    size_t moveCount = 0;

    SECTION("call by value notifier")
    {
      auto byValueNotifier = Notifier<Param>{};

      SECTION("with by value observer")
      {
        const auto c = byValueNotifier.connect([](Param) {});

        copyCount = moveCount = 0;
        auto p = Param{copyCount, moveCount};
        byValueNotifier.notify(p);
        CHECK(copyCount == 1);
        CHECK(moveCount == 1);

        copyCount = moveCount = 0;
        byValueNotifier.notify(Param{copyCount, moveCount});
        CHECK(copyCount == 0);
        CHECK(moveCount == 2);
      }

      SECTION("with by lvalue ref observer")
      {
        // const auto c = byValueNotifier.connect([](Param&) {}); // does not compile
      }

      SECTION("with by const lvalue ref observer")
      {
        const auto c = byValueNotifier.connect([](const Param&) {});

        copyCount = moveCount = 0;
        auto p = Param{copyCount, moveCount};
        byValueNotifier.notify(p);
        CHECK(copyCount == 1);
        CHECK(moveCount == 0);

        copyCount = moveCount = 0;
        byValueNotifier.notify(Param{copyCount, moveCount});
        CHECK(copyCount == 0);
        CHECK(moveCount == 1);
      }

      SECTION("with by rvalue ref observer")
      {
        const auto c = byValueNotifier.connect([](Param&&) {});

        copyCount = moveCount = 0;
        auto p = Param{copyCount, moveCount};
        byValueNotifier.notify(p);
        CHECK(copyCount == 1);
        CHECK(moveCount == 0);

        copyCount = moveCount = 0;
        byValueNotifier.notify(Param{copyCount, moveCount});
        CHECK(copyCount == 0);
        CHECK(moveCount == 1);
      }
    }

    SECTION("call by lvalue ref notifier")
    {
      auto byLvalueRefNotifier = Notifier<Param&>{};

      SECTION("with by value observer")
      {
        const auto c = byLvalueRefNotifier.connect([](Param) {});

        copyCount = moveCount = 0;
        auto p = Param{copyCount, moveCount};
        byLvalueRefNotifier.notify(p);
        CHECK(copyCount == 1);
        CHECK(moveCount == 0);
      }

      SECTION("with by lvalue ref observer")
      {
        const auto c = byLvalueRefNotifier.connect([](Param&) {});

        copyCount = moveCount = 0;
        auto p = Param{copyCount, moveCount};
        byLvalueRefNotifier.notify(p);
        CHECK(copyCount == 0);
        CHECK(moveCount == 0);
      }

      SECTION("with by const lvalue ref observer")
      {
        const auto c = byLvalueRefNotifier.connect([](const Param&) {});

        copyCount = moveCount = 0;
        auto p = Param{copyCount, moveCount};
        byLvalueRefNotifier.notify(p);
        CHECK(copyCount == 0);
        CHECK(moveCount == 0);
      }

      SECTION("with by rvalue ref observer")
      {
        // const auto c = byLvalueRefNotifier.connect([](Param&&) {}); // does not compile
      }
    }

    SECTION("call by const lvalue ref notifier")
    {
      auto byConstLvalueRefNotifier = Notifier<const Param&>{};

      SECTION("with by value observer")
      {
        const auto c = byConstLvalueRefNotifier.connect([](Param) {});

        copyCount = moveCount = 0;
        auto p = Param{copyCount, moveCount};
        byConstLvalueRefNotifier.notify(p);
        CHECK(copyCount == 1);
        CHECK(moveCount == 0);

        copyCount = moveCount = 0;
        byConstLvalueRefNotifier.notify(Param{copyCount, moveCount});
        CHECK(copyCount == 1);
        CHECK(moveCount == 0);
      }

      SECTION("with by lvalue ref observer")
      {
        // const auto c = byConstLvalueRefNotifier.connect([](Param&) {}); // does not
        // compile
      }

      SECTION("with by const lvalue ref observer")
      {
        const auto c = byConstLvalueRefNotifier.connect([](const Param&) {});

        copyCount = moveCount = 0;
        auto p = Param{copyCount, moveCount};
        byConstLvalueRefNotifier.notify(p);
        CHECK(copyCount == 0);
        CHECK(moveCount == 0);

        copyCount = moveCount = 0;
        byConstLvalueRefNotifier.notify(Param{copyCount, moveCount});
        CHECK(copyCount == 0);
        CHECK(moveCount == 0);
      }

      SECTION("with by rvalue ref observer")
      {
        // const auto c = byConstLvalueRefNotifier.connect([](Param&&) {}); // does not
        // compile
      }
    }

    SECTION("call by rvalue ref notifier")
    {
      auto byRvalueRef = Notifier<Param&&>{};

      SECTION("with by value observer")
      {
        const auto c = byRvalueRef.connect([](Param) {});

        copyCount = moveCount = 0;
        byRvalueRef.notify(Param{copyCount, moveCount});
        CHECK(copyCount == 0);
        CHECK(moveCount == 1);
      }

      SECTION("with by lvalue ref observer")
      {
        // const auto c = byValueNotifier.connect([](Param&) {}); // does not compile
      }

      SECTION("with by const lvalue ref observer")
      {
        const auto c = byRvalueRef.connect([](const Param&) {});

        copyCount = moveCount = 0;
        byRvalueRef.notify(Param{copyCount, moveCount});
        CHECK(copyCount == 0);
        CHECK(moveCount == 0);
      }

      SECTION("with by rvalue ref observer")
      {
        const auto c = byRvalueRef.connect([](Param&&) {});

        copyCount = moveCount = 0;
        byRvalueRef.notify(Param{copyCount, moveCount});
        CHECK(copyCount == 0);
        CHECK(moveCount == 0);
      }
    }
  }

  SECTION("connecting an observer while notifying")
  {
    SECTION("defers the observer until the next notification")
    {
      auto lateCalls = 0;
      auto lateConnection = NotifierConnection{};
      auto connected = false;

      const auto connection = obs.noArgNotifier.connect([&]() {
        if (!std::exchange(connected, true))
        {
          lateConnection = obs.noArgNotifier.connect([&]() { ++lateCalls; });
        }
      });

      // the observer was added while notifying, so it is not called yet
      obs.notify0();
      CHECK(lateCalls == 0);

      obs.notify0();
      CHECK(lateCalls == 1);

      obs.notify0();
      CHECK(lateCalls == 2);
    }

    SECTION(
      "does not call the observer at all if it is disconnected again while "
      "notifying")
    {
      auto lateCalls = 0;
      auto connected = false;

      const auto connection = obs.noArgNotifier.connect([&]() {
        if (!std::exchange(connected, true))
        {
          // this connection is disconnected at the end of the scope, before the
          // observer is ever transferred out of the pending list
          const auto lateConnection = obs.noArgNotifier.connect([&]() { ++lateCalls; });
        }
      });

      obs.notify0();
      obs.notify0();
      CHECK(lateCalls == 0);
    }
  }

  SECTION("disconnecting an observer while notifying")
  {
    SECTION("does not call the disconnected observer again")
    {
      auto calls1 = 0;
      auto calls2 = 0;
      auto connection2 = NotifierConnection{};

      const auto connection1 = obs.noArgNotifier.connect([&]() {
        ++calls1;
        connection2.disconnect();
      });
      connection2 = obs.noArgNotifier.connect([&]() { ++calls2; });

      // the second observer is disconnected before it is reached
      obs.notify0();
      CHECK(calls1 == 1);
      CHECK(calls2 == 0);

      obs.notify0();
      CHECK(calls1 == 2);
      CHECK(calls2 == 0);
    }
  }
}

TEST_CASE("NotifyAfter")
{
  auto n = Notifier<const Param&>{};

  size_t copyCount = 0;
  size_t moveCount = 0;

  SECTION("call with lvalue")
  {
    {
      const auto p = Param{copyCount, moveCount};
      auto after = NotifyAfter{true, n, p};
      CHECK(copyCount == 0);
      CHECK(moveCount == 0);
    }
    CHECK(copyCount == 0);
    CHECK(moveCount == 0);
  }

  SECTION("call with rvalue")
  {
    {
      auto after = NotifyAfter{true, n, Param{copyCount, moveCount}};
      CHECK(copyCount == 0);
      CHECK(moveCount <= 3);
    }
    CHECK(copyCount == 0);
    CHECK(moveCount <= 3);
  }

  SECTION("notifies when it is destroyed, and only if notify is true")
  {
    const auto notify = GENERATE(true, false);
    CAPTURE(notify);

    auto calls = 0;
    const auto connection = n.connect([&](const Param&) { ++calls; });

    {
      const auto p = Param{copyCount, moveCount};
      const auto after = NotifyAfter{notify, n, p};

      CHECK(calls == 0);
    }

    CHECK(calls == (notify ? 1 : 0));
  }
}

TEST_CASE("NotifyBeforeAndAfter")
{
  auto b = Notifier<const Param&>{};
  auto a = Notifier<const Param&>{};

  size_t copyCount = 0;
  size_t moveCount = 0;

  SECTION("call with lvalue")
  {
    {
      const auto p = Param{copyCount, moveCount};
      auto after = NotifyBeforeAndAfter{true, b, a, p};
      CHECK(copyCount == 0);
      CHECK(moveCount == 0);
    }
    CHECK(copyCount == 0);
    CHECK(moveCount == 0);
  }

  SECTION("call with rvalue")
  {
    {
      auto after = NotifyBeforeAndAfter{true, b, a, Param{copyCount, moveCount}};
      CHECK(copyCount == 0);
      CHECK(moveCount <= 3);
    }
    CHECK(copyCount == 0);
    CHECK(moveCount <= 3);
  }
}


} // namespace tb