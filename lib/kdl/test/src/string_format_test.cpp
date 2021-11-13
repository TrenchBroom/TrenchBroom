/*
 Copyright 2010-2019 Kristian Duske

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
 NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
 OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <kdl/string_format.h>

#include <catch2/catch.hpp>

namespace kdl {
TEST_CASE("string_format_test.str_select", "[string_format_test]") {
  CHECK(str_select(true, "yes", "no") == "yes");
  CHECK(str_select(false, "yes", "no") == "no");
}

TEST_CASE("string_format_test.str_plural", "[string_format_test]") {
  CHECK(str_plural(0, "one", "many") == "many");
  CHECK(str_plural(1, "one", "many") == "one");
  CHECK(str_plural(2, "one", "many") == "many");
}

TEST_CASE("string_format_test.str_plural_with_prefix_suffix", "[string_format_test]") {
  CHECK(str_plural("prefix ", 0, "one", "many", " suffix") == "prefix many suffix");
  CHECK(str_plural("prefix ", 1, "one", "many", " suffix") == "prefix one suffix");
  CHECK(str_plural("prefix ", 2, "one", "many", " suffix") == "prefix many suffix");
}

TEST_CASE("string_format_test.str_trim", "[string_format_test]") {
  CHECK(str_trim("") == "");
  CHECK(str_trim("abc") == "abc");
  CHECK(str_trim(" abc") == "abc");
  CHECK(str_trim("abc  ") == "abc");
  CHECK(str_trim("  abc   ") == "abc");
  CHECK(str_trim("  abc   ") == "abc");
  CHECK(str_trim("xyxxabczzxzyz", "xyz") == "abc");
}

TEST_CASE("string_format_test.str_to_lower_char", "[string_format_test]") {
  constexpr auto input =
    " !\"#$%&\\'()*+,-./"
    "0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
  constexpr auto expected =
    " !\"#$%&\\'()*+,-./"
    "0123456789:;<=>?@abcdefghijklmnopqrstuvwxyz[\\\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
  for (std::size_t i = 0u; i < 98u; ++i) {
    CHECK(str_to_lower(input[i]) == expected[i]);
  }
}

TEST_CASE("string_format_test.str_to_upper_char", "[string_format_test]") {
  constexpr auto input =
    " !\"#$%&\\'()*+,-./"
    "0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
  constexpr auto expected =
    " !\"#$%&\\'()*+,-./"
    "0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\\\]^_`ABCDEFGHIJKLMNOPQRSTUVWXYZ{|}~";
  for (std::size_t i = 0u; i < 98u; ++i) {
    CHECK(str_to_upper(input[i]) == expected[i]);
  }
}

TEST_CASE("string_format_test.str_to_lower", "[string_format_test]") {
  CHECK(str_to_lower("") == "");
  CHECK(str_to_lower("#?\"abc73474") == "#?\"abc73474");
  CHECK(str_to_lower("#?\"abC73474") == "#?\"abc73474");
  CHECK(str_to_lower("#?\"ABC73474") == "#?\"abc73474");
  CHECK(str_to_lower("XYZ") == "xyz");
}

TEST_CASE("string_format_test.str_to_upper", "[string_format_test]") {
  CHECK(str_to_lower("") == "");
  CHECK(str_to_upper("#?\"ABC73474") == "#?\"ABC73474");
  CHECK(str_to_upper("#?\"ABc73474") == "#?\"ABC73474");
  CHECK(str_to_upper("#?\"ABC73474") == "#?\"ABC73474");
  CHECK(str_to_upper("xyz") == "XYZ");
}

TEST_CASE("string_format_test.str_capitalize", "[string_format_test]") {
  CHECK(str_capitalize("the quick brown fOX, .he jumped!") == "The Quick Brown FOX, .he Jumped!");
}

TEST_CASE("string_format_test.str_escape", "[string_format_test]") {
  CHECK(str_escape("", "") == "");
  CHECK(str_escape("", ";") == "");
  CHECK(str_escape("asdf", "") == "asdf");
  CHECK(str_escape("\\", "") == "\\\\");

  CHECK(str_escape("c:\\blah\\fasel\\test.jpg", "\\") == "c:\\\\blah\\\\fasel\\\\test.jpg");
  CHECK(str_escape("c:\\blah\\fasel\\test.jpg", "\\:.") == "c\\:\\\\blah\\\\fasel\\\\test\\.jpg");
  CHECK(str_escape("asdf", "a") == "\\asdf");
  CHECK(str_escape("asdf", "f") == "asd\\f");
}

TEST_CASE("string_format_test.str_escape_if_necessary", "[string_format_test]") {
  CHECK(
    str_escape_if_necessary(
      "this # should be escaped, but not this \\#; this \\\\# however, should!", "#") ==
    "this \\# should be escaped, but not this \\#; this \\\\\\# however, should!");
}

TEST_CASE("string_format_test.str_unescape", "[string_format_test]") {
  CHECK(str_unescape("", "") == "");
  CHECK(str_unescape("", ";") == "");
  CHECK(str_unescape("asdf", "") == "asdf");

  CHECK(str_unescape("c:\\\\blah\\\\fasel\\\\test.jpg", "\\") == "c:\\blah\\fasel\\test.jpg");
  CHECK(str_unescape("c\\:\\\\blah\\\\fasel\\\\test\\.jpg", "\\:.") == "c:\\blah\\fasel\\test.jpg");
  CHECK(str_unescape("\\asdf", "a") == "asdf");
  CHECK(str_unescape("asd\\f", "f") == "asdf");
  CHECK(str_unescape("\\asdf", "a") == "asdf");
  CHECK(str_unescape("asdf\\", "") == "asdf\\");
  CHECK(str_unescape("asdf\\\\", "") == "asdf\\");
  CHECK(str_unescape("asdf\\\\\\\\", "") == "asdf\\\\");
}

TEST_CASE("string_format_test.str_is_blank", "[string_format_test]") {
  CHECK(str_is_blank(""));
  CHECK(str_is_blank(" "));
  CHECK(str_is_blank(" \n\r\t"));
  CHECK_FALSE(str_is_blank("a \n\r\t"));
  CHECK_FALSE(str_is_blank("  a \n\r\t"));
  CHECK_FALSE(str_is_blank(" another one bites    "));
}

TEST_CASE("string_format_test.str_is_numeric", "[string_format_test]") {
  CHECK(str_is_numeric(""));
  CHECK_FALSE(str_is_numeric("a"));
  CHECK_FALSE(str_is_numeric("66a"));
  CHECK_FALSE(str_is_numeric("66a33"));
  CHECK_FALSE(str_is_numeric("a33"));
  CHECK(str_is_numeric("1"));
  CHECK(str_is_numeric("1234567890"));
}
} // namespace kdl
