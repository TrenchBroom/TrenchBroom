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

#include "kdl/string_utils.h"

#include <optional>
#include <ostream>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

namespace kdl
{
using namespace Catch::Matchers;

namespace
{

struct to_string
{
  std::string x;
};

[[maybe_unused]] std::ostream& operator<<(std::ostream& o, const to_string& t)
{
  o << t.x << ";";
  return o;
}

} // namespace

TEST_CASE("string_utils")
{
  SECTION("str_find_next_delimited_string")
  {
    using T = std::tuple<
      std::string_view,
      std::string_view,
      std::string_view,
      std::optional<char>,
      std::optional<delimited_string>>;

    // clang-format off
    const auto 
    [str, 
     start_delim, end_delim, escape_char,  expectedResult] = GENERATE(values<T>({
    {R"()",  
     "${",        "}",       '\\',         std::nullopt},
    {R"(${asdf})",  
     "${",        "}",       '\\',         delimited_string{0, 7}},
    {R"(   ${asdf}   )",  
     "${",        "}",       '\\',         delimited_string{3, 7}},
    {R"(${as\}df})",  
     "${",        "}",       '\\',         delimited_string{0, 9}},
    {R"(\${asdf\})",  
     "${",       "}",        std::nullopt, delimited_string{1, 8}},
    {R"(as}df)",  
     "${",        "}",       '\\',         std::nullopt},
     {R"(${as}df})",  
     "${",        "}",       '\\',         delimited_string{0, 5}},
    {R"(${as${df}gh})",  
     "${",        "}",       '\\',         delimited_string{0, 12}},
    {R"(${asdf)",  
     "${",       "}",        '\\',         delimited_string{0, std::nullopt}},
     }));
      // clang-format off

    CHECK(
      str_find_next_delimited_string(str, start_delim, end_delim, escape_char)
      == expectedResult);
  }

  SECTION("str_next_token") {
    CHECK(str_next_token("", " ") == std::nullopt);
    CHECK(str_next_token("", "") == std::nullopt);
    CHECK(str_next_token(" ", "") == std::tuple{0, 1});
    CHECK(str_next_token("asdf", "") == std::tuple{0,4});
    CHECK(str_next_token("asdf", " ") == std::tuple{0, 4});
    CHECK(str_next_token(" asdf", " ") == std::tuple{1, 5});
    CHECK(str_next_token(" asdf  ", " ") == std::tuple{1, 5});
    CHECK(str_next_token(" as df  ", " ") == std::tuple{1, 3});
    CHECK(str_next_token("as;df", ";") == std::tuple{0, 2});
    CHECK(str_next_token("as\\;df", ";") == std::tuple{0, 6});
  }

  SECTION("str_next_tokens") {
    CHECK(str_next_tokens("", "", 0) == std::tuple{std::vector<std::string>{}, 0});
    CHECK(str_next_tokens("", "", 1) == std::tuple{std::vector<std::string>{}, 0});
    CHECK(str_next_tokens("", " ", 0) == std::tuple{std::vector<std::string>{}, 0});
    CHECK(str_next_tokens("", " ", 1) == std::tuple{std::vector<std::string>{}, 0});
    CHECK(str_next_tokens("as", "", 0) == std::tuple{std::vector<std::string>{}, 0});
    CHECK(str_next_tokens("as", "", 1) == std::tuple{std::vector<std::string>{"as"}, 2});
    CHECK(str_next_tokens("as", " ", 0) == std::tuple{std::vector<std::string>{}, 0});
    CHECK(str_next_tokens("as", " ", 1) == std::tuple{std::vector<std::string>{"as"}, 2});
    CHECK(str_next_tokens(" as df ", " ", 2) == std::tuple{std::vector<std::string>{"as", "df"}, 6});
    CHECK(str_next_tokens(" as df ", " ", 3) == std::tuple{std::vector<std::string>{"as", "df"}, 6});
  }

  SECTION("str_split")
  {
    CHECK_THAT(str_split("", " "), Equals(std::vector<std::string>{}));
    CHECK_THAT(str_split(" ", " "), Equals(std::vector<std::string>{}));
    CHECK_THAT(str_split("asdf", " "), Equals(std::vector<std::string>{"asdf"}));
    CHECK_THAT(
      str_split("d asdf", " "), Equals(std::vector<std::string>{"d", "asdf"}));
    CHECK_THAT(
      str_split("asdf d", " "), Equals(std::vector<std::string>{"asdf", "d"}));
    CHECK_THAT(
      str_split("The quick brown fox", " "),
      Equals(std::vector<std::string>{"The", "quick", "brown", "fox"}));
    CHECK_THAT(
      str_split(" The quick brown fox", " "),
      Equals(std::vector<std::string>{"The", "quick", "brown", "fox"}));
    CHECK_THAT(
      str_split("  The quick brown fox ", " "),
      Equals(std::vector<std::string>{"The", "quick", "brown", "fox"}));
    CHECK_THAT(
      str_split("The quick   brown fox", " "),
      Equals(std::vector<std::string>{"The", "quick", "brown", "fox"}));
    CHECK_THAT(
      str_split("The quick   brown fox", " f"),
      Equals(std::vector<std::string>{"The", "quick", "brown", "ox"}));
    CHECK_THAT(
      str_split("The; quick brown; fox", ";"),
      Equals(std::vector<std::string>{"The", " quick brown", " fox"}));
    CHECK_THAT(
      str_split("The;quick brown; fox", " ;"),
      Equals(std::vector<std::string>{"The", "quick", "brown", "fox"}));
    CHECK_THAT(
      str_split("The\\; quick brown; fox", ";"),
      Equals(std::vector<std::string>{"The\\; quick brown", " fox"}));
    CHECK_THAT(
      str_split("The\\\\; quick brown; fox", ";"),
      Equals(std::vector<std::string>{"The\\\\", " quick brown", " fox"}));
    CHECK_THAT(
      str_split("c:\\x\\y", "\\"),
      Equals(std::vector<std::string>{"c:", "x", "y"}));
  }

  SECTION("str_join")
  {
    CHECK(str_join(std::vector<std::string_view>{}, ", ", " and ", ", and ") == "");
    CHECK(
      str_join(std::vector<std::string_view>{"one"}, ", ", " and ", ", and ") == "one");
    CHECK(
      str_join(std::vector<std::string_view>{"one", "two"}, ", ", ", and ", " and ")
      == "one and two");
    CHECK(
      str_join(
        std::vector<std::string_view>{"one", "two", "three"}, ", ", ", and ", " and ")
      == "one, two, and three");

    CHECK(str_join(std::vector<std::string_view>{}, ", ") == "");
    CHECK(str_join(std::vector<std::string_view>{"one"}, ", ") == "one");
    CHECK(str_join(std::vector<std::string_view>{"one", "two"}, ", ") == "one, two");
    CHECK(
      str_join(std::vector<std::string_view>{"one", "two", "three"}, ", ")
      == "one, two, three");
  }

  SECTION("str_replace_every")
  {
    CHECK(str_replace_every("", "", "haha") == "");
    CHECK(str_replace_every("asdf", "", "haha") == "asdf");
    CHECK(str_replace_every("asdf", "haha", "haha") == "asdf");
    CHECK(str_replace_every("asdf", "sd", "sd") == "asdf");
    CHECK(str_replace_every("asdf", "sd", "ds") == "adsf");
    CHECK(str_replace_every("asdf", "df", "ds") == "asds");
    CHECK(str_replace_every("asdf asdf", "df", "ds") == "asds asds");
    CHECK(str_replace_every("the brick brown fox", "e", "E") == "thE brick brown fox");
    CHECK(
      str_replace_every("the brick brown fox", "the", "TEH") == "TEH brick brown fox");
    CHECK(str_replace_every("the brick brown fox", "br", "cl") == "the click clown fox");
    CHECK(
      str_replace_every("the brick brown fox", "bro", "cro") == "the brick crown fox");
  }

  SECTION("string_format_test.str_to_string")
  {
    CHECK(str_to_string("abc") == "abc");
    CHECK(str_to_string(1234) == "1234");
    CHECK(str_to_string(1.0) == "1");
    CHECK(str_to_string(to_string{"xyz"}) == "xyz;");
  }

  SECTION("string_format_test.str_to_int")
  {
    CHECK(str_to_int("0") == 0);
    CHECK(str_to_int("1") == 1);
    CHECK(str_to_int("123231") == 123231);
    CHECK(str_to_int("-123231") == -123231);
    CHECK(str_to_int("123231b") == 123231);
    CHECK(str_to_int("   123231   ") == 123231);
    CHECK(str_to_int("a123231") == std::nullopt);
    CHECK(str_to_int(" ") == std::nullopt);
    CHECK(str_to_int("") == std::nullopt);
  }

  SECTION("string_format_test.str_to_long")
  {
    CHECK(str_to_long("0") == 0l);
    CHECK(str_to_long("1") == 1l);
    CHECK(str_to_long("123231") == 123231l);
    CHECK(str_to_long("-123231") == -123231l);
    CHECK(str_to_long("2147483647") == 2147483647l);
    CHECK(str_to_long("-2147483646") == -2147483646l);
    CHECK(str_to_long("123231b") == 123231l);
    CHECK(str_to_long("   123231   ") == 123231l);
    CHECK(str_to_long("a123231") == std::nullopt);
    CHECK(str_to_long(" ") == std::nullopt);
    CHECK(str_to_long("") == std::nullopt);
  }

  SECTION("string_format_test.str_to_long_long")
  {
    CHECK(str_to_long_long("0") == 0ll);
    CHECK(str_to_long_long("1") == 1ll);
    CHECK(str_to_long_long("123231") == 123231ll);
    CHECK(str_to_long_long("-123231") == -123231ll);
    CHECK(str_to_long_long("2147483647") == 2147483647ll);
    CHECK(str_to_long_long("-2147483646") == -2147483646ll);
    CHECK(str_to_long_long("9223372036854775807") == 9'223'372'036'854'775'807ll);
    CHECK(str_to_long_long("-9223372036854775806") == -9'223'372'036'854'775'806ll);
    CHECK(str_to_long_long("123231b") == 123231ll);
    CHECK(str_to_long_long("   123231   ") == 123231ll);
    CHECK(str_to_long_long("a123231") == std::nullopt);
    CHECK(str_to_long_long(" ") == std::nullopt);
    CHECK(str_to_long_long("") == std::nullopt);
  }

  SECTION("string_format_test.str_to_u_long")
  {

    CHECK(str_to_u_long("0") == 0ul);
    CHECK(str_to_u_long("1") == 1ul);
    CHECK(str_to_u_long("123231") == 123231ul);
    CHECK(str_to_u_long("2147483647") == 2147483647ul);
    CHECK(str_to_u_long("123231b") == 123231ul);
    CHECK(str_to_u_long("   123231   ") == 123231ul);
    CHECK(str_to_u_long("a123231") == std::nullopt);
    CHECK(str_to_u_long(" ") == std::nullopt);
    CHECK(str_to_u_long("") == std::nullopt);
  }

  SECTION("string_format_test.str_to_u_long_long")
  {
    CHECK(str_to_u_long_long("0") == 0ull);
    CHECK(str_to_u_long_long("1") == 1ull);
    CHECK(str_to_u_long_long("123231") == 123231ull);
    CHECK(str_to_u_long_long("2147483647") == 2147483647ull);
    CHECK(str_to_u_long_long("9223372036854775807") == 9'223'372'036'854'775'807ull);
    CHECK(str_to_u_long_long("123231b") == 123231ull);
    CHECK(str_to_u_long_long("   123231   ") == 123231ull);
    CHECK(str_to_u_long_long("a123231") == std::nullopt);
    CHECK(str_to_u_long_long(" ") == std::nullopt);
    CHECK(str_to_u_long_long("") == std::nullopt);
  }

  SECTION("string_format_test.str_to_size")
  {
    CHECK(str_to_size("0") == 0u);
    CHECK(str_to_size("1") == 1u);
    CHECK(str_to_size("123231") == 123231u);
    CHECK(str_to_size("2147483647") == 2147483647u);
    CHECK(str_to_size("123231b") == 123231u);
    CHECK(str_to_size("   123231   ") == 123231u);
    CHECK(str_to_size("a123231") == std::nullopt);
    CHECK(str_to_size(" ") == std::nullopt);
    CHECK(str_to_size("") == std::nullopt);
  }

  SECTION("string_format_test.str_to_float")
  {
    CHECK(str_to_float("0") == 0.0f);
    CHECK(str_to_float("1.0") == 1.0f);
    CHECK(str_to_float("  1.0     ") == 1.0f);
    CHECK(str_to_float("a123231.0") == std::nullopt);
    CHECK(str_to_float(" ") == std::nullopt);
    CHECK(str_to_float("") == std::nullopt);
  }

  SECTION("string_format_test.str_to_double")
  {
    CHECK(str_to_double("0") == 0.0);
    CHECK(str_to_double("1.0") == 1.0);
    CHECK(str_to_double("  1.0     ") == 1.0);
    CHECK(str_to_double("a123231.0") == std::nullopt);
    CHECK(str_to_double(" ") == std::nullopt);
    CHECK(str_to_double("") == std::nullopt);
  }
}
} // namespace kdl
