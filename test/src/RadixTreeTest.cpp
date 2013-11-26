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

#include "RadixTree.h"

namespace TrenchBroom {
    TEST(RadixTreeTest, insert) {
        RadixTree<String> tree;
        tree.insert("key", "value");
        tree.insert("key2", "value");
        tree.insert("key22", "value2");
        tree.insert("k1", "value3");
        tree.insert("k", "value4");
        
        StringSet result1 = tree.query("woops");
        ASSERT_TRUE(result1.empty());
        
        StringSet result2 = tree.query("key222");
        ASSERT_TRUE(result2.empty());
        
        StringSet result3 = tree.query("key");
        ASSERT_EQ(2u, result3.size());
        ASSERT_TRUE(result3.count("value"));
        ASSERT_TRUE(result3.count("value2"));
        
        StringSet result4 = tree.query("k");
        ASSERT_EQ(4u, result4.size());
        ASSERT_TRUE(result4.count("value"));
        ASSERT_TRUE(result4.count("value2"));
        ASSERT_TRUE(result4.count("value3"));
        ASSERT_TRUE(result4.count("value4"));
    }
}
