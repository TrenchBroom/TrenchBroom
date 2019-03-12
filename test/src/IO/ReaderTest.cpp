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

#include "IO/Reader.h"

namespace TrenchBroom {
    namespace IO {
        TEST(ReaderTest, createEmpty) {
            const char foo = 'x';
            auto r = Reader::from(&foo, &foo);

            EXPECT_EQ(0U, r.size());
            EXPECT_EQ(0U, r.position());
            EXPECT_NO_THROW(r.seekFromBegin(0U));
            EXPECT_NO_THROW(r.seekFromEnd(0U));
            EXPECT_NO_THROW(r.seekForward(0U));
            EXPECT_FALSE(r.canRead(1U));
            EXPECT_TRUE(r.canRead(0U));
            EXPECT_TRUE(r.eof());
            EXPECT_THROW(r.readChar<char>(), ReaderException);
        }

        TEST(ReaderTest, createSingleChar) {
            const char* foo = "x";
            auto r = Reader::from(foo, foo + 1);

            EXPECT_EQ(1U, r.size());
            EXPECT_EQ(0U, r.position());
            EXPECT_TRUE(r.canRead(0U));
            EXPECT_TRUE(r.canRead(1U));
            EXPECT_FALSE(r.canRead(2U));
            EXPECT_FALSE(r.eof());

            // read the char
            EXPECT_EQ('x', r.readChar<char>());

            EXPECT_EQ(1U, r.position());
            EXPECT_FALSE(r.canRead(1U));
            EXPECT_TRUE(r.canRead(0U));
            EXPECT_TRUE(r.eof());
            EXPECT_THROW(r.readChar<char>(), ReaderException);
        }

        TEST(ReaderTest, testSeekFromBegin) {
            const char* foo = "xy";
            auto r = Reader::from(foo, foo + 2);

            EXPECT_EQ(2U, r.size());
            EXPECT_EQ(0U, r.position());

            r.seekFromBegin(0U);
            EXPECT_EQ(0U, r.position());

            r.seekFromBegin(1U);
            EXPECT_EQ(1U, r.position());

            r.seekFromBegin(2U);
            EXPECT_EQ(2U, r.position());

            EXPECT_THROW(r.seekFromBegin(3U), ReaderException);
            EXPECT_EQ(2U, r.position());
        }

        TEST(ReaderTest, testSeekFromEnd) {
            const char* foo = "xy";
            auto r = Reader::from(foo, foo + 2);

            EXPECT_EQ(2U, r.size());
            EXPECT_EQ(0U, r.position());

            r.seekFromEnd(0U);
            EXPECT_EQ(2U, r.position());

            r.seekFromEnd(1U);
            EXPECT_EQ(1U, r.position());

            r.seekFromEnd(2U);
            EXPECT_EQ(0U, r.position());

            EXPECT_THROW(r.seekFromEnd(3U), ReaderException);
            EXPECT_EQ(0U, r.position());
        }

        TEST(ReaderTest, testSeekForward) {
            const char* foo = "xy";
            auto r = Reader::from(foo, foo + 2);

            EXPECT_EQ(2U, r.size());
            EXPECT_EQ(0U, r.position());

            r.seekForward(1U);
            EXPECT_EQ(1U, r.position());

            r.seekForward(1U);
            EXPECT_EQ(2U, r.position());

            EXPECT_THROW(r.seekForward(1U), ReaderException);
            EXPECT_EQ(2U, r.position());
        }
    }
}
