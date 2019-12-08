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

#include "kdl/collection_utils.h"
#include "kdl/string_compare.h"

namespace kdl {
    namespace cs {
        TEST(string_utils_cs_test, mismatch) {
            ASSERT_EQ(0u, mismatch("", ""));
            ASSERT_EQ(4u, mismatch("asdf", "asdf"));
            ASSERT_EQ(0u, mismatch("ssdf", "asdf"));
            ASSERT_EQ(0u, mismatch("asdf", "ssdf"));
            ASSERT_EQ(1u, mismatch("aadf", "asdf"));
            ASSERT_EQ(1u, mismatch("asdf", "aadf"));
            ASSERT_EQ(2u, mismatch("asaf", "asdf"));
            ASSERT_EQ(2u, mismatch("asdf", "asaf"));
            ASSERT_EQ(3u, mismatch("asda", "asdf"));
            ASSERT_EQ(3u, mismatch("asdf", "asda"));

            ASSERT_EQ(0u, mismatch("asdf", "Asdf"));
            ASSERT_EQ(2u, mismatch("asDf", "asdf"));
        }

        TEST(string_utils_cs_test, contains) {
            ASSERT_FALSE(contains("", ""));
            ASSERT_TRUE(contains("asdf", ""));
            ASSERT_TRUE(contains("asdf", "a"));
            ASSERT_TRUE(contains("asdf", "s"));
            ASSERT_TRUE(contains("asdf", "d"));
            ASSERT_TRUE(contains("asdf", "f"));
            ASSERT_TRUE(contains("asdf", "as"));
            ASSERT_TRUE(contains("asdf", "sd"));
            ASSERT_TRUE(contains("asdf", "df"));
            ASSERT_TRUE(contains("asdf", "asd"));
            ASSERT_TRUE(contains("asdf", "asdf"));

            ASSERT_FALSE(contains("asdf", "m"));
            ASSERT_FALSE(contains("asdf", "sdf2"));
            ASSERT_FALSE(contains("asdf", "asf"));
            ASSERT_FALSE(contains("asdf", "sde"));
            ASSERT_FALSE(contains("asdf", "esd"));

            ASSERT_FALSE(contains("asdf", "Asdf"));
            ASSERT_FALSE(contains("asdf", "A"));
            ASSERT_FALSE(contains("asdf", "S"));
            ASSERT_FALSE(contains("asdf", "D"));
            ASSERT_FALSE(contains("asdf", "F"));
            ASSERT_FALSE(contains("asdf", "ASDF"));
        }

        TEST(string_utils_cs_test, is_prefix) {
            ASSERT_TRUE(is_prefix("asdf", ""));
            ASSERT_TRUE(is_prefix("asdf", "a"));
            ASSERT_TRUE(is_prefix("asdf", "as"));
            ASSERT_TRUE(is_prefix("asdf", "asd"));
            ASSERT_TRUE(is_prefix("asdf", "asdf"));
            ASSERT_FALSE(is_prefix("asdf", "sdf"));
            ASSERT_FALSE(is_prefix("asdf", "aasdf"));
            ASSERT_FALSE(is_prefix("asdf", "df"));

            ASSERT_FALSE(is_prefix("asdf", "A"));
            ASSERT_FALSE(is_prefix("asdf", "aS"));
            ASSERT_FALSE(is_prefix("asdf", "Asd"));
            ASSERT_FALSE(is_prefix("asdf", "asDF"));
        }

        TEST(string_utils_cs_test, is_suffix) {
            ASSERT_TRUE(is_suffix("asdf", ""));
            ASSERT_TRUE(is_suffix("asdf", "f"));
            ASSERT_TRUE(is_suffix("asdf", "df"));
            ASSERT_TRUE(is_suffix("asdf", "sdf"));
            ASSERT_TRUE(is_suffix("asdf", "asdf"));
            ASSERT_FALSE(is_suffix("asdf", "ff"));
            ASSERT_FALSE(is_suffix("asdf", "aasdf"));

            ASSERT_FALSE(is_suffix("asdf", "F"));
            ASSERT_FALSE(is_suffix("asdf", "Df"));
            ASSERT_FALSE(is_suffix("asdf", "Sdf"));
            ASSERT_FALSE(is_suffix("asdf", "ASDf"));
        }

        TEST(string_utils_cs_test, compare) {
            ASSERT_EQ(0, compare("", ""));
            ASSERT_EQ(0, compare("a", "a"));
            ASSERT_EQ(-1, compare("", "a"));
            ASSERT_EQ(+1, compare("a", ""));
            ASSERT_EQ(-1, compare("as", "asd"));
            ASSERT_EQ(+1, compare("asdf", "asd"));
            ASSERT_EQ(-1, compare("asdf", "wxyt"));
            ASSERT_EQ(+1, compare("asdf", "Wxyt"));
            ASSERT_EQ(-1, compare("Asdf", "Wxyt"));
        }

        TEST(string_utils_cs_test, is_equal) {
            ASSERT_TRUE(is_equal("", ""));
            ASSERT_TRUE(is_equal("asdf", "asdf"));
            ASSERT_FALSE(is_equal("asdf", "asdF"));
            ASSERT_FALSE(is_equal("AsdF", "Asdf"));
        }

        TEST(string_utils_cs_test, matches_glob) {
            ASSERT_TRUE(matches_glob("", ""));
            ASSERT_TRUE(matches_glob("", "*"));
            ASSERT_FALSE(matches_glob("", "?"));
            ASSERT_TRUE(matches_glob("asdf", "asdf"));
            ASSERT_TRUE(matches_glob("asdf", "*"));
            ASSERT_TRUE(matches_glob("asdf", "a??f"));
            ASSERT_FALSE(matches_glob("asdf", "a?f"));
            ASSERT_TRUE(matches_glob("asdf", "*f"));
            ASSERT_TRUE(matches_glob("asdf", "a*f"));
            ASSERT_TRUE(matches_glob("asdf", "?s?f"));
            ASSERT_TRUE(matches_glob("asdfjkl", "a*f*l"));
            ASSERT_TRUE(matches_glob("asdfjkl", "*a*f*l*"));
            ASSERT_TRUE(matches_glob("asd*fjkl", "*a*f*l*"));
            ASSERT_TRUE(matches_glob("asd*fjkl", "asd\\*fjkl"));
            ASSERT_TRUE(matches_glob("asd*?fj\\kl", "asd\\*\\?fj\\\\kl"));
            ASSERT_FALSE(matches_glob("asdf", "*F"));
            ASSERT_FALSE(matches_glob("asdF", "a*f"));
            ASSERT_FALSE(matches_glob("ASDF", "?S?f"));

            ASSERT_FALSE(matches_glob("classname", "*_color"));
        }

        template <typename C>
        C sorted(C c) {
            kdl::sort(c, string_less());
            return c;
        }

        TEST(string_utils_cs_test, sort) {
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
        TEST(string_utils_ci_test, mismatch) {
            ASSERT_EQ(0u, mismatch("", ""));
            ASSERT_EQ(4u, mismatch("asdf", "asdf"));
            ASSERT_EQ(0u, mismatch("ssdf", "asdf"));
            ASSERT_EQ(0u, mismatch("asdf", "ssdf"));
            ASSERT_EQ(1u, mismatch("aadf", "asdf"));
            ASSERT_EQ(1u, mismatch("asdf", "aadf"));
            ASSERT_EQ(2u, mismatch("asaf", "asdf"));
            ASSERT_EQ(2u, mismatch("asdf", "asaf"));
            ASSERT_EQ(3u, mismatch("asda", "asdf"));
            ASSERT_EQ(3u, mismatch("asdf", "asda"));

            ASSERT_EQ(4u, mismatch("asdf", "Asdf"));
            ASSERT_EQ(4u, mismatch("asDf", "asdf"));
        }

        TEST(string_utils_ci_test, contains) {
            ASSERT_FALSE(contains("", ""));
            ASSERT_TRUE(contains("asdf", ""));
            ASSERT_TRUE(contains("asdf", "a"));
            ASSERT_TRUE(contains("asdf", "s"));
            ASSERT_TRUE(contains("asdf", "d"));
            ASSERT_TRUE(contains("asdf", "f"));
            ASSERT_TRUE(contains("asdf", "as"));
            ASSERT_TRUE(contains("asdf", "sd"));
            ASSERT_TRUE(contains("asdf", "df"));
            ASSERT_TRUE(contains("asdf", "asd"));
            ASSERT_TRUE(contains("asdf", "asdf"));

            ASSERT_FALSE(contains("asdf", "m"));
            ASSERT_FALSE(contains("asdf", "sdf2"));
            ASSERT_FALSE(contains("asdf", "asf"));
            ASSERT_FALSE(contains("asdf", "sde"));
            ASSERT_FALSE(contains("asdf", "esd"));

            ASSERT_TRUE(contains("asdf", "Asdf"));
            ASSERT_TRUE(contains("asdf", "A"));
            ASSERT_TRUE(contains("asdf", "S"));
            ASSERT_TRUE(contains("asdf", "D"));
            ASSERT_TRUE(contains("asdf", "F"));
            ASSERT_TRUE(contains("asdf", "ASDF"));
        }


        TEST(string_utils_ci_test, is_prefix) {
            ASSERT_TRUE(is_prefix("asdf", ""));
            ASSERT_TRUE(is_prefix("asdf", "a"));
            ASSERT_TRUE(is_prefix("asdf", "as"));
            ASSERT_TRUE(is_prefix("asdf", "asd"));
            ASSERT_TRUE(is_prefix("asdf", "asdf"));
            ASSERT_FALSE(is_prefix("asdf", "sdf"));
            ASSERT_FALSE(is_prefix("asdf", "aasdf"));
            ASSERT_FALSE(is_prefix("asdf", "df"));

            ASSERT_TRUE(is_prefix("asdf", "A"));
            ASSERT_TRUE(is_prefix("asdf", "aS"));
            ASSERT_TRUE(is_prefix("asdf", "Asd"));
            ASSERT_TRUE(is_prefix("asdf", "asDF"));
            ASSERT_FALSE(is_prefix("asdf", "aAsdf"));
            ASSERT_FALSE(is_prefix("asdf", "DF"));
        }


        TEST(string_utils_ci_test, is_suffix) {
            ASSERT_TRUE(is_suffix("asdf", ""));
            ASSERT_TRUE(is_suffix("asdf", "f"));
            ASSERT_TRUE(is_suffix("asdf", "df"));
            ASSERT_TRUE(is_suffix("asdf", "sdf"));
            ASSERT_TRUE(is_suffix("asdf", "asdf"));
            ASSERT_FALSE(is_suffix("asdf", "ff"));
            ASSERT_FALSE(is_suffix("asdf", "aasdf"));
            ASSERT_FALSE(is_suffix("asdf", "FF"));
            ASSERT_FALSE(is_suffix("asdf", "aSDdf"));

            ASSERT_TRUE(is_suffix("asdf", "F"));
            ASSERT_TRUE(is_suffix("asdf", "Df"));
            ASSERT_TRUE(is_suffix("asdf", "Sdf"));
            ASSERT_TRUE(is_suffix("asdf", "ASDf"));
        }

        TEST(string_utils_ci_test, compare) {
            ASSERT_EQ(0, compare("", ""));
            ASSERT_EQ(0, compare("a", "a"));
            ASSERT_EQ(-1, compare("", "a"));
            ASSERT_EQ(+1, compare("a", ""));
            ASSERT_EQ(-1, compare("as", "asd"));
            ASSERT_EQ(+1, compare("asdf", "asd"));
            ASSERT_EQ(-1, compare("asdf", "wxyt"));
            ASSERT_EQ(-1, compare("asdf", "Wxyt"));
            ASSERT_EQ(-1, compare("Asdf", "Wxyt"));
        }

        TEST(string_utils_ci_test, is_equal) {
            ASSERT_TRUE(is_equal("", ""));
            ASSERT_TRUE(is_equal("asdf", "asdf"));
            ASSERT_TRUE(is_equal("asdf", "asdF"));
            ASSERT_TRUE(is_equal("AsdF", "Asdf"));
            ASSERT_FALSE(is_equal("asdff", "asdF"));
            ASSERT_FALSE(is_equal("dfdd", "Asdf"));
        }

        TEST(string_utils_ci_test, matches_glob) {
            ASSERT_TRUE(matches_glob("ASdf", "asdf"));
            ASSERT_TRUE(matches_glob("AsdF", "*"));
            ASSERT_TRUE(matches_glob("ASdf", "a??f"));
            ASSERT_FALSE(matches_glob("AsDF", "a?f"));
            ASSERT_TRUE(matches_glob("asdF", "*f"));
            ASSERT_TRUE(matches_glob("aSDF", "a*f"));
            ASSERT_TRUE(matches_glob("ASDF", "?s?f"));
            ASSERT_TRUE(matches_glob("AsDfjkl", "a*f*l"));
            ASSERT_TRUE(matches_glob("AsDfjkl", "*a*f*l*"));
            ASSERT_TRUE(matches_glob("ASd*fjKl", "*a*f*l*"));
            ASSERT_TRUE(matches_glob("ASd*fjKl", "asd\\*fjkl"));
            ASSERT_TRUE(matches_glob("aSD*?fJ\\kL", "asd\\*\\?fj\\\\kl"));
        }

        template <typename C>
        C sorted(C c) {
            kdl::sort(c, string_less());
            return c;
        }

        TEST(string_utils_ci_test, sort) {
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

