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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "Notifier.h"

namespace TrenchBroom {
    class Observed {
    public:
        Notifier0 noArgNotifier;
        Notifier1<const int&> oneArgNotifier;
        Notifier2<const int&, const int&> twoArgNotifier;
        
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
        MOCK_METHOD0(notify0, void());
        MOCK_METHOD1(notify1, void(const int&));
        MOCK_METHOD2(notify2, void(const int&, const int&));
    };
    
    TEST(NotifierTest, testAddRemoveObservers) {
        Observer o1;
        Observer o2;
        
        Observed obs;
        ASSERT_TRUE(obs.noArgNotifier.addObserver(&o1, &Observer::notify0));
        ASSERT_TRUE(obs.noArgNotifier.addObserver(&o2, &Observer::notify0));
        ASSERT_FALSE(obs.noArgNotifier.addObserver(&o1, &Observer::notify0));
        ASSERT_FALSE(obs.noArgNotifier.addObserver(&o2, &Observer::notify0));

        ASSERT_TRUE(obs.oneArgNotifier.addObserver(&o1, &Observer::notify1));
        ASSERT_TRUE(obs.oneArgNotifier.addObserver(&o2, &Observer::notify1));
        ASSERT_FALSE(obs.oneArgNotifier.addObserver(&o1, &Observer::notify1));
        ASSERT_FALSE(obs.oneArgNotifier.addObserver(&o2, &Observer::notify1));
        
        ASSERT_TRUE(obs.twoArgNotifier.addObserver(&o1, &Observer::notify2));
        ASSERT_TRUE(obs.twoArgNotifier.addObserver(&o2, &Observer::notify2));
        ASSERT_FALSE(obs.twoArgNotifier.addObserver(&o1, &Observer::notify2));
        ASSERT_FALSE(obs.twoArgNotifier.addObserver(&o2, &Observer::notify2));
        

        ASSERT_TRUE(obs.noArgNotifier.removeObserver(&o1, &Observer::notify0));
        ASSERT_TRUE(obs.noArgNotifier.removeObserver(&o2, &Observer::notify0));
        ASSERT_FALSE(obs.noArgNotifier.removeObserver(&o1, &Observer::notify0));
        ASSERT_FALSE(obs.noArgNotifier.removeObserver(&o2, &Observer::notify0));

        ASSERT_TRUE(obs.oneArgNotifier.removeObserver(&o1, &Observer::notify1));
        ASSERT_TRUE(obs.oneArgNotifier.removeObserver(&o2, &Observer::notify1));
        ASSERT_FALSE(obs.oneArgNotifier.removeObserver(&o1, &Observer::notify1));
        ASSERT_FALSE(obs.oneArgNotifier.removeObserver(&o2, &Observer::notify1));
        
        ASSERT_TRUE(obs.twoArgNotifier.removeObserver(&o1, &Observer::notify2));
        ASSERT_TRUE(obs.twoArgNotifier.removeObserver(&o2, &Observer::notify2));
        ASSERT_FALSE(obs.twoArgNotifier.removeObserver(&o1, &Observer::notify2));
        ASSERT_FALSE(obs.twoArgNotifier.removeObserver(&o2, &Observer::notify2));
    }
    
    TEST(NotifierTest, testNotifyObservers) {
        Observer o1;
        Observer o2;
        
        Observed obs;
        obs.noArgNotifier.addObserver(&o1, &Observer::notify0);
        obs.noArgNotifier.addObserver(&o2, &Observer::notify0);
        obs.oneArgNotifier.addObserver(&o1, &Observer::notify1);
        obs.oneArgNotifier.addObserver(&o2, &Observer::notify1);
        obs.twoArgNotifier.addObserver(&o1, &Observer::notify2);
        obs.twoArgNotifier.addObserver(&o2, &Observer::notify2);
        
        EXPECT_CALL(o1, notify0());
        EXPECT_CALL(o2, notify0());

        EXPECT_CALL(o1, notify1(1));
        EXPECT_CALL(o2, notify1(1));
        EXPECT_CALL(o1, notify1(2));
        EXPECT_CALL(o2, notify1(2));
        
        EXPECT_CALL(o1, notify2(1, 2));
        EXPECT_CALL(o2, notify2(1, 2));

        obs.notify0();
        obs.notify1(1);
        obs.notify1(2);
        obs.notify2(1, 2);
    }
}
