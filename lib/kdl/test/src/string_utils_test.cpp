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

#include <catch2/catch.hpp>

namespace kdl {
    TEST_CASE("string_utils_test.str_split", "[string_utils_test]") {
        CHECK_THAT(str_split("", " "),                          Catch::Equals(std::vector<std::string>{}));
        CHECK_THAT(str_split(" ", " "),                         Catch::Equals(std::vector<std::string>{}));
        CHECK_THAT(str_split("asdf", " "),                      Catch::Equals(std::vector<std::string>{ "asdf" }));
        CHECK_THAT(str_split("d asdf", " "),                    Catch::Equals(std::vector<std::string>{ "d", "asdf" }));
        CHECK_THAT(str_split("asdf d", " "),                    Catch::Equals(std::vector<std::string>{ "asdf", "d" }));
        CHECK_THAT(str_split("The quick brown fox", " "),       Catch::Equals(std::vector<std::string>{ "The", "quick", "brown", "fox" }));
        CHECK_THAT(str_split(" The quick brown fox", " "),      Catch::Equals(std::vector<std::string>{ "The", "quick", "brown", "fox" }));
        CHECK_THAT(str_split("  The quick brown fox ", " "),    Catch::Equals(std::vector<std::string>{ "The", "quick", "brown", "fox" }));
        CHECK_THAT(str_split("The quick   brown fox", " "),     Catch::Equals(std::vector<std::string>{ "The", "quick", "brown", "fox" }));
        CHECK_THAT(str_split("The quick   brown fox", " f"),    Catch::Equals(std::vector<std::string>{ "The", "quick", "brown", "ox" }));
        CHECK_THAT(str_split("The; quick brown; fox", ";"),     Catch::Equals(std::vector<std::string>{ "The", "quick brown", "fox" }));
        CHECK_THAT(str_split("The;quick brown; fox", " ;"),     Catch::Equals(std::vector<std::string>{ "The", "quick", "brown", "fox" }));
        CHECK_THAT(str_split("The\\; quick brown; fox", ";"),   Catch::Equals(std::vector<std::string>{ "The; quick brown", "fox" }));
        CHECK_THAT(str_split("The\\\\; quick brown; fox", ";"), Catch::Equals(std::vector<std::string>{ "The\\", "quick brown", "fox" }));
        CHECK_THAT(str_split("c:\\x\\y", "\\"),                 Catch::Equals(std::vector<std::string>{ "c:", "x", "y" }));
    }

    TEST_CASE("string_utils_test.str_join", "[string_utils_test]") {
        CHECK(str_join(std::vector<std::string_view>{}, ", ", " and ", ", and ") == "");
        CHECK(str_join(std::vector<std::string_view>{"one"}, ", ", " and ", ", and ") == "one");
        CHECK(str_join(std::vector<std::string_view>{"one", "two"}, ", ", ", and ", " and ") == "one and two");
        CHECK(str_join(std::vector<std::string_view>{"one", "two", "three"}, ", ", ", and ", " and ") == "one, two, and three");

        CHECK(str_join(std::vector<std::string_view>{}, ", ") == "");
        CHECK(str_join(std::vector<std::string_view>{"one"}, ", ") == "one");
        CHECK(str_join(std::vector<std::string_view>{"one", "two"}, ", ") == "one, two");
        CHECK(str_join(std::vector<std::string_view>{"one", "two", "three"}, ", ") == "one, two, three");
    }

    TEST_CASE("string_utils_test.str_replace_every", "[string_utils_test]") {
        CHECK(str_replace_every("", "", "haha") == "");
        CHECK(str_replace_every("asdf", "", "haha") == "asdf");
        CHECK(str_replace_every("asdf", "haha", "haha") == "asdf");
        CHECK(str_replace_every("asdf", "sd", "sd") == "asdf");
        CHECK(str_replace_every("asdf", "sd", "ds") == "adsf");
        CHECK(str_replace_every("asdf", "df", "ds") == "asds");
        CHECK(str_replace_every("asdf asdf", "df", "ds") == "asds asds");
        CHECK(str_replace_every("the brick brown fox", "e", "E") == "thE brick brown fox");
        CHECK(str_replace_every("the brick brown fox", "the", "TEH") == "TEH brick brown fox");
        CHECK(str_replace_every("the brick brown fox", "br", "cl") == "the click clown fox");
        CHECK(str_replace_every("the brick brown fox", "bro", "cro") == "the brick crown fox");
    }

    struct to_string {
        std::string x;
    };

    inline std::ostream& operator<<(std::ostream& o, const to_string& t) {
        o << t.x << ";";
        return o;
    }

    TEST_CASE("string_format_test.str_to_string", "[string_format_test]") {
        CHECK(str_to_string("abc") == "abc");
        CHECK(str_to_string(1234) == "1234");
        CHECK(str_to_string(1.0) == "1");
        CHECK(str_to_string(to_string{"xyz"}) == "xyz;");
    }

    TEST_CASE("string_format_test.str_to_int", "[string_format_test]") {
        CHECK(str_to_int("0") == std::optional<int>{0});
        CHECK(str_to_int("1") == std::optional<int>{1});
        CHECK(str_to_int("123231") == std::optional<int>{123231});
        CHECK(str_to_int("-123231") == std::optional<int>{-123231});
        CHECK(str_to_int("123231b") == std::optional<int>{123231});
        CHECK(str_to_int("   123231   ") == std::optional<int>{123231});
        CHECK(str_to_int("a123231") == std::nullopt);
        CHECK(str_to_int(" ") == std::nullopt);
        CHECK(str_to_int("") == std::nullopt);
    }

    TEST_CASE("string_format_test.str_to_long", "[string_format_test]") {
        CHECK(str_to_long("0") == std::optional<long>{0l});
        CHECK(str_to_long("1") == std::optional<long>{1l});
        CHECK(str_to_long("123231") == std::optional<long>{123231l});
        CHECK(str_to_long("-123231") == std::optional<long>{-123231l});
        CHECK(str_to_long("2147483647") == std::optional<long>{2147483647l});
        CHECK(str_to_long("-2147483646") == std::optional<long>{-2147483646l});
        CHECK(str_to_long("123231b") == std::optional<long>{123231l});
        CHECK(str_to_long("   123231   ") == std::optional<long>{123231l});
        CHECK(str_to_long("a123231") == std::nullopt);
        CHECK(str_to_long(" ") == std::nullopt);
        CHECK(str_to_long("") == std::nullopt);
    }

    TEST_CASE("string_format_test.str_to_long_long", "[string_format_test]") {
        CHECK(str_to_long_long("0") == std::optional<long long>{0ll});
        CHECK(str_to_long_long("1") == std::optional<long long>{1ll});
        CHECK(str_to_long_long("123231") == std::optional<long long>{123231ll});
        CHECK(str_to_long_long("-123231") == std::optional<long long>{-123231ll});
        CHECK(str_to_long_long("2147483647") == std::optional<long long>{2147483647ll});
        CHECK(str_to_long_long("-2147483646") == std::optional<long long>{-2147483646ll});
        CHECK(str_to_long_long("9223372036854775807") == std::optional<long long>{9'223'372'036'854'775'807ll});
        CHECK(str_to_long_long("-9223372036854775806") == std::optional<long long>{-9'223'372'036'854'775'806ll});
        CHECK(str_to_long_long("123231b") == std::optional<long long>{123231ll});
        CHECK(str_to_long_long("   123231   ") == std::optional<long long>{123231ll});
        CHECK(str_to_long_long("a123231") == std::nullopt);
        CHECK(str_to_long_long(" ") == std::nullopt);
        CHECK(str_to_long_long("") == std::nullopt);
    }

    TEST_CASE("string_format_test.str_to_u_long", "[string_format_test]") {

        CHECK(str_to_u_long("0") == std::optional<unsigned long>{0ul});
        CHECK(str_to_u_long("1") == std::optional<unsigned long>{1ul});
        CHECK(str_to_u_long("123231") == std::optional<unsigned long>{123231ul});
        CHECK(str_to_u_long("2147483647") == std::optional<unsigned long>{2147483647ul});
        CHECK(str_to_u_long("123231b") == std::optional<unsigned long>{123231ul});
        CHECK(str_to_u_long("   123231   ") == std::optional<unsigned long>{123231ul});
        CHECK(str_to_u_long("a123231") == std::nullopt);
        CHECK(str_to_u_long(" ") == std::nullopt);
        CHECK(str_to_u_long("") == std::nullopt);
    }

    TEST_CASE("string_format_test.str_to_u_long_long", "[string_format_test]") {
        CHECK(str_to_u_long_long("0") == std::optional<unsigned long long>{0ull});
        CHECK(str_to_u_long_long("1") == std::optional<unsigned long long>{1ull});
        CHECK(str_to_u_long_long("123231") == std::optional<unsigned long long>{123231ull});
        CHECK(str_to_u_long_long("2147483647") == std::optional<unsigned long long>{2147483647ull});
        CHECK(str_to_u_long_long("9223372036854775807") == std::optional<unsigned long long>{9'223'372'036'854'775'807ull});
        CHECK(str_to_u_long_long("123231b") == std::optional<unsigned long long>{123231ull});
        CHECK(str_to_u_long_long("   123231   ") == std::optional<unsigned long long>{123231ull});
        CHECK(str_to_u_long_long("a123231") == std::nullopt);
        CHECK(str_to_u_long_long(" ") == std::nullopt);
        CHECK(str_to_u_long_long("") == std::nullopt);
    }

    TEST_CASE("string_format_test.str_to_size", "[string_format_test]") {
        CHECK(str_to_size("0") == std::optional<std::size_t>{0u});
        CHECK(str_to_size("1") == std::optional<std::size_t>{1u});
        CHECK(str_to_size("123231") == std::optional<std::size_t>{123231u});
        CHECK(str_to_size("2147483647") == std::optional<std::size_t>{2147483647u});
        CHECK(str_to_size("123231b") == std::optional<std::size_t>{123231u});
        CHECK(str_to_size("   123231   ") == std::optional<std::size_t>{123231u});
        CHECK(str_to_size("a123231") == std::nullopt);
        CHECK(str_to_size(" ") == std::nullopt);
        CHECK(str_to_size("") == std::nullopt);
    }

    TEST_CASE("string_format_test.str_to_float", "[string_format_test]") {
        CHECK(str_to_float("0") == std::optional<float>{0.0f});
        CHECK(str_to_float("1.0") == std::optional<float>{1.0f});
        CHECK(str_to_float("a123231.0") == std::nullopt);
        CHECK(str_to_float(" ") == std::nullopt);
        CHECK(str_to_float("") == std::nullopt);
    }

    TEST_CASE("string_format_test.str_to_double", "[string_format_test]") {
        CHECK(str_to_double("0") == std::optional<double>{0.0});
        CHECK(str_to_double("1.0") == std::optional<double>{1.0});
        CHECK(str_to_double("a123231.0") == std::nullopt);
        CHECK(str_to_double(" ") == std::nullopt);
        CHECK(str_to_double("") == std::nullopt);
    }

    TEST_CASE("string_format_test.str_to_long_double", "[string_format_test]") {
        CHECK(str_to_long_double("0") == std::optional<long double>{0.0L});
        CHECK(str_to_long_double("1.0") == std::optional<long double>{1.0L});
        CHECK(str_to_long_double("a123231.0") == std::nullopt);
        CHECK(str_to_long_double(" ") == std::nullopt);
        CHECK(str_to_long_double("") == std::nullopt);
    }
}
