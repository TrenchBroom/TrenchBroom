/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

TEST(CollectionUtilsTest, vecShiftLeftEmpty) {
    typedef std::vector<size_t> Vec;
    Vec vec;
    VectorUtils::shiftLeft(vec, 0);
    VectorUtils::shiftLeft(vec, 1);
}

TEST(CollectionUtilsTest, vecShiftLeftBy2) {
    typedef std::vector<size_t> Vec;
    Vec vec;
    
    vec.push_back('a');
    vec.push_back('b');
    vec.push_back('c');
    vec.push_back('d');
    vec.push_back('e');
    vec.push_back('f');
    vec.push_back('g');
    
    Vec leftBy2;
    leftBy2.push_back('c');
    leftBy2.push_back('d');
    leftBy2.push_back('e');
    leftBy2.push_back('f');
    leftBy2.push_back('g');
    leftBy2.push_back('a');
    leftBy2.push_back('b');
    
    Vec actual = vec;
    VectorUtils::shiftLeft(actual, 2);
    ASSERT_TRUE(std::equal(leftBy2.begin(), leftBy2.end(), actual.begin()));
}

TEST(CollectionUtilsTest, vecShiftLeftBySize) {
    typedef std::vector<size_t> Vec;
    Vec vec;
    
    vec.push_back('a');
    vec.push_back('b');
    vec.push_back('c');
    vec.push_back('d');
    vec.push_back('e');
    vec.push_back('f');
    vec.push_back('g');
    
    Vec actual = vec;
    VectorUtils::shiftLeft(actual, vec.size());
    ASSERT_TRUE(std::equal(vec.begin(), vec.end(), actual.begin()));
}

TEST(CollectionUtilsTest, vecShiftLeftByMoreThanSize) {
    typedef std::vector<size_t> Vec;
    Vec vec;
    
    vec.push_back('a');
    vec.push_back('b');
    vec.push_back('c');
    vec.push_back('d');
    vec.push_back('e');
    vec.push_back('f');
    vec.push_back('g');
    
    Vec leftBy10;
    leftBy10.push_back('d');
    leftBy10.push_back('e');
    leftBy10.push_back('f');
    leftBy10.push_back('g');
    leftBy10.push_back('a');
    leftBy10.push_back('b');
    leftBy10.push_back('c');

    Vec actual = vec;
    VectorUtils::shiftLeft(actual, 10);
    ASSERT_TRUE(std::equal(leftBy10.begin(), leftBy10.end(), actual.begin()));
}

TEST(CollectionUtilsTest, vecShiftRightEmpty) {
    typedef std::vector<size_t> Vec;
    Vec vec;
    VectorUtils::shiftRight(vec, 0);
    VectorUtils::shiftRight(vec, 1);
}

TEST(CollectionUtilsTest, vecShiftRightBy2) {
    typedef std::vector<size_t> Vec;
    Vec vec;
    
    vec.push_back('a');
    vec.push_back('b');
    vec.push_back('c');
    vec.push_back('d');
    vec.push_back('e');
    vec.push_back('f');
    vec.push_back('g');
    
    Vec rightBy2;
    rightBy2.push_back('f');
    rightBy2.push_back('g');
    rightBy2.push_back('a');
    rightBy2.push_back('b');
    rightBy2.push_back('c');
    rightBy2.push_back('d');
    rightBy2.push_back('e');
    
    Vec actual = vec;
    VectorUtils::shiftRight(actual, 2);
    ASSERT_TRUE(std::equal(rightBy2.begin(), rightBy2.end(), actual.begin()));
}

TEST(CollectionUtilsTest, vecShiftRightBySize) {
    typedef std::vector<size_t> Vec;
    Vec vec;
    
    vec.push_back('a');
    vec.push_back('b');
    vec.push_back('c');
    vec.push_back('d');
    vec.push_back('e');
    vec.push_back('f');
    vec.push_back('g');
    
    Vec actual = vec;
    VectorUtils::shiftRight(actual, vec.size());
    ASSERT_TRUE(std::equal(vec.begin(), vec.end(), actual.begin()));
}

TEST(CollectionUtilsTest, vecShiftRightByMoreThanSize) {
    typedef std::vector<size_t> Vec;
    Vec vec;
    
    vec.push_back('a');
    vec.push_back('b');
    vec.push_back('c');
    vec.push_back('d');
    vec.push_back('e');
    vec.push_back('f');
    vec.push_back('g');
    
    Vec rightBy10;
    rightBy10.push_back('e');
    rightBy10.push_back('f');
    rightBy10.push_back('g');
    rightBy10.push_back('a');
    rightBy10.push_back('b');
    rightBy10.push_back('c');
    rightBy10.push_back('d');
    
    Vec actual = vec;
    VectorUtils::shiftRight(actual, 10);
    ASSERT_TRUE(std::equal(rightBy10.begin(), rightBy10.end(), actual.begin()));
}

TEST(CollectionUtilsTest, vecEraseAndDelete1InRange) {
    typedef std::vector<TestObject*> TestVec;

    static const size_t count = 3;
    TestVec vec;
    bool deleted[count];
    for (size_t i = 0; i < count; i++)
        vec.push_back(new TestObject(deleted[i]));
    
    VectorUtils::eraseAndDelete(vec, vec.begin() + 1, vec.end() - 1);
    ASSERT_EQ(2u, vec.size());
    ASSERT_FALSE(deleted[0]);
    ASSERT_TRUE(deleted[1]);
    ASSERT_FALSE(deleted[2]);
    VectorUtils::clearAndDelete(vec);
}

TEST(CollectionUtilsTest, vecEraseAndDelete2InRange) {
    typedef std::vector<TestObject*> TestVec;
    
    static const size_t count = 4;
    TestVec vec;
    bool deleted[count];
    for (size_t i = 0; i < count; i++)
        vec.push_back(new TestObject(deleted[i]));
    
    VectorUtils::eraseAndDelete(vec, vec.begin() + 1, vec.end() - 1);
    ASSERT_EQ(2u, vec.size());
    ASSERT_FALSE(deleted[0]);
    ASSERT_TRUE(deleted[1]);
    ASSERT_TRUE(deleted[2]);
    ASSERT_FALSE(deleted[3]);
    VectorUtils::clearAndDelete(vec);
}

TEST(CollectionUtilsTest, vecEraseAndDeleteAllFrom) {
    typedef std::vector<TestObject*> TestVec;
    
    static const size_t count = 4;
    TestVec vec;
    bool deleted[count];
    for (size_t i = 0; i < count; i++)
        vec.push_back(new TestObject(deleted[i]));
    
    VectorUtils::eraseAndDelete(vec, vec.begin() + 2);
    ASSERT_EQ(2u, vec.size());
    ASSERT_FALSE(deleted[0]);
    ASSERT_FALSE(deleted[1]);
    ASSERT_TRUE(deleted[2]);
    ASSERT_TRUE(deleted[3]);
    VectorUtils::clearAndDelete(vec);
}

TEST(CollectionUtilsTest, vecClearAndDelete) {
    typedef std::vector<TestObject*> TestVec;
    
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

TEST(CollectionUtilsTest, vecRemove) {
    typedef std::vector<TestObject*> TestVec;
    
    static const size_t count = 4;
    TestVec vec;
    bool deleted[count];
    for (size_t i = 0; i < count; i++)
        vec.push_back(new TestObject(deleted[i]));
    
    VectorUtils::remove(vec, vec[2]);
    ASSERT_EQ(count - 1, vec.size());
    ASSERT_FALSE(deleted[2]);
    VectorUtils::clearAndDelete(vec);
}

TEST(CollectionUtilsTest, vecRemoveAndDelete) {
    typedef std::vector<TestObject*> TestVec;
    
    static const size_t count = 4;
    TestVec vec;
    bool deleted[count];
    for (size_t i = 0; i < count; i++)
        vec.push_back(new TestObject(deleted[i]));
    
    VectorUtils::removeAndDelete(vec, vec[2]);
    ASSERT_EQ(count - 1, vec.size());
    ASSERT_TRUE(deleted[2]);
    VectorUtils::clearAndDelete(vec);
}

TEST(CollectionUtilsTest, vecContains) {
    typedef std::vector<int> TestVec;
    TestVec vec;
    
    vec.push_back(10);
    vec.push_back(4);
    vec.push_back(-232);
    vec.push_back(11111);
    
    ASSERT_TRUE(VectorUtils::contains(vec, 10));
    ASSERT_TRUE(VectorUtils::contains(vec, 4));
    ASSERT_TRUE(VectorUtils::contains(vec, -232));
    ASSERT_TRUE(VectorUtils::contains(vec, 11111));
    ASSERT_FALSE(VectorUtils::contains(vec, 11));
    ASSERT_FALSE(VectorUtils::contains(vec, 0));
    ASSERT_FALSE(VectorUtils::contains(vec, 110));
}

TEST(CollectionUtilsTest, vecContainsPtr) {
    typedef std::vector<int*> TestVec;
    TestVec vec;
    
    vec.push_back(new int(10));
    vec.push_back(new int(4));
    vec.push_back(new int(-232));
    vec.push_back(new int(11111));
    
    const int* i10 = new int(10);
    const int* i4 = new int(4);
    const int* i232 = new int(-232);
    const int* i11111 = new int(11111);
    const int* i11 = new int(11);
    const int* i0 = new int(0);
    const int* i110 = new int(110);
    
    ASSERT_TRUE(VectorUtils::contains(vec, i10));
    ASSERT_TRUE(VectorUtils::contains(vec, i4));
    ASSERT_TRUE(VectorUtils::contains(vec, i232));
    ASSERT_TRUE(VectorUtils::contains(vec, i11111));
    ASSERT_FALSE(VectorUtils::contains(vec, i11));
    ASSERT_FALSE(VectorUtils::contains(vec, i0));
    ASSERT_FALSE(VectorUtils::contains(vec, i110));
    
    VectorUtils::clearAndDelete(vec);
    delete i10;
    delete i4;
    delete i232;
    delete i11111;
    delete i11;
    delete i0;
    delete i110;
}

TEST(CollectionUtilsTest, vecSetInsertSingle) {
    int i1(1);
    int i2(2);
    
    std::vector<int> vec;
    ASSERT_TRUE(VectorUtils::setInsert(vec, i1));
    ASSERT_TRUE(VectorUtils::contains(vec, i1));
    ASSERT_TRUE(VectorUtils::setInsert(vec, i2));
    ASSERT_TRUE(VectorUtils::contains(vec, i2));
    ASSERT_FALSE(VectorUtils::setInsert(vec, i1));
    ASSERT_EQ(2u, vec.size());
    ASSERT_TRUE(VectorUtils::contains(vec, i1));
    ASSERT_TRUE(VectorUtils::contains(vec, i2));
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

TEST(CollectionUtilsTest, vecSetInsertRange) {
    int i1(1);
    int i2(2);
    
    std::vector<int> range;
    range.push_back(i2);
    range.push_back(i2);
    range.push_back(i1);
    
    std::vector<int> set;
    VectorUtils::setInsert(set, range.begin(), range.end());
    ASSERT_EQ(2u, set.size());
    ASSERT_EQ(i1, set[0]);
    ASSERT_EQ(i2, set[1]);
}

TEST(CollectionUtilsTest, mapFindOrInsert) {
    typedef std::map<std::string, std::string> TestMap;

    TestMap testMap;
    TestMap::iterator it = MapUtils::findOrInsert(testMap, std::string("Key"));
    ASSERT_EQ(1u, testMap.size());
    ASSERT_EQ(testMap.begin(), it);
    ASSERT_EQ(std::string("Key"), it->first);
    ASSERT_EQ(std::string(""), it->second);
    ASSERT_EQ(it, MapUtils::findOrInsert(testMap, std::string("Key")));
    ASSERT_EQ(1u, testMap.size());
    
    it = MapUtils::findOrInsert(testMap, std::string("Key2"));
    ASSERT_EQ(2u, testMap.size());
    ASSERT_NE(testMap.end(), it);
    ASSERT_EQ(std::string("Key2"), it->first);
    ASSERT_EQ(std::string(""), it->second);
}

TEST(CollectionUtilsTest, mapInsertOrReplaceCopy) {
    typedef std::map<std::string, std::string> TestMap;

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

TEST(CollectionUtilsTest, mapInsertOrReplacePointer) {
    typedef std::map<std::string, TestObject*> TestMap;
    
    TestMap testMap;
    const std::string key("Key");
    bool deleted1 = false;
    bool deleted2 = false;
    TestObject* value1 = new TestObject(deleted1);
    TestObject* value2 = new TestObject(deleted2);
    
    MapUtils::insertOrReplace(testMap, key, value1);
    ASSERT_EQ(1u, testMap.size());
    ASSERT_EQ(value1, testMap[key]);
    ASSERT_FALSE(deleted1);
    
    MapUtils::insertOrReplace(testMap, key, value2);
    ASSERT_EQ(1u, testMap.size());
    ASSERT_EQ(value2, testMap[key]);
    ASSERT_TRUE(deleted1);
    
    MapUtils::clearAndDelete(testMap);
}

TEST(CollectionUtilsTest, mapClearAndDelete) {
    typedef std::map<std::string, TestObject*> TestMap;
    
    TestMap testMap;
    bool deleted1 = false;
    bool deleted2 = false;
    testMap["k1"] = new TestObject(deleted1);
    testMap["k2"] = new TestObject(deleted2);
    
    MapUtils::clearAndDelete(testMap);
    ASSERT_TRUE(deleted1);
    ASSERT_TRUE(deleted2);
}
