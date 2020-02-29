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

#include <catch2/catch.hpp>

#include "GTestCompat.h"

#include <kdl/string_format.h>

namespace kdl {
    TEST_CASE("string_format_test.str_select", "[string_format_test]") {
        ASSERT_EQ("yes", str_select(true, "yes", "no"));
        ASSERT_EQ("no", str_select(false, "yes", "no"));
    }

    TEST_CASE("string_format_test.str_plural", "[string_format_test]") {
        ASSERT_EQ("many", str_plural(0, "one", "many"));
        ASSERT_EQ("one", str_plural(1, "one", "many"));
        ASSERT_EQ("many", str_plural(2, "one", "many"));
    }

    TEST_CASE("string_format_test.str_plural_with_prefix_suffix", "[string_format_test]") {
        ASSERT_EQ("prefix many suffix", str_plural("prefix ", 0, "one", "many", " suffix"));
        ASSERT_EQ("prefix one suffix", str_plural("prefix ", 1, "one", "many", " suffix"));
        ASSERT_EQ("prefix many suffix", str_plural("prefix ", 2, "one", "many", " suffix"));
    }

    TEST_CASE("string_format_test.str_trim", "[string_format_test]") {
        ASSERT_EQ("", str_trim(""));
        ASSERT_EQ("abc", str_trim("abc"));
        ASSERT_EQ("abc", str_trim(" abc"));
        ASSERT_EQ("abc", str_trim("abc  "));
        ASSERT_EQ("abc", str_trim("  abc   "));
        ASSERT_EQ("abc", str_trim("  abc   "));
        ASSERT_EQ("abc", str_trim("xyxxabczzxzyz", "xyz"));
    }

    TEST_CASE("string_format_test.str_to_lower_char", "[string_format_test]") {
        constexpr auto input = " !\"#$%&\\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
        constexpr auto expected = " !\"#$%&\\'()*+,-./0123456789:;<=>?@abcdefghijklmnopqrstuvwxyz[\\\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
        for (std::size_t i = 0u; i < 98u; ++i) {
            ASSERT_EQ(expected[i], str_to_lower(input[i]));
        }
    }

    TEST_CASE("string_format_test.str_to_upper_char", "[string_format_test]") {
        constexpr auto input = " !\"#$%&\\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
        constexpr auto expected = " !\"#$%&\\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\\\]^_`ABCDEFGHIJKLMNOPQRSTUVWXYZ{|}~";
        for (std::size_t i = 0u; i < 98u; ++i) {
            ASSERT_EQ(expected[i], str_to_upper(input[i]));
        }
    }

    TEST_CASE("string_format_test.str_to_lower", "[string_format_test]") {
        ASSERT_EQ("", str_to_lower(""));
        ASSERT_EQ("#?\"abc73474", str_to_lower("#?\"abc73474"));
        ASSERT_EQ("#?\"abc73474", str_to_lower("#?\"abC73474"));
        ASSERT_EQ("#?\"abc73474", str_to_lower("#?\"ABC73474"));
        ASSERT_EQ("xyz", str_to_lower("XYZ"));
    }

    TEST_CASE("string_format_test.str_to_upper", "[string_format_test]") {
        ASSERT_EQ("", str_to_lower(""));
        ASSERT_EQ("#?\"ABC73474", str_to_upper("#?\"ABC73474"));
        ASSERT_EQ("#?\"ABC73474", str_to_upper("#?\"ABc73474"));
        ASSERT_EQ("#?\"ABC73474", str_to_upper("#?\"ABC73474"));
        ASSERT_EQ("XYZ", str_to_upper("xyz"));
    }

    TEST_CASE("string_format_test.str_capitalize", "[string_format_test]") {
        ASSERT_EQ("The Quick Brown FOX, .he Jumped!", str_capitalize("the quick brown fOX, .he jumped!"));
    }

    TEST_CASE("string_format_test.str_escape", "[string_format_test]") {
        ASSERT_EQ("", str_escape("", ""));
        ASSERT_EQ("", str_escape("", ";"));
        ASSERT_EQ("asdf", str_escape("asdf", ""));
        ASSERT_EQ("\\\\", str_escape("\\", ""));

        ASSERT_EQ("c:\\\\blah\\\\fasel\\\\test.jpg", str_escape("c:\\blah\\fasel\\test.jpg", "\\"));
        ASSERT_EQ("c\\:\\\\blah\\\\fasel\\\\test\\.jpg", str_escape("c:\\blah\\fasel\\test.jpg", "\\:."));
        ASSERT_EQ("\\asdf", str_escape("asdf", "a"));
        ASSERT_EQ("asd\\f", str_escape("asdf", "f"));
    }

    TEST_CASE("string_format_test.str_escape_if_necessary", "[string_format_test]") {
        ASSERT_EQ("this \\# should be escaped, but not this \\#; this \\\\\\# however, should!", str_escape_if_necessary("this # should be escaped, but not this \\#; this \\\\# however, should!", "#"));
    }

    TEST_CASE("string_format_test.str_unescape", "[string_format_test]") {
        ASSERT_EQ("", str_unescape("", ""));
        ASSERT_EQ("", str_unescape("", ";"));
        ASSERT_EQ("asdf", str_unescape("asdf", ""));

        ASSERT_EQ("c:\\blah\\fasel\\test.jpg", str_unescape("c:\\\\blah\\\\fasel\\\\test.jpg", "\\"));
        ASSERT_EQ("c:\\blah\\fasel\\test.jpg", str_unescape("c\\:\\\\blah\\\\fasel\\\\test\\.jpg", "\\:."));
        ASSERT_EQ("asdf", str_unescape("\\asdf", "a"));
        ASSERT_EQ("asdf", str_unescape("asd\\f", "f"));
        ASSERT_EQ("asdf", str_unescape("\\asdf", "a"));
        ASSERT_EQ("asdf\\", str_unescape("asdf\\", ""));
        ASSERT_EQ("asdf\\", str_unescape("asdf\\\\", ""));
        ASSERT_EQ("asdf\\\\", str_unescape("asdf\\\\\\\\", ""));
    }

    TEST_CASE("string_format_test.str_is_blank", "[string_format_test]") {
        ASSERT_TRUE(str_is_blank(""));
        ASSERT_TRUE(str_is_blank(" "));
        ASSERT_TRUE(str_is_blank(" \n\r\t"));
        ASSERT_FALSE(str_is_blank("a \n\r\t"));
        ASSERT_FALSE(str_is_blank("  a \n\r\t"));
        ASSERT_FALSE(str_is_blank(" another one bites    "));
    }

    TEST_CASE("string_format_test.str_is_numeric", "[string_format_test]") {
        ASSERT_TRUE(str_is_numeric(""));
        ASSERT_FALSE(str_is_numeric("a"));
        ASSERT_FALSE(str_is_numeric("66a"));
        ASSERT_FALSE(str_is_numeric("66a33"));
        ASSERT_FALSE(str_is_numeric("a33"));
        ASSERT_TRUE(str_is_numeric("1"));
        ASSERT_TRUE(str_is_numeric("1234567890"));
    }
}
