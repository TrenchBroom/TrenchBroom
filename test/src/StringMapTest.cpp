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
#include "Exceptions.h"
#include "StringMap.h"
#include "StringUtils.h"

namespace TrenchBroom {
    typedef StringMap<String, StringMultiMapValueContainer<String> > TestMultiMap;
    
    TEST(StringMultiMapTest, insert) {
        TestMultiMap index;
        index.insert("key", "value");
        index.insert("key2", "value");
        index.insert("key22", "value2");
        index.insert("k1", "value3");
        index.insert("test", "value4");
        
        StringSet result1 = index.queryPrefixMatches("woops");
        ASSERT_TRUE(result1.empty());
        
        StringSet result2 = index.queryPrefixMatches("key222");
        ASSERT_TRUE(result2.empty());
        
        StringSet result3 = index.queryPrefixMatches("key");
        ASSERT_EQ(2u, result3.size());
        ASSERT_TRUE(result3.count("value") > 0);
        ASSERT_TRUE(result3.count("value2") > 0);
        
        StringSet result4 = index.queryPrefixMatches("k");
        ASSERT_EQ(3u, result4.size());
        ASSERT_TRUE(result4.count("value") > 0);
        ASSERT_TRUE(result4.count("value2") > 0);
        ASSERT_TRUE(result4.count("value3") > 0);

        StringSet result5 = index.queryPrefixMatches("test");
        ASSERT_EQ(1u, result5.size());
        ASSERT_TRUE(result5.count("value4") > 0);
        
        index.insert("k", "value4");

        StringSet result6 = index.queryPrefixMatches("k");
        ASSERT_EQ(4u, result6.size());
        ASSERT_TRUE(result6.count("value") > 0);
        ASSERT_TRUE(result6.count("value2") > 0);
        ASSERT_TRUE(result6.count("value3") > 0);
        ASSERT_TRUE(result6.count("value4") > 0);
        
        StringSet result7 = index.queryPrefixMatches("");
        ASSERT_EQ(4u, result7.size());
        ASSERT_TRUE(result7.count("value") > 0);
        ASSERT_TRUE(result7.count("value2") > 0);
        ASSERT_TRUE(result7.count("value3") > 0);
        ASSERT_TRUE(result7.count("value4") > 0);
    }
    
    TEST(StringMultiMapTest, remove) {
        TestMultiMap index;
        index.insert("andrew", "value");
        index.insert("andreas", "value");
        index.insert("andrar", "value2");
        index.insert("andrary", "value3");
        index.insert("andy", "value4");

        ASSERT_THROW(index.remove("andrary", "value2"), Exception);
        
        index.remove("andrary", "value3");
        
        StringSet result1 = index.queryPrefixMatches("andrary");
        ASSERT_TRUE(result1.empty());
        
        StringSet result2 = index.queryPrefixMatches("andrar");
        ASSERT_EQ(1u, result2.size());
        ASSERT_TRUE(result2.count("value2") > 0);

        index.remove("andrar", "value2");
        
        StringSet result3 = index.queryPrefixMatches("andrar");
        ASSERT_TRUE(result3.empty());
        
        StringSet result4 = index.queryPrefixMatches("andre");
        ASSERT_EQ(1u, result4.size());
        ASSERT_TRUE(result4.count("value") > 0);

        StringSet result5 = index.queryPrefixMatches("andreas");
        ASSERT_EQ(1u, result5.size());
        ASSERT_TRUE(result5.count("value") > 0);

        index.remove("andy", "value4");
        ASSERT_TRUE(index.queryPrefixMatches("andy").empty());
        
        StringSet result6 = index.queryExactMatches("andreas");
        ASSERT_EQ(1u, result6.size());
        ASSERT_TRUE(result6.count("value") > 0);
        
        StringSet result7 = index.queryExactMatches("andrew");
        ASSERT_EQ(1u, result7.size());
        ASSERT_TRUE(result7.count("value") > 0);
        
        index.remove("andreas", "value");
        ASSERT_TRUE(index.queryPrefixMatches("andreas").empty());
        
        StringSet result8 = index.queryPrefixMatches("andrew");
        ASSERT_EQ(1u, result8.size());
        ASSERT_TRUE(result8.count("value") > 0);
        
        index.remove("andrew", "value");
        ASSERT_TRUE(index.queryPrefixMatches("andrew").empty());
    }

    TEST(StringMultiMapTest, queryExactMatches) {
        TestMultiMap index;
        index.insert("key", "value");
        index.insert("key2", "value");
        index.insert("key22", "value2");
        index.insert("k1", "value3");
        
        StringSet result1 = index.queryExactMatches("woops");
        ASSERT_TRUE(result1.empty());
        
        StringSet result2 = index.queryExactMatches("key222");
        ASSERT_TRUE(result2.empty());
        
        StringSet result3 = index.queryExactMatches("key");
        ASSERT_EQ(1u, result3.size());
        ASSERT_TRUE(result3.count("value") > 0);
        
        StringSet result4 = index.queryExactMatches("k");
        ASSERT_TRUE(result4.empty());
        
        index.insert("key", "value4");
        StringSet result5 = index.queryExactMatches("key");
        ASSERT_EQ(2u, result5.size());
        ASSERT_TRUE(result5.count("value") > 0);
        ASSERT_TRUE(result5.count("value4") > 0);

        StringSet result6 = index.queryExactMatches("");
        ASSERT_TRUE(result6.empty());
    }
    
    TEST(StringMultiMapTest, queryNumberedMatches) {
        TestMultiMap index;
        index.insert("key", "value");
        index.insert("key2", "value");
        index.insert("key22", "value2");
        index.insert("key22bs", "value4");
        index.insert("k1", "value3");

        StringSet result1 = index.queryNumberedMatches("woops");
        ASSERT_TRUE(result1.empty());

        StringSet result2 = index.queryNumberedMatches("key");
        ASSERT_EQ(2u, result2.size());
        ASSERT_TRUE(result2.count("value") > 0);
        ASSERT_TRUE(result2.count("value2") > 0);
        
        StringSet result3 = index.queryNumberedMatches("key2");
        ASSERT_EQ(2u, result3.size());
        ASSERT_TRUE(result3.count("value") > 0);
        ASSERT_TRUE(result3.count("value2") > 0);
        
        StringSet result4 = index.queryNumberedMatches("k");
        ASSERT_EQ(1u, result4.size());
        ASSERT_TRUE(result4.count("value3") > 0);
        
        index.remove("k1", "value3");

        StringSet result5 = index.queryNumberedMatches("k");
        ASSERT_TRUE(result5.empty());
    }
    
    TEST(StringMultiMapTest, splitMergeWithNumbers) {
        TestMultiMap index;
        index.insert("3.67", "value3");
        index.insert("3.6", "value2");
        index.insert("3.5", "value1");
        
        /*
         The insertion of the given values in the given order results in a tree with the following structure. Note that
         splitting nodes can introduce new numbered values at the new child nodes because a non-number string can have
         a numbered prefix or suffix. This must be considered when nodes are split.
         
          3.67 no numbered values
         
          3.6  no numbered values
           |
           7   no numbered values (error)
         
           3.  no numbered values
          / \
         5   6 no numbered values (error)
             |
             7 no numbered values (error)
         
         
         */
        
        // This statement will attempt to remove "value2" from node '6'. Because '6' is a number, it will consider "value"
        // as numbered at node '6', but the value was not added to the numbered values when the node was split. Thus, an
        // exception will be thrown.
        ASSERT_NO_THROW(index.remove("3.6", "value2"));
    }
    
    TEST(StringMultiMapTest, getKeys) {
        TestMultiMap index;
        index.insert("key", "value");
        index.insert("key2", "value");
        index.insert("key22", "value2");
        index.insert("k1", "value3");
        index.insert("test", "value4");

        ASSERT_EQ((StringSet{"key", "key2", "key22", "k1", "test"}),
                  SetUtils::makeSet(index.getKeys()));
    }
}
