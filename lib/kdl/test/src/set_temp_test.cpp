/*
 Copyright 2010-2019 Kristian Duske

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <gtest/gtest.h>

#include "kdl/set_temp.h"

namespace kdl {
    TEST(set_temp_test, set_unset) {
        int value = 0;
        {
            set_temp s(value, 1);
            ASSERT_EQ(1, value);
        }
        ASSERT_EQ(0, value);
    }

    TEST(set_temp_test, set_unset_bool) {
        bool value = false;
        {
            set_temp s(value, true);
            ASSERT_TRUE(value);
        }
        ASSERT_FALSE(value);

        {
            set_temp s(value);
            ASSERT_TRUE(value);

            {
                set_temp t(value, false);
                ASSERT_FALSE(value);
            }
            ASSERT_TRUE(value);
        }
        ASSERT_FALSE(value);
    }

    TEST(set_later_test, set) {
        int value = 0;

        {
            set_later s(value, 1);
            ASSERT_EQ(0, value);
        }
        ASSERT_EQ(1, value);
    }

    TEST(inc_temp, inc_dec) {
        int value = 0;

        {
            inc_temp i(value);
            ASSERT_EQ(1, value);
        }
        ASSERT_EQ(0, value);
    }

    TEST(dec_temp, dec_inc) {
        int value = 0;
        {
            dec_temp d(value);
            ASSERT_EQ(-1, value);
        }
        ASSERT_EQ(0, value);
    }
}
