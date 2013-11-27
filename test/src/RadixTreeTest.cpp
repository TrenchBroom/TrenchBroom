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
#include "RadixTree.h"
#include "StringUtils.h"

namespace TrenchBroom {
    TEST(RadixTreeTest, insert) {
        RadixTree<String> tree;
        tree.insert("key", "value");
        tree.insert("key2", "value");
        tree.insert("key22", "value2");
        tree.insert("k1", "value3");
        
        StringList result1 = tree.query("woops");
        ASSERT_TRUE(result1.empty());
        
        StringList result2 = tree.query("key222");
        ASSERT_TRUE(result2.empty());
        
        StringList result3 = tree.query("key");
        ASSERT_EQ(2u, result3.size());
        ASSERT_TRUE(VectorUtils::contains(result3, String("value")));
        ASSERT_TRUE(VectorUtils::contains(result3, String("value2")));
        
        StringList result4 = tree.query("k");
        ASSERT_EQ(3u, result4.size());
        ASSERT_TRUE(VectorUtils::contains(result4, String("value")));
        ASSERT_TRUE(VectorUtils::contains(result4, String("value2")));
        ASSERT_TRUE(VectorUtils::contains(result4, String("value3")));

        tree.insert("k", "value4");

        StringList result5 = tree.query("k");
        ASSERT_EQ(4u, result5.size());
        ASSERT_TRUE(VectorUtils::contains(result5, String("value")));
        ASSERT_TRUE(VectorUtils::contains(result5, String("value2")));
        ASSERT_TRUE(VectorUtils::contains(result5, String("value3")));
        ASSERT_TRUE(VectorUtils::contains(result5, String("value4")));
    }
    
    TEST(RadixTreeTest, remove) {
        RadixTree<String> tree;
        tree.insert("andrew", "value");
        tree.insert("andreas", "value");
        tree.insert("andrar", "value2");
        tree.insert("andrary", "value3");
        tree.insert("andy", "value4");

        ASSERT_THROW(tree.remove("andrary", "value2"), Exception);
        
        tree.remove("andrary", "value3");
        
        StringList result1 = tree.query("andrary");
        ASSERT_TRUE(result1.empty());
        
        StringList result2 = tree.query("andrar");
        ASSERT_EQ(1u, result2.size());
        ASSERT_TRUE(VectorUtils::contains(result2, String("value2")));

        tree.remove("andrar", "value2");
        
        StringList result3 = tree.query("andrar");
        ASSERT_TRUE(result3.empty());
        
        StringList result4 = tree.query("andre");
        ASSERT_EQ(1u, result4.size());
        ASSERT_TRUE(VectorUtils::contains(result4, String("value")));

        StringList result5 = tree.query("andreas");
        ASSERT_EQ(1u, result5.size());
        ASSERT_TRUE(VectorUtils::contains(result5, String("value")));

        tree.remove("andy", "value4");
        ASSERT_TRUE(tree.query("andy").empty());
        ASSERT_EQ(tree.query("andreas"), StringList(1, String("value")));
        ASSERT_EQ(tree.query("andrew"), StringList(1, String("value")));
        
        tree.remove("andreas", "value");
        ASSERT_TRUE(tree.query("andreas").empty());
        ASSERT_EQ(tree.query("andrew"), StringList(1, String("value")));
        
        tree.remove("andrew", "value");
        ASSERT_TRUE(tree.query("andrew").empty());
    }
}
