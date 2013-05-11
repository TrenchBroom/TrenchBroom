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

TEST(CollectionUtilsTest, VecDeleteandErase1InRange) {
    typedef std::vector<TestObject*> TestVec;

    static const size_t count = 3;
    TestVec vec;
    bool deleted[count];
    for (size_t i = 0; i < count; i++)
        vec.push_back(new TestObject(deleted[i]));
    
    VectorUtils::deleteAndErase(vec, vec.begin() + 1, vec.end() - 1);
    ASSERT_EQ(2, vec.size());
    ASSERT_FALSE(deleted[0]);
    ASSERT_TRUE(deleted[1]);
    ASSERT_FALSE(deleted[2]);
    VectorUtils::deleteAndClear(vec);
}

TEST(CollectionUtilsTest, VecDeleteAndErase2InRange) {
    typedef std::vector<TestObject*> TestVec;
    
    static const size_t count = 4;
    TestVec vec;
    bool deleted[count];
    for (size_t i = 0; i < count; i++)
        vec.push_back(new TestObject(deleted[i]));
    
    VectorUtils::deleteAndErase(vec, vec.begin() + 1, vec.end() - 1);
    ASSERT_EQ(2, vec.size());
    ASSERT_FALSE(deleted[0]);
    ASSERT_TRUE(deleted[1]);
    ASSERT_TRUE(deleted[2]);
    ASSERT_FALSE(deleted[3]);
    VectorUtils::deleteAndClear(vec);
}

TEST(CollectionUtilsTest, VecDeleteAndEraseAllFrom) {
    typedef std::vector<TestObject*> TestVec;
    
    static const size_t count = 4;
    TestVec vec;
    bool deleted[count];
    for (size_t i = 0; i < count; i++)
        vec.push_back(new TestObject(deleted[i]));
    
    VectorUtils::deleteAndErase(vec, vec.begin() + 2);
    ASSERT_EQ(2, vec.size());
    ASSERT_FALSE(deleted[0]);
    ASSERT_FALSE(deleted[1]);
    ASSERT_TRUE(deleted[2]);
    ASSERT_TRUE(deleted[3]);
    VectorUtils::deleteAndClear(vec);
}

TEST(CollectionUtilsTest, VecDeleteAndClear) {
    typedef std::vector<TestObject*> TestVec;
    
    static const size_t count = 4;
    TestVec vec;
    bool deleted[count];
    for (size_t i = 0; i < count; i++)
        vec.push_back(new TestObject(deleted[i]));
    
    VectorUtils::deleteAndClear(vec);
    ASSERT_TRUE(vec.empty());
    for (size_t i = 0; i < count; i++)
        ASSERT_TRUE(deleted[i]);
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
    
    MapUtils::deleteAndClear(testMap);
}

TEST(CollectionUtilsTest, MapDeleteAndClear) {
    typedef std::map<std::string, TestObject*> TestMap;
    
    TestMap testMap;
    bool deleted1 = false;
    bool deleted2 = false;
    testMap["k1"] = new TestObject(deleted1);
    testMap["k2"] = new TestObject(deleted2);
    
    MapUtils::deleteAndClear(testMap);
    ASSERT_TRUE(deleted1);
    ASSERT_TRUE(deleted2);
}
