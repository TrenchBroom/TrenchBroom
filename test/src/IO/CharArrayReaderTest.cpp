/*
 Copyright (C) 2018 Eric Wasylishen

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

#include "IO/CharArrayReader.h"

namespace TrenchBroom {
    namespace IO {
        TEST(CharArrayReaderTest, createEmpty) {
            const char foo = 'x';
            CharArrayReader r(&foo, &foo);

            EXPECT_EQ(0U, r.size());
            EXPECT_EQ(0U, r.currentOffset());
            EXPECT_NO_THROW(r.seekFromBegin(0U));
            EXPECT_NO_THROW(r.seekFromEnd(0U));
            EXPECT_NO_THROW(r.seekForward(0U));
            EXPECT_EQ(&foo, r.cur<char>());
            EXPECT_FALSE(r.canRead(1U));
            EXPECT_TRUE(r.canRead(0U));
            EXPECT_TRUE(r.eof());
            EXPECT_THROW(r.readChar<char>(), CharArrayReaderException);
        }

        TEST(CharArrayReaderTest, createSingleChar) {
            const char* foo = "x";
            CharArrayReader r(foo, foo + 1);

            EXPECT_EQ(1U, r.size());
            EXPECT_EQ(0U, r.currentOffset());
            EXPECT_EQ(foo, r.cur<char>());
            EXPECT_TRUE(r.canRead(0U));
            EXPECT_TRUE(r.canRead(1U));
            EXPECT_FALSE(r.canRead(2U));
            EXPECT_FALSE(r.eof());

            // read the char
            EXPECT_EQ('x', r.readChar<char>());

            EXPECT_EQ(1U, r.currentOffset());
            EXPECT_EQ(foo + 1, r.cur<char>());
            EXPECT_FALSE(r.canRead(1U));
            EXPECT_TRUE(r.canRead(0U));
            EXPECT_TRUE(r.eof());
            EXPECT_THROW(r.readChar<char>(), CharArrayReaderException);
        }

        TEST(CharArrayReaderTest, testSeekFromBegin) {
            const char* foo = "xy";
            CharArrayReader r(foo, foo + 2);

            EXPECT_EQ(2U, r.size());
            EXPECT_EQ(0U, r.currentOffset());

            r.seekFromBegin(0U);
            EXPECT_EQ(0U, r.currentOffset());

            r.seekFromBegin(1U);
            EXPECT_EQ(1U, r.currentOffset());

            r.seekFromBegin(2U);
            EXPECT_EQ(2U, r.currentOffset());

            EXPECT_THROW(r.seekFromBegin(3U), CharArrayReaderException);
            EXPECT_EQ(2U, r.currentOffset());
        }

        TEST(CharArrayReaderTest, testSeekFromEnd) {
            const char* foo = "xy";
            CharArrayReader r(foo, foo + 2);

            EXPECT_EQ(2U, r.size());
            EXPECT_EQ(0U, r.currentOffset());

            r.seekFromEnd(0U);
            EXPECT_EQ(2U, r.currentOffset());

            r.seekFromEnd(1U);
            EXPECT_EQ(1U, r.currentOffset());

            r.seekFromEnd(2U);
            EXPECT_EQ(0U, r.currentOffset());

            EXPECT_THROW(r.seekFromEnd(3U), CharArrayReaderException);
            EXPECT_EQ(0U, r.currentOffset());
        }

        TEST(CharArrayReaderTest, testSeekFromCurrent) {
            const char* foo = "xy";
            CharArrayReader r(foo, foo + 2);

            EXPECT_EQ(2U, r.size());
            EXPECT_EQ(0U, r.currentOffset());

            r.seekForward(1U);
            EXPECT_EQ(1U, r.currentOffset());

            r.seekForward(1U);
            EXPECT_EQ(2U, r.currentOffset());

            EXPECT_THROW(r.seekForward(1U), CharArrayReaderException);
            EXPECT_EQ(2U, r.currentOffset());
        }
    }
}
