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

#include "Notifier.h"

#include <vector>
#include <tuple>

#include "Catch2.h"

namespace TrenchBroom {
    class Observed {
    public:
        Notifier<> noArgNotifier;
        Notifier<const int&> oneArgNotifier;
        Notifier<const int&, const int&> twoArgNotifier;

        void notify0() {
            noArgNotifier.notify();
        }

        void notify1(const int& a1) {
            oneArgNotifier.notify(a1);
        }

        void notify2(const int& a1, const int& a2) {
            twoArgNotifier.notify(a1, a2);
        }
    };

    class Observer {
    public:
        int notify0Calls;
        std::vector<int> notify1Calls;
        std::vector<std::tuple<int,int>> notify2Calls;

        Observer() : notify0Calls(0) {}

        void notify0() {
            ++notify0Calls;
        }

        void notify1(const int& a1) {
            notify1Calls.push_back(a1);
        }

        void notify2(const int& a1, const int& a2) {
            notify2Calls.push_back({a1, a2});
        }
    };

    TEST_CASE("NotifierTest.testAddRemoveObservers", "[NotifierTest]") {
        Observer o1;
        Observer o2;

        Observed obs;
        CHECK(obs.noArgNotifier.addObserver(&o1, &Observer::notify0));
        CHECK(obs.noArgNotifier.addObserver(&o2, &Observer::notify0));
        CHECK_FALSE(obs.noArgNotifier.addObserver(&o1, &Observer::notify0));
        CHECK_FALSE(obs.noArgNotifier.addObserver(&o2, &Observer::notify0));

        CHECK(obs.oneArgNotifier.addObserver(&o1, &Observer::notify1));
        CHECK(obs.oneArgNotifier.addObserver(&o2, &Observer::notify1));
        CHECK_FALSE(obs.oneArgNotifier.addObserver(&o1, &Observer::notify1));
        CHECK_FALSE(obs.oneArgNotifier.addObserver(&o2, &Observer::notify1));

        CHECK(obs.twoArgNotifier.addObserver(&o1, &Observer::notify2));
        CHECK(obs.twoArgNotifier.addObserver(&o2, &Observer::notify2));
        CHECK_FALSE(obs.twoArgNotifier.addObserver(&o1, &Observer::notify2));
        CHECK_FALSE(obs.twoArgNotifier.addObserver(&o2, &Observer::notify2));


        CHECK(obs.noArgNotifier.removeObserver(&o1, &Observer::notify0));
        CHECK(obs.noArgNotifier.removeObserver(&o2, &Observer::notify0));
        CHECK_FALSE(obs.noArgNotifier.removeObserver(&o1, &Observer::notify0));
        CHECK_FALSE(obs.noArgNotifier.removeObserver(&o2, &Observer::notify0));

        CHECK(obs.oneArgNotifier.removeObserver(&o1, &Observer::notify1));
        CHECK(obs.oneArgNotifier.removeObserver(&o2, &Observer::notify1));
        CHECK_FALSE(obs.oneArgNotifier.removeObserver(&o1, &Observer::notify1));
        CHECK_FALSE(obs.oneArgNotifier.removeObserver(&o2, &Observer::notify1));

        CHECK(obs.twoArgNotifier.removeObserver(&o1, &Observer::notify2));
        CHECK(obs.twoArgNotifier.removeObserver(&o2, &Observer::notify2));
        CHECK_FALSE(obs.twoArgNotifier.removeObserver(&o1, &Observer::notify2));
        CHECK_FALSE(obs.twoArgNotifier.removeObserver(&o2, &Observer::notify2));
    }

    TEST_CASE("NotifierTest.testNotifyObservers", "[NotifierTest]") {
        Observer o1;
        Observer o2;

        Observed obs;
        obs.noArgNotifier.addObserver(&o1, &Observer::notify0);
        obs.noArgNotifier.addObserver(&o2, &Observer::notify0);
        obs.oneArgNotifier.addObserver(&o1, &Observer::notify1);
        obs.oneArgNotifier.addObserver(&o2, &Observer::notify1);
        obs.twoArgNotifier.addObserver(&o1, &Observer::notify2);
        obs.twoArgNotifier.addObserver(&o2, &Observer::notify2);

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
        CHECK(std::vector<std::tuple<int,int>>{{1, 2}} == o1.notify2Calls);

        CHECK(1 == o2.notify0Calls);
        CHECK(std::vector<int>{1, 2} == o2.notify1Calls);
        CHECK(std::vector<std::tuple<int,int>>{{1, 2}} == o2.notify2Calls);
    }
}
