/*
 Copyright 2010-2019 Kristian Duske

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <gtest/gtest.h>

#include <kdl/string_utils.h>

namespace kdl {
    TEST(string_utils_test, str_split) {
        ASSERT_EQ(std::vector<std::string>({}), str_split("", " "));
        ASSERT_EQ(std::vector<std::string>({}), str_split(" ", " "));
        ASSERT_EQ(std::vector<std::string>({ "asdf" }), str_split("asdf", " "));
        ASSERT_EQ(std::vector<std::string>({ "d", "asdf" }), str_split("d asdf", " "));
        ASSERT_EQ(std::vector<std::string>({ "asdf", "d" }), str_split("asdf d", " "));
        ASSERT_EQ(std::vector<std::string>({ "The", "quick", "brown", "fox" }), str_split("The quick brown fox", " "));
        ASSERT_EQ(std::vector<std::string>({ "The", "quick", "brown", "fox" }), str_split(" The quick brown fox", " "));
        ASSERT_EQ(std::vector<std::string>({ "The", "quick", "brown", "fox" }), str_split("  The quick brown fox ", " "));
        ASSERT_EQ(std::vector<std::string>({ "The", "quick", "brown", "fox" }), str_split("The quick   brown fox", " "));
        ASSERT_EQ(std::vector<std::string>({ "The", "quick", "brown", "ox" }), str_split("The quick   brown fox", " f"));
    }

    TEST(string_utils_test, str_join) {
        ASSERT_EQ("", str_join(std::vector<std::string_view>({}), ", ", " and ", ", and "));
        ASSERT_EQ("one", str_join(std::vector<std::string_view>({"one"}), ", ", " and ", ", and "));
        ASSERT_EQ("one and two", str_join(std::vector<std::string_view>({"one", "two"}), ", ", ", and ", " and "));
        ASSERT_EQ("one, two, and three", str_join(std::vector<std::string_view>({"one", "two", "three"}), ", ", ", and ", " and "));

        ASSERT_EQ("", str_join(std::vector<std::string_view>({}), ", "));
        ASSERT_EQ("one", str_join(std::vector<std::string_view>({"one"}), ", "));
        ASSERT_EQ("one, two", str_join(std::vector<std::string_view>({"one", "two"}), ", "));
        ASSERT_EQ("one, two, three", str_join(std::vector<std::string_view>({"one", "two", "three"}), ", "));
    }

    TEST(string_utils_test, str_replace_every) {
        ASSERT_EQ("", str_replace_every("", "", "haha"));
        ASSERT_EQ("asdf", str_replace_every("asdf", "", "haha"));
        ASSERT_EQ("asdf", str_replace_every("asdf", "haha", "haha"));
        ASSERT_EQ("asdf", str_replace_every("asdf", "sd", "sd"));
        ASSERT_EQ("adsf", str_replace_every("asdf", "sd", "ds"));
        ASSERT_EQ("thE brick brown fox", str_replace_every("the brick brown fox", "e", "E"));
        ASSERT_EQ("TEH brick brown fox", str_replace_every("the brick brown fox", "the", "TEH"));
        ASSERT_EQ("the click clown fox", str_replace_every("the brick brown fox", "br", "cl"));
        ASSERT_EQ("the brick crown fox", str_replace_every("the brick brown fox", "bro", "cro"));
    }
}
