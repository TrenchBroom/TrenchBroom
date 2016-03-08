/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#ifndef TrenchBroom_MockObserver
#define TrenchBroom_MockObserver

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "Notifier.h"

namespace TrenchBroom {
    class MockObserver0 {
    public:
        MockObserver0(Notifier0& notifier, const size_t times) {
            expect(times);
            notifier.addObserver(this, &MockObserver0::notify);
        }
        
        void expect(const size_t times = 1) {
            for (size_t i = 0; i < times; ++i)
                EXPECT_CALL(*this, notify());
        }
        
        MOCK_METHOD0(notify, void());
    };

    template <typename A1>
    class MockObserver1 {
    public:
        MockObserver1(Notifier1<A1>& notifier) {
            notifier.addObserver(this, &MockObserver1<A1>::notify);
        }
        
        MockObserver1(Notifier1<A1>& notifier, A1 arg) {
            expect(arg);
            notifier.addObserver(this, &MockObserver1<A1>::notify);
        }

        template <typename I>
        MockObserver1(Notifier1<A1>& notifier, I it, I end) {
            expect(it, end);
            notifier.addObserver(this, &MockObserver1<A1>::notify);
        }
        
        void expect(A1 arg) {
            EXPECT_CALL(*this, notify(arg));
        }
        
        template <typename I>
        void expect(I it, I end) {
            while (it != end)
                EXPECT_CALL(*this, notify(*it++));
        }
        
        MOCK_METHOD1_T(notify, void(A1));
    };
}

#endif /* defined(TrenchBroom_MockObserver) */
