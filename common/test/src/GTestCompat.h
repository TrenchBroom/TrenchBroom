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
