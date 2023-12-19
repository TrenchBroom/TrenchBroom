/*
 Copyright 2020 Kristian Duske

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

#include "kdl/overload.h"
#include "kdl/reflection_decl.h"
#include "kdl/reflection_impl.h"
#include "kdl/result.h"
#include "kdl/result_fold.h"
#include "kdl/result_io.h"

#include <iostream>
#include <string>

#include "catch2.h"

namespace kdl
{
struct Error1
{
  kdl_reflect_inline_empty(Error1);
};

struct Error2
{
  kdl_reflect_inline_empty(Error2);
};

struct Error3
{
  kdl_reflect_inline_empty(Error3);
};

struct MoveOnly
{
  MoveOnly(const MoveOnly&) = delete;
  MoveOnly& operator=(const MoveOnly&) = delete;

  MoveOnly(MoveOnly&&) noexcept = default;
  MoveOnly& operator=(MoveOnly&&) noexcept = default;

  MoveOnly() = default;

  kdl_reflect_inline_empty(MoveOnly);
};

struct Counter
{
  std::size_t copies = 0u;
  std::size_t moves = 0u;

  Counter() = default;

  Counter(const Counter& c)
    : copies(c.copies + 1u)
    , moves(c.moves)
  {
  }

  Counter(Counter&& c)
    : copies(c.copies)
    , moves(c.moves + 1u)
  {
  }

  Counter& operator=(const Counter& c)
  {
    this->copies = c.copies + 1u;
    this->moves = c.moves;
    return *this;
  }

  Counter& operator=(Counter&& c)
  {
    this->copies = c.copies;
    this->moves = c.moves + 1u;
    return *this;
  }

  kdl_reflect_inline(Counter, copies, moves);
};

TEST_CASE("result_test.void_success")
{
  CHECK(void_success == result<void>{});
  CHECK(void_success.is_success());
  CHECK_FALSE(void_success.is_error());

  const auto make_void_success = []() { return void_success; };
  make_void_success(); // no warning because void_success is not marked as nodiscard
}

TEST_CASE("result_test.constructor")
{
  SECTION("non-void result")
  {
    CHECK((result<int, Error1, Error2>{1}.value() == 1));
    CHECK(
      (result<int, Error1, Error2>{Error1{}}.error()
       == std::variant<Error1, Error2>{Error1{}}));
    CHECK(
      (result<int, Error1, Error2>{Error2{}}.error()
       == std::variant<Error1, Error2>{Error2{}}));
  }

  SECTION("multi-valued result")
  {
    CHECK(
      result<multi_value<int, float>, Error1, Error2>{multi_value{1, 1.0f}}.value()
      == multi_value{1, 1.0f});
    CHECK(
      result<multi_value<int, float>, Error1, Error2>{Error1{}}.error()
      == std::variant<Error1, Error2>{Error1{}});
    CHECK(
      result<multi_value<int, float>, Error1, Error2>{Error2{}}.error()
      == std::variant<Error1, Error2>{Error2{}});
  }

  SECTION("void result with errors")
  {
    CHECK((result<void, Error1, Error2>{}.is_success()));
    CHECK(
      (result<void, Error1, Error2>{Error1{}}.error()
       == std::variant<Error1, Error2>{Error1{}}));
    CHECK(
      (result<void, Error1, Error2>{Error2{}}.error()
       == std::variant<Error1, Error2>{Error2{}}));
  }

  SECTION("void result without errors")
  {
    CHECK((result<void>{}.is_success()));
  }
}

TEST_CASE("result_test.converting_constructor")
{
  SECTION("non-void result")
  {
    CHECK(
      result<MoveOnly, Error1, Error2>{result<MoveOnly, Error1, Error2>{MoveOnly{}}}
      == result<MoveOnly, Error1, Error2>{MoveOnly{}});
    CHECK(
      result<int, Error1, Error2>{result<int, Error1, Error2>{Error1{}}}
      == result<int, Error1, Error2>{Error1{}});

    CHECK(
      result<MoveOnly, Error1, Error2>{result<MoveOnly, Error2, Error1>{MoveOnly{}}}
      == result<MoveOnly, Error1, Error2>{MoveOnly{}});
    CHECK(
      result<int, Error1, Error2>{result<int, Error2, Error1>{Error1{}}}
      == result<int, Error1, Error2>{Error1{}});

    CHECK(
      result<MoveOnly, Error1, Error2>{result<MoveOnly, Error1>{MoveOnly{}}}
      == result<MoveOnly, Error1, Error2>{MoveOnly{}});
    CHECK(
      result<int, Error1, Error2>{result<int, Error1>{Error1{}}}
      == result<int, Error1, Error2>{Error1{}});

    CHECK(
      result<MoveOnly, Error1, Error2>{result<MoveOnly, Error2>{MoveOnly{}}}
      == result<MoveOnly, Error1, Error2>{MoveOnly{}});
    CHECK(
      result<int, Error1, Error2>{result<int, Error2>{Error2{}}}
      == result<int, Error1, Error2>{Error2{}});
  }

  SECTION("multi-valued result")
  {
    CHECK(
      result<multi_value<MoveOnly, int>, Error1, Error2>{
        result<multi_value<MoveOnly, int>, Error1, Error2>{multi_value{MoveOnly{}, 1}}}
      == result<multi_value<MoveOnly, int>, Error1, Error2>{multi_value{MoveOnly{}, 1}});
    CHECK(
      result<multi_value<int, float>, Error1, Error2>{
        result<multi_value<int, float>, Error1, Error2>{Error1{}}}
      == result<multi_value<int, float>, Error1, Error2>{Error1{}});

    CHECK(
      result<multi_value<MoveOnly, int>, Error1, Error2>{
        result<multi_value<MoveOnly, int>, Error2, Error1>{multi_value{MoveOnly{}, 1}}}
      == result<multi_value<MoveOnly, int>, Error1, Error2>{multi_value{MoveOnly{}, 1}});
    CHECK(
      result<multi_value<int, float>, Error1, Error2>{
        result<multi_value<int, float>, Error2, Error1>{Error1{}}}
      == result<multi_value<int, float>, Error1, Error2>{Error1{}});

    CHECK(
      result<multi_value<MoveOnly, int>, Error1, Error2>{
        result<multi_value<MoveOnly, int>, Error1>{multi_value{MoveOnly{}, 1}}}
      == result<multi_value<MoveOnly, int>, Error1, Error2>{multi_value{MoveOnly{}, 1}});
    CHECK(
      result<multi_value<int, float>, Error1, Error2>{
        result<multi_value<int, float>, Error1>{Error1{}}}
      == result<multi_value<int, float>, Error1, Error2>{Error1{}});

    CHECK(
      result<multi_value<MoveOnly, int>, Error1, Error2>{
        result<multi_value<MoveOnly, int>, Error2>{multi_value{MoveOnly{}, 1}}}
      == result<multi_value<MoveOnly, int>, Error1, Error2>{multi_value{MoveOnly{}, 1}});
    CHECK(
      result<multi_value<int, float>, Error1, Error2>{
        result<multi_value<int, float>, Error2>{Error2{}}}
      == result<multi_value<int, float>, Error1, Error2>{Error2{}});
  }

  SECTION("void result with errors")
  {
    CHECK(
      result<void, Error1, Error2>{result<void, Error1, Error2>{}}
      == result<void, Error1, Error2>{});
    CHECK(
      result<void, Error1, Error2>{result<void, Error1, Error2>{Error1{}}}
      == result<void, Error1, Error2>{Error1{}});

    CHECK(
      result<void, Error1, Error2>{result<void, Error2, Error1>{}}
      == result<void, Error1, Error2>{});
    CHECK(
      result<void, Error1, Error2>{result<void, Error2, Error1>{Error1{}}}
      == result<void, Error1, Error2>{Error1{}});

    CHECK(
      result<void, Error1, Error2>{result<void, Error1>{}}
      == result<void, Error1, Error2>{});
    CHECK(
      result<void, Error1, Error2>{result<void, Error1>{Error1{}}}
      == result<void, Error1, Error2>{Error1{}});

    CHECK(
      result<void, Error1, Error2>{result<void, Error2>{}}
      == result<void, Error1, Error2>{});
    CHECK(
      result<void, Error1, Error2>{result<void, Error2>{Error2{}}}
      == result<void, Error1, Error2>{Error2{}});
  }

  SECTION("void result without errors")
  {
    CHECK(result<void>{result<void>{}} == result<void>{});
  }
}

TEST_CASE("result_test.visit")
{
  SECTION("non-void result")
  {
    const auto constLValueSuccess = result<int, Error1, Error2>{1};
    CHECK(constLValueSuccess.visit(
      overload([](const int& x) { return x == 1; }, [](const auto&) { return false; })));

    const auto constLValueError1 = result<int, Error1, Error2>{Error1{}};
    CHECK(constLValueError1.visit(overload(
      [](const int&) { return false; },
      [](const Error1&) { return true; },
      [](const Error2&) { return false; })));

    const auto constLValueError2 = result<int, Error1, Error2>{Error2{}};
    CHECK(constLValueError2.visit(overload(
      [](const int&) { return false; },
      [](const Error1&) { return false; },
      [](const Error2&) { return true; })));

    auto nonConstLValueSuccess = result<int, Error1, Error2>{1};
    CHECK(nonConstLValueSuccess.visit(
      overload([](int& x) { return x == 1; }, [](auto&) { return false; })));

    auto nonConstLValueError1 = result<int, Error1, Error2>{Error1{}};
    CHECK(nonConstLValueError1.visit(overload(
      [](int&) { return false; },
      [](Error1&) { return true; },
      [](Error2&) { return false; })));

    auto nonConstLValueError2 = result<int, Error1, Error2>{Error2{}};
    CHECK(nonConstLValueError2.visit(overload(
      [](int&) { return false; },
      [](Error1&) { return false; },
      [](Error2&) { return true; })));

    CHECK(result<MoveOnly, Error1, Error2>{MoveOnly{}}.visit(
      overload([](MoveOnly&&) { return true; }, [](auto&&) { return false; })));

    CHECK(result<int, Error1, Error2>{1}.visit(
      overload([](int&& x) { return x == 1; }, [](auto&&) { return false; })));

    CHECK(result<int, Error1, Error2>{Error1{}}.visit(overload(
      [](int&&) { return false; },
      [](Error1&&) { return true; },
      [](Error2&&) { return false; })));

    CHECK(result<int, Error1, Error2>{Error2{}}.visit(overload(
      [](int&&) { return false; },
      [](Error1&&) { return false; },
      [](Error2&&) { return true; })));
  }

  SECTION("multi-valued result")
  {
    const auto constLValueSuccess =
      result<multi_value<int, float>, Error1, Error2>{multi_value{1, 2.0f}};
    CHECK(constLValueSuccess.visit(overload(
      [](const multi_value<int, float>& x) {
        return x == multi_value{1, 2.0f};
      },
      [](const auto&) { return false; })));

    const auto constLValueError1 =
      result<multi_value<int, float>, Error1, Error2>{Error1{}};
    CHECK(constLValueError1.visit(overload(
      [](const multi_value<int, float>&) { return false; },
      [](const Error1&) { return true; },
      [](const Error2&) { return false; })));

    const auto constLValueError2 =
      result<multi_value<int, float>, Error1, Error2>{Error2{}};
    CHECK(constLValueError2.visit(overload(
      [](const multi_value<int, float>&) { return false; },
      [](const Error1&) { return false; },
      [](const Error2&) { return true; })));

    auto nonConstLValueSuccess =
      result<multi_value<int, float>, Error1, Error2>{multi_value{1, 2.0f}};
    CHECK(nonConstLValueSuccess.visit(overload(
      [](multi_value<int, float>& x) {
        return x == multi_value{1, 2.0f};
      },
      [](auto&) { return false; })));

    auto nonConstLValueError1 = result<multi_value<int, float>, Error1, Error2>{Error1{}};
    CHECK(nonConstLValueError1.visit(overload(
      [](multi_value<int, float>&) { return false; },
      [](Error1&) { return true; },
      [](Error2&) { return false; })));

    auto nonConstLValueError2 = result<multi_value<int, float>, Error1, Error2>{Error2{}};
    CHECK(nonConstLValueError2.visit(overload(
      [](multi_value<int, float>&) { return false; },
      [](Error1&) { return false; },
      [](Error2&) { return true; })));

    CHECK(result<multi_value<MoveOnly, int>, Error1, Error2>{multi_value{MoveOnly{}, 1}}
            .visit(overload(
              [](multi_value<MoveOnly, int>&&) { return true; },
              [](auto&&) { return false; })));

    CHECK(result<multi_value<int, float>, Error1, Error2>{multi_value{1, 2.0f}}.visit(
      overload(
        [](multi_value<int, float>&& x) {
          return x == multi_value{1, 2.0f};
        },
        [](auto&&) { return false; })));

    CHECK(result<multi_value<int, float>, Error1, Error2>{Error1{}}.visit(overload(
      [](multi_value<int, float>&&) { return false; },
      [](Error1&&) { return true; },
      [](Error2&&) { return false; })));

    CHECK(result<multi_value<int, float>, Error1, Error2>{Error2{}}.visit(overload(
      [](multi_value<int, float>&&) { return false; },
      [](Error1&&) { return false; },
      [](Error2&&) { return true; })));
  }

  SECTION("void result with errors")
  {
    const auto constLValueSuccess = result<void, Error1, Error2>{};
    CHECK(constLValueSuccess.visit(
      overload([]() { return true; }, [](const auto&) { return false; })));

    const auto constLValueError1 = result<void, Error1, Error2>{Error1{}};
    CHECK(constLValueError1.visit(overload(
      []() { return false; },
      [](const Error1&) { return true; },
      [](const Error2&) { return false; })));

    const auto constLValueError2 = result<void, Error1, Error2>{Error2{}};
    CHECK(constLValueError2.visit(overload(
      []() { return false; },
      [](const Error1&) { return false; },
      [](const Error2&) { return true; })));

    auto nonConstLValueSuccess = result<void, Error1, Error2>{};
    CHECK(nonConstLValueSuccess.visit(
      overload([]() { return true; }, [](auto&) { return false; })));

    auto nonConstLValueError1 = result<void, Error1, Error2>{Error1{}};
    CHECK(nonConstLValueError1.visit(overload(
      []() { return false; },
      [](Error1&) { return true; },
      [](Error2&) { return false; })));

    auto nonConstLValueError2 = result<void, Error1, Error2>{Error2{}};
    CHECK(nonConstLValueError2.visit(overload(
      []() { return false; },
      [](Error1&) { return false; },
      [](Error2&) { return true; })));

    CHECK(result<void, Error1, Error2>{}.visit(
      overload([]() { return true; }, [](auto&&) { return false; })));

    CHECK(result<void, Error1, Error2>{Error1{}}.visit(overload(
      []() { return false; },
      [](Error1&&) { return true; },
      [](Error2&&) { return false; })));

    CHECK(result<void, Error1, Error2>{Error2{}}.visit(overload(
      []() { return false; },
      [](Error1&&) { return false; },
      [](Error2&&) { return true; })));
  }

  SECTION("void result without errors")
  {
    const auto constLValueSuccess = result<void>{};
    CHECK(constLValueSuccess.visit([]() { return true; }));

    auto nonConstLValueSuccess = result<void>{};
    CHECK(nonConstLValueSuccess.visit([]() { return true; }));

    CHECK(result<void>{}.visit([]() { return true; }));
  }
}

TEST_CASE("result_test.join")
{
  SECTION("non-void result")
  {
    const auto constLValueSuccess = result<int, Error1, Error2>{1};
    const auto constLValueError = result<int, Error1, Error2>{Error1{}};
    auto nonConstLValueSuccess = result<int, Error1, Error2>{1};
    auto nonConstLValueError = result<int, Error1, Error2>{Error1{}};

    SECTION("with non void result")
    {
      CHECK(
        constLValueSuccess.join(result<float, Error3>{2.0f})
        == result<multi_value<int, float>, Error1, Error2, Error3>{multi_value{1, 2.0f}});
      CHECK(
        constLValueSuccess.join(result<float, Error3>{Error3{}})
        == result<multi_value<int, float>, Error1, Error2, Error3>{Error3{}});

      CHECK(
        constLValueError.join(result<float, Error3>{2.0f})
        == result<multi_value<int, float>, Error1, Error2, Error3>{Error1{}});
      CHECK(
        constLValueError.join(result<float, Error3>{Error3{}})
        == result<multi_value<int, float>, Error1, Error2, Error3>{Error1{}});

      CHECK(
        nonConstLValueSuccess.join(result<float, Error3>{2.0f})
        == result<multi_value<int, float>, Error1, Error2, Error3>{multi_value{1, 2.0f}});
      CHECK(
        nonConstLValueSuccess.join(result<float, Error3>{Error3{}})
        == result<multi_value<int, float>, Error1, Error2, Error3>{Error3{}});

      CHECK(
        nonConstLValueError.join(result<float, Error3>{2.0f})
        == result<multi_value<int, float>, Error1, Error2, Error3>{Error1{}});
      CHECK(
        nonConstLValueError.join(result<float, Error3>{Error3{}})
        == result<multi_value<int, float>, Error1, Error2, Error3>{Error1{}});

      CHECK(
        result<MoveOnly, Error1, Error2>{MoveOnly{}}.join(result<float, Error3>{2.0f})
        == result<multi_value<MoveOnly, float>, Error1, Error2, Error3>{
          multi_value{MoveOnly{}, 2.0f}});
      CHECK(
        result<MoveOnly, Error1, Error2>{MoveOnly{}}.join(result<float, Error3>{Error3{}})
        == result<multi_value<MoveOnly, float>, Error1, Error2, Error3>{Error3{}});

      CHECK(
        result<MoveOnly, Error1, Error2>{Error1{}}.join(result<float, Error3>{2.0f})
        == result<multi_value<MoveOnly, float>, Error1, Error2, Error3>{Error1{}});
      CHECK(
        result<MoveOnly, Error1, Error2>{Error1{}}.join(result<float, Error3>{Error3{}})
        == result<multi_value<MoveOnly, float>, Error1, Error2, Error3>{Error1{}});
    }

    SECTION("with multi-valued result")
    {
      CHECK(
        result<MoveOnly, Error1, Error2>{MoveOnly{}}.join(
          result<multi_value<int, float>, Error3>{multi_value{1, 2.0f}})
        == result<multi_value<MoveOnly, int, float>, Error1, Error2, Error3>{
          multi_value{MoveOnly{}, 1, 2.0f}});

      CHECK(
        result<MoveOnly, Error1, Error2>{MoveOnly{}}.join(
          result<multi_value<int, float>, Error3>{Error3{}})
        == result<multi_value<MoveOnly, int, float>, Error1, Error2, Error3>{Error3{}});

      CHECK(
        result<MoveOnly, Error1, Error2>{Error1{}}.join(
          result<multi_value<int, float>, Error3>{multi_value{1, 2.0f}})
        == result<multi_value<MoveOnly, int, float>, Error1, Error2, Error3>{Error1{}});
      CHECK(
        result<MoveOnly, Error1, Error2>{Error1{}}.join(
          result<multi_value<int, float>, Error3>{Error3{}})
        == result<multi_value<MoveOnly, int, float>, Error1, Error2, Error3>{Error1{}});
    }
  }

  SECTION("multi-valued result")
  {
    const auto constLValueSuccess =
      result<multi_value<std::string, int>, Error1, Error2>{multi_value{"asdf", 1}};
    const auto constLValueError =
      result<multi_value<std::string, int>, Error1, Error2>{Error1{}};
    auto nonConstLValueSuccess =
      result<multi_value<std::string, int>, Error1, Error2>{multi_value{"asdf", 1}};
    auto nonConstLValueError =
      result<multi_value<std::string, int>, Error1, Error2>{Error1{}};

    SECTION("with non void result")
    {
      CHECK(
        constLValueSuccess.join(result<float, Error3>{2.0f})
        == result<multi_value<std::string, int, float>, Error1, Error2, Error3>{
          multi_value{"asdf", 1, 2.0f}});
      CHECK(
        constLValueSuccess.join(result<float, Error3>{Error3{}})
        == result<multi_value<std::string, int, float>, Error1, Error2, Error3>{
          Error3{}});

      CHECK(
        constLValueError.join(result<float, Error3>{2.0f})
        == result<multi_value<std::string, int, float>, Error1, Error2, Error3>{
          Error1{}});
      CHECK(
        constLValueError.join(result<float, Error3>{Error3{}})
        == result<multi_value<std::string, int, float>, Error1, Error2, Error3>{
          Error1{}});

      CHECK(
        nonConstLValueSuccess.join(result<float, Error3>{2.0f})
        == result<multi_value<std::string, int, float>, Error1, Error2, Error3>{
          multi_value{"asdf", 1, 2.0f}});
      CHECK(
        nonConstLValueSuccess.join(result<float, Error3>{Error3{}})
        == result<multi_value<std::string, int, float>, Error1, Error2, Error3>{
          Error3{}});

      CHECK(
        nonConstLValueError.join(result<float, Error3>{2.0f})
        == result<multi_value<std::string, int, float>, Error1, Error2, Error3>{
          Error1{}});
      CHECK(
        nonConstLValueError.join(result<float, Error3>{Error3{}})
        == result<multi_value<std::string, int, float>, Error1, Error2, Error3>{
          Error1{}});

      CHECK(
        result<multi_value<MoveOnly, int>, Error1, Error2>{multi_value{MoveOnly{}, 1}}
          .join(result<float, Error3>{2.0f})
        == result<multi_value<MoveOnly, int, float>, Error1, Error2, Error3>{
          multi_value{MoveOnly{}, 1, 2.0f}});
      CHECK(
        result<multi_value<MoveOnly, int>, Error1, Error2>{multi_value{MoveOnly{}, 1}}
          .join(result<float, Error3>{Error3{}})
        == result<multi_value<MoveOnly, int, float>, Error1, Error2, Error3>{Error3{}});

      CHECK(
        result<multi_value<MoveOnly, int>, Error1, Error2>{Error1{}}.join(
          result<float, Error3>{2.0f})
        == result<multi_value<MoveOnly, int, float>, Error1, Error2, Error3>{Error1{}});
      CHECK(
        result<multi_value<MoveOnly, int>, Error1, Error2>{Error1{}}.join(
          result<float, Error3>{Error3{}})
        == result<multi_value<MoveOnly, int, float>, Error1, Error2, Error3>{Error1{}});
    }

    SECTION("with multi-valued result")
    {
      CHECK(
        constLValueSuccess.join(
          result<multi_value<float, MoveOnly>, Error3>{multi_value{2.0f, MoveOnly{}}})
        == result<multi_value<std::string, int, float, MoveOnly>, Error1, Error2, Error3>{
          multi_value{"asdf", 1, 2.0f, MoveOnly{}}});
      CHECK(
        constLValueSuccess.join(result<multi_value<float, MoveOnly>, Error3>{Error3{}})
        == result<multi_value<std::string, int, float, MoveOnly>, Error1, Error2, Error3>{
          Error3{}});

      CHECK(
        constLValueError.join(
          result<multi_value<float, MoveOnly>, Error3>{multi_value{2.0f, MoveOnly{}}})
        == result<multi_value<std::string, int, float, MoveOnly>, Error1, Error2, Error3>{
          Error1{}});
      CHECK(
        constLValueError.join(result<multi_value<float, MoveOnly>, Error3>{Error3{}})
        == result<multi_value<std::string, int, float, MoveOnly>, Error1, Error2, Error3>{
          Error1{}});

      CHECK(
        nonConstLValueSuccess.join(
          result<multi_value<float, MoveOnly>, Error3>{multi_value{2.0f, MoveOnly{}}})
        == result<multi_value<std::string, int, float, MoveOnly>, Error1, Error2, Error3>{
          multi_value{"asdf", 1, 2.0f, MoveOnly{}}});
      CHECK(
        nonConstLValueSuccess.join(result<multi_value<float, MoveOnly>, Error3>{Error3{}})
        == result<multi_value<std::string, int, float, MoveOnly>, Error1, Error2, Error3>{
          Error3{}});

      CHECK(
        nonConstLValueError.join(
          result<multi_value<float, MoveOnly>, Error3>{multi_value{2.0f, MoveOnly{}}})
        == result<multi_value<std::string, int, float, MoveOnly>, Error1, Error2, Error3>{
          Error1{}});
      CHECK(
        nonConstLValueError.join(result<multi_value<float, MoveOnly>, Error3>{Error3{}})
        == result<multi_value<std::string, int, float, MoveOnly>, Error1, Error2, Error3>{
          Error1{}});

      CHECK(
        result<multi_value<MoveOnly, int>, Error1, Error2>{multi_value{MoveOnly{}, 1}}
          .join(
            result<multi_value<float, MoveOnly>, Error3>{multi_value{2.0f, MoveOnly{}}})
        == result<multi_value<MoveOnly, int, float, MoveOnly>, Error1, Error2, Error3>{
          multi_value{MoveOnly{}, 1, 2.0f, MoveOnly{}}});
      CHECK(
        result<multi_value<MoveOnly, int>, Error1, Error2>{multi_value{MoveOnly{}, 1}}
          .join(result<multi_value<float, MoveOnly>, Error3>{Error3{}})
        == result<multi_value<MoveOnly, int, float, MoveOnly>, Error1, Error2, Error3>{
          Error3{}});

      CHECK(
        result<multi_value<MoveOnly, int>, Error1, Error2>{Error1{}}.join(
          result<multi_value<float, MoveOnly>, Error3>{multi_value{2.0f, MoveOnly{}}})
        == result<multi_value<MoveOnly, int, float, MoveOnly>, Error1, Error2, Error3>{
          Error1{}});
      CHECK(
        result<multi_value<MoveOnly, int>, Error1, Error2>{Error1{}}.join(
          result<multi_value<float, MoveOnly>, Error3>{Error3{}})
        == result<multi_value<MoveOnly, int, float, MoveOnly>, Error1, Error2, Error3>{
          Error1{}});
    }
  }
}

TEST_CASE("result_test.and_then")
{
  SECTION("non-void result")
  {
    const auto constLValueSuccessToSuccess = result<int, Error1, Error2>{1};
    CHECK(constLValueSuccessToSuccess.and_then([](const int& x) {
      CHECK(x == 1);
      return result<float, Error3>{2.0f};
    }) == result<float, Error1, Error2, Error3>{2.0f});

    const auto constLValueSuccessToError = result<int, Error1, Error2>{1};
    CHECK(constLValueSuccessToError.and_then([](const int& x) {
      CHECK(x == 1);
      return result<float, Error3>{Error3{}};
    }) == result<float, Error1, Error2, Error3>{Error3{}});

    const auto constLValueError = result<int, Error1, Error2>{Error1{}};
    CHECK(constLValueError.and_then([](const int&) {
      FAIL();
      return result<float, Error3>{2.0f};
    }) == result<float, Error1, Error2, Error3>{Error1{}});

    auto nonConstLValueSuccessToSuccess = result<int, Error1, Error2>{1};
    CHECK(nonConstLValueSuccessToSuccess.and_then([](int& x) {
      CHECK(x == 1);
      return result<float, Error3>{2.0f};
    }) == result<float, Error1, Error2, Error3>{2.0f});

    auto nonConstLValueSuccessToError = result<int, Error1, Error2>{1};
    CHECK(nonConstLValueSuccessToError.and_then([](int& x) {
      CHECK(x == 1);
      return result<float, Error3>{Error3{}};
    }) == result<float, Error1, Error2, Error3>{Error3{}});

    auto nonConstLValueError = result<int, Error1, Error2>{Error1{}};
    CHECK(nonConstLValueError.and_then([](int&) {
      FAIL();
      return result<float, Error3>{2.0f};
    }) == result<float, Error1, Error2, Error3>{Error1{}});

    CHECK(result<MoveOnly, Error1, Error2>{MoveOnly{}}.and_then([](MoveOnly&& x) {
      return result<MoveOnly, Error3>{std::move(x)};
    }) == result<MoveOnly, Error1, Error2, Error3>{MoveOnly{}});

    CHECK(result<int, Error1, Error2>{1}.and_then([](int&& x) {
      CHECK(x == 1);
      return result<float, Error3>{2.0f};
    }) == result<float, Error1, Error2, Error3>{2.0f});

    CHECK(result<int, Error1, Error2>{1}.and_then([](int&& x) {
      CHECK(x == 1);
      return result<float, Error3>{Error3{}};
    }) == result<float, Error1, Error2, Error3>{Error3{}});

    CHECK(result<int, Error1, Error2>{Error1{}}.and_then([](int&&) {
      FAIL();
      return result<float, Error3>{2.0f};
    }) == result<float, Error1, Error2, Error3>{Error1{}});
  }

  SECTION("multi-valued result")
  {
    const auto constLValueSuccessToSuccess =
      result<multi_value<int, float>, Error1, Error2>{multi_value{1, 2.0f}};
    CHECK(constLValueSuccessToSuccess.and_then([](const int& x, const float& y) {
      CHECK(x == 1);
      CHECK(y == 2.0f);
      return result<float, Error3>{3.0f};
    }) == result<float, Error1, Error2, Error3>{3.0f});

    const auto constLValueSuccessToError =
      result<multi_value<int, float>, Error1, Error2>{multi_value{1, 2.0f}};
    CHECK(constLValueSuccessToError.and_then([](const int& x, const float& y) {
      CHECK(x == 1);
      CHECK(y == 2.0f);
      return result<float, Error3>{Error3{}};
    }) == result<float, Error1, Error2, Error3>{Error3{}});

    const auto constLValueError =
      result<multi_value<int, float>, Error1, Error2>{Error1{}};
    CHECK(constLValueError.and_then([](const int&, const float&) {
      FAIL();
      return result<float, Error3>{3.0f};
    }) == result<float, Error1, Error2, Error3>{Error1{}});

    auto nonConstLValueSuccessToSuccess =
      result<multi_value<int, float>, Error1, Error2>{multi_value{1, 2.0f}};
    CHECK(nonConstLValueSuccessToSuccess.and_then([](int& x, float& y) {
      CHECK(x == 1);
      CHECK(y == 2.0f);
      return result<float, Error3>{3.0f};
    }) == result<float, Error1, Error2, Error3>{3.0f});

    auto nonConstLValueSuccessToError =
      result<multi_value<int, float>, Error1, Error2>{multi_value{1, 2.0f}};
    CHECK(nonConstLValueSuccessToError.and_then([](int& x, float& y) {
      CHECK(x == 1);
      CHECK(y == 2.0f);
      return result<float, Error3>{Error3{}};
    }) == result<float, Error1, Error2, Error3>{Error3{}});

    auto nonConstLValueError = result<multi_value<int, float>, Error1, Error2>{Error1{}};
    CHECK(nonConstLValueError.and_then([](int&, float&) {
      FAIL();
      return result<float, Error3>{3.0f};
    }) == result<float, Error1, Error2, Error3>{Error1{}});

    CHECK(
      result<multi_value<MoveOnly, float>, Error1, Error2>{multi_value{MoveOnly{}, 2.0f}}
        .and_then([](MoveOnly&& x, float&& y) {
          CHECK(y == 2.0f);
          return result<MoveOnly, Error3>{std::move(x)};
        })
      == result<MoveOnly, Error1, Error2, Error3>{MoveOnly{}});

    CHECK(
      result<multi_value<int, float>, Error1, Error2>{multi_value{1, 2.0f}}.and_then(
        [](int&& x, float&& y) {
          CHECK(x == 1);
          CHECK(y == 2.0f);
          return result<float, Error3>{3.0f};
        })
      == result<float, Error1, Error2, Error3>{3.0f});

    CHECK(
      result<multi_value<int, float>, Error1, Error2>{multi_value{1, 2.0f}}.and_then(
        [](int&& x, float&& y) {
          CHECK(x == 1);
          CHECK(y == 2.0f);
          return result<float, Error3>{Error3{}};
        })
      == result<float, Error1, Error2, Error3>{Error3{}});

    CHECK(
      result<multi_value<int, float>, Error1, Error2>{Error1{}}.and_then(
        [](int&&, float&&) {
          FAIL();
          return result<float, Error3>{3.0f};
        })
      == result<float, Error1, Error2, Error3>{Error1{}});
  }

  SECTION("void result with errors")
  {
    const auto constLValueSuccessToSuccess = result<void, Error1, Error2>{};
    CHECK(constLValueSuccessToSuccess.and_then([]() {
      return result<float, Error3>{2.0f};
    }) == result<float, Error1, Error2, Error3>{2.0f});

    const auto constLValueSuccessToError = result<void, Error1, Error2>{};
    CHECK(constLValueSuccessToError.and_then([]() {
      return result<float, Error3>{Error3{}};
    }) == result<float, Error1, Error2, Error3>{Error3{}});

    const auto constLValueError = result<void, Error1, Error2>{Error1{}};
    CHECK(constLValueError.and_then([]() {
      FAIL();
      return result<float, Error3>{2.0f};
    }) == result<float, Error1, Error2, Error3>{Error1{}});

    auto nonConstLValueSuccessToSuccess = result<void, Error1, Error2>{};
    CHECK(nonConstLValueSuccessToSuccess.and_then([]() {
      return result<float, Error3>{2.0f};
    }) == result<float, Error1, Error2, Error3>{2.0f});

    auto nonConstLValueSuccessToError = result<void, Error1, Error2>{};
    CHECK(nonConstLValueSuccessToError.and_then([]() {
      return result<float, Error3>{Error3{}};
    }) == result<float, Error1, Error2, Error3>{Error3{}});

    auto nonConstLValueError = result<void, Error1, Error2>{Error1{}};
    CHECK(nonConstLValueError.and_then([]() {
      FAIL();
      return result<float, Error3>{2.0f};
    }) == result<float, Error1, Error2, Error3>{Error1{}});

    CHECK(result<void, Error1, Error2>{}.and_then([]() {
      return result<float, Error3>{2.0f};
    }) == result<float, Error1, Error2, Error3>{2.0f});

    CHECK(result<void, Error1, Error2>{}.and_then([]() {
      return result<float, Error3>{Error3{}};
    }) == result<float, Error1, Error2, Error3>{Error3{}});

    CHECK(result<void, Error1, Error2>{Error1{}}.and_then([]() {
      FAIL();
      return result<float, Error3>{2.0f};
    }) == result<float, Error1, Error2, Error3>{Error1{}});
  }

  SECTION("void result without errors")
  {
    const auto constLValueSuccessToSuccess = result<void>{};
    CHECK(constLValueSuccessToSuccess.and_then([]() {
      return result<float, Error3>{2.0f};
    }) == result<float, Error3>{2.0f});

    const auto constLValueSuccessToError = result<void>{};
    CHECK(constLValueSuccessToError.and_then([]() {
      return result<float, Error3>{Error3{}};
    }) == result<float, Error3>{Error3{}});

    auto nonConstLValueSuccessToSuccess = result<void>{};
    CHECK(nonConstLValueSuccessToSuccess.and_then([]() {
      return result<float, Error3>{2.0f};
    }) == result<float, Error3>{2.0f});

    auto nonConstLValueSuccessToError = result<void>{};
    CHECK(nonConstLValueSuccessToError.and_then([]() {
      return result<float, Error3>{Error3{}};
    }) == result<float, Error3>{Error3{}});

    CHECK(result<void>{}.and_then([]() {
      return result<float, Error3>{2.0f};
    }) == result<float, Error3>{2.0f});

    CHECK(result<void>{}.and_then([]() {
      return result<float, Error3>{Error3{}};
    }) == result<float, Error3>{Error3{}});
  }
}

TEST_CASE("result_test.or_else")
{
  SECTION("non-void result")
  {
    const auto constLValueSuccess = result<int, Error1, Error2>{1};
    CHECK(constLValueSuccess.or_else([](const auto&) {
      FAIL();
      return result<int, Error3>{2};
    }) == result<int, Error3>{1});

    const auto constLValueErrorToSuccess = result<int, Error1, Error2>{Error1{}};
    CHECK(
      constLValueErrorToSuccess.or_else(overload(
        [](const Error1&) { return result<int, Error3>{2}; },
        [](const Error2&) {
          FAIL();
          return result<int, Error3>{3};
        }))
      == result<int, Error3>{2});

    const auto constLValueErrorToError = result<int, Error1, Error2>{Error1{}};
    CHECK(
      constLValueErrorToError.or_else(overload(
        [](const Error1&) { return result<int, Error3>{Error3{}}; },
        [](const Error2&) {
          FAIL();
          return result<int, Error3>{2};
        }))
      == result<int, Error3>{Error3{}});

    auto nonConstLValueSuccess = result<int, Error1, Error2>{1};
    CHECK(nonConstLValueSuccess.or_else([](auto&) {
      FAIL();
      return result<int, Error3>{2};
    }) == result<int, Error3>{1});

    auto nonConstLValueErrorToSuccess = result<int, Error1, Error2>{Error1{}};
    CHECK(
      nonConstLValueErrorToSuccess.or_else(overload(
        [](Error1&) { return result<int, Error3>{2}; },
        [](Error2&) {
          FAIL();
          return result<int, Error3>{3};
        }))
      == result<int, Error3>{2});

    auto nonConstLValueErrorToError = result<int, Error1, Error2>{Error1{}};
    CHECK(
      nonConstLValueErrorToError.or_else(overload(
        [](Error1&) { return result<int, Error3>{Error3{}}; },
        [](Error2&) {
          FAIL();
          return result<int, Error3>{2};
        }))
      == result<int, Error3>{Error3{}});

    CHECK(result<MoveOnly, Error1, Error2>{MoveOnly{}}.or_else([](auto&&) {
      FAIL();
      return result<MoveOnly, Error3>{Error3{}};
    }) == result<MoveOnly, Error3>{MoveOnly{}});

    CHECK(
      result<int, Error1, Error2>{Error1{}}.or_else(overload(
        [](Error1&&) { return result<int, Error3>{2}; },
        [](Error2&&) {
          FAIL();
          return result<int, Error3>{3};
        }))
      == result<int, Error3>{2});

    CHECK(
      result<int, Error1, Error2>{Error1{}}.or_else(overload(
        [](Error1&&) { return result<int, Error3>{Error3{}}; },
        [](Error2&&) {
          FAIL();
          return result<int, Error3>{2};
        }))
      == result<int, Error3>{Error3{}});
  }

  SECTION("multi-valued result")
  {
    const auto constLValueSuccess =
      result<multi_value<int, float>, Error1, Error2>{multi_value{1, 2.0f}};
    CHECK(constLValueSuccess.or_else([](const auto&) {
      FAIL();
      return result<multi_value<int, float>, Error3>{multi_value{2, 3.0f}};
    }) == result<multi_value<int, float>, Error3>{multi_value{1, 2.0f}});

    const auto constLValueErrorToSuccess =
      result<multi_value<int, float>, Error1, Error2>{Error1{}};
    CHECK(
      constLValueErrorToSuccess.or_else(overload(
        [](const Error1&) {
          return result<multi_value<int, float>, Error3>{multi_value{2, 3.0f}};
        },
        [](const Error2&) {
          FAIL();
          return result<multi_value<int, float>, Error3>{multi_value{3, 4.0f}};
        }))
      == result<multi_value<int, float>, Error3>{multi_value{2, 3.0f}});

    const auto constLValueErrorToError =
      result<multi_value<int, float>, Error1, Error2>{Error1{}};
    CHECK(
      constLValueErrorToError.or_else(overload(
        [](const Error1&) { return result<multi_value<int, float>, Error3>{Error3{}}; },
        [](const Error2&) {
          FAIL();
          return result<multi_value<int, float>, Error3>{multi_value{2, 3.0f}};
        }))
      == result<multi_value<int, float>, Error3>{Error3{}});

    auto nonConstLValueSuccess =
      result<multi_value<int, float>, Error1, Error2>{multi_value{1, 2.0f}};
    CHECK(nonConstLValueSuccess.or_else([](auto&) {
      FAIL();
      return result<multi_value<int, float>, Error3>{multi_value{2, 3.0f}};
    }) == result<multi_value<int, float>, Error3>{multi_value{1, 2.0f}});

    auto nonConstLValueErrorToSuccess =
      result<multi_value<int, float>, Error1, Error2>{Error1{}};
    CHECK(
      nonConstLValueErrorToSuccess.or_else(overload(
        [](Error1&) {
          return result<multi_value<int, float>, Error3>{multi_value{2, 3.0f}};
        },
        [](Error2&) {
          FAIL();
          return result<multi_value<int, float>, Error3>{multi_value{3, 4.0f}};
        }))
      == result<multi_value<int, float>, Error3>{multi_value{2, 3.0f}});

    auto nonConstLValueErrorToError =
      result<multi_value<int, float>, Error1, Error2>{Error1{}};
    CHECK(
      nonConstLValueErrorToError.or_else(overload(
        [](Error1&) { return result<multi_value<int, float>, Error3>{Error3{}}; },
        [](Error2&) {
          FAIL();
          return result<multi_value<int, float>, Error3>{multi_value{2, 3.0f}};
        }))
      == result<multi_value<int, float>, Error3>{Error3{}});

    CHECK(result<MoveOnly, Error1, Error2>{MoveOnly{}}.or_else([](auto&&) {
      FAIL();
      return result<MoveOnly, Error3>{Error3{}};
    }) == result<MoveOnly, Error3>{MoveOnly{}});

    CHECK(
      result<multi_value<int, float>, Error1, Error2>{Error1{}}.or_else(overload(
        [](Error1&&) {
          return result<multi_value<int, float>, Error3>{multi_value{2, 3.0f}};
        },
        [](Error2&&) {
          FAIL();
          return result<multi_value<int, float>, Error3>{multi_value{3, 4.0f}};
        }))
      == result<multi_value<int, float>, Error3>{multi_value{2, 3.0f}});

    CHECK(
      result<multi_value<int, float>, Error1, Error2>{Error1{}}.or_else(overload(
        [](Error1&&) { return result<multi_value<int, float>, Error3>{Error3{}}; },
        [](Error2&&) {
          FAIL();
          return result<multi_value<int, float>, Error3>{multi_value{2, 3.0f}};
        }))
      == result<multi_value<int, float>, Error3>{Error3{}});
  }

  SECTION("void result")
  {
    const auto constLValueSuccess = result<void, Error1, Error2>{};
    CHECK(constLValueSuccess.or_else([](const auto&) {
      FAIL();
      return result<void, Error3>{};
    }) == result<void, Error3>{});

    const auto constLValueErrorToSuccess = result<void, Error1, Error2>{Error1{}};
    CHECK(
      constLValueErrorToSuccess.or_else(overload(
        [](const Error1&) { return result<void, Error3>{}; },
        [](const Error2&) {
          FAIL();
          return result<void, Error3>{Error3{}};
        }))
      == result<void, Error3>{});

    const auto constLValueErrorToError = result<void, Error1, Error2>{Error1{}};
    CHECK(
      constLValueErrorToError.or_else(overload(
        [](const Error1&) { return result<void, Error3>{Error3{}}; },
        [](const Error2&) {
          FAIL();
          return result<void, Error3>{};
        }))
      == result<void, Error3>{Error3{}});

    auto nonConstLValueSuccess = result<void, Error1, Error2>{};
    CHECK(nonConstLValueSuccess.or_else([](auto&) {
      FAIL();
      return result<void, Error3>{};
    }) == result<void, Error3>{});

    auto nonConstLValueErrorToSuccess = result<void, Error1, Error2>{Error1{}};
    CHECK(
      nonConstLValueErrorToSuccess.or_else(overload(
        [](Error1&) { return result<void, Error3>{}; },
        [](Error2&) {
          FAIL();
          return result<void, Error3>{Error3{}};
        }))
      == result<void, Error3>{});

    auto nonConstLValueErrorToError = result<void, Error1, Error2>{Error1{}};
    CHECK(
      nonConstLValueErrorToError.or_else(overload(
        [](Error1&) { return result<void, Error3>{Error3{}}; },
        [](Error2&) {
          FAIL();
          return result<void, Error3>{};
        }))
      == result<void, Error3>{Error3{}});

    CHECK(result<void, Error1, Error2>{}.or_else([](auto&&) {
      FAIL();
      return result<void, Error3>{};
    }) == result<void, Error3>{});

    CHECK(
      result<void, Error1, Error2>{Error1{}}.or_else(overload(
        [](Error1&&) { return result<void, Error3>{}; },
        [](Error2&&) {
          FAIL();
          return result<void, Error3>{Error3{}};
        }))
      == result<void, Error3>{});

    CHECK(
      result<void, Error1, Error2>{Error1{}}.or_else(overload(
        [](Error1&&) { return result<void, Error3>{Error3{}}; },
        [](Error2&&) {
          FAIL();
          return result<void, Error3>{};
        }))
      == result<void, Error3>{Error3{}});
  }
}

TEST_CASE("result_test.transform")
{
  SECTION("non-void result")
  {
    SECTION("transform to value")
    {
      const auto constLValueSuccessToSuccess = result<int, Error1, Error2>{1};
      CHECK(constLValueSuccessToSuccess.transform([](const int& x) {
        CHECK(x == 1);
        return 2.0f;
      }) == result<float, Error1, Error2>{2.0f});

      const auto constLValueError = result<int, Error1, Error2>{Error1{}};
      CHECK(constLValueError.transform([](const int&) {
        FAIL();
        return 2.0f;
      }) == result<float, Error1, Error2>{Error1{}});

      auto nonConstLValueSuccessToSuccess = result<int, Error1, Error2>{1};
      CHECK(nonConstLValueSuccessToSuccess.transform([](int& x) {
        CHECK(x == 1);
        return 2.0f;
      }) == result<float, Error1, Error2>{2.0f});

      auto nonConstLValueError = result<int, Error1, Error2>{Error1{}};
      CHECK(nonConstLValueError.transform([](int&) {
        FAIL();
        return 2.0f;
      }) == result<float, Error1, Error2>{Error1{}});

      CHECK(result<int, Error1, Error2>{1}.transform([](int&& x) {
        CHECK(x == 1);
        return 2.0f;
      }) == result<float, Error1, Error2>{2.0f});

      CHECK(result<int, Error1, Error2>{Error1{}}.transform([](int&&) {
        FAIL();
        return 2.0f;
      }) == result<float, Error1, Error2>{Error1{}});
    }

    SECTION("transform to void")
    {
      const auto constLValueSuccessToSuccess = result<int, Error1, Error2>{1};
      CHECK(constLValueSuccessToSuccess.transform([](const int& x) {
        CHECK(x == 1);
      }) == result<void, Error1, Error2>{});

      const auto constLValueError = result<int, Error1, Error2>{Error1{}};
      CHECK(constLValueError.transform([](const int&) {
        FAIL();
      }) == result<void, Error1, Error2>{Error1{}});

      auto nonConstLValueSuccessToSuccess = result<int, Error1, Error2>{1};
      CHECK(nonConstLValueSuccessToSuccess.transform([](int& x) {
        CHECK(x == 1);
      }) == result<void, Error1, Error2>{});

      auto nonConstLValueError = result<int, Error1, Error2>{Error1{}};
      CHECK(nonConstLValueError.transform([](int&) {
        FAIL();
      }) == result<void, Error1, Error2>{Error1{}});

      CHECK(result<int, Error1, Error2>{1}.transform([](int&& x) {
        CHECK(x == 1);
      }) == result<void, Error1, Error2>{});

      CHECK(result<int, Error1, Error2>{Error1{}}.transform([](int&&) {
        FAIL();
      }) == result<void, Error1, Error2>{Error1{}});
    }
  }

  SECTION("multi-valued result")
  {
    SECTION("transform to value")
    {
      const auto constLValueSuccessToSuccess =
        result<multi_value<int, float>, Error1, Error2>{multi_value{1, 2.0f}};
      CHECK(constLValueSuccessToSuccess.transform([](const int& x, const float& y) {
        CHECK(x == 1);
        CHECK(y == 2.0f);
        return 3.0f;
      }) == result<float, Error1, Error2>{3.0f});

      const auto constLValueError =
        result<multi_value<int, float>, Error1, Error2>{Error1{}};
      CHECK(constLValueError.transform([](const int&, const float&) {
        FAIL();
        return 2.0f;
      }) == result<float, Error1, Error2>{Error1{}});

      auto nonConstLValueSuccessToSuccess =
        result<multi_value<int, float>, Error1, Error2>{multi_value{1, 2.0f}};
      CHECK(nonConstLValueSuccessToSuccess.transform([](int& x, float& y) {
        CHECK(x == 1);
        CHECK(y == 2.0f);
        return 3.0f;
      }) == result<float, Error1, Error2>{3.0f});

      auto nonConstLValueError =
        result<multi_value<int, float>, Error1, Error2>{Error1{}};
      CHECK(nonConstLValueError.transform([](int&, float&) {
        FAIL();
        return 2.0f;
      }) == result<float, Error1, Error2>{Error1{}});

      CHECK(
        result<multi_value<int, float>, Error1, Error2>{multi_value{1, 2.0f}}.transform(
          [](int&& x, float&& y) {
            CHECK(x == 1);
            CHECK(y == 2.0f);
            return 3.0f;
          })
        == result<float, Error1, Error2>{3.0f});

      CHECK(
        result<multi_value<int, float>, Error1, Error2>{Error1{}}.transform(
          [](int&&, float&&) {
            FAIL();
            return 2.0f;
          })
        == result<float, Error1, Error2>{Error1{}});
    }

    SECTION("transform to void")
    {
      const auto constLValueSuccessToSuccess =
        result<multi_value<int, float>, Error1, Error2>{multi_value{1, 2.0f}};
      CHECK(constLValueSuccessToSuccess.transform([](const int& x, const float& y) {
        CHECK(x == 1);
        CHECK(y == 2.0f);
      }) == result<void, Error1, Error2>{});

      const auto constLValueError =
        result<multi_value<int, float>, Error1, Error2>{Error1{}};
      CHECK(constLValueError.transform([](const int&, const float&) {
        FAIL();
      }) == result<void, Error1, Error2>{Error1{}});

      auto nonConstLValueSuccessToSuccess =
        result<multi_value<int, float>, Error1, Error2>{multi_value{1, 2.0f}};
      CHECK(nonConstLValueSuccessToSuccess.transform([](int& x, float& y) {
        CHECK(x == 1);
        CHECK(y == 2.0f);
      }) == result<void, Error1, Error2>{});

      auto nonConstLValueError =
        result<multi_value<int, float>, Error1, Error2>{Error1{}};
      CHECK(nonConstLValueError.transform([](int&, float&) {
        FAIL();
      }) == result<void, Error1, Error2>{Error1{}});

      CHECK(
        result<multi_value<int, float>, Error1, Error2>{multi_value{1, 2.0f}}.transform(
          [](int&& x, float&& y) {
            CHECK(x == 1);
            CHECK(y == 2.0f);
          })
        == result<void, Error1, Error2>{});

      CHECK(
        result<multi_value<int, float>, Error1, Error2>{Error1{}}.transform(
          [](int&&, float&&) { FAIL(); })
        == result<void, Error1, Error2>{Error1{}});
    }
  }

  SECTION("void result with errors")
  {
    SECTION("transform to value")
    {
      const auto constLValueSuccessToSuccess = result<void, Error1, Error2>{};
      CHECK(constLValueSuccessToSuccess.transform([]() {
        return 2.0f;
      }) == result<float, Error1, Error2>{2.0f});

      const auto constLValueError = result<void, Error1, Error2>{Error1{}};
      CHECK(constLValueError.transform([]() {
        FAIL();
        return 2.0f;
      }) == result<float, Error1, Error2>{Error1{}});

      auto nonConstLValueSuccessToSuccess = result<void, Error1, Error2>{};
      CHECK(nonConstLValueSuccessToSuccess.transform([]() {
        return 2.0f;
      }) == result<float, Error1, Error2>{2.0f});

      auto nonConstLValueError = result<void, Error1, Error2>{Error1{}};
      CHECK(nonConstLValueError.transform([]() {
        FAIL();
        return 2.0f;
      }) == result<float, Error1, Error2>{Error1{}});

      CHECK(result<void, Error1, Error2>{}.transform([]() {
        return 2.0f;
      }) == result<float, Error1, Error2>{2.0f});

      CHECK(result<void, Error1, Error2>{Error1{}}.transform([]() {
        FAIL();
        return 2.0f;
      }) == result<float, Error1, Error2>{Error1{}});
    }

    SECTION("transform to void")
    {
      const auto constLValueSuccessToSuccess = result<void, Error1, Error2>{};
      CHECK(
        constLValueSuccessToSuccess.transform([]() {}) == result<void, Error1, Error2>{});

      const auto constLValueError = result<void, Error1, Error2>{Error1{}};
      CHECK(constLValueError.transform([]() {
        FAIL();
      }) == result<void, Error1, Error2>{Error1{}});

      auto nonConstLValueSuccessToSuccess = result<void, Error1, Error2>{};
      CHECK(nonConstLValueSuccessToSuccess.transform([]() {
      }) == result<void, Error1, Error2>{});

      auto nonConstLValueError = result<void, Error1, Error2>{Error1{}};
      CHECK(nonConstLValueError.transform([]() {
        FAIL();
      }) == result<void, Error1, Error2>{Error1{}});

      CHECK(result<void, Error1, Error2>{}.transform([]() {
      }) == result<void, Error1, Error2>{});

      CHECK(result<void, Error1, Error2>{Error1{}}.transform([]() {
        FAIL();
      }) == result<void, Error1, Error2>{Error1{}});
    }
  }

  SECTION("void result without errors")
  {
    SECTION("transform to value")
    {
      const auto constLValueSuccessToSuccess = result<void>{};
      CHECK(constLValueSuccessToSuccess.transform([]() {
        return 2.0f;
      }) == result<float>{2.0f});

      auto nonConstLValueSuccessToSuccess = result<void>{};
      CHECK(nonConstLValueSuccessToSuccess.transform([]() {
        return 2.0f;
      }) == result<float>{2.0f});

      CHECK(result<void>{}.transform([]() { return 2.0f; }) == result<float>{2.0f});
    }

    SECTION("transform to void")
    {
      const auto constLValueSuccessToSuccess = result<void>{};
      CHECK(constLValueSuccessToSuccess.transform([]() {}) == result<void>{});

      auto nonConstLValueSuccessToSuccess = result<void>{};
      CHECK(nonConstLValueSuccessToSuccess.transform([]() {}) == result<void>{});

      CHECK(result<void>{}.transform([]() {}) == result<void>{});
    }
  }
}


TEST_CASE("result_test.transform_error")
{
  SECTION("result can be discarded")
  {
    result<void, Error1>{}.transform_error([](const auto&) {}); // must not warn
  }

  SECTION("non-void result")
  {
    const auto constLValueSuccess = result<int, Error1, Error2>{1};
    CHECK(constLValueSuccess.transform_error([](const auto&) {
      FAIL();
      return 2;
    }) == result<int>{1});

    const auto constLValueError = result<int, Error1, Error2>{Error1{}};
    CHECK(
      constLValueError.transform_error(
        overload([](const Error1&) { return 2; }, [](const Error2&) { return 3; }))
      == result<int>{2});

    auto nonConstLValueSuccess = result<int, Error1, Error2>{1};
    CHECK(nonConstLValueSuccess.transform_error([](auto&) {
      FAIL();
      return 2;
    }) == result<int>{1});

    auto nonConstLValueError = result<int, Error1, Error2>{Error1{}};
    CHECK(
      nonConstLValueError.transform_error(
        overload([](Error1&) { return 2; }, [](Error2&) { return 3; }))
      == result<int>{2});

    CHECK(result<MoveOnly, Error1, Error2>{MoveOnly{}}.transform_error([](auto&&) {
      FAIL();
      return MoveOnly{};
    }) == result<MoveOnly>{MoveOnly{}});

    CHECK(
      result<int, Error1, Error2>{Error1{}}.transform_error(
        overload([](Error1&&) { return 2; }, [](Error2&&) { return 3; }))
      == result<int>{2});
  }

  SECTION("multi-valued result")
  {
    const auto constLValueSuccess =
      result<multi_value<int, float>, Error1, Error2>{multi_value{1, 2.0f}};
    CHECK(constLValueSuccess.transform_error([](const auto&) {
      FAIL();
      return multi_value{2, 3.0f};
    }) == result<multi_value<int, float>>{multi_value{1, 2.0f}});

    const auto constLValueError =
      result<multi_value<int, float>, Error1, Error2>{Error1{}};
    CHECK(
      constLValueError.transform_error(overload(
        [](const Error1&) {
          return multi_value{2, 3.0f};
        },
        [](const Error2&) {
          return multi_value{3, 4.0f};
        }))
      == result<multi_value<int, float>>{multi_value{2, 3.0f}});

    auto nonConstLValueSuccess =
      result<multi_value<int, float>, Error1, Error2>{multi_value{1, 2.0f}};
    CHECK(nonConstLValueSuccess.transform_error([](auto&) {
      FAIL();
      return multi_value{2, 3.0f};
    }) == result<multi_value<int, float>>{multi_value{1, 2.0f}});

    auto nonConstLValueError = result<multi_value<int, float>, Error1, Error2>{Error1{}};
    CHECK(
      nonConstLValueError.transform_error(overload(
        [](Error1&) {
          return multi_value{2, 3.0f};
        },
        [](Error2&) {
          return multi_value{3, 4.0f};
        }))
      == result<multi_value<int, float>>{multi_value{2, 3.0f}});

    CHECK(result<MoveOnly, Error1, Error2>{MoveOnly{}}.transform_error([](auto&&) {
      FAIL();
      return MoveOnly{};
    }) == result<MoveOnly>{MoveOnly{}});

    CHECK(
      result<multi_value<int, float>, Error1, Error2>{Error1{}}.transform_error(overload(
        [](Error1&&) {
          return multi_value{2, 3.0f};
        },
        [](Error2&&) {
          return multi_value{3, 4.0f};
        }))
      == result<multi_value<int, float>>{multi_value{2, 3.0f}});
  }

  SECTION("void result")
  {
    const auto constLValueSuccess = result<void, Error1, Error2>{};
    CHECK(
      constLValueSuccess.transform_error([](const auto&) { FAIL(); }) == result<void>{});

    const auto constLValueError = result<void, Error1, Error2>{Error1{}};
    CHECK(
      constLValueError.transform_error(
        overload([](const Error1&) { SUCCEED(); }, [](const Error2&) { FAIL(); }))
      == result<void>{});

    auto nonConstLValueSuccess = result<void, Error1, Error2>{};
    CHECK(nonConstLValueSuccess.transform_error([](auto&) { FAIL(); }) == result<void>{});

    auto nonConstLValueError = result<void, Error1, Error2>{Error1{}};
    CHECK(
      nonConstLValueError.transform_error(
        overload([](Error1&) { SUCCEED(); }, [](Error2&) { FAIL(); }))
      == result<void>{});

    CHECK(result<void, Error1, Error2>{}.transform_error([](auto&&) {
      FAIL();
    }) == result<void>{});

    CHECK(
      result<void, Error1, Error2>{Error1{}}.transform_error(
        overload([](Error1&&) { SUCCEED(); }, [](Error2&&) { FAIL(); }))
      == result<void>{});
  }
}

TEST_CASE("result_test.if_error")
{
  auto called = false;
  SECTION("non-void result")
  {
    called = false;
    const auto constLValueSuccess = result<int, Error1, Error2>{1};
    CHECK(constLValueSuccess.if_error([&](const auto&) {
      called = true;
    }) == result<int, Error1, Error2>{1});
    CHECK_FALSE(called);

    called = false;
    const auto constLValueError = result<int, Error1, Error2>{Error1{}};
    CHECK(constLValueError.if_error([&](const auto&) {
      called = true;
    }) == result<int, Error1, Error2>{Error1{}});
    CHECK(called);
  }

  SECTION("multi-valued result")
  {
    called = false;
    const auto constLValueSuccess =
      result<multi_value<int, float>, Error1, Error2>{multi_value{1, 2.0f}};
    CHECK(constLValueSuccess.if_error([&](const auto&) {
      called = true;
    }) == result<multi_value<int, float>, Error1, Error2>{multi_value{1, 2.0f}});
    CHECK_FALSE(called);

    called = false;
    const auto constLValueError =
      result<multi_value<int, float>, Error1, Error2>{Error1{}};
    CHECK(constLValueError.if_error([&](const auto&) {
      called = true;
    }) == result<multi_value<int, float>, Error1, Error2>{Error1{}});
    CHECK(called);
  }

  SECTION("void result")
  {
    called = false;
    const auto constLValueSuccess = result<void, Error1, Error2>{};
    CHECK(constLValueSuccess.if_error([&](const auto&) {
      called = true;
    }) == kdl::result<void, Error1, Error2>{});
    CHECK_FALSE(called);

    called = false;
    const auto constLValueError = result<void, Error1, Error2>{Error1{}};
    CHECK(constLValueError.if_error([&](const auto&) {
      called = true;
    }) == kdl::result<void, Error1, Error2>{Error1{}});
    CHECK(called);
  }
}

TEST_CASE("result.fold_results")
{
  SECTION("with empty range")
  {
    CHECK(
      fold_results(std::vector<result<int>>{})
      == result<std::vector<int>>{std::vector<int>{}});
  }

  SECTION("success case")
  {
    CHECK(
      fold_results(std::vector<result<int>>{{1}, {2}, {3}})
      == result<std::vector<int>>{std::vector<int>{1, 2, 3}});
  }

  SECTION("error case")
  {
    CHECK(
      fold_results(std::vector<result<int, std::string>>{{1}, {"error"}, {3}})
      == result<std::vector<int>, std::string>{"error"});
  }
}

TEST_CASE("void_result.fold_results")
{
  SECTION("with empty range")
  {
    CHECK(fold_results(std::vector<result<void>>{}) == void_success);
  }

  SECTION("success case")
  {
    CHECK(
      fold_results(std::vector{void_success, void_success, void_success})
      == void_success);
  }

  SECTION("error case")
  {
    CHECK(
      fold_results(
        std::vector<result<void, std::string>>{void_success, "error", void_success})
      == result<void, std::string>{"error"});
  }
}
} // namespace kdl
