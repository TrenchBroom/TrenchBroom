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
#include "Exceptions.h"
#include "StringIndex.h"
#include "StringUtils.h"

namespace TrenchBroom {
    TEST(StringIndexTest, insert) {
        StringIndex<String> index;
        index.insert("key", "value");
        index.insert("key2", "value");
        index.insert("key22", "value2");
        index.insert("k1", "value3");
        index.insert("test", "value4");
        
        StringList result1 = index.queryPrefixMatches("woops");
        ASSERT_TRUE(result1.empty());
        
        StringList result2 = index.queryPrefixMatches("key222");
        ASSERT_TRUE(result2.empty());
        
        StringList result3 = index.queryPrefixMatches("key");
        ASSERT_EQ(2u, result3.size());
        ASSERT_TRUE(VectorUtils::contains(result3, String("value")));
        ASSERT_TRUE(VectorUtils::contains(result3, String("value2")));
        
        StringList result4 = index.queryPrefixMatches("k");
        ASSERT_EQ(3u, result4.size());
        ASSERT_TRUE(VectorUtils::contains(result4, String("value")));
        ASSERT_TRUE(VectorUtils::contains(result4, String("value2")));
        ASSERT_TRUE(VectorUtils::contains(result4, String("value3")));

        StringList result5 = index.queryPrefixMatches("test");
        ASSERT_EQ(1u, result5.size());
        ASSERT_TRUE(VectorUtils::contains(result5, String("value4")));
        
        index.insert("k", "value4");

        StringList result6 = index.queryPrefixMatches("k");
        ASSERT_EQ(4u, result6.size());
        ASSERT_TRUE(VectorUtils::contains(result6, String("value")));
        ASSERT_TRUE(VectorUtils::contains(result6, String("value2")));
        ASSERT_TRUE(VectorUtils::contains(result6, String("value3")));
        ASSERT_TRUE(VectorUtils::contains(result6, String("value4")));
        
        StringList result7 = index.queryPrefixMatches("");
        ASSERT_EQ(4u, result7.size());
        ASSERT_TRUE(VectorUtils::contains(result7, String("value")));
        ASSERT_TRUE(VectorUtils::contains(result7, String("value2")));
        ASSERT_TRUE(VectorUtils::contains(result7, String("value3")));
        ASSERT_TRUE(VectorUtils::contains(result7, String("value4")));
    }
    
    TEST(StringIndexTest, remove) {
        StringIndex<String> index;
        index.insert("andrew", "value");
        index.insert("andreas", "value");
        index.insert("andrar", "value2");
        index.insert("andrary", "value3");
        index.insert("andy", "value4");

        ASSERT_THROW(index.remove("andrary", "value2"), Exception);
        
        index.remove("andrary", "value3");
        
        StringList result1 = index.queryPrefixMatches("andrary");
        ASSERT_TRUE(result1.empty());
        
        StringList result2 = index.queryPrefixMatches("andrar");
        ASSERT_EQ(1u, result2.size());
        ASSERT_TRUE(VectorUtils::contains(result2, String("value2")));

        index.remove("andrar", "value2");
        
        StringList result3 = index.queryPrefixMatches("andrar");
        ASSERT_TRUE(result3.empty());
        
        StringList result4 = index.queryPrefixMatches("andre");
        ASSERT_EQ(1u, result4.size());
        ASSERT_TRUE(VectorUtils::contains(result4, String("value")));

        StringList result5 = index.queryPrefixMatches("andreas");
        ASSERT_EQ(1u, result5.size());
        ASSERT_TRUE(VectorUtils::contains(result5, String("value")));

        index.remove("andy", "value4");
        ASSERT_TRUE(index.queryPrefixMatches("andy").empty());
        ASSERT_EQ(index.queryExactMatches("andreas"), StringList(1, String("value")));
        ASSERT_EQ(index.queryExactMatches("andrew"), StringList(1, String("value")));
        
        index.remove("andreas", "value");
        ASSERT_TRUE(index.queryPrefixMatches("andreas").empty());
        ASSERT_EQ(index.queryPrefixMatches("andrew"), StringList(1, String("value")));
        
        index.remove("andrew", "value");
        ASSERT_TRUE(index.queryPrefixMatches("andrew").empty());
    }

    TEST(StringIndexTest, queryExactMatches) {
        StringIndex<String> index;
        index.insert("key", "value");
        index.insert("key2", "value");
        index.insert("key22", "value2");
        index.insert("k1", "value3");
        
        StringList result1 = index.queryExactMatches("woops");
        ASSERT_TRUE(result1.empty());
        
        StringList result2 = index.queryExactMatches("key222");
        ASSERT_TRUE(result2.empty());
        
        StringList result3 = index.queryExactMatches("key");
        ASSERT_EQ(1u, result3.size());
        ASSERT_TRUE(VectorUtils::contains(result3, String("value")));
        
        StringList result4 = index.queryExactMatches("k");
        ASSERT_TRUE(result4.empty());
        
        index.insert("key", "value4");
        StringList result5 = index.queryExactMatches("key");
        ASSERT_EQ(2u, result5.size());
        ASSERT_TRUE(VectorUtils::contains(result5, String("value")));
        ASSERT_TRUE(VectorUtils::contains(result5, String("value4")));

        StringList result6 = index.queryExactMatches("");
        ASSERT_TRUE(result6.empty());
    }
    
    TEST(StringIndexTest, queryNumberedMatches) {
        StringIndex<String> index;
        index.insert("key", "value");
        index.insert("key2", "value");
        index.insert("key22", "value2");
        index.insert("key22bs", "value4");
        index.insert("k1", "value3");

        StringList result1 = index.queryNumberedMatches("woops");
        ASSERT_TRUE(result1.empty());

        StringList result2 = index.queryNumberedMatches("key");
        ASSERT_EQ(2u, result2.size());
        ASSERT_TRUE(VectorUtils::contains(result2, String("value")));
        ASSERT_TRUE(VectorUtils::contains(result2, String("value2")));
        
        StringList result3 = index.queryNumberedMatches("key2");
        ASSERT_EQ(2u, result3.size());
        ASSERT_TRUE(VectorUtils::contains(result3, String("value")));
        ASSERT_TRUE(VectorUtils::contains(result3, String("value2")));
        
        StringList result4 = index.queryNumberedMatches("k");
        ASSERT_EQ(1u, result4.size());
        ASSERT_TRUE(VectorUtils::contains(result4, String("value3")));
        
        index.remove("k1", "value3");

        StringList result5 = index.queryNumberedMatches("k");
        ASSERT_TRUE(result5.empty());
    }
}
