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

#include "kdl/collection_utils.h"
#include "kdl/string_compare.h"

#include "GTestCompat.h"

#include <catch2/catch.hpp>

namespace kdl {
    namespace cs {
        TEST_CASE("string_utils_cs_test.str_mismatch", "[string_utils_cs_test]") {
            ASSERT_EQ(0u, str_mismatch("", ""));
            ASSERT_EQ(4u, str_mismatch("asdf", "asdf"));
            ASSERT_EQ(0u, str_mismatch("ssdf", "asdf"));
            ASSERT_EQ(0u, str_mismatch("asdf", "ssdf"));
            ASSERT_EQ(1u, str_mismatch("aadf", "asdf"));
            ASSERT_EQ(1u, str_mismatch("asdf", "aadf"));
            ASSERT_EQ(2u, str_mismatch("asaf", "asdf"));
            ASSERT_EQ(2u, str_mismatch("asdf", "asaf"));
            ASSERT_EQ(3u, str_mismatch("asda", "asdf"));
            ASSERT_EQ(3u, str_mismatch("asdf", "asda"));

            ASSERT_EQ(0u, str_mismatch("asdf", "Asdf"));
            ASSERT_EQ(2u, str_mismatch("asDf", "asdf"));
        }

        TEST_CASE("string_utils_cs_test.str_contains", "[string_utils_cs_test]") {
            ASSERT_FALSE(str_contains("", ""));
            ASSERT_TRUE(str_contains("asdf", ""));
            ASSERT_TRUE(str_contains("asdf", "a"));
            ASSERT_TRUE(str_contains("asdf", "s"));
            ASSERT_TRUE(str_contains("asdf", "d"));
            ASSERT_TRUE(str_contains("asdf", "f"));
            ASSERT_TRUE(str_contains("asdf", "as"));
            ASSERT_TRUE(str_contains("asdf", "sd"));
            ASSERT_TRUE(str_contains("asdf", "df"));
            ASSERT_TRUE(str_contains("asdf", "asd"));
            ASSERT_TRUE(str_contains("asdf", "asdf"));

            ASSERT_FALSE(str_contains("asdf", "m"));
            ASSERT_FALSE(str_contains("asdf", "sdf2"));
            ASSERT_FALSE(str_contains("asdf", "asf"));
            ASSERT_FALSE(str_contains("asdf", "sde"));
            ASSERT_FALSE(str_contains("asdf", "esd"));

            ASSERT_FALSE(str_contains("asdf", "Asdf"));
            ASSERT_FALSE(str_contains("asdf", "A"));
            ASSERT_FALSE(str_contains("asdf", "S"));
            ASSERT_FALSE(str_contains("asdf", "D"));
            ASSERT_FALSE(str_contains("asdf", "F"));
            ASSERT_FALSE(str_contains("asdf", "ASDF"));
        }

        TEST_CASE("string_utils_cs_test.str_is_prefix", "[string_utils_cs_test]") {
            ASSERT_TRUE(str_is_prefix("asdf", ""));
            ASSERT_TRUE(str_is_prefix("asdf", "a"));
            ASSERT_TRUE(str_is_prefix("asdf", "as"));
            ASSERT_TRUE(str_is_prefix("asdf", "asd"));
            ASSERT_TRUE(str_is_prefix("asdf", "asdf"));
            ASSERT_FALSE(str_is_prefix("asdf", "sdf"));
            ASSERT_FALSE(str_is_prefix("asdf", "aasdf"));
            ASSERT_FALSE(str_is_prefix("asdf", "df"));

            ASSERT_FALSE(str_is_prefix("asdf", "A"));
            ASSERT_FALSE(str_is_prefix("asdf", "aS"));
            ASSERT_FALSE(str_is_prefix("asdf", "Asd"));
            ASSERT_FALSE(str_is_prefix("asdf", "asDF"));
        }

        TEST_CASE("string_utils_cs_test.str_is_suffix", "[string_utils_cs_test]") {
            ASSERT_TRUE(str_is_suffix("asdf", ""));
            ASSERT_TRUE(str_is_suffix("asdf", "f"));
            ASSERT_TRUE(str_is_suffix("asdf", "df"));
            ASSERT_TRUE(str_is_suffix("asdf", "sdf"));
            ASSERT_TRUE(str_is_suffix("asdf", "asdf"));
            ASSERT_FALSE(str_is_suffix("asdf", "ff"));
            ASSERT_FALSE(str_is_suffix("asdf", "aasdf"));

            ASSERT_FALSE(str_is_suffix("asdf", "F"));
            ASSERT_FALSE(str_is_suffix("asdf", "Df"));
            ASSERT_FALSE(str_is_suffix("asdf", "Sdf"));
            ASSERT_FALSE(str_is_suffix("asdf", "ASDf"));
        }

        TEST_CASE("string_utils_cs_test.str_compare", "[string_utils_cs_test]") {
            ASSERT_EQ(0, str_compare("", ""));
            ASSERT_EQ(0, str_compare("a", "a"));
            ASSERT_EQ(-1, str_compare("", "a"));
            ASSERT_EQ(+1, str_compare("a", ""));
            ASSERT_EQ(-1, str_compare("as", "asd"));
            ASSERT_EQ(+1, str_compare("asdf", "asd"));
            ASSERT_EQ(-1, str_compare("asdf", "wxyt"));
            ASSERT_EQ(+1, str_compare("asdf", "Wxyt"));
            ASSERT_EQ(-1, str_compare("Asdf", "Wxyt"));
        }

        TEST_CASE("string_utils_cs_test.str_is_equal", "[string_utils_cs_test]") {
            ASSERT_TRUE(str_is_equal("", ""));
            ASSERT_TRUE(str_is_equal("asdf", "asdf"));
            ASSERT_FALSE(str_is_equal("asdf", "asdF"));
            ASSERT_FALSE(str_is_equal("AsdF", "Asdf"));
        }

        TEST_CASE("string_utils_cs_test.str_matches_glob", "[string_utils_cs_test]") {
            ASSERT_TRUE(str_matches_glob("", ""));
            ASSERT_TRUE(str_matches_glob("", "*"));
            ASSERT_FALSE(str_matches_glob("", "?"));
            ASSERT_TRUE(str_matches_glob("asdf", "asdf"));
            ASSERT_TRUE(str_matches_glob("asdf", "*"));
            ASSERT_TRUE(str_matches_glob("asdf", "a??f"));
            ASSERT_FALSE(str_matches_glob("asdf", "a?f"));
            ASSERT_TRUE(str_matches_glob("asdf", "*f"));
            ASSERT_TRUE(str_matches_glob("asdf", "a*f"));
            ASSERT_TRUE(str_matches_glob("asdf", "?s?f"));
            ASSERT_TRUE(str_matches_glob("asdfjkl", "a*f*l"));
            ASSERT_TRUE(str_matches_glob("asdfjkl", "*a*f*l*"));
            ASSERT_TRUE(str_matches_glob("asd*fjkl", "*a*f*l*"));
            ASSERT_TRUE(str_matches_glob("asd*fjkl", "asd\\*fjkl"));
            ASSERT_TRUE(str_matches_glob("asd*?fj\\kl", "asd\\*\\?fj\\\\kl"));
            ASSERT_FALSE(str_matches_glob("asdf", "*F"));
            ASSERT_FALSE(str_matches_glob("asdF", "a*f"));
            ASSERT_FALSE(str_matches_glob("ASDF", "?S?f"));

            ASSERT_FALSE(str_matches_glob("classname", "*_color"));

            ASSERT_FALSE(str_matches_glob("", "%"));
            ASSERT_TRUE(str_matches_glob("", "%*"));
            ASSERT_TRUE(str_matches_glob("0", "%"));
            ASSERT_TRUE(str_matches_glob("1", "%"));
            ASSERT_TRUE(str_matches_glob("2", "%"));
            ASSERT_TRUE(str_matches_glob("9", "%"));
            ASSERT_FALSE(str_matches_glob("99", "%"));
            ASSERT_FALSE(str_matches_glob("a", "%"));
            ASSERT_FALSE(str_matches_glob("Z", "%"));
            ASSERT_FALSE(str_matches_glob("3Z", "%*"));
            ASSERT_FALSE(str_matches_glob("Zasdf", "*%"));
            ASSERT_TRUE(str_matches_glob("Zasdf3", "*%"));
            ASSERT_TRUE(str_matches_glob("Zasdf33", "*%"));
            ASSERT_TRUE(str_matches_glob("Zasdf33", "Z*%%"));
            ASSERT_TRUE(str_matches_glob("Zasdf3376", "Z*%*"));
            ASSERT_TRUE(str_matches_glob("Zasdf3376bdc", "Z*%*"));
            ASSERT_FALSE(str_matches_glob("Zasdf3376bdc", "Zasdf%*"));
            ASSERT_TRUE(str_matches_glob("Zasdf3376bdc", "Z*%*bdc"));
            ASSERT_TRUE(str_matches_glob("Zasdf3376bdc", "Z*%**"));
            ASSERT_TRUE(str_matches_glob("78777Zasdf3376bdc", "%*Z*%**"));

            ASSERT_TRUE(str_matches_glob("34dkadj%773", "*\\%%*"));
        }

        template <typename C>
        C sorted(C c) {
            return kdl::col_sort(std::move(c), string_less());
        }

        TEST_CASE("string_utils_cs_test.sort", "[string_utils_cs_test]") {
            ASSERT_EQ(std::vector<std::string>({}), sorted(std::vector<std::string>({})));
            ASSERT_EQ(std::vector<std::string>({
                "Ab",
                "Zasdf",
                "a",
                "aab",
                "c",
                "def",
            }), sorted(std::vector<std::string>({
                "Zasdf",
                "Ab",
                "c",
                "a",
                "def",
                "aab",
            })));
        }
    }

    namespace ci {
        TEST_CASE("string_utils_ci_test.str_mismatch", "[string_utils_ci_test]") {
            ASSERT_EQ(0u, str_mismatch("", ""));
            ASSERT_EQ(4u, str_mismatch("asdf", "asdf"));
            ASSERT_EQ(0u, str_mismatch("ssdf", "asdf"));
            ASSERT_EQ(0u, str_mismatch("asdf", "ssdf"));
            ASSERT_EQ(1u, str_mismatch("aadf", "asdf"));
            ASSERT_EQ(1u, str_mismatch("asdf", "aadf"));
            ASSERT_EQ(2u, str_mismatch("asaf", "asdf"));
            ASSERT_EQ(2u, str_mismatch("asdf", "asaf"));
            ASSERT_EQ(3u, str_mismatch("asda", "asdf"));
            ASSERT_EQ(3u, str_mismatch("asdf", "asda"));

            ASSERT_EQ(4u, str_mismatch("asdf", "Asdf"));
            ASSERT_EQ(4u, str_mismatch("asDf", "asdf"));
        }

        TEST_CASE("string_utils_ci_test.str_contains", "[string_utils_ci_test]") {
            ASSERT_FALSE(str_contains("", ""));
            ASSERT_TRUE(str_contains("asdf", ""));
            ASSERT_TRUE(str_contains("asdf", "a"));
            ASSERT_TRUE(str_contains("asdf", "s"));
            ASSERT_TRUE(str_contains("asdf", "d"));
            ASSERT_TRUE(str_contains("asdf", "f"));
            ASSERT_TRUE(str_contains("asdf", "as"));
            ASSERT_TRUE(str_contains("asdf", "sd"));
            ASSERT_TRUE(str_contains("asdf", "df"));
            ASSERT_TRUE(str_contains("asdf", "asd"));
            ASSERT_TRUE(str_contains("asdf", "asdf"));

            ASSERT_FALSE(str_contains("asdf", "m"));
            ASSERT_FALSE(str_contains("asdf", "sdf2"));
            ASSERT_FALSE(str_contains("asdf", "asf"));
            ASSERT_FALSE(str_contains("asdf", "sde"));
            ASSERT_FALSE(str_contains("asdf", "esd"));

            ASSERT_TRUE(str_contains("asdf", "Asdf"));
            ASSERT_TRUE(str_contains("asdf", "A"));
            ASSERT_TRUE(str_contains("asdf", "S"));
            ASSERT_TRUE(str_contains("asdf", "D"));
            ASSERT_TRUE(str_contains("asdf", "F"));
            ASSERT_TRUE(str_contains("asdf", "ASDF"));
        }


        TEST_CASE("string_utils_ci_test.str_is_prefix", "[string_utils_ci_test]") {
            ASSERT_TRUE(str_is_prefix("asdf", ""));
            ASSERT_TRUE(str_is_prefix("asdf", "a"));
            ASSERT_TRUE(str_is_prefix("asdf", "as"));
            ASSERT_TRUE(str_is_prefix("asdf", "asd"));
            ASSERT_TRUE(str_is_prefix("asdf", "asdf"));
            ASSERT_FALSE(str_is_prefix("asdf", "sdf"));
            ASSERT_FALSE(str_is_prefix("asdf", "aasdf"));
            ASSERT_FALSE(str_is_prefix("asdf", "df"));

            ASSERT_TRUE(str_is_prefix("asdf", "A"));
            ASSERT_TRUE(str_is_prefix("asdf", "aS"));
            ASSERT_TRUE(str_is_prefix("asdf", "Asd"));
            ASSERT_TRUE(str_is_prefix("asdf", "asDF"));
            ASSERT_FALSE(str_is_prefix("asdf", "aAsdf"));
            ASSERT_FALSE(str_is_prefix("asdf", "DF"));
        }


        TEST_CASE("string_utils_ci_test.str_is_suffix", "[string_utils_ci_test]") {
            ASSERT_TRUE(str_is_suffix("asdf", ""));
            ASSERT_TRUE(str_is_suffix("asdf", "f"));
            ASSERT_TRUE(str_is_suffix("asdf", "df"));
            ASSERT_TRUE(str_is_suffix("asdf", "sdf"));
            ASSERT_TRUE(str_is_suffix("asdf", "asdf"));
            ASSERT_FALSE(str_is_suffix("asdf", "ff"));
            ASSERT_FALSE(str_is_suffix("asdf", "aasdf"));
            ASSERT_FALSE(str_is_suffix("asdf", "FF"));
            ASSERT_FALSE(str_is_suffix("asdf", "aSDdf"));

            ASSERT_TRUE(str_is_suffix("asdf", "F"));
            ASSERT_TRUE(str_is_suffix("asdf", "Df"));
            ASSERT_TRUE(str_is_suffix("asdf", "Sdf"));
            ASSERT_TRUE(str_is_suffix("asdf", "ASDf"));
        }

        TEST_CASE("string_utils_ci_test.str_compare", "[string_utils_ci_test]") {
            ASSERT_EQ(0, str_compare("", ""));
            ASSERT_EQ(0, str_compare("a", "a"));
            ASSERT_EQ(-1, str_compare("", "a"));
            ASSERT_EQ(+1, str_compare("a", ""));
            ASSERT_EQ(-1, str_compare("as", "asd"));
            ASSERT_EQ(+1, str_compare("asdf", "asd"));
            ASSERT_EQ(-1, str_compare("asdf", "wxyt"));
            ASSERT_EQ(-1, str_compare("asdf", "Wxyt"));
            ASSERT_EQ(-1, str_compare("Asdf", "Wxyt"));
        }

        TEST_CASE("string_utils_ci_test.str_is_equal", "[string_utils_ci_test]") {
            ASSERT_TRUE(str_is_equal("", ""));
            ASSERT_TRUE(str_is_equal("asdf", "asdf"));
            ASSERT_TRUE(str_is_equal("asdf", "asdF"));
            ASSERT_TRUE(str_is_equal("AsdF", "Asdf"));
            ASSERT_FALSE(str_is_equal("asdff", "asdF"));
            ASSERT_FALSE(str_is_equal("dfdd", "Asdf"));
        }

        TEST_CASE("string_utils_ci_test.str_matches_glob", "[string_utils_ci_test]") {
            ASSERT_TRUE(str_matches_glob("ASdf", "asdf"));
            ASSERT_TRUE(str_matches_glob("AsdF", "*"));
            ASSERT_TRUE(str_matches_glob("ASdf", "a??f"));
            ASSERT_FALSE(str_matches_glob("AsDF", "a?f"));
            ASSERT_TRUE(str_matches_glob("asdF", "*f"));
            ASSERT_TRUE(str_matches_glob("aSDF", "a*f"));
            ASSERT_TRUE(str_matches_glob("ASDF", "?s?f"));
            ASSERT_TRUE(str_matches_glob("AsDfjkl", "a*f*l"));
            ASSERT_TRUE(str_matches_glob("AsDfjkl", "*a*f*l*"));
            ASSERT_TRUE(str_matches_glob("ASd*fjKl", "*a*f*l*"));
            ASSERT_TRUE(str_matches_glob("ASd*fjKl", "asd\\*fjkl"));
            ASSERT_TRUE(str_matches_glob("aSD*?fJ\\kL", "asd\\*\\?fj\\\\kl"));
        }

        template <typename C>
        C sorted(C c) {
            return kdl::col_sort(std::move(c), string_less());
        }

        TEST_CASE("string_utils_ci_test.sort", "[string_utils_ci_test]") {
            ASSERT_EQ(std::vector<std::string>({}), sorted(std::vector<std::string>({})));
            ASSERT_EQ(std::vector<std::string>({
                "a",
                "aab",
                "Ab",
                "c",
                "def",
                "Zasdf",
            }), sorted(std::vector<std::string>({
                "Zasdf",
                "Ab",
                "c",
                "a",
                "def",
                "aab",
            })));
        }
    }
}

