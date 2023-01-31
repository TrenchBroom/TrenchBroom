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
#include "kdl/result_for_each.h"
#include "kdl/result_io.h"

#include <iostream>
#include <string>

#include <catch2/catch.hpp>

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
};

inline std::ostream& operator<<(std::ostream& str, const Counter&)
{
  str << "Counter";
  return str;
}

/**
 * Tests construction of a successful result.
 */
template <typename ResultType, typename... V>
void test_construct_success(V&&... v)
{
  auto result = ResultType(std::forward<V>(v)...);
  CHECK(result.is_success());
  CHECK_FALSE(result.is_error());
  CHECK(result);
}

/**
 * Tests construction of an error result.
 */
template <typename ResultType, typename E>
void test_construct_error(E&& e)
{
  auto result = ResultType(std::forward<E>(e));
  CHECK_FALSE(result.is_success());
  CHECK(result.is_error());
  CHECK_FALSE(result);
}

/**
 * Tests visiting a successful result and passing by const lvalue reference to the
 * visitor.
 */
template <typename ResultType, typename V>
void test_visit_success_const_lvalue_ref(V&& v)
{
  auto result = ResultType(std::forward<V>(v));

  CHECK(result.visit(overload(
    [&](const auto& x) { return x == v; },
    [](const Error1&) { return false; },
    [](const Error2&) { return false; })));
}

/**
 * Tests visiting a successful result and passing by rvalue reference to the visitor.
 */
template <typename ResultType, typename V>
void test_visit_success_rvalue_ref(V&& v)
{
  auto result = ResultType(std::forward<V>(v));

  CHECK(std::move(result).visit(overload(
    [&](auto&&) { return true; },
    [](Error1&&) { return false; },
    [](Error2&&) { return false; })));

  typename ResultType::value_type y;
  std::move(result).visit(
    overload([&](auto&& x) { y = std::move(x); }, [](Error1&&) {}, [](Error2&&) {}));

  CHECK(y.copies == 0u);
}

/**
 * Tests visiting an error result and passing by const lvalue reference to the visitor.
 */
template <typename ResultType, typename E>
void test_visit_error_const_lvalue_ref(E&& e)
{
  auto result = ResultType(std::forward<E>(e));

  CHECK(result.visit(overload(
    [](const auto&) { return false; },
    [&](const E& x) { return x == e; },
    [](const Error2&) { return false; })));
}

/**
 * Tests visiting an error result and passing by rvalue reference to the visitor.
 */
template <typename ResultType, typename E>
void test_visit_error_rvalue_ref(E&& e)
{
  auto result = ResultType(std::forward<E>(e));

  CHECK(std::move(result).visit(overload(
    [](auto&&) { return false; },
    [&](E&&) { return true; },
    [](Error2&&) { return false; })));

  E y;
  std::move(result).visit(
    overload([](auto&&) {}, [&](E&& x) { y = std::move(x); }, [](Error2&&) {}));

  CHECK(y.copies == 0u);
}

/**
 * Tests mapping a successful result and passing by const lvalue reference to the mapping
 * function.
 */
template <typename FromResult, typename ToValueType, typename V>
void test_and_then_const_lvalue_ref(V&& v)
{
  auto from = FromResult(std::forward<V>(v));

  SECTION("mapping function returns a result type")
  {
    const auto to = from.and_then([](const typename FromResult::value_type& x) {
      return kdl::result<ToValueType, Error3>(static_cast<ToValueType>(x));
    });
    CHECK(to.is_success());
    CHECK_FALSE(to.is_error());
    CHECK(to);

    CHECK(to.visit(overload(
      [](const ToValueType&) { return true; }, [](const auto&) { return false; })));
  }

  SECTION("mapping function returns some other type")
  {
    const auto to = from.and_then([](const typename FromResult::value_type& x) {
      return static_cast<ToValueType>(x);
    });
    CHECK(to.is_success());
    CHECK_FALSE(to.is_error());
    CHECK(to);

    CHECK(to.visit(overload(
      [](const ToValueType&) { return true; }, [](const auto&) { return false; })));
  }
}

/**
 * Tests mapping a successful result and passing by rvalue reference to the mapping
 * function.
 */
template <typename FromResult, typename ToValueType, typename V>
void test_and_then_rvalue_ref(V&& v)
{
  auto from = FromResult(std::forward<V>(v));

  SECTION("mapping function returns a result type")
  {
    const auto to = std::move(from).and_then([](typename FromResult::value_type&& x) {
      return kdl::result<ToValueType, Error3>(std::move(static_cast<ToValueType>(x)));
    });
    CHECK(to.is_success());
    CHECK_FALSE(to.is_error());
    CHECK(to);

    CHECK(to.visit(overload(
      [](const ToValueType&) { return true; }, [](const auto&) { return false; })));

    ToValueType y;
    std::move(to).visit(overload([&](ToValueType&& x) { y = x; }, [](auto&&) {}));

    CHECK(y.copies == 0u);
  }

  SECTION("mapping function returns some other type")
  {
    const auto to = std::move(from).and_then([](typename FromResult::value_type&& x) {
      return std::move(static_cast<ToValueType>(x));
    });
    CHECK(to.is_success());
    CHECK_FALSE(to.is_error());
    CHECK(to);

    CHECK(to.visit(overload(
      [](const ToValueType&) { return true; }, [](const auto&) { return false; })));

    ToValueType y;
    std::move(to).visit(overload([&](ToValueType&& x) { y = x; }, [](auto&&) {}));

    CHECK(y.copies == 0u);
  }
}

/**
 * Tests visiting a successful result when there is no value.
 */
template <typename ResultType>
void test_visit_success_with_opt_value()
{
  auto result = ResultType();

  CHECK(result.visit(overload([]() { return true; }, [](const auto&) { return false; })));
}

/**
 * Tests visiting a successful result and passing by const lvalue reference to the visitor
 * when the value is optional (or void).
 */
template <typename ResultType, typename V>
void test_visit_success_const_lvalue_ref_with_opt_value(V&& v)
{
  auto result = ResultType(std::forward<V>(v));

  CHECK(result.visit(overload(
    []() { return false; },
    [&](const auto& x) { return x == v; },
    [](const Error1&) { return false; },
    [](const Error2&) { return false; })));
}

/**
 * Tests visiting a successful result and passing by rvalue reference to the visitor
 * when the value is optional (or void).
 */
template <typename ResultType, typename V>
void test_visit_success_rvalue_ref_with_opt_value(V&& v)
{
  auto result = ResultType(std::forward<V>(v));

  CHECK(std::move(result).visit(overload(
    []() { return false; },
    [&](auto&&) { return true; },
    [](Error1&&) { return false; },
    [](Error2&&) { return false; })));

  typename ResultType::value_type y;
  std::move(result).visit(overload(
    []() {}, [&](auto&& x) { y = std::move(x); }, [](Error1&&) {}, [](Error2&&) {}));

  CHECK(y.copies == 0u);
}

/**
 * Tests visiting an error result and passing by const lvalue reference to the visitor
 * when the value is optional (or void).
 */
template <typename ResultType, typename E>
void test_visit_error_const_lvalue_ref_with_opt_value(E&& e)
{
  auto result = ResultType(std::forward<E>(e));
  REQUIRE(result.is_error());

  CHECK(result.visit(overload(
    []() { return false; },
    [](const auto&) { return false; },
    [&](const E& x) { return x == e; })));
}

/**
 * Tests visiting an error result and passing by rvalue reference to the visitor
 * when the value is optional (or void).
 */
template <typename ResultType, typename E>
void test_visit_error_rvalue_ref_with_opt_value(E&& e)
{
  auto result = ResultType(std::forward<E>(e));

  CHECK(std::move(result).visit(overload(
    []() { return false; }, [](auto&&) { return false; }, [&](E&&) { return true; })));

  E y;
  std::move(result).visit(
    overload([]() {}, [](auto&&) {}, [&](E&& x) { y = std::move(x); }));

  CHECK(y.copies == 0u);
}

TEST_CASE("result_test.void_success")
{
  kdl::result<void, Error1> r1 =
    result<int, Error1>(1).and_then([](int) { return void_success; });

  CHECK(r1.is_success());
}

TEST_CASE("result_test.constructor")
{
  CHECK((result<int, float, std::string>(1).is_success()));
  CHECK((result<int, float, std::string>(1.0f).is_error()));
  CHECK((result<int, float, std::string>("").is_error()));

  test_construct_success<const result<int, Error1, Error2>>(1);
  test_construct_success<result<int, Error1, Error2>>(1);
  test_construct_success<const result<const int, Error1, Error2>>(1);
  test_construct_success<result<const int, Error1, Error2>>(1);

  test_construct_error<const result<int, Error1, Error2>>(Error1{});
  test_construct_error<result<int, Error1, Error2>>(Error1{});
  test_construct_error<const result<const int, Error1, Error2>>(Error1{});
  test_construct_error<result<const int, Error1, Error2>>(Error1{});

  test_construct_error<const result<int, Error1, Error2>>(Error2{});
  test_construct_error<result<int, Error1, Error2>>(Error2{});
  test_construct_error<const result<const int, Error1, Error2>>(Error2{});
  test_construct_error<result<const int, Error1, Error2>>(Error2{});
}

TEST_CASE("result_test.converting_constructor")
{
  CHECK(result<int, std::string, float>{result<int, std::string, float>{1}}.is_success());
  CHECK(
    result<int, std::string, float>{result<int, std::string, float>{"asdf"}}.is_error());

  CHECK(result<int, std::string, float>{result<int, float, std::string>{1}}.is_success());
  CHECK(
    result<int, std::string, float>{result<int, float, std::string>{"asdf"}}.is_error());

  CHECK(result<int, std::string, float>{result<int, std::string>{1}}.is_success());
  CHECK(result<int, std::string, float>{result<int, std::string>{"asdf"}}.is_error());

  CHECK(result<int, std::string, float>{result<int, float>{1}}.is_success());
  CHECK(result<int, std::string, float>{result<int, float>{1.0f}}.is_error());

  // must trigger static assert
  // CHECK(result<int, std::string, float>{result<int, float, double>{1.0f}}.is_error());
}

TEST_CASE("result_test.visit")
{
  test_visit_success_const_lvalue_ref<const result<int, Error1, Error2>>(1);
  test_visit_success_const_lvalue_ref<result<int, Error1, Error2>>(1);
  test_visit_success_const_lvalue_ref<const result<const int, Error1, Error2>>(1);
  test_visit_success_const_lvalue_ref<result<const int, Error1, Error2>>(1);
  test_visit_success_rvalue_ref<result<Counter, Error1, Error2>>(Counter{});

  test_visit_error_const_lvalue_ref<const result<int, Error1, Error2>>(Error1{});
  test_visit_error_const_lvalue_ref<result<int, Error1, Error2>>(Error1{});
  test_visit_error_const_lvalue_ref<const result<const int, Error1, Error2>>(Error1{});
  test_visit_error_const_lvalue_ref<result<const int, Error1, Error2>>(Error1{});
  test_visit_error_rvalue_ref<result<int, Counter, Error2>>(Counter{});
}

TEST_CASE("result_test.and_then")
{
  test_and_then_const_lvalue_ref<const result<int, Error1, Error2>, float>(1);
  test_and_then_const_lvalue_ref<result<int, Error1, Error2>, float>(1);
  test_and_then_const_lvalue_ref<const result<const int, Error1, Error2>, float>(1);
  test_and_then_const_lvalue_ref<result<const int, Error1, Error2>, float>(1);
  test_and_then_rvalue_ref<result<Counter, Error1, Error2>, Counter>(Counter{});
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
    CHECK(constLValueErrorToSuccess.or_else([](const auto&) {
      return result<int, Error3>{2};
    }) == result<int, Error3>{2});

    const auto constLValueErrorToError = result<int, Error1, Error2>{Error1{}};
    CHECK(constLValueErrorToError.or_else([](const auto&) {
      return result<int, Error3>{Error3{}};
    }) == result<int, Error3>{Error3{}});

    auto nonConstLValueSuccess = result<int, Error1, Error2>{1};
    CHECK(nonConstLValueSuccess.or_else([](const auto&) {
      FAIL();
      return result<int, Error3>{2};
    }) == result<int, Error3>{1});

    auto nonConstLValueErrorToSuccess = result<int, Error1, Error2>{Error1{}};
    CHECK(nonConstLValueErrorToSuccess.or_else([](const auto&) {
      return result<int, Error3>{2};
    }) == result<int, Error3>{2});

    auto nonConstLValueErrorToError = result<int, Error1, Error2>{Error1{}};
    CHECK(nonConstLValueErrorToError.or_else([](const auto&) {
      return result<int, Error3>{Error3{}};
    }) == result<int, Error3>{Error3{}});

    CHECK(result<int, Error1, Error2>{1}.or_else([](auto&&) {
      FAIL();
      return result<int, Error3>{2};
    }) == result<int, Error3>{1});

    CHECK(result<int, Error1, Error2>{Error1{}}.or_else([](auto&&) {
      return result<int, Error3>{2};
    }) == result<int, Error3>{2});

    CHECK(result<int, Error1, Error2>{Error1{}}.or_else([](auto&&) {
      return result<int, Error3>{Error3{}};
    }) == result<int, Error3>{Error3{}});
  }

  SECTION("void result")
  {
    const auto constLValueSuccess = result<void, Error1, Error2>{};
    CHECK(constLValueSuccess.or_else([](const auto&) {
      FAIL();
      return result<void, Error3>{};
    }) == result<void, Error3>{});

    const auto constLValueErrorToSuccess = result<void, Error1, Error2>{Error1{}};
    CHECK(constLValueErrorToSuccess.or_else([](const auto&) {
      return result<void, Error3>{};
    }) == result<void, Error3>{});

    const auto constLValueErrorToError = result<void, Error1, Error2>{Error1{}};
    CHECK(constLValueErrorToError.or_else([](const auto&) {
      return result<void, Error3>{Error3{}};
    }) == result<void, Error3>{Error3{}});

    auto nonConstLValueSuccess = result<void, Error1, Error2>{};
    CHECK(nonConstLValueSuccess.or_else([](const auto&) {
      FAIL();
      return result<void, Error3>{};
    }) == result<void, Error3>{});

    auto nonConstLValueErrorToSuccess = result<void, Error1, Error2>{Error1{}};
    CHECK(nonConstLValueErrorToSuccess.or_else([](const auto&) {
      return result<void, Error3>{};
    }) == result<void, Error3>{});

    auto nonConstLValueErrorToError = result<void, Error1, Error2>{Error1{}};
    CHECK(nonConstLValueErrorToError.or_else([](const auto&) {
      return result<void, Error3>{Error3{}};
    }) == result<void, Error3>{Error3{}});

    CHECK(result<void, Error1, Error2>{}.or_else([](auto&&) {
      FAIL();
      return result<void, Error3>{};
    }) == result<void, Error3>{});

    CHECK(result<void, Error1, Error2>{Error1{}}.or_else([](auto&&) {
      return result<void, Error3>{};
    }) == result<void, Error3>{});

    CHECK(result<void, Error1, Error2>{Error1{}}.or_else([](auto&&) {
      return result<void, Error3>{Error3{}};
    }) == result<void, Error3>{Error3{}});
  }
}

TEST_CASE("result_test.map_errors")
{
  SECTION("map error of success result by const lvalue")
  {
    const auto r = result<int, Error1>{1};
    const auto rm =
      r.map_errors([](const Error1&) { return result<int, Error2>{Error2{}}; });
    CHECK(rm.is_success());
    CHECK(rm.value() == 1);
  }

  SECTION("map error of success result by rvalue")
  {
    const auto rm = result<int, Error1>{1}.map_errors(
      [](Error1&&) { return result<int, Error2>{Error2{}}; });
    CHECK(rm.is_success());
    CHECK(rm.value() == 1);
  }

  SECTION("map error of error result by const lvalue")
  {
    const auto r = result<int, Error1>{Error1{}};
    const auto rm =
      r.map_errors([](const Error1&) { return result<int, Error2>{Error2{}}; });
    CHECK(rm.is_error());
    CHECK(rm.error() == std::variant<Error2>{Error2{}});
  }

  SECTION("map error of error result by rvalue")
  {
    const auto rm = result<int, Error1>{Error1{}}.map_errors(
      [](Error1&&) { return result<int, Error2>{Error2{}}; });
    CHECK(rm.is_error());
    CHECK(rm.error() == std::variant<Error2>{Error2{}});
  }
}

TEST_CASE("void_result_test.constructor")
{
  CHECK((result<void, float, std::string>().is_success()));
  CHECK((result<void, float, std::string>(1.0f).is_error()));
  CHECK((result<void, float, std::string>("").is_error()));

  test_construct_success<const result<void, Error1, Error2>>();
  test_construct_success<result<void, Error1, Error2>>();

  test_construct_error<const result<void, Error1, Error2>>(Error1{});
  test_construct_error<result<void, Error1, Error2>>(Error1{});

  test_construct_error<const result<void, Error1, Error2>>(Error2{});
  test_construct_error<result<void, Error1, Error2>>(Error2{});
}

TEST_CASE("void_result_test.converting_constructor")
{
  CHECK(
    result<void, std::string, float>{result<void, std::string, float>{}}.is_success());
  CHECK(result<void, std::string, float>{result<void, std::string, float>{"asdf"}}
          .is_error());

  CHECK(
    result<void, std::string, float>{result<void, float, std::string>{}}.is_success());
  CHECK(result<void, std::string, float>{result<void, float, std::string>{"asdf"}}
          .is_error());

  CHECK(result<void, std::string, float>{result<void, std::string>{}}.is_success());
  CHECK(result<void, std::string, float>{result<void, std::string>{"asdf"}}.is_error());

  CHECK(result<void, std::string, float>{result<void, float>{}}.is_success());
  CHECK(result<void, std::string, float>{result<void, float>{1.0f}}.is_error());

  // must trigger static assert
  // CHECK(result<void, std::string, float>{result<void, float,
  // double>{1.0f}}.is_error());
}

TEST_CASE("void_result_test.visit")
{
  test_visit_success_with_opt_value<const result<void, Error1, Error2>>();
  test_visit_success_with_opt_value<result<void, Error1, Error2>>();

  test_visit_error_const_lvalue_ref_with_opt_value<const result<void, Error1, Error2>>(
    Error1{});
  test_visit_error_const_lvalue_ref_with_opt_value<result<void, Error1, Error2>>(
    Error1{});
  test_visit_error_rvalue_ref_with_opt_value<result<void, Counter, Error2>>(Counter{});
}

TEST_CASE("void_result_test.and_then")
{
  const auto r_success = result<void, Error1, Error2>();
  const auto r_error = result<void, Error1, Error2>(Error2{});

  SECTION("mapping function returns a result type")
  {
    const auto f_success = []() { return kdl::result<bool, Error3>(true); };
    const auto f_error = []() { return kdl::result<bool, Error3>(Error3{}); };
    const auto f_void = []() { return kdl::result<void, Error3>(); };

    CHECK(r_success.and_then(f_success) == result<bool, Error1, Error2, Error3>(true));
    CHECK(r_success.and_then(f_error) == result<bool, Error1, Error2, Error3>(Error3{}));
    CHECK(r_error.and_then(f_success) == result<bool, Error1, Error2, Error3>(Error2{}));
    CHECK(r_success.and_then(f_void) == result<void, Error1, Error2, Error3>());
  }

  SECTION("mapping function returns some other type")
  {
    const auto f_success = []() { return true; };
    const auto f_void = []() {};

    CHECK(r_success.and_then(f_success) == result<bool, Error1, Error2>(true));
    CHECK(r_error.and_then(f_success) == result<bool, Error1, Error2>(Error2{}));
    CHECK(r_success.and_then(f_void) == result<void, Error1, Error2>());
  }
}

TEST_CASE("combine_results", "result_test")
{
  using R1 = result<int, Error1, Error2>;
  using R2 = result<double, Error2, Error3>;

  auto r1 = R1(1);
  auto r2 = R2(2.0);
  auto r3 = R2(Error2{});

  CHECK(combine_results(r1, r2).is_success());
  CHECK(combine_results(r1, r2).visit(kdl::overload(
    [](const std::tuple<int, double>& t) {
      CHECK(t == std::make_tuple(1, 2.0));
      return true;
    },
    [](const auto&) { return false; })));

  CHECK(combine_results(r1, r3).is_error());
  CHECK(combine_results(r1, r3).visit(kdl::overload(
    [](const std::tuple<int, double>&) { return false; },
    [](const Error2&) { return true; },
    [](const auto&) { return false; })));

  CHECK(combine_results(r1, R2(2.0)).is_success());
  CHECK(combine_results(r1, R2(2.0))
          .visit(kdl::overload(
            [](const std::tuple<int, double>& t) {
              CHECK(t == std::make_tuple(1, 2.0));
              return true;
            },
            [](const auto&) { return false; })));

  CHECK(combine_results(r1, R2(Error2{})).is_error());
  CHECK(combine_results(r1, R2(Error2{}))
          .visit(kdl::overload(
            [](const std::tuple<int, double>&) { return false; },
            [](const Error2&) { return true; },
            [](const auto&) { return false; })));
}

TEST_CASE("result.for_each_result", "result_test")
{
  SECTION("with empty range")
  {
    const auto vec = std::vector<int>{};
    auto r = for_each_result(std::begin(vec), std::end(vec), [](const auto i) {
      return result<int, std::string>{i * 2};
    });
    CHECK(r.is_success());
    CHECK_THAT(r.value(), Catch::UnorderedEquals(std::vector<int>{}));
  }

  SECTION("success case")
  {
    const auto vec = std::vector<int>{1, 2, 3};
    auto r = for_each_result(std::begin(vec), std::end(vec), [](const auto i) {
      return result<int, std::string>{i * 2};
    });
    CHECK(r.is_success());
    CHECK_THAT(r.value(), Catch::UnorderedEquals(std::vector<int>{2, 4, 6}));
  }

  SECTION("error case")
  {
    const auto vec = std::vector<int>{1, 2, 3};
    auto r = for_each_result(std::begin(vec), std::end(vec), [](const auto i) {
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

TEST_CASE("void_result.for_each_result", "result_test")
{
  SECTION("with empty range")
  {
    const auto vec = std::vector<int>{};
    auto r = for_each_result(
      std::begin(vec), std::end(vec), [](const auto) { return void_success; });
    CHECK(r.is_success());
  }

  SECTION("success case")
  {
    const auto vec = std::vector<int>{1, 2, 3};
    auto vec_transformed = std::vector<int>{};
    auto r = for_each_result(std::begin(vec), std::end(vec), [&](const auto i) {
      vec_transformed.push_back(i * 2);
      return void_success;
    });
    CHECK(r.is_success());
    CHECK_THAT(vec_transformed, Catch::UnorderedEquals(std::vector<int>{2, 4, 6}));
  }

  SECTION("error case")
  {
    const auto vec = std::vector<int>{1, 2, 3};
    auto r = for_each_result(
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

TEST_CASE("result.collect_values", "result_test")
{
  auto errors = std::vector<std::string>{};
  const auto errorHandler = [&](std::string&& error) {
    errors.push_back(std::move(error));
  };

  SECTION("with empty range")
  {
    const auto vec = std::vector<kdl::result<int, std::string>>{};
    auto r = collect_values(std::begin(vec), std::end(vec), errorHandler);
    CHECK_THAT(r, Catch::Equals(std::vector<int>{}));
    CHECK_THAT(errors, Catch::Equals(std::vector<std::string>{}));
  }

  SECTION("nonempty range")
  {
    const auto vec = std::vector<kdl::result<int, std::string>>{
      kdl::result<int, std::string>{1},
      kdl::result<int, std::string>{"error 1"},
      kdl::result<int, std::string>{2},
      kdl::result<int, std::string>{"error 2"},
    };
    auto r = collect_values(std::begin(vec), std::end(vec), errorHandler);
    CHECK_THAT(r, Catch::Equals(std::vector<int>{1, 2}));
    CHECK_THAT(errors, Catch::Equals(std::vector<std::string>{"error 1", "error 2"}));
  }
}
} // namespace kdl
