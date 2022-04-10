/*
 Copyright 2022 Kristian Duske

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

#include "kdl/range_io.h"
#include "kdl/reflection_decl.h"
#include "kdl/reflection_impl.h"
#include "kdl/string_utils.h"

#include <vector>

#include <catch2/catch.hpp>

namespace kdl {
namespace detail {
TEST_CASE("reflection_count_tokens") {
  static_assert(reflection_count_tokens("") == 0);
  static_assert(reflection_count_tokens("  ") == 0);
  static_assert(reflection_count_tokens(",") == 0);
  static_assert(reflection_count_tokens(" ,  ") == 0);
  static_assert(reflection_count_tokens("asdf") == 1);
  static_assert(reflection_count_tokens("asdf,blah") == 2);
  static_assert(reflection_count_tokens(" asdf ,  blah ") == 2);
}

TEST_CASE("reflection_split_tokens") {
  CHECK(reflection_split_tokens<0>("") == std::array<std::string_view, 0>{});
  CHECK(reflection_split_tokens<0>("   ") == std::array<std::string_view, 0>{});
  CHECK(reflection_split_tokens<0>(",") == std::array<std::string_view, 0>{});
  CHECK(reflection_split_tokens<0>(" ,  ") == std::array<std::string_view, 0>{});
  CHECK(reflection_split_tokens<1>("asdf") == std::array<std::string_view, 1>{"asdf"});
  CHECK(reflection_split_tokens<2>("asdf,blah") == std::array<std::string_view, 2>{"asdf", "blah"});
  CHECK(
    reflection_split_tokens<2>(" asdf ,  blah ") ==
    std::array<std::string_view, 2>{"asdf", "blah"});
}
} // namespace detail

struct empty {
  kdl_reflect_inline_empty(empty);
};

struct test {
  int someName;
  std::string otherName;

  kdl_reflect_inline(test, someName, otherName);
};

TEST_CASE("member_names") {
  CHECK(empty::member_names() == std::array<std::string_view, 0>{});
  CHECK(test::member_names() == std::array<std::string_view, 2>{"someName", "otherName"});
}

TEST_CASE("members") {
  CHECK(empty{}.members() == std::make_tuple());
  CHECK(test{2, "asdf"}.members() == std::make_tuple(2, "asdf"));
}

TEST_CASE("operator==") {
  CHECK(test{2, "asdf"} == test{2, "asdf"});
  CHECK_FALSE(test{2, "asdf"} == test{3, "asdf"});
  CHECK_FALSE(test{2, "asdf"} == test{2, "x"});
}

TEST_CASE("operator!=") {
  CHECK(test{2, "asdf"} != test{3, "asdf"});
  CHECK(test{2, "asdf"} != test{2, "x"});
  CHECK_FALSE(test{2, "asdf"} != test{2, "asdf"});
}

TEST_CASE("operator<") {
  CHECK(test{1, "asdf"} < test{2, "asdf"});
  CHECK_FALSE(test{2, "asdf"} < test{2, "asdf"});
  CHECK_FALSE(test{3, "asdf"} < test{2, "asdf"});

  CHECK(test{2, "asdf"} < test{2, "bsdf"});
  CHECK_FALSE(test{2, "asdf"} < test{2, "abdf"});
}

struct incomparable {};

[[maybe_unused]] static std::ostream& operator<<(std::ostream& lhs, const incomparable&) {
  return lhs << "incomparable";
}

struct test_incomparable_member {
  incomparable x;

  kdl_reflect_inline(test_incomparable_member, x);
};

namespace test_ns {
struct custom {
  std::vector<int> v;

  kdl_reflect_inline(custom, v);
};

} // namespace test_ns

TEST_CASE("operator<<") {
  CHECK(str_to_string(empty{}) == "empty{}");
  CHECK(str_to_string(test{1, "asdf"}) == "test{someName: 1, otherName: asdf}");
  CHECK(str_to_string(test_ns::custom{{1, 2, 3}}) == "custom{v: [1,2,3]}");
}
} // namespace kdl
