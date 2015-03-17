/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include "StringUtils.h"

namespace StringUtils {
    TEST(StringUtilsTest, trim) {
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
    
    TEST(StringUtilsTest, split) {
        StringList result;
        
        result = split("", ' ');
        ASSERT_TRUE(result.empty());
        
        result = split(" ", ' ');
        ASSERT_TRUE(result.empty());
        
        result = split("asdf", ' ');
        ASSERT_EQ(1u, result.size());
        ASSERT_EQ(String("asdf"), result[0]);
        
        result = split("d asdf", ' ');
        ASSERT_EQ(2u, result.size());
        ASSERT_EQ(String("d"), result[0]);
        ASSERT_EQ(String("asdf"), result[1]);
        
        result = split("asdf d", ' ');
        ASSERT_EQ(2u, result.size());
        ASSERT_EQ(String("asdf"), result[0]);
        ASSERT_EQ(String("d"), result[1]);
        
        result = split("The quick brown fox", ' ');
        ASSERT_EQ(4u, result.size());
        ASSERT_EQ(String("The"), result[0]);
        ASSERT_EQ(String("quick"), result[1]);
        ASSERT_EQ(String("brown"), result[2]);
        ASSERT_EQ(String("fox"), result[3]);

        result = split(" The quick brown fox", ' ');
        ASSERT_EQ(4u, result.size());
        ASSERT_EQ(String("The"), result[0]);
        ASSERT_EQ(String("quick"), result[1]);
        ASSERT_EQ(String("brown"), result[2]);
        ASSERT_EQ(String("fox"), result[3]);

        result = split("  The quick brown fox", ' ');
        ASSERT_EQ(4u, result.size());
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
        ASSERT_EQ(5u, result.size());
        ASSERT_EQ(String("The"), result[0]);
        ASSERT_EQ(String("quick"), result[1]);
        ASSERT_EQ(String(""), result[2]);
        ASSERT_EQ(String("brown"), result[3]);
        ASSERT_EQ(String("fox"), result[4]);
    }
    
    TEST(StringUtilsTest, join) {
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
    
    TEST(StringUtilsTest, sortCaseSensitive) {
        StringList strs;
        strs.push_back("bam");
        strs.push_back("Asdf");
        strs.push_back("asdf");
        strs.push_back("1");
        strs.push_back("BAM");
        strs.push_back("bambam");
        
        sortCaseSensitive(strs);
        ASSERT_EQ(String("1"), strs[0]);
        ASSERT_EQ(String("Asdf"), strs[1]);
        ASSERT_EQ(String("BAM"), strs[2]);
        ASSERT_EQ(String("asdf"), strs[3]);
        ASSERT_EQ(String("bam"), strs[4]);
        ASSERT_EQ(String("bambam"), strs[5]);
    }
    
    TEST(StringUtilsTest, sortCaseInsensitive) {
        StringList strs;
        strs.push_back("bam");
        strs.push_back("Asdf");
        strs.push_back("asdf");
        strs.push_back("1");
        strs.push_back("BAM");
        strs.push_back("bambam");
        
        sortCaseInsensitive(strs);
        ASSERT_EQ(String("1"), strs[0]);
        ASSERT_TRUE(strs[1] == "Asdf" || strs[1] == "asdf");
        ASSERT_TRUE(strs[2] == "Asdf" || strs[2] == "asdf");
        ASSERT_TRUE(strs[3] == "BAM" || strs[3] == "bam");
        ASSERT_TRUE(strs[4] == "BAM" || strs[4] == "bam");
        ASSERT_EQ(String("bambam"), strs[5]);
    }
    
    TEST(StringUtilsTest, caseSensitivePrefix) {
        ASSERT_TRUE(caseSensitivePrefix("", ""));
        ASSERT_TRUE(caseSensitivePrefix("asdf", "a"));
        ASSERT_TRUE(caseSensitivePrefix("asdf", "as"));
        ASSERT_TRUE(caseSensitivePrefix("asdf", "asd"));
        ASSERT_TRUE(caseSensitivePrefix("asdf", "asdf"));
        ASSERT_FALSE(caseSensitivePrefix("asdf", "asdfa"));

        ASSERT_FALSE(caseSensitivePrefix("asdf", "A"));
        ASSERT_FALSE(caseSensitivePrefix("asdf", "As"));
        ASSERT_FALSE(caseSensitivePrefix("asdf", "Asd"));
        ASSERT_FALSE(caseSensitivePrefix("asdf", "Asdf"));
    }
    
    TEST(StringUtilsTest, caseSensitiveSuffix) {
        ASSERT_TRUE(caseSensitiveSuffix("", ""));
        ASSERT_TRUE(caseSensitiveSuffix("asdf", "f"));
        ASSERT_TRUE(caseSensitiveSuffix("asdf", "df"));
        ASSERT_TRUE(caseSensitiveSuffix("asdf", "sdf"));
        ASSERT_TRUE(caseSensitiveSuffix("asdf", "asdf"));
        ASSERT_FALSE(caseSensitiveSuffix("asdf", "asdfa"));

        ASSERT_FALSE(caseSensitiveSuffix("asdf", "F"));
        ASSERT_FALSE(caseSensitiveSuffix("asdf", "dF"));
        ASSERT_FALSE(caseSensitiveSuffix("asdf", "sdF"));
        ASSERT_FALSE(caseSensitiveSuffix("asdf", "asdF"));
    }
    
    TEST(StringUtilsTest, matchesPattern) {
        ASSERT_TRUE(matchesPattern("", ""));
        ASSERT_TRUE(matchesPattern("", "*"));
        ASSERT_FALSE(matchesPattern("", "?"));
        ASSERT_TRUE(matchesPattern("asdf", "asdf"));
        ASSERT_TRUE(matchesPattern("asdf", "*"));
        ASSERT_TRUE(matchesPattern("asdf", "a??f"));
        ASSERT_FALSE(matchesPattern("asdf", "a?f"));
        ASSERT_TRUE(matchesPattern("asdf", "*f"));
        ASSERT_TRUE(matchesPattern("asdf", "a*f"));
        ASSERT_TRUE(matchesPattern("asdf", "?s?f"));
        ASSERT_TRUE(matchesPattern("asdfjkl", "a*f*l"));
        ASSERT_TRUE(matchesPattern("asdfjkl", "*a*f*l*"));
        ASSERT_TRUE(matchesPattern("asd*fjkl", "*a*f*l*"));
        ASSERT_TRUE(matchesPattern("asd*fjkl", "asd\\*fjkl"));
        ASSERT_TRUE(matchesPattern("asd*?fj\\kl", "asd\\*\\?fj\\\\kl"));
    }
    
    TEST(StringUtilsTest, escape) {
        ASSERT_EQ(String(""), StringUtils::escape("", ""));
        ASSERT_EQ(String(""), StringUtils::escape("", ";"));
        ASSERT_EQ(String("asdf"), StringUtils::escape("asdf", ""));
        ASSERT_EQ(String("\\\\"), StringUtils::escape("\\", ""));
        
        ASSERT_EQ(String("c:\\\\blah\\\\fasel\\\\test.jpg"), StringUtils::escape("c:\\blah\\fasel\\test.jpg", "\\"));
        ASSERT_EQ(String("c\\:\\\\blah\\\\fasel\\\\test\\.jpg"), StringUtils::escape("c:\\blah\\fasel\\test.jpg", "\\:."));
        ASSERT_EQ(String("\\asdf"), StringUtils::escape("asdf", "a"));
        ASSERT_EQ(String("asd\\f"), StringUtils::escape("asdf", "f"));
    }
    
    TEST(StringUtilsTest, unescape) {
        ASSERT_EQ(String(""), StringUtils::unescape("", ""));
        ASSERT_EQ(String(""), StringUtils::unescape("", ";"));
        ASSERT_EQ(String("asdf"), StringUtils::unescape("asdf", ""));

        ASSERT_EQ(String("c:\\blah\\fasel\\test.jpg"), StringUtils::unescape("c:\\\\blah\\\\fasel\\\\test.jpg", "\\"));
        ASSERT_EQ(String("c:\\blah\\fasel\\test.jpg"), StringUtils::unescape("c\\:\\\\blah\\\\fasel\\\\test\\.jpg", "\\:."));
        ASSERT_EQ(String("asdf"), StringUtils::unescape("\\asdf", "a"));
        ASSERT_EQ(String("asdf"), StringUtils::unescape("asd\\f", "f"));
        ASSERT_EQ(String("asdf"), StringUtils::unescape("\\asdf", "a"));
        ASSERT_EQ(String("asdf\\"), StringUtils::unescape("asdf\\", ""));
        ASSERT_EQ(String("asdf\\"), StringUtils::unescape("asdf\\\\", ""));
    }
}
