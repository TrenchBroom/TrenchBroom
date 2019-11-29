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

#include "CollectionUtils.h"
#include "StringUtils.h"

#include <algorithm>
#include <map>
#include <vector>

class TestObject {
private:
    bool& m_flag;
public:
    TestObject(bool& flag) :
    m_flag(flag) {
        m_flag = false;
    }

    ~TestObject() {
        m_flag = true;
    }
};

TEST(CollectionUtilsTest, vecClearAndDelete) {
    using TestVec = std::vector<TestObject*>;

    static const size_t count = 4;
    TestVec vec;
    bool deleted[count];
    for (size_t i = 0; i < count; i++)
        vec.push_back(new TestObject(deleted[i]));

    VectorUtils::clearAndDelete(vec);
    ASSERT_TRUE(vec.empty());
    for (size_t i = 0; i < count; i++)
        ASSERT_TRUE(deleted[i]);
}

TEST(CollectionUtilsTest, vecSetRemoveSingle) {
    int i1(1);
    int i2(2);
    int i3(3);

    std::vector<int> vec;
    VectorUtils::setInsert(vec, i1);
    VectorUtils::setInsert(vec, i2);

    ASSERT_FALSE(VectorUtils::setRemove(vec, i3));
    ASSERT_TRUE(VectorUtils::setRemove(vec, i1));
    ASSERT_EQ(1u, vec.size());
    ASSERT_EQ(i2, vec[0]);
    ASSERT_FALSE(VectorUtils::setRemove(vec, i1));
    ASSERT_TRUE(VectorUtils::setRemove(vec, i2));
    ASSERT_TRUE(vec.empty());
}

TEST(CollectionUtilsTest, vecSetContains) {
    const String a("a");
    const String b("b");
    const String ab("ab");
    const String c("c");

    StringList set;
    VectorUtils::setInsert(set, a);
    VectorUtils::setInsert(set, b);
    VectorUtils::setInsert(set, ab);
    VectorUtils::setInsert(set, c);

    ASSERT_TRUE(VectorUtils::setContains(set, a));
    ASSERT_TRUE(VectorUtils::setContains(set, b));
    ASSERT_TRUE(VectorUtils::setContains(set, ab));
    ASSERT_TRUE(VectorUtils::setContains(set, c));
    ASSERT_FALSE(VectorUtils::setContains(set, String("d")));
}

TEST(CollectionUtilsTest, vecSetUnion) {
    const int i0(0);
    const int i1(1);
    const int i2(2);
    const int i3(3);
    const int i4(4);
    const int i5(5);
    const int i6(6);
    const int i7(7);
    const int i8(8);
    const int i9(9);

    std::vector<int> set1;
    VectorUtils::setInsert(set1, i0);
    VectorUtils::setInsert(set1, i2);
    VectorUtils::setInsert(set1, i4);
    VectorUtils::setInsert(set1, i7);
    VectorUtils::setInsert(set1, i9);

    std::vector<int> set2;
    VectorUtils::setInsert(set2, i0);
    VectorUtils::setInsert(set2, i1);
    VectorUtils::setInsert(set2, i2);
    VectorUtils::setInsert(set2, i3);
    VectorUtils::setInsert(set2, i4);
    VectorUtils::setInsert(set2, i5);
    VectorUtils::setInsert(set2, i6);
    VectorUtils::setInsert(set2, i8);

    const std::vector<int> uni = VectorUtils::setUnion(set1, set2);
    ASSERT_EQ(10u, uni.size());
    ASSERT_TRUE(VectorUtils::setContains(uni, i0));
    ASSERT_TRUE(VectorUtils::setContains(uni, i1));
    ASSERT_TRUE(VectorUtils::setContains(uni, i2));
    ASSERT_TRUE(VectorUtils::setContains(uni, i3));
    ASSERT_TRUE(VectorUtils::setContains(uni, i4));
    ASSERT_TRUE(VectorUtils::setContains(uni, i5));
    ASSERT_TRUE(VectorUtils::setContains(uni, i6));
    ASSERT_TRUE(VectorUtils::setContains(uni, i7));
    ASSERT_TRUE(VectorUtils::setContains(uni, i8));
    ASSERT_TRUE(VectorUtils::setContains(uni, i9));
}

TEST(CollectionUtilsTest, mapFindOrInsert) {
    using TestMap = std::map<std::string, std::string>;

    TestMap testMap;
    TestMap::iterator it = MapUtils::findOrInsert(testMap, std::string("Key"));
    ASSERT_EQ(1u, testMap.size());
    ASSERT_EQ(std::begin(testMap), it);
    ASSERT_EQ(std::string("Key"), it->first);
    ASSERT_EQ(std::string(""), it->second);
    ASSERT_EQ(it, MapUtils::findOrInsert(testMap, std::string("Key")));
    ASSERT_EQ(1u, testMap.size());

    it = MapUtils::findOrInsert(testMap, std::string("Key2"));
    ASSERT_EQ(2u, testMap.size());
    ASSERT_NE(std::end(testMap), it);
    ASSERT_EQ(std::string("Key2"), it->first);
    ASSERT_EQ(std::string(""), it->second);
}

TEST(CollectionUtilsTest, mapInsertOrReplaceCopy) {
    using TestMap = std::map<std::string, std::string>;

    TestMap testMap;
    const std::string key("Key");
    std::string value1("Value");
    std::string value2("Value2");

    MapUtils::insertOrReplace(testMap, key, value1);
    ASSERT_EQ(1u, testMap.size());
    ASSERT_EQ(value1, testMap[key]);

    MapUtils::insertOrReplace(testMap, key, value2);
    ASSERT_EQ(1u, testMap.size());
    ASSERT_EQ(value2, testMap[key]);
}

TEST(CollectionUtilsTest, mapInsertOrReplaceAndDelete) {
    using TestMap = std::map<std::string, TestObject*>;

    TestMap testMap;
    const std::string key("Key");
    bool deleted1 = false;
    bool deleted2 = false;
    TestObject* value1 = new TestObject(deleted1);
    TestObject* value2 = new TestObject(deleted2);

    MapUtils::insertOrReplaceAndDelete(testMap, key, value1);
    ASSERT_EQ(1u, testMap.size());
    ASSERT_EQ(value1, testMap[key]);
    ASSERT_FALSE(deleted1);

    MapUtils::insertOrReplaceAndDelete(testMap, key, value2);
    ASSERT_EQ(1u, testMap.size());
    ASSERT_EQ(value2, testMap[key]);
    ASSERT_TRUE(deleted1);

    MapUtils::clearAndDelete(testMap);
}

TEST(CollectionUtilsTest, mapClearAndDelete) {
    using TestMap = std::map<std::string, TestObject*>;

    TestMap testMap;
    bool deleted1 = false;
    bool deleted2 = false;
    testMap["k1"] = new TestObject(deleted1);
    testMap["k2"] = new TestObject(deleted2);

    MapUtils::clearAndDelete(testMap);
    ASSERT_TRUE(deleted1);
    ASSERT_TRUE(deleted2);
}

TEST(CollectionUtilsTest, setMinus) {
    ASSERT_EQ(std::set<int>{1}, SetUtils::minus(std::set<int>{1, 2, 3}, std::set<int>{2, 3}));
    ASSERT_EQ(std::set<int>{1}, SetUtils::minus(std::set<int>{1}, std::set<int>{}));
    ASSERT_EQ(std::set<int>{}, SetUtils::minus(std::set<int>{}, std::set<int>{}));
}

TEST(CollectionUtilsTest, setRetainMaximalElements) {
    ASSERT_EQ(std::set<int>{}, SetUtils::findMaximalElements(std::set<int>{}));
    ASSERT_EQ(std::set<int>{1}, SetUtils::findMaximalElements(std::set<int>{1}));
    ASSERT_EQ(std::set<int>{1}, SetUtils::findMaximalElements(std::set<int>{0,1}));
    ASSERT_EQ(std::set<int>{2}, SetUtils::findMaximalElements(std::set<int>{0,1,2}));
}

TEST(CollectionUtilsTest, vectorMap) {
    EXPECT_EQ((std::vector<int>{11, 12, 13}),            VectorUtils::map(std::vector<int>{1,2,3}, [](auto x){return x + 10;}));
    EXPECT_EQ((std::vector<float>{11.0f, 12.0f, 13.0f}), VectorUtils::map(std::vector<int>{1,2,3}, [](auto x){return x + 10.0f;}));
}
