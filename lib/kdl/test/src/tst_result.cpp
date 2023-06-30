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
#include "kdl/result_combine.h"
#include "kdl/result_fold.h"
#include "kdl/result_io.h"

#include <iostream>
#include <string>

#include <catch2/catch.hpp>

/*
missing tests:
if_error
value all value categories?
value_or all value categories?
error()
 */

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

  SECTION("void result without errors") { CHECK((result<void>{}.is_success())); }
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
    CHECK(constLValueSuccess.visit(kdl::overload(
      [](const int& x) { return x == 1; }, [](const auto&) { return false; })));

    const auto constLValueError1 = result<int, Error1, Error2>{Error1{}};
    CHECK(constLValueError1.visit(kdl::overload(
      [](const int&) { return false; },
      [](const Error1&) { return true; },
      [](const Error2&) { return false; })));

    const auto constLValueError2 = result<int, Error1, Error2>{Error2{}};
    CHECK(constLValueError2.visit(kdl::overload(
      [](const int&) { return false; },
      [](const Error1&) { return false; },
      [](const Error2&) { return true; })));

    auto nonConstLValueSuccess = result<int, Error1, Error2>{1};
    CHECK(nonConstLValueSuccess.visit(
      kdl::overload([](int& x) { return x == 1; }, [](auto&) { return false; })));

    auto nonConstLValueError1 = result<int, Error1, Error2>{Error1{}};
    CHECK(nonConstLValueError1.visit(kdl::overload(
      [](int&) { return false; },
      [](Error1&) { return true; },
      [](Error2&) { return false; })));

    auto nonConstLValueError2 = result<int, Error1, Error2>{Error2{}};
    CHECK(nonConstLValueError2.visit(kdl::overload(
      [](int&) { return false; },
      [](Error1&) { return false; },
      [](Error2&) { return true; })));

    CHECK(result<MoveOnly, Error1, Error2>{MoveOnly{}}.visit(
      kdl::overload([](MoveOnly&&) { return true; }, [](auto&&) { return false; })));

    CHECK(result<int, Error1, Error2>{1}.visit(
      kdl::overload([](int&& x) { return x == 1; }, [](auto&&) { return false; })));

    CHECK(result<int, Error1, Error2>{Error1{}}.visit(kdl::overload(
      [](int&&) { return false; },
      [](Error1&&) { return true; },
      [](Error2&&) { return false; })));

    CHECK(result<int, Error1, Error2>{Error2{}}.visit(kdl::overload(
      [](int&&) { return false; },
      [](Error1&&) { return false; },
      [](Error2&&) { return true; })));
  }

  SECTION("void result with errors")
  {
    const auto constLValueSuccess = result<void, Error1, Error2>{};
    CHECK(constLValueSuccess.visit(
      kdl::overload([]() { return true; }, [](const auto&) { return false; })));

    const auto constLValueError1 = result<void, Error1, Error2>{Error1{}};
    CHECK(constLValueError1.visit(kdl::overload(
      []() { return false; },
      [](const Error1&) { return true; },
      [](const Error2&) { return false; })));

    const auto constLValueError2 = result<void, Error1, Error2>{Error2{}};
    CHECK(constLValueError2.visit(kdl::overload(
      []() { return false; },
      [](const Error1&) { return false; },
      [](const Error2&) { return true; })));

    auto nonConstLValueSuccess = result<void, Error1, Error2>{};
    CHECK(nonConstLValueSuccess.visit(
      kdl::overload([]() { return true; }, [](auto&) { return false; })));

    auto nonConstLValueError1 = result<void, Error1, Error2>{Error1{}};
    CHECK(nonConstLValueError1.visit(kdl::overload(
      []() { return false; },
      [](Error1&) { return true; },
      [](Error2&) { return false; })));

    auto nonConstLValueError2 = result<void, Error1, Error2>{Error2{}};
    CHECK(nonConstLValueError2.visit(kdl::overload(
      []() { return false; },
      [](Error1&) { return false; },
      [](Error2&) { return true; })));

    CHECK(result<void, Error1, Error2>{}.visit(
      kdl::overload([]() { return true; }, [](auto&&) { return false; })));

    CHECK(result<void, Error1, Error2>{Error1{}}.visit(kdl::overload(
      []() { return false; },
      [](Error1&&) { return true; },
      [](Error2&&) { return false; })));

    CHECK(result<void, Error1, Error2>{Error2{}}.visit(kdl::overload(
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

TEST_CASE("result_test.and_then")
{
  SECTION("non-void result")
  {
    const auto constLValueSuccessToSuccess = result<int, Error1, Error2>{1};
    CHECK(constLValueSuccessToSuccess.and_then([](const int& x) {
      CHECK(x == 1);
      return kdl::result<float, Error3>{2.0f};
    }) == kdl::result<float, Error1, Error2, Error3>{2.0f});

    const auto constLValueSuccessToError = result<int, Error1, Error2>{1};
    CHECK(constLValueSuccessToError.and_then([](const int& x) {
      CHECK(x == 1);
      return kdl::result<float, Error3>{Error3{}};
    }) == kdl::result<float, Error1, Error2, Error3>{Error3{}});

    const auto constLValueError = result<int, Error1, Error2>{Error1{}};
    CHECK(constLValueError.and_then([](const int&) {
      FAIL();
      return kdl::result<float, Error3>{2.0f};
    }) == kdl::result<float, Error1, Error2, Error3>{Error1{}});

    auto nonConstLValueSuccessToSuccess = result<int, Error1, Error2>{1};
    CHECK(nonConstLValueSuccessToSuccess.and_then([](int& x) {
      CHECK(x == 1);
      return kdl::result<float, Error3>{2.0f};
    }) == kdl::result<float, Error1, Error2, Error3>{2.0f});

    auto nonConstLValueSuccessToError = result<int, Error1, Error2>{1};
    CHECK(nonConstLValueSuccessToError.and_then([](int& x) {
      CHECK(x == 1);
      return kdl::result<float, Error3>{Error3{}};
    }) == kdl::result<float, Error1, Error2, Error3>{Error3{}});

    auto nonConstLValueError = result<int, Error1, Error2>{Error1{}};
    CHECK(nonConstLValueError.and_then([](int&) {
      FAIL();
      return kdl::result<float, Error3>{2.0f};
    }) == kdl::result<float, Error1, Error2, Error3>{Error1{}});

    CHECK(result<MoveOnly, Error1, Error2>{MoveOnly{}}.and_then([](MoveOnly&& x) {
      return kdl::result<MoveOnly, Error3>{std::move(x)};
    }) == kdl::result<MoveOnly, Error1, Error2, Error3>{MoveOnly{}});

    CHECK(result<int, Error1, Error2>{1}.and_then([](int&& x) {
      CHECK(x == 1);
      return kdl::result<float, Error3>{2.0f};
    }) == kdl::result<float, Error1, Error2, Error3>{2.0f});

    CHECK(result<int, Error1, Error2>{1}.and_then([](int&& x) {
      CHECK(x == 1);
      return kdl::result<float, Error3>{Error3{}};
    }) == kdl::result<float, Error1, Error2, Error3>{Error3{}});

    CHECK(result<int, Error1, Error2>{Error1{}}.and_then([](int&&) {
      FAIL();
      return kdl::result<float, Error3>{2.0f};
    }) == kdl::result<float, Error1, Error2, Error3>{Error1{}});
  }

  SECTION("void result with errors")
  {
    const auto constLValueSuccessToSuccess = result<void, Error1, Error2>{};
    CHECK(constLValueSuccessToSuccess.and_then([]() {
      return kdl::result<float, Error3>{2.0f};
    }) == kdl::result<float, Error1, Error2, Error3>{2.0f});

    const auto constLValueSuccessToError = result<void, Error1, Error2>{};
    CHECK(constLValueSuccessToError.and_then([]() {
      return kdl::result<float, Error3>{Error3{}};
    }) == kdl::result<float, Error1, Error2, Error3>{Error3{}});

    const auto constLValueError = result<void, Error1, Error2>{Error1{}};
    CHECK(constLValueError.and_then([]() {
      FAIL();
      return kdl::result<float, Error3>{2.0f};
    }) == kdl::result<float, Error1, Error2, Error3>{Error1{}});

    auto nonConstLValueSuccessToSuccess = result<void, Error1, Error2>{};
    CHECK(nonConstLValueSuccessToSuccess.and_then([]() {
      return kdl::result<float, Error3>{2.0f};
    }) == kdl::result<float, Error1, Error2, Error3>{2.0f});

    auto nonConstLValueSuccessToError = result<void, Error1, Error2>{};
    CHECK(nonConstLValueSuccessToError.and_then([]() {
      return kdl::result<float, Error3>{Error3{}};
    }) == kdl::result<float, Error1, Error2, Error3>{Error3{}});

    auto nonConstLValueError = result<void, Error1, Error2>{Error1{}};
    CHECK(nonConstLValueError.and_then([]() {
      FAIL();
      return kdl::result<float, Error3>{2.0f};
    }) == kdl::result<float, Error1, Error2, Error3>{Error1{}});

    CHECK(result<void, Error1, Error2>{}.and_then([]() {
      return kdl::result<float, Error3>{2.0f};
    }) == kdl::result<float, Error1, Error2, Error3>{2.0f});

    CHECK(result<void, Error1, Error2>{}.and_then([]() {
      return kdl::result<float, Error3>{Error3{}};
    }) == kdl::result<float, Error1, Error2, Error3>{Error3{}});

    CHECK(result<void, Error1, Error2>{Error1{}}.and_then([]() {
      FAIL();
      return kdl::result<float, Error3>{2.0f};
    }) == kdl::result<float, Error1, Error2, Error3>{Error1{}});
  }

  SECTION("void result without errors")
  {
    const auto constLValueSuccessToSuccess = result<void>{};
    CHECK(constLValueSuccessToSuccess.and_then([]() {
      return kdl::result<float, Error3>{2.0f};
    }) == kdl::result<float, Error3>{2.0f});

    const auto constLValueSuccessToError = result<void>{};
    CHECK(constLValueSuccessToError.and_then([]() {
      return kdl::result<float, Error3>{Error3{}};
    }) == kdl::result<float, Error3>{Error3{}});

    auto nonConstLValueSuccessToSuccess = result<void>{};
    CHECK(nonConstLValueSuccessToSuccess.and_then([]() {
      return kdl::result<float, Error3>{2.0f};
    }) == kdl::result<float, Error3>{2.0f});

    auto nonConstLValueSuccessToError = result<void>{};
    CHECK(nonConstLValueSuccessToError.and_then([]() {
      return kdl::result<float, Error3>{Error3{}};
    }) == kdl::result<float, Error3>{Error3{}});

    CHECK(result<void>{}.and_then([]() {
      return kdl::result<float, Error3>{2.0f};
    }) == kdl::result<float, Error3>{2.0f});

    CHECK(result<void>{}.and_then([]() {
      return kdl::result<float, Error3>{Error3{}};
    }) == kdl::result<float, Error3>{Error3{}});
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
      }) == kdl::result<float, Error1, Error2>{2.0f});

      const auto constLValueError = result<int, Error1, Error2>{Error1{}};
      CHECK(constLValueError.transform([](const int&) {
        FAIL();
        return 2.0f;
      }) == kdl::result<float, Error1, Error2>{Error1{}});

      auto nonConstLValueSuccessToSuccess = result<int, Error1, Error2>{1};
      CHECK(nonConstLValueSuccessToSuccess.transform([](int& x) {
        CHECK(x == 1);
        return 2.0f;
      }) == kdl::result<float, Error1, Error2>{2.0f});

      auto nonConstLValueError = result<int, Error1, Error2>{Error1{}};
      CHECK(nonConstLValueError.transform([](int&) {
        FAIL();
        return 2.0f;
      }) == kdl::result<float, Error1, Error2>{Error1{}});

      CHECK(result<int, Error1, Error2>{1}.transform([](int&& x) {
        CHECK(x == 1);
        return 2.0f;
      }) == kdl::result<float, Error1, Error2>{2.0f});

      CHECK(result<int, Error1, Error2>{Error1{}}.transform([](int&&) {
        FAIL();
        return 2.0f;
      }) == kdl::result<float, Error1, Error2>{Error1{}});
    }

    SECTION("transform to void")
    {
      const auto constLValueSuccessToSuccess = result<int, Error1, Error2>{1};
      CHECK(constLValueSuccessToSuccess.transform([](const int& x) {
        CHECK(x == 1);
      }) == kdl::result<void, Error1, Error2>{});

      const auto constLValueError = result<int, Error1, Error2>{Error1{}};
      CHECK(constLValueError.transform([](const int&) {
        FAIL();
      }) == kdl::result<void, Error1, Error2>{Error1{}});

      auto nonConstLValueSuccessToSuccess = result<int, Error1, Error2>{1};
      CHECK(nonConstLValueSuccessToSuccess.transform([](int& x) {
        CHECK(x == 1);
      }) == kdl::result<void, Error1, Error2>{});

      auto nonConstLValueError = result<int, Error1, Error2>{Error1{}};
      CHECK(nonConstLValueError.transform([](int&) {
        FAIL();
      }) == kdl::result<void, Error1, Error2>{Error1{}});

      CHECK(result<int, Error1, Error2>{1}.transform([](int&& x) {
        CHECK(x == 1);
      }) == kdl::result<void, Error1, Error2>{});

      CHECK(result<int, Error1, Error2>{Error1{}}.transform([](int&&) {
        FAIL();
      }) == kdl::result<void, Error1, Error2>{Error1{}});
    }
  }

  SECTION("void result with errors")
  {
    SECTION("transform to value")
    {
      const auto constLValueSuccessToSuccess = result<void, Error1, Error2>{};
      CHECK(constLValueSuccessToSuccess.transform([]() {
        return 2.0f;
      }) == kdl::result<float, Error1, Error2>{2.0f});

      const auto constLValueError = result<void, Error1, Error2>{Error1{}};
      CHECK(constLValueError.transform([]() {
        FAIL();
        return 2.0f;
      }) == kdl::result<float, Error1, Error2>{Error1{}});

      auto nonConstLValueSuccessToSuccess = result<void, Error1, Error2>{};
      CHECK(nonConstLValueSuccessToSuccess.transform([]() {
        return 2.0f;
      }) == kdl::result<float, Error1, Error2>{2.0f});

      auto nonConstLValueError = result<void, Error1, Error2>{Error1{}};
      CHECK(nonConstLValueError.transform([]() {
        FAIL();
        return 2.0f;
      }) == kdl::result<float, Error1, Error2>{Error1{}});

      CHECK(result<void, Error1, Error2>{}.transform([]() {
        return 2.0f;
      }) == kdl::result<float, Error1, Error2>{2.0f});

      CHECK(result<void, Error1, Error2>{Error1{}}.transform([]() {
        FAIL();
        return 2.0f;
      }) == kdl::result<float, Error1, Error2>{Error1{}});
    }

    SECTION("transform to void")
    {
      const auto constLValueSuccessToSuccess = result<void, Error1, Error2>{};
      CHECK(constLValueSuccessToSuccess.transform([]() {
      }) == kdl::result<void, Error1, Error2>{});

      const auto constLValueError = result<void, Error1, Error2>{Error1{}};
      CHECK(constLValueError.transform([]() {
        FAIL();
      }) == kdl::result<void, Error1, Error2>{Error1{}});

      auto nonConstLValueSuccessToSuccess = result<void, Error1, Error2>{};
      CHECK(nonConstLValueSuccessToSuccess.transform([]() {
      }) == kdl::result<void, Error1, Error2>{});

      auto nonConstLValueError = result<void, Error1, Error2>{Error1{}};
      CHECK(nonConstLValueError.transform([]() {
        FAIL();
      }) == kdl::result<void, Error1, Error2>{Error1{}});

      CHECK(result<void, Error1, Error2>{}.transform([]() {
      }) == kdl::result<void, Error1, Error2>{});

      CHECK(result<void, Error1, Error2>{Error1{}}.transform([]() {
        FAIL();
      }) == kdl::result<void, Error1, Error2>{Error1{}});
    }
  }

  SECTION("void result without errors")
  {
    SECTION("transform to value")
    {
      const auto constLValueSuccessToSuccess = result<void>{};
      CHECK(constLValueSuccessToSuccess.transform([]() {
        return 2.0f;
      }) == kdl::result<float>{2.0f});

      auto nonConstLValueSuccessToSuccess = result<void>{};
      CHECK(nonConstLValueSuccessToSuccess.transform([]() {
        return 2.0f;
      }) == kdl::result<float>{2.0f});

      CHECK(result<void>{}.transform([]() { return 2.0f; }) == kdl::result<float>{2.0f});
    }

    SECTION("transform to void")
    {
      const auto constLValueSuccessToSuccess = result<void>{};
      CHECK(constLValueSuccessToSuccess.transform([]() {}) == kdl::result<void>{});

      auto nonConstLValueSuccessToSuccess = result<void>{};
      CHECK(nonConstLValueSuccessToSuccess.transform([]() {}) == kdl::result<void>{});

      CHECK(result<void>{}.transform([]() {}) == kdl::result<void>{});
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

  SECTION("void result")
  {
    called = false;
    const auto constLValueSuccess = result<void, Error1, Error2>{};
    CHECK(constLValueSuccess.if_error([&](const auto&) {
      called = true;
    }) == result<void, Error1, Error2>{});
    CHECK_FALSE(called);

    called = false;
    const auto constLValueError = result<void, Error1, Error2>{Error1{}};
    CHECK(constLValueError.if_error([&](const auto&) {
      called = true;
    }) == result<void, Error1, Error2>{Error1{}});
    CHECK(called);
  }
}

TEST_CASE("combine_results")
{
  using R1 = result<int, Error1, Error2>;
  using R2 = result<double, Error2, Error3>;

  const auto r1 = R1{1};
  const auto r2 = R2{2.0};
  const auto r3 = R2{Error2{}};

  CHECK(
    combine_results(r1, r2)
    == result<std::tuple<int, double>, Error1, Error2, Error3>{std::tuple{1, 2.0}});

  CHECK(
    combine_results(r1, r3)
    == result<std::tuple<int, double>, Error1, Error2, Error3>{Error2{}});

  CHECK(
    combine_results(r1, R2{2.0})
    == result<std::tuple<int, double>, Error1, Error2, Error3>{std::tuple{1, 2.0}});

  CHECK(
    combine_results(r1, R2{Error2{}})
    == result<std::tuple<int, double>, Error1, Error2, Error3>{Error2{}});
}

TEST_CASE("result.fold_results")
{
  SECTION("with empty range")
  {
    const auto vec = std::vector<int>{};
    auto r = fold_results(std::begin(vec), std::end(vec), [](const auto i) {
      return result<int, std::string>{i * 2};
    });
    CHECK(r.is_success());
    CHECK_THAT(r.value(), Catch::UnorderedEquals(std::vector<int>{}));
  }

  SECTION("success case")
  {
    const auto vec = std::vector<int>{1, 2, 3};
    auto r = fold_results(std::begin(vec), std::end(vec), [](const auto i) {
      return result<int, std::string>{i * 2};
    });
    CHECK(r.is_success());
    CHECK_THAT(r.value(), Catch::UnorderedEquals(std::vector<int>{2, 4, 6}));
  }

  SECTION("error case")
  {
    const auto vec = std::vector<int>{1, 2, 3};
    auto r = fold_results(std::begin(vec), std::end(vec), [](const auto i) {
      if (i % 2 != 0)
      {
        return result<int, std::string>{i * 2};
      }
      else
      {
        return result<int, std::string>{"error"};
      }
    });
    CHECK(r.is_error());
    CHECK(std::visit([](const auto& e) { return e; }, r.error()) == "error");
  }
}

TEST_CASE("void_result.fold_results")
{
  SECTION("with empty range")
  {
    const auto vec = std::vector<int>{};
    auto r = fold_results(
      std::begin(vec), std::end(vec), [](const auto) { return void_success; });
    CHECK(r.is_success());
  }

  SECTION("success case")
  {
    const auto vec = std::vector<int>{1, 2, 3};
    auto vec_transformed = std::vector<int>{};
    auto r = fold_results(std::begin(vec), std::end(vec), [&](const auto i) {
      vec_transformed.push_back(i * 2);
      return void_success;
    });
    CHECK(r.is_success());
    CHECK_THAT(vec_transformed, Catch::UnorderedEquals(std::vector<int>{2, 4, 6}));
  }

  SECTION("error case")
  {
    const auto vec = std::vector<int>{1, 2, 3};
    auto r = fold_results(
      std::begin(vec), std::end(vec), [](const auto i) -> result<void, std::string> {
        if (i % 2 != 0)
        {
          return void_success;
        }
        else
        {
          return "error";
        }
      });
    CHECK(r.is_error());
    CHECK(std::visit([](const auto& e) { return e; }, r.error()) == "error");
  }
}
} // namespace kdl
