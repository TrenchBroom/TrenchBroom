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

#include "kdl/collection_utils.h"
#include "kdl/string_compare.h"

#include <catch2/catch.hpp>

namespace kdl {
namespace cs {
TEST_CASE("string_utils_cs_test.str_mismatch", "[string_utils_cs_test]") {
  CHECK(str_mismatch("", "") == 0u);
  CHECK(str_mismatch("asdf", "asdf") == 4u);
  CHECK(str_mismatch("ssdf", "asdf") == 0u);
  CHECK(str_mismatch("asdf", "ssdf") == 0u);
  CHECK(str_mismatch("aadf", "asdf") == 1u);
  CHECK(str_mismatch("asdf", "aadf") == 1u);
  CHECK(str_mismatch("asaf", "asdf") == 2u);
  CHECK(str_mismatch("asdf", "asaf") == 2u);
  CHECK(str_mismatch("asda", "asdf") == 3u);
  CHECK(str_mismatch("asdf", "asda") == 3u);

  CHECK(str_mismatch("asdf", "Asdf") == 0u);
  CHECK(str_mismatch("asDf", "asdf") == 2u);
}

TEST_CASE("string_utils_cs_test.str_contains", "[string_utils_cs_test]") {
  CHECK_FALSE(str_contains("", ""));
  CHECK(str_contains("asdf", ""));
  CHECK(str_contains("asdf", "a"));
  CHECK(str_contains("asdf", "s"));
  CHECK(str_contains("asdf", "d"));
  CHECK(str_contains("asdf", "f"));
  CHECK(str_contains("asdf", "as"));
  CHECK(str_contains("asdf", "sd"));
  CHECK(str_contains("asdf", "df"));
  CHECK(str_contains("asdf", "asd"));
  CHECK(str_contains("asdf", "asdf"));

  CHECK_FALSE(str_contains("asdf", "m"));
  CHECK_FALSE(str_contains("asdf", "sdf2"));
  CHECK_FALSE(str_contains("asdf", "asf"));
  CHECK_FALSE(str_contains("asdf", "sde"));
  CHECK_FALSE(str_contains("asdf", "esd"));

  CHECK_FALSE(str_contains("asdf", "Asdf"));
  CHECK_FALSE(str_contains("asdf", "A"));
  CHECK_FALSE(str_contains("asdf", "S"));
  CHECK_FALSE(str_contains("asdf", "D"));
  CHECK_FALSE(str_contains("asdf", "F"));
  CHECK_FALSE(str_contains("asdf", "ASDF"));
}

TEST_CASE("string_utils_cs_test.str_is_prefix", "[string_utils_cs_test]") {
  CHECK(str_is_prefix("asdf", ""));
  CHECK(str_is_prefix("asdf", "a"));
  CHECK(str_is_prefix("asdf", "as"));
  CHECK(str_is_prefix("asdf", "asd"));
  CHECK(str_is_prefix("asdf", "asdf"));
  CHECK_FALSE(str_is_prefix("asdf", "sdf"));
  CHECK_FALSE(str_is_prefix("asdf", "aasdf"));
  CHECK_FALSE(str_is_prefix("asdf", "df"));

  CHECK_FALSE(str_is_prefix("asdf", "A"));
  CHECK_FALSE(str_is_prefix("asdf", "aS"));
  CHECK_FALSE(str_is_prefix("asdf", "Asd"));
  CHECK_FALSE(str_is_prefix("asdf", "asDF"));
}

TEST_CASE("string_utils_cs_test.str_is_suffix", "[string_utils_cs_test]") {
  CHECK(str_is_suffix("asdf", ""));
  CHECK(str_is_suffix("asdf", "f"));
  CHECK(str_is_suffix("asdf", "df"));
  CHECK(str_is_suffix("asdf", "sdf"));
  CHECK(str_is_suffix("asdf", "asdf"));
  CHECK_FALSE(str_is_suffix("asdf", "ff"));
  CHECK_FALSE(str_is_suffix("asdf", "aasdf"));

  CHECK_FALSE(str_is_suffix("asdf", "F"));
  CHECK_FALSE(str_is_suffix("asdf", "Df"));
  CHECK_FALSE(str_is_suffix("asdf", "Sdf"));
  CHECK_FALSE(str_is_suffix("asdf", "ASDf"));
}

TEST_CASE("string_utils_cs_test.str_compare", "[string_utils_cs_test]") {
  CHECK(str_compare("", "") == 0);
  CHECK(str_compare("a", "a") == 0);
  CHECK(str_compare("", "a") == -1);
  CHECK(str_compare("a", "") == +1);
  CHECK(str_compare("as", "asd") == -1);
  CHECK(str_compare("asdf", "asd") == +1);
  CHECK(str_compare("asdf", "wxyt") == -1);
  CHECK(str_compare("asdf", "Wxyt") == +1);
  CHECK(str_compare("Asdf", "Wxyt") == -1);
}

TEST_CASE("string_utils_cs_test.str_is_equal", "[string_utils_cs_test]") {
  CHECK(str_is_equal("", ""));
  CHECK(str_is_equal("asdf", "asdf"));
  CHECK_FALSE(str_is_equal("asdf", "asdF"));
  CHECK_FALSE(str_is_equal("AsdF", "Asdf"));
}

TEST_CASE("string_utils_cs_test.str_matches_glob", "[string_utils_cs_test]") {
  CHECK(str_matches_glob("", ""));
  CHECK(str_matches_glob("", "*"));
  CHECK_FALSE(str_matches_glob("", "?"));
  CHECK(str_matches_glob("asdf", "asdf"));
  CHECK(str_matches_glob("asdf", "*"));
  CHECK(str_matches_glob("asdf", "a??f"));
  CHECK_FALSE(str_matches_glob("asdf", "a?f"));
  CHECK(str_matches_glob("asdf", "*f"));
  CHECK(str_matches_glob("asdf", "a*f"));
  CHECK(str_matches_glob("asdf", "?s?f"));
  CHECK(str_matches_glob("asdfjkl", "a*f*l"));
  CHECK(str_matches_glob("asdfjkl", "*a*f*l*"));
  CHECK(str_matches_glob("asd*fjkl", "*a*f*l*"));
  CHECK(str_matches_glob("asd*fjkl", "asd\\*fjkl"));
  CHECK(str_matches_glob("asd*?fj\\kl", "asd\\*\\?fj\\\\kl"));
  CHECK_FALSE(str_matches_glob("asdf", "*F"));
  CHECK_FALSE(str_matches_glob("asdF", "a*f"));
  CHECK_FALSE(str_matches_glob("ASDF", "?S?f"));

  CHECK_FALSE(str_matches_glob("classname", "*_color"));

  CHECK_FALSE(str_matches_glob("", "%"));
  CHECK(str_matches_glob("", "%*"));
  CHECK(str_matches_glob("0", "%"));
  CHECK(str_matches_glob("1", "%"));
  CHECK(str_matches_glob("2", "%"));
  CHECK(str_matches_glob("9", "%"));
  CHECK_FALSE(str_matches_glob("99", "%"));
  CHECK_FALSE(str_matches_glob("a", "%"));
  CHECK_FALSE(str_matches_glob("Z", "%"));
  CHECK_FALSE(str_matches_glob("3Z", "%*"));
  CHECK_FALSE(str_matches_glob("Zasdf", "*%"));
  CHECK(str_matches_glob("Zasdf3", "*%"));
  CHECK(str_matches_glob("Zasdf33", "*%"));
  CHECK(str_matches_glob("Zasdf33", "Z*%%"));
  CHECK(str_matches_glob("Zasdf3376", "Z*%*"));
  CHECK(str_matches_glob("Zasdf3376bdc", "Z*%*"));
  CHECK_FALSE(str_matches_glob("Zasdf3376bdc", "Zasdf%*"));
  CHECK(str_matches_glob("Zasdf3376bdc", "Z*%*bdc"));
  CHECK(str_matches_glob("Zasdf3376bdc", "Z*%**"));
  CHECK(str_matches_glob("78777Zasdf3376bdc", "%*Z*%**"));

  CHECK(str_matches_glob("34dkadj%773", "*\\%%*"));
}

template <typename C> C sorted(C c) {
  return kdl::col_sort(std::move(c), string_less());
}

TEST_CASE("string_utils_cs_test.sort", "[string_utils_cs_test]") {
  CHECK_THAT(sorted(std::vector<std::string>{}), Catch::Equals(std::vector<std::string>{}));

  CHECK_THAT(
    sorted(std::vector<std::string>{
      "Zasdf",
      "Ab",
      "c",
      "a",
      "def",
      "aab",
    }),
    Catch::Equals(std::vector<std::string>{
      "Ab",
      "Zasdf",
      "a",
      "aab",
      "c",
      "def",
    }));
}
} // namespace cs

namespace ci {
TEST_CASE("string_utils_ci_test.str_mismatch", "[string_utils_ci_test]") {
  CHECK(str_mismatch("", "") == 0u);
  CHECK(str_mismatch("asdf", "asdf") == 4u);
  CHECK(str_mismatch("ssdf", "asdf") == 0u);
  CHECK(str_mismatch("asdf", "ssdf") == 0u);
  CHECK(str_mismatch("aadf", "asdf") == 1u);
  CHECK(str_mismatch("asdf", "aadf") == 1u);
  CHECK(str_mismatch("asaf", "asdf") == 2u);
  CHECK(str_mismatch("asdf", "asaf") == 2u);
  CHECK(str_mismatch("asda", "asdf") == 3u);
  CHECK(str_mismatch("asdf", "asda") == 3u);

  CHECK(str_mismatch("asdf", "Asdf") == 4u);
  CHECK(str_mismatch("asDf", "asdf") == 4u);
}

TEST_CASE("string_utils_ci_test.str_contains", "[string_utils_ci_test]") {
  CHECK_FALSE(str_contains("", ""));
  CHECK(str_contains("asdf", ""));
  CHECK(str_contains("asdf", "a"));
  CHECK(str_contains("asdf", "s"));
  CHECK(str_contains("asdf", "d"));
  CHECK(str_contains("asdf", "f"));
  CHECK(str_contains("asdf", "as"));
  CHECK(str_contains("asdf", "sd"));
  CHECK(str_contains("asdf", "df"));
  CHECK(str_contains("asdf", "asd"));
  CHECK(str_contains("asdf", "asdf"));

  CHECK_FALSE(str_contains("asdf", "m"));
  CHECK_FALSE(str_contains("asdf", "sdf2"));
  CHECK_FALSE(str_contains("asdf", "asf"));
  CHECK_FALSE(str_contains("asdf", "sde"));
  CHECK_FALSE(str_contains("asdf", "esd"));

  CHECK(str_contains("asdf", "Asdf"));
  CHECK(str_contains("asdf", "A"));
  CHECK(str_contains("asdf", "S"));
  CHECK(str_contains("asdf", "D"));
  CHECK(str_contains("asdf", "F"));
  CHECK(str_contains("asdf", "ASDF"));
}

TEST_CASE("string_utils_ci_test.str_is_prefix", "[string_utils_ci_test]") {
  CHECK(str_is_prefix("asdf", ""));
  CHECK(str_is_prefix("asdf", "a"));
  CHECK(str_is_prefix("asdf", "as"));
  CHECK(str_is_prefix("asdf", "asd"));
  CHECK(str_is_prefix("asdf", "asdf"));
  CHECK_FALSE(str_is_prefix("asdf", "sdf"));
  CHECK_FALSE(str_is_prefix("asdf", "aasdf"));
  CHECK_FALSE(str_is_prefix("asdf", "df"));

  CHECK(str_is_prefix("asdf", "A"));
  CHECK(str_is_prefix("asdf", "aS"));
  CHECK(str_is_prefix("asdf", "Asd"));
  CHECK(str_is_prefix("asdf", "asDF"));
  CHECK_FALSE(str_is_prefix("asdf", "aAsdf"));
  CHECK_FALSE(str_is_prefix("asdf", "DF"));
}

TEST_CASE("string_utils_ci_test.str_is_suffix", "[string_utils_ci_test]") {
  CHECK(str_is_suffix("asdf", ""));
  CHECK(str_is_suffix("asdf", "f"));
  CHECK(str_is_suffix("asdf", "df"));
  CHECK(str_is_suffix("asdf", "sdf"));
  CHECK(str_is_suffix("asdf", "asdf"));
  CHECK_FALSE(str_is_suffix("asdf", "ff"));
  CHECK_FALSE(str_is_suffix("asdf", "aasdf"));
  CHECK_FALSE(str_is_suffix("asdf", "FF"));
  CHECK_FALSE(str_is_suffix("asdf", "aSDdf"));

  CHECK(str_is_suffix("asdf", "F"));
  CHECK(str_is_suffix("asdf", "Df"));
  CHECK(str_is_suffix("asdf", "Sdf"));
  CHECK(str_is_suffix("asdf", "ASDf"));
}

TEST_CASE("string_utils_ci_test.str_compare", "[string_utils_ci_test]") {
  CHECK(str_compare("", "") == 0);
  CHECK(str_compare("a", "a") == 0);
  CHECK(str_compare("", "a") == -1);
  CHECK(str_compare("a", "") == +1);
  CHECK(str_compare("as", "asd") == -1);
  CHECK(str_compare("asdf", "asd") == +1);
  CHECK(str_compare("asdf", "wxyt") == -1);
  CHECK(str_compare("asdf", "Wxyt") == -1);
  CHECK(str_compare("Asdf", "Wxyt") == -1);
}

TEST_CASE("string_utils_ci_test.str_is_equal", "[string_utils_ci_test]") {
  CHECK(str_is_equal("", ""));
  CHECK(str_is_equal("asdf", "asdf"));
  CHECK(str_is_equal("asdf", "asdF"));
  CHECK(str_is_equal("AsdF", "Asdf"));
  CHECK_FALSE(str_is_equal("asdff", "asdF"));
  CHECK_FALSE(str_is_equal("dfdd", "Asdf"));
}

TEST_CASE("string_utils_ci_test.str_matches_glob", "[string_utils_ci_test]") {
  CHECK(str_matches_glob("ASdf", "asdf"));
  CHECK(str_matches_glob("AsdF", "*"));
  CHECK(str_matches_glob("ASdf", "a??f"));
  CHECK_FALSE(str_matches_glob("AsDF", "a?f"));
  CHECK(str_matches_glob("asdF", "*f"));
  CHECK(str_matches_glob("aSDF", "a*f"));
  CHECK(str_matches_glob("ASDF", "?s?f"));
  CHECK(str_matches_glob("AsDfjkl", "a*f*l"));
  CHECK(str_matches_glob("AsDfjkl", "*a*f*l*"));
  CHECK(str_matches_glob("ASd*fjKl", "*a*f*l*"));
  CHECK(str_matches_glob("ASd*fjKl", "asd\\*fjkl"));
  CHECK(str_matches_glob("aSD*?fJ\\kL", "asd\\*\\?fj\\\\kl"));
}

template <typename C> C sorted(C c) {
  return kdl::col_sort(std::move(c), string_less());
}

TEST_CASE("string_utils_ci_test.sort", "[string_utils_ci_test]") {
  CHECK_THAT(sorted(std::vector<std::string>{}), Catch::Equals(std::vector<std::string>{}));

  CHECK_THAT(
    sorted(std::vector<std::string>{
      "Zasdf",
      "Ab",
      "c",
      "a",
      "def",
      "aab",
    }),
    Catch::Equals(std::vector<std::string>{
      "a",
      "aab",
      "Ab",
      "c",
      "def",
      "Zasdf",
    }));
}
} // namespace ci
} // namespace kdl
