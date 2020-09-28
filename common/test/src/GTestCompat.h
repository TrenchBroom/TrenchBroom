/*
 Copyright (C) 2020 Eric Wasylishen

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

#define ASSERT_TRUE(x) REQUIRE(x)
#define EXPECT_TRUE(x) CHECK(x)

#define ASSERT_FALSE(x) REQUIRE_FALSE(x)
#define EXPECT_FALSE(x) CHECK_FALSE(x)

#define ASSERT_EQ(x,y) REQUIRE(x == y)
#define EXPECT_EQ(x,y) CHECK(x == y)

#define ASSERT_LT(x,y) REQUIRE(x < y)
#define EXPECT_LT(x,y) CHECK(x < y)

#define ASSERT_GT(x,y) REQUIRE(x > y)
#define EXPECT_GT(x,y) CHECK(x > y)

#define ASSERT_LE(x,y) REQUIRE(x <= y)
#define EXPECT_LE(x,y) CHECK(x <= y)

#define ASSERT_GE(x,y) REQUIRE(x >= y)
#define EXPECT_GE(x,y) CHECK(x >= y)

#define ASSERT_STREQ(x, y) REQUIRE(std::string(x) == std::string(y))
#define EXPECT_STREQ(x,y) CHECK(std::string(x) == std::string(y))

#define ASSERT_NE(x,y) REQUIRE(x != y)
#define EXPECT_NE(x,y) CHECK(x != y)

#define ASSERT_THROW(x,y) REQUIRE_THROWS_AS(x,y)
#define EXPECT_THROW(x,y) CHECK_THROWS_AS(x,y)

#define ASSERT_ANY_THROW(x) REQUIRE_THROWS(x)
#define EXPECT_ANY_THROW(x) CHECK_THROWS(x)

#define ASSERT_NO_THROW(x) REQUIRE_NOTHROW(x)
#define EXPECT_NO_THROW(x) CHECK_NOTHROW(x)

#define ASSERT_DOUBLE_EQ(x, y) REQUIRE(Approx(x) == y)
#define EXPECT_DOUBLE_EQ(x, y) CHECK(Approx(x) == y)

#define ASSERT_FLOAT_EQ(x, y) REQUIRE(Approx(x) == y)
#define EXPECT_FLOAT_EQ(x, y) CHECK(Approx(x) == y)
