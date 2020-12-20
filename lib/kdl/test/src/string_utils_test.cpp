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

#include <kdl/string_utils.h>

#include <optional>
#include <ostream>

#include "GTestCompat.h"

#include <catch2/catch.hpp>

namespace kdl {
    TEST_CASE("string_utils_test.str_split", "[string_utils_test]") {
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

    TEST_CASE("string_utils_test.str_join", "[string_utils_test]") {
        ASSERT_EQ("", str_join(std::vector<std::string_view>({}), ", ", " and ", ", and "));
        ASSERT_EQ("one", str_join(std::vector<std::string_view>({"one"}), ", ", " and ", ", and "));
        ASSERT_EQ("one and two", str_join(std::vector<std::string_view>({"one", "two"}), ", ", ", and ", " and "));
        ASSERT_EQ("one, two, and three", str_join(std::vector<std::string_view>({"one", "two", "three"}), ", ", ", and ", " and "));

        ASSERT_EQ("", str_join(std::vector<std::string_view>({}), ", "));
        ASSERT_EQ("one", str_join(std::vector<std::string_view>({"one"}), ", "));
        ASSERT_EQ("one, two", str_join(std::vector<std::string_view>({"one", "two"}), ", "));
        ASSERT_EQ("one, two, three", str_join(std::vector<std::string_view>({"one", "two", "three"}), ", "));
    }

    TEST_CASE("string_utils_test.str_replace_every", "[string_utils_test]") {
        ASSERT_EQ("", str_replace_every("", "", "haha"));
        ASSERT_EQ("asdf", str_replace_every("asdf", "", "haha"));
        ASSERT_EQ("asdf", str_replace_every("asdf", "haha", "haha"));
        ASSERT_EQ("asdf", str_replace_every("asdf", "sd", "sd"));
        ASSERT_EQ("adsf", str_replace_every("asdf", "sd", "ds"));
        ASSERT_EQ("asds", str_replace_every("asdf", "df", "ds"));
        ASSERT_EQ("asds asds", str_replace_every("asdf asdf", "df", "ds"));
        ASSERT_EQ("thE brick brown fox", str_replace_every("the brick brown fox", "e", "E"));
        ASSERT_EQ("TEH brick brown fox", str_replace_every("the brick brown fox", "the", "TEH"));
        ASSERT_EQ("the click clown fox", str_replace_every("the brick brown fox", "br", "cl"));
        ASSERT_EQ("the brick crown fox", str_replace_every("the brick brown fox", "bro", "cro"));
    }

    struct to_string {
        std::string x;
    };

    inline std::ostream& operator<<(std::ostream& o, const to_string& t) {
        o << t.x << ";";
        return o;
    }

    TEST_CASE("string_format_test.str_to_string", "[string_format_test]") {
        ASSERT_EQ("abc", str_to_string("abc"));
        ASSERT_EQ("1234", str_to_string(1234));
        ASSERT_EQ("1", str_to_string(1.0));
        ASSERT_EQ("xyz;", str_to_string(to_string{"xyz"}));
    }

    TEST_CASE("string_format_test.str_to_int", "[string_format_test]") {
        ASSERT_EQ(std::optional<int>{0}, str_to_int("0"));
        ASSERT_EQ(std::optional<int>{1}, str_to_int("1"));
        ASSERT_EQ(std::optional<int>{123231}, str_to_int("123231"));
        ASSERT_EQ(std::optional<int>{-123231}, str_to_int("-123231"));
        ASSERT_EQ(std::optional<int>{123231}, str_to_int("123231b"));
        ASSERT_EQ(std::optional<int>{123231}, str_to_int("   123231   "));
        ASSERT_EQ(std::nullopt, str_to_int("a123231"));
        ASSERT_EQ(std::nullopt, str_to_int(" "));
        ASSERT_EQ(std::nullopt, str_to_int(""));
    }

    TEST_CASE("string_format_test.str_to_long", "[string_format_test]") {
        ASSERT_EQ(std::optional<long>{0l}, str_to_long("0"));
        ASSERT_EQ(std::optional<long>{1l}, str_to_long("1"));
        ASSERT_EQ(std::optional<long>{123231l}, str_to_long("123231"));
        ASSERT_EQ(std::optional<long>{-123231l}, str_to_long("-123231"));
        ASSERT_EQ(std::optional<long>{2147483647l}, str_to_long("2147483647"));
        ASSERT_EQ(std::optional<long>{-2147483646l}, str_to_long("-2147483646"));
        ASSERT_EQ(std::optional<long>{123231l}, str_to_long("123231b"));
        ASSERT_EQ(std::optional<long>{123231l}, str_to_long("   123231   "));
        ASSERT_EQ(std::nullopt, str_to_long("a123231"));
        ASSERT_EQ(std::nullopt, str_to_long(" "));
        ASSERT_EQ(std::nullopt, str_to_long(""));
    }

    TEST_CASE("string_format_test.str_to_long_long", "[string_format_test]") {
        ASSERT_EQ(std::optional<long long>{0ll}, str_to_long_long("0"));
        ASSERT_EQ(std::optional<long long>{1ll}, str_to_long_long("1"));
        ASSERT_EQ(std::optional<long long>{123231ll}, str_to_long_long("123231"));
        ASSERT_EQ(std::optional<long long>{-123231ll}, str_to_long_long("-123231"));
        ASSERT_EQ(std::optional<long long>{2147483647ll}, str_to_long_long("2147483647"));
        ASSERT_EQ(std::optional<long long>{-2147483646ll}, str_to_long_long("-2147483646"));
        ASSERT_EQ(std::optional<long long>{9'223'372'036'854'775'807ll}, str_to_long_long("9223372036854775807"));
        ASSERT_EQ(std::optional<long long>{-9'223'372'036'854'775'806ll}, str_to_long_long("-9223372036854775806"));
        ASSERT_EQ(std::optional<long long>{123231ll}, str_to_long_long("123231b"));
        ASSERT_EQ(std::optional<long long>{123231ll}, str_to_long_long("   123231   "));
        ASSERT_EQ(std::nullopt, str_to_long_long("a123231"));
        ASSERT_EQ(std::nullopt, str_to_long_long(" "));
        ASSERT_EQ(std::nullopt, str_to_long_long(""));
    }

    TEST_CASE("string_format_test.str_to_u_long", "[string_format_test]") {

        ASSERT_EQ(std::optional<unsigned long>{0ul}, str_to_u_long("0"));
        ASSERT_EQ(std::optional<unsigned long>{1ul}, str_to_u_long("1"));
        ASSERT_EQ(std::optional<unsigned long>{123231ul}, str_to_u_long("123231"));
        ASSERT_EQ(std::optional<unsigned long>{2147483647ul}, str_to_u_long("2147483647"));
        ASSERT_EQ(std::optional<unsigned long>{123231ul}, str_to_u_long("123231b"));
        ASSERT_EQ(std::optional<unsigned long>{123231ul}, str_to_u_long("   123231   "));
        ASSERT_EQ(std::nullopt, str_to_u_long("a123231"));
        ASSERT_EQ(std::nullopt, str_to_u_long(" "));
        ASSERT_EQ(std::nullopt, str_to_u_long(""));
    }

    TEST_CASE("string_format_test.str_to_u_long_long", "[string_format_test]") {
        ASSERT_EQ(std::optional<unsigned long long>{0ull}, str_to_u_long_long("0"));
        ASSERT_EQ(std::optional<unsigned long long>{1ull}, str_to_u_long_long("1"));
        ASSERT_EQ(std::optional<unsigned long long>{123231ull}, str_to_u_long_long("123231"));
        ASSERT_EQ(std::optional<unsigned long long>{2147483647ull}, str_to_u_long_long("2147483647"));
        ASSERT_EQ(std::optional<unsigned long long>{9'223'372'036'854'775'807ull}, str_to_u_long_long("9223372036854775807"));
        ASSERT_EQ(std::optional<unsigned long long>{123231ull}, str_to_u_long_long("123231b"));
        ASSERT_EQ(std::optional<unsigned long long>{123231ull}, str_to_u_long_long("   123231   "));
        ASSERT_EQ(std::nullopt, str_to_u_long_long("a123231"));
        ASSERT_EQ(std::nullopt, str_to_u_long_long(" "));
        ASSERT_EQ(std::nullopt, str_to_u_long_long(""));
    }

    TEST_CASE("string_format_test.str_to_size", "[string_format_test]") {
        ASSERT_EQ(std::optional<std::size_t>{0u}, str_to_size("0"));
        ASSERT_EQ(std::optional<std::size_t>{1u}, str_to_size("1"));
        ASSERT_EQ(std::optional<std::size_t>{123231u}, str_to_size("123231"));
        ASSERT_EQ(std::optional<std::size_t>{2147483647u}, str_to_size("2147483647"));
        ASSERT_EQ(std::optional<std::size_t>{123231u}, str_to_size("123231b"));
        ASSERT_EQ(std::optional<std::size_t>{123231u}, str_to_size("   123231   "));
        ASSERT_EQ(std::nullopt, str_to_size("a123231"));
        ASSERT_EQ(std::nullopt, str_to_size(" "));
        ASSERT_EQ(std::nullopt, str_to_size(""));
    }

    TEST_CASE("string_format_test.str_to_float", "[string_format_test]") {
        ASSERT_EQ(std::optional<float>{0.0f}, str_to_float("0"));
        ASSERT_EQ(std::optional<float>{1.0f}, str_to_float("1.0"));
        ASSERT_EQ(std::nullopt, str_to_float("a123231.0"));
        ASSERT_EQ(std::nullopt, str_to_float(" "));
        ASSERT_EQ(std::nullopt, str_to_float(""));
    }

    TEST_CASE("string_format_test.str_to_double", "[string_format_test]") {
        ASSERT_EQ(std::optional<double>{0.0}, str_to_double("0"));
        ASSERT_EQ(std::optional<double>{1.0}, str_to_double("1.0"));
        ASSERT_EQ(std::nullopt, str_to_double("a123231.0"));
        ASSERT_EQ(std::nullopt, str_to_double(" "));
        ASSERT_EQ(std::nullopt, str_to_double(""));
    }

    TEST_CASE("string_format_test.str_to_long_double", "[string_format_test]") {
        ASSERT_EQ(std::optional<long double>{0.0L}, str_to_long_double("0"));
        ASSERT_EQ(std::optional<long double>{1.0L}, str_to_long_double("1.0"));
        ASSERT_EQ(std::nullopt, str_to_long_double("a123231.0"));
        ASSERT_EQ(std::nullopt, str_to_long_double(" "));
        ASSERT_EQ(std::nullopt, str_to_long_double(""));
    }
}
