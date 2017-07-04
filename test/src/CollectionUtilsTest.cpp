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
    ASSERT_TRUE(std::equal(std::begin(leftBy2), std::end(leftBy2), std::begin(actual)));
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
    ASSERT_TRUE(std::equal(std::begin(vec), std::end(vec), std::begin(actual)));
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
    ASSERT_TRUE(std::equal(std::begin(leftBy10), std::end(leftBy10), std::begin(actual)));
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
    ASSERT_TRUE(std::equal(std::begin(rightBy2), std::end(rightBy2), std::begin(actual)));
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
    ASSERT_TRUE(std::equal(std::begin(vec), std::end(vec), std::begin(actual)));
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
    ASSERT_TRUE(std::equal(std::begin(rightBy10), std::end(rightBy10), std::begin(actual)));
}

TEST(CollectionUtilsTest, vecEraseAndDelete1InRange) {
    typedef std::vector<TestObject*> TestVec;

    static const size_t count = 3;
    TestVec vec;
    bool deleted[count];
    for (size_t i = 0; i < count; i++)
        vec.push_back(new TestObject(deleted[i]));
    
    VectorUtils::eraseAndDelete(vec, std::begin(vec) + 1, std::end(vec) - 1);
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
    
    VectorUtils::eraseAndDelete(vec, std::begin(vec) + 1, std::end(vec) - 1);
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
    
    VectorUtils::eraseAndDelete(vec, std::begin(vec) + 2);
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
    
    VectorUtils::erase(vec, vec[2]);
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
    
    VectorUtils::eraseAndDelete(vec, vec[2]);
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
    
    ASSERT_TRUE(VectorUtils::containsPtr(vec, i10));
    ASSERT_TRUE(VectorUtils::containsPtr(vec, i4));
    ASSERT_TRUE(VectorUtils::containsPtr(vec, i232));
    ASSERT_TRUE(VectorUtils::containsPtr(vec, i11111));
    ASSERT_FALSE(VectorUtils::containsPtr(vec, i11));
    ASSERT_FALSE(VectorUtils::containsPtr(vec, i0));
    ASSERT_FALSE(VectorUtils::containsPtr(vec, i110));
    
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
    VectorUtils::setInsert(set, std::begin(range), std::end(range));
    ASSERT_EQ(2u, set.size());
    ASSERT_EQ(i1, set[0]);
    ASSERT_EQ(i2, set[1]);
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

TEST(CollectionUtilsTest, vecSetMinus) {
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
    
    const std::vector<int> uni = VectorUtils::setMinus(set1, set2);
    ASSERT_EQ(2u, uni.size());
    ASSERT_TRUE(VectorUtils::setContains(uni, i7));
    ASSERT_TRUE(VectorUtils::setContains(uni, i9));
}

TEST(CollectionUtilsTest, vecSetIntersection) {
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
    
    const std::vector<int> uni = VectorUtils::setIntersection(set1, set2);
    ASSERT_EQ(3u, uni.size());
    ASSERT_TRUE(VectorUtils::setContains(uni, i0));
    ASSERT_TRUE(VectorUtils::setContains(uni, i2));
    ASSERT_TRUE(VectorUtils::setContains(uni, i4));
}

TEST(CollectionUtilsTest, vecOrderedDifference) {
    std::vector<int> minuend;
    std::vector<int> subtrahend;
    
    minuend.push_back(1);
    minuend.push_back(3);
    minuend.push_back(4);
    minuend.push_back(5);
    minuend.push_back(7);
    minuend.push_back(8);
    minuend.push_back(8);
    minuend.push_back(8);
    minuend.push_back(8);
    minuend.push_back(9);
    minuend.push_back(10);
    minuend.push_back(10);
    minuend.push_back(10);
    
    subtrahend.push_back(2);
    subtrahend.push_back(2);
    subtrahend.push_back(4);
    subtrahend.push_back(4);
    subtrahend.push_back(8);
    subtrahend.push_back(9);
    
    VectorUtils::orderedDifference(minuend, subtrahend);
    
    ASSERT_EQ(7u, minuend.size());
    ASSERT_EQ(1, minuend[0]);
    ASSERT_EQ(3, minuend[1]);
    ASSERT_EQ(5, minuend[2]);
    ASSERT_EQ(7, minuend[3]);
    ASSERT_EQ(10, minuend[4]);
    ASSERT_EQ(10, minuend[5]);
    ASSERT_EQ(10, minuend[6]);
}

TEST(CollectionUtilsTest, mapFindOrInsert) {
    typedef std::map<std::string, std::string> TestMap;

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

TEST(CollectionUtilsTest, mapInsertOrReplaceAndDelete) {
    typedef std::map<std::string, TestObject*> TestMap;
    
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

TEST(CollectionUtilsTest, setSubset) {
    ASSERT_TRUE(SetUtils::subset(std::set<int>{1}, std::set<int>{1, 2, 3}));
    ASSERT_TRUE(SetUtils::subset(std::set<int>{1}, std::set<int>{1}));
    ASSERT_TRUE(SetUtils::subset(std::set<int>{}, std::set<int>{1}));
    ASSERT_TRUE(SetUtils::subset(std::set<int>{}, std::set<int>{}));

    ASSERT_FALSE(SetUtils::subset(std::set<int>{4}, std::set<int>{1, 2, 3}));
    ASSERT_FALSE(SetUtils::subset(std::set<int>{0, 1}, std::set<int>{1}));
}

TEST(CollectionUtilsTest, setFindSupersets) {
    typedef std::set<int> S;
    typedef std::set<S> SS;
    
    ASSERT_EQ(SS{}, SetUtils::findSupersets(SS{}));
    ASSERT_EQ(SS{S{1}}, SetUtils::findSupersets(SS{S{1}}));
    ASSERT_EQ((SS{S{1,2}}), SetUtils::findSupersets(SS{S{1},S{1,2}}));
    ASSERT_EQ((SS{S{1,2}}), SetUtils::findSupersets(SS{S{1},S{2},S{1,2}}));
    ASSERT_EQ((SS{S{1,2},S{3}}), SetUtils::findSupersets(SS{S{1},S{2},S{3},S{1,2}}));
    ASSERT_EQ((SS{S{1,2},S{2,1},S{3}}), SetUtils::findSupersets(SS{S{1},S{2},S{3},S{1,2},S{2,1}}));
    ASSERT_EQ((SS{S{1,2},S{3,1}}), SetUtils::findSupersets(SS{S{1},S{2},S{3},S{1,2},S{3,1}}));
}

TEST(CollectionUtilsTest, setMinus) {
    ASSERT_EQ(std::set<int>{1}, SetUtils::minus(std::set<int>{1, 2, 3}, std::set<int>{2, 3}));
    ASSERT_EQ(std::set<int>{1}, SetUtils::minus(std::set<int>{1}, std::set<int>{}));
    ASSERT_EQ(std::set<int>{}, SetUtils::minus(std::set<int>{}, std::set<int>{}));
}

TEST(CollectionUtilsTest, setIntersectionEmpty) {
    ASSERT_TRUE(SetUtils::intersectionEmpty(std::set<int>{1, 2, 3}, std::set<int>{4, 5, 6}));
    ASSERT_TRUE(SetUtils::intersectionEmpty(std::set<int>{1, 2, 3}, std::set<int>{}));
    ASSERT_TRUE(SetUtils::intersectionEmpty(std::set<int>{}, std::set<int>{}));
    
    ASSERT_FALSE(SetUtils::intersectionEmpty(std::set<int>{1, 2, 3}, std::set<int>{3, 4, 5}));
    ASSERT_FALSE(SetUtils::intersectionEmpty(std::set<int>{1, 2, 3}, std::set<int>{1, 2, 3, 4}));
    ASSERT_FALSE(SetUtils::intersectionEmpty(std::set<int>{1, 2, 3, 6}, std::set<int>{4, 5, 6}));
}

TEST(CollectionUtilsTest, setPowerSet) {
    typedef std::set<int> IntSet;
    typedef std::set<IntSet> PSet;

    ASSERT_EQ(PSet{ IntSet{} }, SetUtils::powerSet(IntSet{}));
    ASSERT_EQ((PSet{
        IntSet{},
        IntSet{1}
    }), SetUtils::powerSet(IntSet{1}));
    ASSERT_EQ((PSet{
        IntSet{},
        IntSet{1},
        IntSet{2},
        IntSet{3},
        IntSet{1,2},
        IntSet{2,3},
        IntSet{1,3},
        IntSet{1,2,3}
    }), SetUtils::powerSet(IntSet{1,2,3}));
}

TEST(CollectionUtilsTest, setRetainMaximalElements) {
    ASSERT_EQ(std::set<int>{}, SetUtils::findMaximalElements(std::set<int>{}));
    ASSERT_EQ(std::set<int>{1}, SetUtils::findMaximalElements(std::set<int>{1}));
    ASSERT_EQ(std::set<int>{1}, SetUtils::findMaximalElements(std::set<int>{0,1}));
    ASSERT_EQ(std::set<int>{2}, SetUtils::findMaximalElements(std::set<int>{0,1,2}));
}

TEST(CollectionUtilsTest, listReplaceEmpty) {
    std::list<int> list1 {0, 1, 2, 3};
    std::list<int> list2;
    
    auto replacePos = std::begin(list1);
    ASSERT_EQ(0, *replacePos);
    
    // this will be a no-op because list2 is empty
    
    const auto it = ListUtils::replace(list1, replacePos, list2);
    ASSERT_EQ(0, *it);
    
    ASSERT_EQ(std::list<int>(), list2);
    ASSERT_EQ((std::list<int>{0, 1, 2, 3}), list1);
}

TEST(CollectionUtilsTest, listReplace) {
    std::list<int> list1 {0, 1, 2, 3};
    std::list<int> list2 {100, 200};
    
    // this will replace the element "2" with {100,200}
    
    auto replacePos = std::begin(list1);
    std::advance(replacePos, 2);
    ASSERT_EQ(2, *replacePos);
    
    const auto it = ListUtils::replace(list1, replacePos, list2);
    
    ASSERT_EQ(std::list<int>(), list2);
    ASSERT_EQ((std::list<int>{0, 1, 100, 200, 3}), list1);
    ASSERT_EQ(100, *it);
}
