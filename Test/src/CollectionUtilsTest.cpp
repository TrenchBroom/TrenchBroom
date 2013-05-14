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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>

#include "CollectionUtils.h"

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

TEST(CollectionUtilsTest, VecEraseAndDelete1InRange) {
    typedef std::vector<TestObject*> TestVec;

    static const size_t count = 3;
    TestVec vec;
    bool deleted[count];
    for (size_t i = 0; i < count; i++)
        vec.push_back(new TestObject(deleted[i]));
    
    VectorUtils::eraseAndDelete(vec, vec.begin() + 1, vec.end() - 1);
    ASSERT_EQ(2, vec.size());
    ASSERT_FALSE(deleted[0]);
    ASSERT_TRUE(deleted[1]);
    ASSERT_FALSE(deleted[2]);
    VectorUtils::clearAndDelete(vec);
}

TEST(CollectionUtilsTest, VecEraseAndDelete2InRange) {
    typedef std::vector<TestObject*> TestVec;
    
    static const size_t count = 4;
    TestVec vec;
    bool deleted[count];
    for (size_t i = 0; i < count; i++)
        vec.push_back(new TestObject(deleted[i]));
    
    VectorUtils::eraseAndDelete(vec, vec.begin() + 1, vec.end() - 1);
    ASSERT_EQ(2, vec.size());
    ASSERT_FALSE(deleted[0]);
    ASSERT_TRUE(deleted[1]);
    ASSERT_TRUE(deleted[2]);
    ASSERT_FALSE(deleted[3]);
    VectorUtils::clearAndDelete(vec);
}

TEST(CollectionUtilsTest, VecEraseAndDeleteAllFrom) {
    typedef std::vector<TestObject*> TestVec;
    
    static const size_t count = 4;
    TestVec vec;
    bool deleted[count];
    for (size_t i = 0; i < count; i++)
        vec.push_back(new TestObject(deleted[i]));
    
    VectorUtils::eraseAndDelete(vec, vec.begin() + 2);
    ASSERT_EQ(2, vec.size());
    ASSERT_FALSE(deleted[0]);
    ASSERT_FALSE(deleted[1]);
    ASSERT_TRUE(deleted[2]);
    ASSERT_TRUE(deleted[3]);
    VectorUtils::clearAndDelete(vec);
}

TEST(CollectionUtilsTest, VecClearAndDelete) {
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

TEST(CollectionUtilsTest, VecRemove) {
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

TEST(CollectionUtilsTest, VecRemoveAndDelete) {
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

TEST(CollectionUtilsTest, VecContains) {
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

TEST(CollectionUtilsTest, VecContainsPtr) {
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

TEST(CollectionUtilsTest, MapInsertOrReplaceCopy) {
    typedef std::map<std::string, std::string> TestMap;

    TestMap testMap;
    const std::string key("Key");
    std::string value1("Value");
    std::string value2("Value2");
    
    MapUtils::insertOrReplace(testMap, key, value1);
    ASSERT_EQ(1, testMap.size());
    ASSERT_EQ(value1, testMap[key]);
    
    MapUtils::insertOrReplace(testMap, key, value2);
    ASSERT_EQ(1, testMap.size());
    ASSERT_EQ(value2, testMap[key]);
}

TEST(CollectionUtilsTest, MapInsertOrReplacePointer) {
    typedef std::map<std::string, TestObject*> TestMap;
    
    TestMap testMap;
    const std::string key("Key");
    bool deleted1 = false;
    bool deleted2 = false;
    TestObject* value1 = new TestObject(deleted1);
    TestObject* value2 = new TestObject(deleted2);
    
    MapUtils::insertOrReplace(testMap, key, value1);
    ASSERT_EQ(1, testMap.size());
    ASSERT_EQ(value1, testMap[key]);
    ASSERT_FALSE(deleted1);
    
    MapUtils::insertOrReplace(testMap, key, value2);
    ASSERT_EQ(1, testMap.size());
    ASSERT_EQ(value2, testMap[key]);
    ASSERT_TRUE(deleted1);
    
    MapUtils::clearAndDelete(testMap);
}

TEST(CollectionUtilsTest, MapClearAndDelete) {
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
