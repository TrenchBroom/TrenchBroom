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

#include "StringUtils.h"

namespace StringUtils {
    TEST(StringUtilsTest, Trim) {
        String result;
        
        ASSERT_EQ(String(""), trim(""));
        ASSERT_EQ(String(""), trim(" \t\n\r"));
        ASSERT_EQ(String("a"), trim("a"));
        ASSERT_EQ(String("asdf"), trim("asdf"));
        ASSERT_EQ(String("asdf"), trim(" \tasdf"));
        ASSERT_EQ(String("asdf"), trim("asdf\n "));
        ASSERT_EQ(String("asdf"), trim(" \tasdf\n "));
        ASSERT_EQ(String("as df"), trim(" \tas df\n "));
        ASSERT_EQ(String("/this/is/a/path.map"), trim("/this/is/a/path.map"));
    }
    
    TEST(StringUtilsTest, Split) {
        StringList result;
        
        result = split("", ' ');
        ASSERT_TRUE(result.empty());
        
        result = split(" ", ' ');
        ASSERT_TRUE(result.empty());
        
        result = split("asdf", ' ');
        ASSERT_EQ(1, result.size());
        ASSERT_EQ(String("asdf"), result[0]);
        
        result = split("d asdf", ' ');
        ASSERT_EQ(2, result.size());
        ASSERT_EQ(String("d"), result[0]);
        ASSERT_EQ(String("asdf"), result[1]);
        
        result = split("asdf d", ' ');
        ASSERT_EQ(2, result.size());
        ASSERT_EQ(String("asdf"), result[0]);
        ASSERT_EQ(String("d"), result[1]);
        
        result = split("The quick brown fox", ' ');
        ASSERT_EQ(4, result.size());
        ASSERT_EQ(String("The"), result[0]);
        ASSERT_EQ(String("quick"), result[1]);
        ASSERT_EQ(String("brown"), result[2]);
        ASSERT_EQ(String("fox"), result[3]);

        result = split(" The quick brown fox", ' ');
        ASSERT_EQ(4, result.size());
        ASSERT_EQ(String("The"), result[0]);
        ASSERT_EQ(String("quick"), result[1]);
        ASSERT_EQ(String("brown"), result[2]);
        ASSERT_EQ(String("fox"), result[3]);

        result = split("  The quick brown fox", ' ');
        ASSERT_EQ(4, result.size());
        ASSERT_EQ(String("The"), result[0]);
        ASSERT_EQ(String("quick"), result[1]);
        ASSERT_EQ(String("brown"), result[2]);
        ASSERT_EQ(String("fox"), result[3]);
        
        result = split("The quick brown fox ", ' ');
        ASSERT_EQ(String("The"), result[0]);
        ASSERT_EQ(String("quick"), result[1]);
        ASSERT_EQ(String("brown"), result[2]);
        ASSERT_EQ(String("fox"), result[3]);

        result = split("The quick brown fox  ", ' ');
        ASSERT_EQ(String("The"), result[0]);
        ASSERT_EQ(String("quick"), result[1]);
        ASSERT_EQ(String("brown"), result[2]);
        ASSERT_EQ(String("fox"), result[3]);
        
        result = split("The quick  brown fox", ' ');
        ASSERT_EQ(5, result.size());
        ASSERT_EQ(String("The"), result[0]);
        ASSERT_EQ(String("quick"), result[1]);
        ASSERT_EQ(String(""), result[2]);
        ASSERT_EQ(String("brown"), result[3]);
        ASSERT_EQ(String("fox"), result[4]);
    }
    
    TEST(StringUtilsTest, Join) {
        StringList components;
        ASSERT_EQ(String(""), join(components, "/"));

        components.push_back("");
        ASSERT_EQ(String(""), join(components, "/"));
        
        components.push_back("");
        ASSERT_EQ(String("/"), join(components, "/"));
        
        components.clear();
        components.push_back("asdf");
        ASSERT_EQ(String("asdf"), join(components, "/"));

        components.push_back("yo");
        ASSERT_EQ(String("asdf/yo"), join(components, "/"));
    }
}
