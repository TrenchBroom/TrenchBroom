/*
 Copyright (C) 2010 Kristian Duske

 Permission is hereby granted, free of charge, to any person obtaining a copy of this
 software and associated documentation files (the "Software"), to deal in the Software
 without restriction, including without limitation the rights to use, copy, modify, merge,
 publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.
*/

#include "kd/string_format.h"

#include <optional>
#include <string>
#include <tuple>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace kdl
{

TEST_CASE("string_format")
{
  SECTION("str_select")
  {
    // clang-format off
    const auto
    [predicate, expected] = GENERATE(table<bool, std::string>({
    {true,      "yes"},
    {false,     "no"},
    }));
    // clang-format on

    CAPTURE(predicate);

    CHECK(str_select(predicate, "yes", "no") == expected);
  }

  SECTION("str_plural")
  {
    // clang-format off
    const auto
    [count, expected] = GENERATE(table<std::size_t, std::string>({
    {0,     "many"},
    {1,     "one"},
    {2,     "many"},
    }));
    // clang-format on

    CAPTURE(count);

    CHECK(str_plural(count, "one", "many") == expected);
  }

  SECTION("str_plural_with_prefix_suffix")
  {
    // clang-format off
    const auto
    [count, expected] = GENERATE(table<std::size_t, std::string>({
    {0,     "prefix many suffix"},
    {1,     "prefix one suffix"},
    {2,     "prefix many suffix"},
    }));
    // clang-format on

    CAPTURE(count);

    CHECK(str_plural("prefix ", count, "one", "many", " suffix") == expected);
  }

  SECTION("str_trim")
  {
    // clang-format off
    const auto
    [str,             chars,        expected] = GENERATE(table<std::string, std::optional<std::string>, std::string>({
    {"",              std::nullopt, ""},
    {"abc",           std::nullopt, "abc"},
    {" abc",          std::nullopt, "abc"},
    {"abc  ",         std::nullopt, "abc"},
    {"  abc   ",      std::nullopt, "abc"},
    {"xyxxabczzxzyz", "xyz",        "abc"},
    }));
    // clang-format on

    CAPTURE(str, chars);

    CHECK((chars ? str_trim(str, *chars) : str_trim(str)) == expected);
  }

  SECTION("str_to_lower_char")
  {
    constexpr auto input =
      R"( !"#$%&\'()*+,-./)"
      R"(0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|})"
      R"(~)";
    constexpr auto expected =
      R"( !"#$%&\'()*+,-./)"
      R"(0123456789:;<=>?@abcdefghijklmnopqrstuvwxyz[\\]^_`abcdefghijklmnopqrstuvwxyz{|})"
      R"(~)";

    CHECK(str_to_lower(input) == expected);
  }

  SECTION("str_to_upper_char")
  {
    constexpr auto input =
      R"( !"#$%&\'()*+,-./)"
      R"(0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|})"
      R"(~)";
    constexpr auto expected =
      R"( !"#$%&\'()*+,-./)"
      R"(0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`ABCDEFGHIJKLMNOPQRSTUVWXYZ{|})"
      R"(~)";
    CHECK(str_to_upper(input) == expected);
  }

  SECTION("str_to_lower")
  {
    // clang-format off
    const auto
    [str,              expected] = GENERATE(table<std::string, std::string>({
    {"",               ""},
    {R"(#?"abc73474)", R"(#?"abc73474)"},
    {R"(#?"abC73474)", R"(#?"abc73474)"},
    {R"(#?"ABC73474)", R"(#?"abc73474)"},
    {"XYZ",            "xyz"},
    }));
    // clang-format on

    CAPTURE(str);

    CHECK(str_to_lower(str) == expected);
  }

  SECTION("str_to_upper")
  {
    // clang-format off
    const auto
    [str,              expected] = GENERATE(table<std::string, std::string>({
    {"",               ""},
    {R"(#?"ABC73474)", R"(#?"ABC73474)"},
    {R"(#?"ABc73474)", R"(#?"ABC73474)"},
    {R"(#?"ABC73474)", R"(#?"ABC73474)"},
    {"xyz",            "XYZ"},
    }));
    // clang-format on

    CAPTURE(str);

    CHECK(str_to_upper(str) == expected);
  }

  SECTION("str_capitalize")
  {
    CHECK(
      str_capitalize("the quick brown fOX, .he jumped!")
      == "The Quick Brown FOX, .he Jumped!");
  }

  SECTION("str_escape")
  {
    // clang-format off
    const auto
    [str,                         chars,    expected] = GENERATE(table<std::string, std::string, std::string>({
    {"",                          "",       ""},
    {"",                          ";",      ""},
    {"asdf",                      "",       "asdf"},
    {R"(\)",                      "",       R"(\\)"},

    {R"(c:\blah\fasel\test.jpg)", R"(\)",   R"(c:\\blah\\fasel\\test.jpg)"},
    {R"(c:\blah\fasel\test.jpg)", R"(\:.)", R"(c\:\\blah\\fasel\\test\.jpg)"},
    {"asdf",                      "a",      R"(\asdf)"},
    {"asdf",                      "f",      R"(asd\f)"},
    }));
    // clang-format on

    CAPTURE(str, chars);

    CHECK(str_escape(str, chars) == expected);
  }

  SECTION("str_escape_if_necessary")
  {
    CHECK(
      str_escape_if_necessary(
        R"(this # should be escaped, but not this \#; this \\# however, should!)", "#")
      == R"(this \# should be escaped, but not this \#; this \\\# however, should!)");
  }

  SECTION("str_unescape")
  {
    // clang-format off
    const auto
    [str,                              chars,    expected] = GENERATE(table<std::string, std::string, std::string>({
    {"",                               "",       ""},
    {"",                               ";",      ""},
    {"asdf",                           "",       "asdf"},

    {R"(c:\\blah\\fasel\\test.jpg)",   R"(\)",   R"(c:\blah\fasel\test.jpg)"},
    {R"(c\:\\blah\\fasel\\test\.jpg)", R"(\:.)", R"(c:\blah\fasel\test.jpg)"},
    {R"(\asdf)",                       "a",      "asdf"},
    {R"(asd\f)",                       "f",      "asdf"},
    {R"(\asdf)",                       "a",      "asdf"},
    {R"(asdf\)",                       "",       R"(asdf\)"},
    {R"(asdf\\)",                      "",       R"(asdf\)"},
    {R"(asdf\\\\)",                    "",       R"(asdf\\)"},
    }));
    // clang-format on

    CAPTURE(str, chars);

    CHECK(str_unescape(str, chars) == expected);
  }

  SECTION("str_is_blank")
  {
    // clang-format off
    const auto
    [str,                      expected] = GENERATE(table<std::string, bool>({
    {"",                       true},
    {" ",                      true},
    {" \n\r\t",                true},
    {"a \n\r\t",               false},
    {"  a \n\r\t",             false},
    {" another one bites    ", false},
    }));
    // clang-format on

    CAPTURE(str);

    CHECK(str_is_blank(str) == expected);
  }

  SECTION("str_is_numeric")
  {
    // clang-format off
    const auto
    [str,          expected] = GENERATE(table<std::string, bool>({
    {"",           true},
    {"a",          false},
    {"66a",        false},
    {"66a33",      false},
    {"a33",        false},
    {"1",          true},
    {"1234567890", true},
    }));
    // clang-format on

    CAPTURE(str);

    CHECK(str_is_numeric(str) == expected);
  }
}

} // namespace kdl
