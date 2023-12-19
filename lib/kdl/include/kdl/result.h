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

#pragma once

#include "kdl/meta_utils.h"
#include "kdl/overload.h"
#include "kdl/result_forward.h"

#include <exception>
#include <tuple>
#include <type_traits>
#include <variant>

namespace kdl
{
/**
 * Thrown when attempting to access a result success value on a failed result.
 */
class bad_result_access : public std::exception
{
public:
  bad_result_access() noexcept = default;

  const char* what() const noexcept override
  {
    return "access success value on a failed result";
  }
};

template <typename... Values>
struct multi_value
{
  static_assert(sizeof...(Values) > 1, "multi_value has at least two values");

  std::tuple<Values...> values;

  /**
   * Converting constructor. Must be non-explicit to allow initializing a result type with
   * a multi_value of a different type, i.e. `result<multi_value<std::string, int>,
   * Error>{multi_value{"asdf", 1}}`;
   */
  template <typename... OtherValues>
  // NOLINTNEXTLINE
  multi_value(multi_value<OtherValues...> v)
    : values{std::move(v.values)}
  {
  }

  /**
   * Initialize a multi_value from values.
   */
  explicit multi_value(Values&&... v)
    : values{std::forward<Values>(v)...}
  {
  }

  explicit multi_value(std::tuple<Values...> v)
    : values{std::move(v)}
  {
  }

  template <typename... OtherValues>
  static multi_value make(multi_value<OtherValues...> v)
  {
    return multi_value{std::tuple<Values...>(v.values)};
  }

  template <typename Value1, typename Value2>
  static multi_value make(Value1 v1, Value2 v2)
  {
    return multi_value{std::tuple<Values...>{std::move(v1), std::move(v2)}};
  }

  template <typename Value1, typename... Values2>
  static multi_value make(Value1 v1, multi_value<Values2...> v2)
  {
    return multi_value{std::tuple<Values...>{
      std::tuple_cat(std::tuple{std::move(v1)}, std::move(v2.values))}};
  }

  template <typename... Values1, typename Value2>
  static multi_value make(multi_value<Values1...> v1, Value2 v2)
  {
    return multi_value{std::tuple<Values...>{
      std::tuple_cat(std::move(v1.values), std::tuple{std::move(v2)})}};
  }

  template <typename... Values1, typename... Values2>
  static multi_value make(multi_value<Values1...> v1, multi_value<Values2...> v2)
  {
    return multi_value{
      std::tuple<Values...>{std::tuple_cat(std::move(v1.values), std::move(v2.values))}};
  }

  friend bool operator==(const multi_value& lhs, const multi_value& rhs)
  {
    return lhs.values == rhs.values;
  }
  friend bool operator!=(const multi_value& lhs, const multi_value& rhs)
  {
    return !(lhs == rhs);
  }
};

template <typename... Values>
multi_value(Values&&...) -> multi_value<Values...>;

namespace detail
{
template <typename Value, typename Errors>
struct make_result_type
{
};

template <typename Value, typename... Errors>
struct make_result_type<Value, meta_type_list<Errors...>>
{
  using type = result<Value, Errors...>;
};

template <typename Result>
struct is_result : public std::false_type
{
};

template <typename Value, typename... Errors>
struct is_result<result<Value, Errors...>> : public std::true_type
{
};

template <typename Result1, typename Result2>
struct chain_results
{
};

template <typename Value1, typename... Errors1, typename Value2, typename... Errors2>
struct chain_results<result<Value1, Errors1...>, result<Value2, Errors2...>>
{
  using type =
    typename make_result_type<Value2, meta_remove_duplicates_t<Errors1..., Errors2...>>::
      type;
};

template <typename Value, typename... Errors>
struct wrap_result
{
  using type = result<Value, Errors...>;
};

template <typename Value, typename... Errors1, typename... Errors2>
struct wrap_result<result<Value, Errors1...>, Errors2...>
{
  using type =
    typename make_result_type<Value, meta_remove_duplicates_t<Errors1..., Errors2...>>::
      type;
};

template <typename Result1, typename Result2>
struct join_results
{
};

template <typename Value1, typename... Errors1, typename Value2, typename... Errors2>
struct join_results<result<Value1, Errors1...>, result<Value2, Errors2...>>
{
  using type = typename make_result_type<
    multi_value<Value1, Value2>,
    meta_remove_duplicates_t<Errors1..., Errors2...>>::type;
};

template <typename Value1, typename... Errors1, typename... Values2, typename... Errors2>
struct join_results<
  result<Value1, Errors1...>,
  result<multi_value<Values2...>, Errors2...>>
{
  using type = typename make_result_type<
    multi_value<Value1, Values2...>,
    meta_remove_duplicates_t<Errors1..., Errors2...>>::type;
};

template <
  typename... Values1,
  typename... Errors1,
  typename... Values2,
  typename... Errors2>
struct join_results<
  result<multi_value<Values1...>, Errors1...>,
  result<multi_value<Values2...>, Errors2...>>
{
  using type = typename make_result_type<
    multi_value<Values1..., Values2...>,
    meta_remove_duplicates_t<Errors1..., Errors2...>>::type;
};

template <typename... Values1, typename... Errors1, typename Value2, typename... Errors2>
struct join_results<
  result<multi_value<Values1...>, Errors1...>,
  result<Value2, Errors2...>>
{
  using type = typename make_result_type<
    multi_value<Values1..., Value2>,
    meta_remove_duplicates_t<Errors1..., Errors2...>>::type;
};
} // namespace detail

template <typename MaybeResult>
inline constexpr auto is_result_v = detail::is_result<MaybeResult>::value;

template <typename ResultOrValue, typename... Errors>
using wrap_result_t = typename detail::wrap_result<ResultOrValue, Errors...>::type;

/**
 * Wrapper class that can contain either a value or one of several errors.
 *
 * An instance of this class represents an expectation for the result of applying a
 * function if that function returns either a value or throws an error.
 *
 * A result is considered successful if it contains a value, and a failure if it contains
 * an error.
 *
 * @tparam Value the type of the value
 * @tparam Errors the types of the possible errors
 */
template <typename Value, typename... Errors>
class [[nodiscard]] result
{
public:
  using value_type = Value;
  static constexpr auto error_count = std::tuple_size_v<std::tuple<Errors...>>;

  template <typename OtherValue>
  using with_value_type = result<OtherValue, Errors...>;

private:
  using variant_type = std::variant<value_type, Errors...>;
  variant_type m_value;

  explicit result(variant_type&& v)
    : m_value{std::move(v)}
  {
  }

public:
  /**
   * Creates a new result that wraps the given value.
   *
   * v must be convertible to the value type or one of the error types of this result.
   * If the value is passed by (const) lvalue reference, it is copied into this result, if
   * it s passed by rvalue reference, then it is moved into this result.
   *
   * @tparam T the type of the value, must match the value type or one of the error types
   * of this result
   * @param v the value
   */
  template <
    typename T,
    typename std::enable_if<std::disjunction_v<
      std::is_convertible<T, Value>,
      std::is_convertible<T, Errors>...>>::type* = nullptr>
  // NOLINTNEXTLINE
  result(T&& v)
    : m_value{std::forward<T>(v)}
  {
  }

  /**
   * Converting constructor.
   *
   * The given result type must have the same value type and a subset of the error types
   * of this result type. The value or error wrapped by the given result is moved into
   * this result.
   *
   * @param other the result to convert
   */
  template <typename... ErrorSubset>
  // NOLINTNEXTLINE
  result(result<Value, ErrorSubset...> other)
    : m_value{std::move(other).visit(overload(
      [](Value&& v) -> variant_type { return std::move(v); },
      [](auto&& e) -> variant_type { return std::forward<decltype(e)>(e); }))}
  {
    static_assert(
      meta_is_subset_v<meta_type_list<ErrorSubset...>, meta_type_list<Errors...>>,
      "Error types of result type to convert must be a subset of target result type");
  }

public:
  /**
   * Visits the value or error contained in this result.
   *
   * The given visitor must accept the value type and all error types of this result.
   * The value or error contained in this result is passed to the visitor by const lvalue
   * reference.
   *
   * @tparam Visitor the type of the visitor
   * @param visitor the visitor to apply
   * @return the value returned by the given visitor or void if the given visitor does not
   * return anything
   */
  template <typename Visitor>
  auto visit(const Visitor& visitor) const&
  {
    return std::visit(visitor, m_value);
  }

  /**
   * Visits the value or error contained in this result.
   *
   * The given visitor must accept the value type and all error types of this result. The
   * value or error contained in this result is passed to the visitor by non const lvalue
   * reference.
   *
   * @tparam Visitor the type of the visitor
   * @param visitor the visitor to apply
   * @return the value returned by the given visitor or void if the given visitor does not
   * return anything
   */
  template <typename Visitor>
  auto visit(const Visitor& visitor) &
  {
    return std::visit(visitor, m_value);
  }

  /**
   * Visits the value or error contained in this result.
   *
   * The given visitor must accept the value type and all error types of this result.
   * The value or error contained in this result is passed to the visitor by rvalue
   * reference.
   *
   * @tparam Visitor the type of the visitor
   * @param visitor the visitor to apply
   * @return the value returned by the given visitor or void if the given visitor does not
   * return anything
   */
  template <typename Visitor>
  auto visit(const Visitor& visitor) &&
  {
    return std::visit(visitor, std::move(m_value));
  }

  template <typename R>
  auto join(R other) const&
  {
    static_assert(is_result_v<R>, "Can only join a result type");
    static_assert(
      !std::is_same_v<typename R::value_type, void>, "Cannot join a void result");

    using My_Result = result<Value, Errors...>;
    using Cm_Result = typename detail::join_results<My_Result, R>::type;
    using Cm_Value = typename Cm_Result::value_type;

    return std::visit(
      overload(
        [&](const value_type& v) {
          return std::move(other).visit(overload(
            [&](typename R::value_type&& w) {
              return Cm_Result{Cm_Value::make(value_type{v}, std::move(w))};
            },
            [](auto&& e) { return Cm_Result{std::forward<decltype(e)>(e)}; }));
        },
        [](const auto& e) { return Cm_Result{e}; }),
      m_value);
  }

  template <typename R>
  auto join(R other) &
  {
    static_assert(is_result_v<R>, "Can only join a result type");
    static_assert(
      !std::is_same_v<typename R::value_type, void>, "Cannot join a void result");

    using My_Result = result<Value, Errors...>;
    using Cm_Result = typename detail::join_results<My_Result, R>::type;
    using Cm_Value = typename Cm_Result::value_type;

    return std::visit(
      overload(
        [&](value_type& v) {
          return std::move(other).visit(overload(
            [&](typename R::value_type&& w) {
              return Cm_Result{Cm_Value::make(value_type{v}, std::move(w))};
            },
            [](auto&& e) { return Cm_Result{std::forward<decltype(e)>(e)}; }));
        },
        [](auto& e) { return Cm_Result{e}; }),
      m_value);
  }

  template <typename R>
  auto join(R other) &&
  {
    static_assert(is_result_v<R>, "Can only join a result type");
    static_assert(
      !std::is_same_v<typename R::value_type, void>, "Cannot join a void result");

    using My_Result = result<Value, Errors...>;
    using Cm_Result = typename detail::join_results<My_Result, R>::type;
    using Cm_Value = typename Cm_Result::value_type;

    return std::visit(
      overload(
        [&](value_type&& v) {
          return std::move(other).visit(overload(
            [&](typename R::value_type&& w) {
              return Cm_Result{Cm_Value::make(std::move(v), std::move(w))};
            },
            [](auto&& e) { return Cm_Result{std::forward<decltype(e)>(e)}; }));
        },
        [](auto&& e) { return Cm_Result{std::forward<decltype(e)>(e)}; }),
      std::move(m_value));
  }

  /**
   * Applies the given function to the value contained in this result, and returns a
   * combination of this result and the result of the given function.
   *
   * The given function can return a result type of its own, or it can return any other
   * type including `void`. In the following discussion, we assume that the function
   * returns a result type. If the function does return another type `T`, then we treat it
   * as if it had returned `result<T>`.
   *
   * Let the function's result type be `result<Fn_Value, Fn_Errors...>` and let this
   * result's type be `result<My_Value, My_Errors...>`. Then `Fn_Value` and `My_Value` can
   * be the same types, or they can be totally unrelated types. Likewise, there is no
   * restriction on the types in `Fn_Errors` and `My_Errors`. They can be disjoint sets,
   * but they may also intersect.
   *
   * The type of the result returned by this function is a combination of
   * `result<Fn_Value, Fn_Errors...>` and `result<My_Value, My_Errors...>`. Let
   * `result<Rs_Value, Rs_Errors>` be the type of the returned result, then `Rs_Value` is
   * `Fn_Value`, and `Rs_Errors` is the concatenation of `My_Errors` and `Fn_Errors`, but
   * with duplicates removed.
   *
   * For example, let the type of this result be `result<int, Error1, Error2>`, and let
   * the type of the result returned by `f` be `result<float, Error3, Error2>`. Then the
   * type of the combined returned result will be `result<float, Error1, Error2, Error3>`.
   *
   * To determine the value or error contained in the returned result, consider the
   * following rules:
   *
   * - If this result is an error, then the returned result will contain that error, and
   * the given function will not be invoked.
   * - If this result is a success, then `f` will be called, and return a result `r_f`.
   *   - If `r_f` is a success, then the value contained in `r_f` will be returned in the
   * combined result.
   *   - If `r_f` is an error, then the error contained in `r_f` will be returned in the
   * combined result.
   *
   * Note that the value contained in this result is passed to `f` by const lvalue
   * reference.
   *
   * Calling this function allows to chain operations on results and handling any errors
   * that occur in any of the chain in one final call to `or_else`. Consider the following
   * example:
   *
   * ```
   * result<int, Error1> f1();
   *
   * f1().and_then(
   *     [](const int i) {
   *         if (i < 0) {
   *             return result<float, Error2>(Error2{"invalid"});
   *         } else {
   *             return result<float, Error2>(std::sqrt(float(i)));
   *         }
   *     }
   * ).and_then(
   *     [](const float f) {
   *         if (f > 1000.0f) {
   *             return result<std::string, Error3>(Error3{"out of bounds"});
   *         } else {
   *             return result<std::string, Error3>("good value");
   *         }
   *     }
   * ).or_else(
   *     overload {
   *         [](const Error1& e) {
   *             std::cerr << "f1 failed: " << e;
   *         },
   *         [](const Error2& e) {
   *             std::cerr << "first lambda failed: " << e;
   *         },
   *         [](const Error3& e) {
   *             std::cerr << "second lambda failed: " << e;
   *         }
   *     }
   * );
   *
   * ```
   *
   * @tparam F the type of the function to apply
   * @param f the function to apply
   * @return a new combined result with the type and value as described above
   */
  template <typename F>
  auto and_then(const F& f) const&
  {
    using My_Result = result<Value, Errors...>;
    using Fn_Result = decltype(f(std::declval<const Value&>()));

    static_assert(is_result_v<Fn_Result>, "Function must return a result type");

    using Cm_Result = typename detail::chain_results<My_Result, Fn_Result>::type;

    return std::visit(
      overload(
        [&](const value_type& v) { return Cm_Result{f(v)}; },
        [](const auto& e) { return Cm_Result{e}; }),
      m_value);
  }

  /**
   * See the previous function. The only difference is that the value contained in this
   * result is passed to `f` by non const lvalue reference.
   */
  template <typename F>
  auto and_then(const F& f) &
  {
    using My_Result = result<Value, Errors...>;
    using Fn_Result = decltype(f(std::declval<Value&>()));

    static_assert(is_result_v<Fn_Result>, "Function must return a result type");

    using Cm_Result = typename detail::chain_results<My_Result, Fn_Result>::type;

    return std::visit(
      overload(
        [&](value_type& v) { return Cm_Result{f(v)}; },
        [](auto& e) { return Cm_Result{e}; }),
      m_value);
  }

  /**
   * See the previous function. The only difference is that the value contained in this
   * result is passed to `f` by rvalue reference to allow moving.
   */
  template <typename F>
  auto and_then(const F& f) &&
  {
    using My_Result = result<Value, Errors...>;
    using Fn_Result = decltype(f(std::declval<Value&&>()));

    static_assert(is_result_v<Fn_Result>, "Function must return a result type");

    using Cm_Result = typename detail::chain_results<My_Result, Fn_Result>::type;

    return std::visit(
      overload(
        [&](value_type&& v) { return Cm_Result{f(std::move(v))}; },
        [](auto&& e) { return Cm_Result{std::forward<decltype(e)>(e)}; }),
      std::move(m_value));
  }

  /**
   * Applies the given function to any error contained in this result, and returns a new
   * result with the same value type as this result, but different error types.
   *
   * Use this function for error handling or for transforming error types at API
   * boundaries.
   *
   * The given function must handle all error types of this result type. It can return a
   * result type of its own, or it can return another value as long as the returned value
   * type matches this result's value type. In the following discussion, we assume that
   * the function returns a result type. If the function does return a value of type
   * Value, then we treat it as if it had returned `result<Value, Fn_Errors...>`.
   *
   * Let the function's result type be `result<Value, Fn_Errors...>` and let this
   * result's type be `result<Value, My_Errors...>`. Then `or_else` will return a result
   * of type `result<Value, Fn_Errors...>`. If the given function just returns a value,
   * then `or_else` will return `result<Value>`, a result without any errors.
   *
   * Note that the value contained in this result is passed to `f` by const lvalue
   * reference.
   */
  template <typename F>
  auto or_else(const F& f) const&
  {
    static_assert(
      sizeof...(Errors) > 0,
      "Cannot apply or_else to a result type with empty error type list");

    using Fn_Result = decltype(f(std::declval<const meta_front_t<Errors...>&>()));

    static_assert(is_result_v<Fn_Result>, "Function must return a result type");
    static_assert(
      std::is_same_v<typename Fn_Result::value_type, Value>,
      "Function must return result with same value type");

    return std::visit(
      overload(
        [](const value_type& v) { return Fn_Result{v}; },
        [&](const auto& e) { return f(e); }),
      m_value);
  }

  /**
   * See the previous function. The only difference is that the value contained in this
   * result is passed to `f` by rvalue reference to allow moving.
   */
  template <typename F>
  auto or_else(const F& f) &
  {
    static_assert(
      sizeof...(Errors) > 0,
      "Cannot apply or_else to a result type with empty error type list");

    using Fn_Result = decltype(f(std::declval<meta_front_t<Errors...>&>()));

    static_assert(is_result_v<Fn_Result>, "Function must return a result type");
    static_assert(
      std::is_same_v<typename Fn_Result::value_type, Value>,
      "Function must return result with same value type");

    return std::visit(
      overload([](value_type& v) { return Fn_Result{v}; }, [&](auto& e) { return f(e); }),
      m_value);
  }

  /**
   * See the previous function. The only difference is that the value contained in this
   * result is passed to `f` by rvalue reference to allow moving.
   */
  template <typename F>
  auto or_else(const F& f) &&
  {
    static_assert(
      sizeof...(Errors) > 0,
      "Cannot apply or_else to a result type with empty error type list");

    using Fn_Result = decltype(f(std::declval<meta_front_t<Errors...>&&>()));

    static_assert(is_result_v<Fn_Result>, "Function must return a result type");
    static_assert(
      std::is_same_v<typename Fn_Result::value_type, Value>,
      "Function must return result with same value type");

    return std::visit(
      overload(
        [](value_type&& v) { return Fn_Result{std::move(v)}; },
        [&](auto&& e) { return f(std::forward<decltype(e)>(e)); }),
      std::move(m_value));
  }

  template <typename F>
  auto transform(const F& f) const&
  {
    using Fn_Result = decltype(f(std::declval<const Value&>()));
    using Cm_Result = result<Fn_Result, Errors...>;

    return std::visit(
      overload(
        [&](const value_type& v) {
          if constexpr (std::is_same_v<typename Cm_Result::value_type, void>)
          {
            f(v);
            return Cm_Result{};
          }
          else
          {
            return Cm_Result{f(v)};
          }
        },
        [](const auto& e) { return Cm_Result{e}; }),
      m_value);
  }

  template <typename F>
  auto transform(const F& f) &
  {
    using Fn_Result = decltype(f(std::declval<Value&>()));
    using Cm_Result = result<Fn_Result, Errors...>;

    return std::visit(
      overload(
        [&](value_type& v) {
          if constexpr (std::is_same_v<typename Cm_Result::value_type, void>)
          {
            f(v);
            return Cm_Result{};
          }
          else
          {
            return Cm_Result{f(v)};
          }
        },
        [](auto& e) { return Cm_Result{e}; }),
      m_value);
  }

  /**
   * See the previous function. The only difference is that the value contained in this
   * result is passed to `f` by rvalue reference to allow moving.
   */
  template <typename F>
  auto transform(const F& f) &&
  {
    using Fn_Result = decltype(f(std::declval<Value&&>()));
    using Cm_Result = result<Fn_Result, Errors...>;

    return std::visit(
      overload(
        [&](value_type&& v) {
          if constexpr (std::is_same_v<typename Cm_Result::value_type, void>)
          {
            f(std::move(v));
            return Cm_Result{};
          }
          else
          {
            return Cm_Result{f(std::move(v))};
          }
        },
        [](auto&& e) { return Cm_Result{std::forward<decltype(e)>(e)}; }),
      std::move(m_value));
  }

#ifdef _MSC_VER
// MSVC issues an unreachable code warning if the given function throws.
#pragma warning(push)
#pragma warning(disable : 4702)
#endif
  template <typename F>
  auto transform_error(const F& f) const&
  {
    using Cm_Result = result<Value>;

    return std::visit(
      overload(
        [](const value_type& v) { return Cm_Result{v}; },
        [&](const auto& e) { return Cm_Result{f(e)}; }),
      m_value);
  }

  template <typename F>
  auto transform_error(const F& f) &
  {
    using Cm_Result = result<Value>;

    return std::visit(
      overload(
        [](value_type& v) { return Cm_Result{v}; },
        [&](auto& e) { return Cm_Result{f(e)}; }),
      m_value);
  }

  template <typename F>
  auto transform_error(const F& f) &&
  {
    using Cm_Result = result<Value>;

    return std::visit(
      overload(
        [](value_type&& v) { return Cm_Result{std::move(v)}; },
        [&](auto&& e) { return Cm_Result{f(std::forward<decltype(e)>(e))}; }),
      std::move(m_value));
  }
#ifdef _MSC_VER
#pragma warning(pop)
#endif

  /**
   * Applies the given function to any error contained in this result, and returns this
   * result.
   *
   * Use this function for error logging and such, but not for fixing any errors.
   */
  template <typename F>
  auto if_error(const F& f) const&
  {
    std::visit(overload([](const Value&) {}, [&](const auto& e) { f(e); }), m_value);
    return *this;
  }

  /**
   * See the previous function. The only difference is that the value contained in this
   * result is passed to `f` by rvalue reference to allow moving.
   */
  template <typename F>
  auto if_error(const F& f) &
  {
    std::visit(overload([](Value&) {}, [&](auto& e) { f(e); }), m_value);
    return std::move(*this);
  }

  /**
   * See the previous function. The only difference is that the value contained in this
   * result is passed to `f` by rvalue reference to allow moving.
   */
  template <typename F>
  auto if_error(const F& f) &&
  {
    std::visit(
      overload([](Value&&) {}, [&](auto&& e) { f(std::forward<decltype(e)>(e)); }),
      std::move(m_value));
    return std::move(*this);
  }

  /**
   * Returns the value contained in this result if it is successful. Otherwise, throws
   * `bad_result_access`.
   *
   * @return a copy of the value in this result
   *
   * @throw bad_result_access if this result is an error
   */
  const value_type& value() const&
  {
    return std::visit(
      overload(
        [](const value_type& v) -> const value_type& { return v; },
        [](const auto&) -> const value_type& { throw bad_result_access{}; }),
      m_value);
  }

  /**
   * Returns the value contained in this result if it is successful. Otherwise, throws
   * `bad_result_access`.
   *
   * @return the value in this result
   *
   * @throw bad_result_access if this result is an error
   */
  value_type&& value() &&
  {
    return std::visit(
      overload(
        [](value_type&& v) -> value_type&& { return std::move(v); },
        [](const auto&) -> value_type&& { throw bad_result_access{}; }),
      std::move(m_value));
  }

  value_type value() const&&
  {
    return std::visit(
      overload(
        [](value_type& v) -> value_type&& { return std::move(v); },
        [](const auto&) -> value_type&& { throw bad_result_access{}; }),
      std::move(m_value));
  }

  value_type value_or(Value x) const&
  {
    return std::visit(
      overload(
        [](const value_type& v) -> value_type { return v; },
        [&](const auto&) -> value_type { return std::move(x); }),
      m_value);
  }

  value_type value_or(Value x) &&
  {
    return std::visit(
      overload(
        [](value_type&& v) -> value_type { return std::move(v); },
        [&](const auto&) -> value_type { return std::move(x); }),
      std::move(m_value));
  }

  /**
   * Returns a the error contained in this result if it not successful. Otherwise,
   * throws `bad_result_access`.
   *
   * @return a std::variant<Errors...> containing a copy of the error in this result
   *
   * @throw bad_result_access if this result is an error
   */
  auto error() const&
  {
    return std::visit(
      overload(
        [](const value_type&) -> std::variant<Errors...> { throw bad_result_access{}; },
        [](const auto& e) -> std::variant<Errors...> { return e; }),
      m_value);
  }

  /**
   * Returns a the error contained in this result if it not successful. Otherwise,
   * throws `bad_result_access`.
   *
   * @return a std::variant<Errors...> containing a copy of the error in this result
   *
   * @throw bad_result_access if this result is an error
   */
  auto error() &
  {
    return std::visit(
      overload(
        [](value_type&) -> std::variant<Errors...> { throw bad_result_access{}; },
        [](auto& e) -> std::variant<Errors...> { return e; }),
      m_value);
  }

  /**
   * Returns a the error contained in this result if it not successful. Otherwise,
   * throws `bad_result_access`.
   *
   * @return a std::variant<Errors...> containing the error in this result
   *
   * @throw bad_result_access if this result is an error
   */
  auto error() &&
  {
    return std::visit(
      overload(
        [](value_type&&) -> std::variant<Errors...> { throw bad_result_access{}; },
        [](auto&& e) -> std::variant<Errors...> { return std::forward<decltype(e)>(e); }),
      std::move(m_value));
  }

  /**
   * Indicates whether the given result contains a value.
   */
  bool is_success() const
  {
    return m_value.index() == 0u;
  }

  /**
   * Indicates whether the given result contains an error.
   */
  bool is_error() const
  {
    return !is_success();
  }

  /**
   * Indicates whether the given result contains the given type of error.
   */
  template <typename E>
  bool is_error_type() const
  {
    static_assert((... || std::is_convertible_v<E, Errors>), "E must be an error type");

    return std::holds_alternative<E>(m_value);
  }

  friend bool operator==(const result& lhs, const result& rhs)
  {
    return lhs.m_value == rhs.m_value;
  }

  friend bool operator!=(const result& lhs, const result& rhs)
  {
    return !(lhs == rhs);
  }
};

template <typename... Values, typename... Errors>
class [[nodiscard]] result<multi_value<Values...>, Errors...>
{
public:
  using value_type = multi_value<Values...>;
  static constexpr auto error_count = std::tuple_size_v<std::tuple<Errors...>>;

  template <typename OtherValue>
  using with_value_type = result<OtherValue, Errors...>;

private:
  using variant_type = std::variant<value_type, Errors...>;
  variant_type m_value;

  explicit result(variant_type&& v)
    : m_value{std::move(v)}
  {
  }

public:
  /**
   * Creates a new result that wraps the given multi value.
   *
   * v must be convertible to the multi value type or one of the error types of this
   * result. If the value is passed by (const) lvalue reference, it is copied into this
   * result, if it s passed by rvalue reference, then it is moved into this result.
   *
   * @tparam T the type of the value, must match the value type or one of the error types
   * of this result
   * @param v the value
   */
  template <
    typename T,
    typename std::enable_if<std::disjunction_v<
      std::is_convertible<T, multi_value<Values...>>,
      std::is_convertible<T, Errors>...>>::type* = nullptr>
  // NOLINTNEXTLINE
  result(T&& v)
    : m_value{std::forward<T>(v)}
  {
  }

  /**
   * Converting constructor.
   *
   * The given result type must have the same multi value type and a subset of the error
   * types of this result type. The value or error wrapped by the given result is moved
   * into this result.
   *
   * @param other the result to convert
   */
  template <typename... ErrorSubset>
  // NOLINTNEXTLINE
  result(result<multi_value<Values...>, ErrorSubset...> other)
    : m_value{std::move(other).visit(overload(
      [](Values&&... v) -> variant_type {
        return multi_value<Values...>{std::move(v)...};
      },
      [](auto&& e) -> variant_type { return std::forward<decltype(e)>(e); }))}
  {
    static_assert(
      meta_is_subset_v<meta_type_list<ErrorSubset...>, meta_type_list<Errors...>>,
      "Error types of result type to convert must be a subset of target result type");
  }

public:
  /**
   * See result<Value, Errors...>::visit. Note that the visitor must accept
   * multi_value<Values...> instead of the values directly.
   */
  template <typename Visitor>
  auto visit(const Visitor& visitor) const&
  {
    return std::visit(visitor, m_value);
  }

  /**
   * See result<Value, Errors...>::visit. Note that the visitor must accept
   * multi_value<Values...> instead of the values directly.
   */
  template <typename Visitor>
  auto visit(const Visitor& visitor) &
  {
    return std::visit(visitor, m_value);
  }

  /**
   * See result<Value, Errors...>::visit. Note that the visitor must accept
   * multi_value<Values...> instead of the values directly.
   */
  template <typename Visitor>
  auto visit(const Visitor& visitor) &&
  {
    return std::visit(visitor, std::move(m_value));
  }

  /**
   * See result<Value, Errors...>::join.
   */
  template <typename R>
  auto join(R other) const&
  {
    static_assert(is_result_v<R>, "Can only join a result type");
    static_assert(
      !std::is_same_v<typename R::value_type, void>, "Cannot join a void result");

    using My_Result = result<multi_value<Values...>, Errors...>;
    using Cm_Result = typename detail::join_results<My_Result, R>::type;
    using Cm_Value = typename Cm_Result::value_type;

    return std::visit(
      overload(
        [&](const value_type& v) {
          return std::move(other).visit(overload(
            [&](typename R::value_type&& w) {
              return Cm_Result{Cm_Value::make(value_type{v}, std::move(w))};
            },
            [](auto&& e) { return Cm_Result{std::forward<decltype(e)>(e)}; }));
        },
        [](const auto& e) { return Cm_Result{e}; }),
      m_value);
  }

  /**
   * See result<Value, Errors...>::join.
   */
  template <typename R>
  auto join(R other) &
  {
    static_assert(is_result_v<R>, "Can only join a result type");
    static_assert(
      !std::is_same_v<typename R::value_type, void>, "Cannot join a void result");

    using My_Result = result<multi_value<Values...>, Errors...>;
    using Cm_Result = typename detail::join_results<My_Result, R>::type;
    using Cm_Value = typename Cm_Result::value_type;

    return std::visit(
      overload(
        [&](value_type& v) {
          return std::move(other).visit(overload(
            [&](typename R::value_type&& w) {
              return Cm_Result{Cm_Value::make(value_type{v}, std::move(w))};
            },
            [](auto&& e) { return Cm_Result{std::forward<decltype(e)>(e)}; }));
        },
        [](auto& e) { return Cm_Result{e}; }),
      m_value);
  }

  /**
   * See result<Value, Errors...>::join.
   */
  template <typename R>
  auto join(R other) &&
  {
    static_assert(is_result_v<R>, "Can only join a result type");
    static_assert(
      !std::is_same_v<typename R::value_type, void>, "Cannot join a void result");

    using My_Result = result<multi_value<Values...>, Errors...>;
    using Cm_Result = typename detail::join_results<My_Result, R>::type;
    using Cm_Value = typename Cm_Result::value_type;

    return std::visit(
      overload(
        [&](value_type&& v) {
          return std::move(other).visit(overload(
            [&](typename R::value_type&& w) {
              return Cm_Result{Cm_Value::make(std::move(v), std::move(w))};
            },
            [](auto&& e) { return Cm_Result{std::forward<decltype(e)>(e)}; }));
        },
        [](auto&& e) { return Cm_Result{std::forward<decltype(e)>(e)}; }),
      std::move(m_value));
  }

  /**
   * See result<Value, Errors...>::and_then. The only difference is that the given
   * function receives the values stored in this result not as a multi_value, but as
   * individual arguments.
   */
  template <typename F>
  auto and_then(const F& f) const&
  {
    using My_Result = result<multi_value<Values...>, Errors...>;
    using Fn_Result = decltype(f(std::declval<const Values&>()...));

    static_assert(is_result_v<Fn_Result>, "Function must return a result type");

    using Cm_Result = typename detail::chain_results<My_Result, Fn_Result>::type;

    return std::visit(
      overload(
        [&](const value_type& v) { return Cm_Result{std::apply(f, v.values)}; },
        [](const auto& e) { return Cm_Result{e}; }),
      m_value);
  }

  /**
   * See the previous function. The only difference is that the values contained in this
   * result are passed to `f` by non const lvalue reference.
   */
  template <typename F>
  auto and_then(const F& f) &
  {
    using My_Result = result<multi_value<Values...>, Errors...>;
    using Fn_Result = decltype(f(std::declval<Values&>()...));

    static_assert(is_result_v<Fn_Result>, "Function must return a result type");

    using Cm_Result = typename detail::chain_results<My_Result, Fn_Result>::type;

    return std::visit(
      overload(
        [&](value_type& v) { return Cm_Result{std::apply(f, v.values)}; },
        [](auto& e) { return Cm_Result{e}; }),
      m_value);
  }

  /**
   * See the previous function. The only difference is that the values contained in this
   * result are passed to `f` by rvalue reference to allow moving.
   */
  template <typename F>
  auto and_then(const F& f) &&
  {
    using My_Result = result<multi_value<Values...>, Errors...>;
    using Fn_Result = decltype(f(std::declval<Values&&>()...));

    static_assert(is_result_v<Fn_Result>, "Function must return a result type");

    using Cm_Result = typename detail::chain_results<My_Result, Fn_Result>::type;

    return std::visit(
      overload(
        [&](value_type&& v) { return Cm_Result{std::apply(f, std::move(v.values))}; },
        [](auto&& e) { return Cm_Result{std::forward<decltype(e)>(e)}; }),
      std::move(m_value));
  }

  /**
   * See result<Value, Errors...>::or_else. The only difference is that the given
   * function receives the values stored in this result not as a multi_value, but as
   * individual arguments.
   */
  template <typename F>
  auto or_else(const F& f) const&
  {
    static_assert(
      sizeof...(Errors) > 0,
      "Cannot apply or_else to a result type with empty error type list");

    using Fn_Result = decltype(f(std::declval<const meta_front_t<Errors...>&>()));

    static_assert(is_result_v<Fn_Result>, "Function must return a result type");
    static_assert(
      std::is_same_v<typename Fn_Result::value_type, multi_value<Values...>>,
      "Function must return result with same value type");

    return std::visit(
      overload(
        [](const value_type& v) { return Fn_Result{v}; },
        [&](const auto& e) { return f(e); }),
      m_value);
  }

  /**
   * See the previous function. The only difference is that the error contained in this
   * result is passed to `f` by rvalue reference to allow moving.
   */
  template <typename F>
  auto or_else(const F& f) &
  {
    static_assert(
      sizeof...(Errors) > 0,
      "Cannot apply or_else to a result type with empty error type list");

    using Fn_Result = decltype(f(std::declval<meta_front_t<Errors...>&>()));

    static_assert(is_result_v<Fn_Result>, "Function must return a result type");
    static_assert(
      std::is_same_v<typename Fn_Result::value_type, multi_value<Values...>>,
      "Function must return result with same value type");

    return std::visit(
      overload([](value_type& v) { return Fn_Result{v}; }, [&](auto& e) { return f(e); }),
      m_value);
  }

  /**
   * See the previous function. The only difference is that the error contained in this
   * result is passed to `f` by rvalue reference to allow moving.
   */
  template <typename F>
  auto or_else(const F& f) &&
  {
    static_assert(
      sizeof...(Errors) > 0,
      "Cannot apply or_else to a result type with empty error type list");

    using Fn_Result = decltype(f(std::declval<meta_front_t<Errors...>&&>()));

    static_assert(is_result_v<Fn_Result>, "Function must return a result type");
    static_assert(
      std::is_same_v<typename Fn_Result::value_type, multi_value<Values...>>,
      "Function must return result with same value type");

    return std::visit(
      overload(
        [](value_type&& v) { return Fn_Result{std::move(v)}; },
        [&](auto&& e) { return f(std::forward<decltype(e)>(e)); }),
      std::move(m_value));
  }

  template <typename F>
  auto transform(const F& f) const&
  {
    using Fn_Result = decltype(f(std::declval<const Values&>()...));
    using Cm_Result = result<Fn_Result, Errors...>;

    return std::visit(
      overload(
        [&](const value_type& v) {
          if constexpr (std::is_same_v<typename Cm_Result::value_type, void>)
          {
            std::apply(f, v.values);
            return Cm_Result{};
          }
          else
          {
            return Cm_Result{std::apply(f, v.values)};
          }
        },
        [](const auto& e) { return Cm_Result{e}; }),
      m_value);
  }

  template <typename F>
  auto transform(const F& f) &
  {
    using Fn_Result = decltype(f(std::declval<Values&>()...));
    using Cm_Result = result<Fn_Result, Errors...>;

    return std::visit(
      overload(
        [&](value_type& v) {
          if constexpr (std::is_same_v<typename Cm_Result::value_type, void>)
          {
            std::apply(f, v.values);
            return Cm_Result{};
          }
          else
          {
            return Cm_Result{std::apply(f, v.values)};
          }
        },
        [](auto& e) { return Cm_Result{e}; }),
      m_value);
  }

  /**
   * See the previous function. The only difference is that the value contained in this
   * result is passed to `f` by rvalue reference to allow moving.
   */
  template <typename F>
  auto transform(const F& f) &&
  {
    using Fn_Result = decltype(f(std::declval<Values&&>()...));
    using Cm_Result = result<Fn_Result, Errors...>;

    return std::visit(
      overload(
        [&](value_type&& v) {
          if constexpr (std::is_same_v<typename Cm_Result::value_type, void>)
          {
            std::apply(f, std::move(v.values));
            return Cm_Result{};
          }
          else
          {
            return Cm_Result{std::apply(f, std::move(v.values))};
          }
        },
        [](auto&& e) { return Cm_Result{std::forward<decltype(e)>(e)}; }),
      std::move(m_value));
  }

#ifdef _MSC_VER
// MSVC issues an unreachable code warning if the given function throws.
#pragma warning(push)
#pragma warning(disable : 4702)
#endif
  template <typename F>
  auto transform_error(const F& f) const&
  {
    using Cm_Result = result<multi_value<Values...>>;

    return std::visit(
      overload(
        [](const value_type& v) { return Cm_Result{v}; },
        [&](const auto& e) { return Cm_Result{f(e)}; }),
      m_value);
  }

  template <typename F>
  auto transform_error(const F& f) &
  {
    using Cm_Result = result<multi_value<Values...>>;

    return std::visit(
      overload(
        [](value_type& v) { return Cm_Result{v}; },
        [&](auto& e) { return Cm_Result{f(e)}; }),
      m_value);
  }

  template <typename F>
  auto transform_error(const F& f) &&
  {
    using Cm_Result = result<multi_value<Values...>>;

    return std::visit(
      overload(
        [](value_type&& v) { return Cm_Result{std::move(v)}; },
        [&](auto&& e) { return Cm_Result{f(std::forward<decltype(e)>(e))}; }),
      std::move(m_value));
  }
#ifdef _MSC_VER
#pragma warning(pop)
#endif

  /**
   * See result<Value, Errors...>::if_error.
   */
  template <typename F>
  auto if_error(const F& f) const&
  {
    std::visit(overload([](const value_type&) {}, [&](const auto& e) { f(e); }), m_value);
    return *this;
  }

  /**
   * See result<Value, Errors...>::if_error.
   */
  template <typename F>
  auto if_error(const F& f) &
  {
    std::visit(overload([](value_type&) {}, [&](auto& e) { f(e); }), m_value);
    return std::move(*this);
  }

  /**
   * See result<Value, Errors...>::if_error.
   */
  template <typename F>
  auto if_error(const F& f) &&
  {
    std::visit(
      overload([](value_type&&) {}, [&](auto&& e) { f(std::forward<decltype(e)>(e)); }),
      std::move(m_value));
    return std::move(*this);
  }

  /**
   * Returns the values contained in this result as a tuple if it is successful.
   * Otherwise, throws `bad_result_access`.
   *
   * @return a tuple of copies of the values in this result
   *
   * @throw bad_result_access if this result is an error
   */
  const value_type& value() const&
  {
    return std::visit(
      overload(
        [](const value_type& v) -> const value_type& { return v; },
        [](const auto&) -> const value_type& { throw bad_result_access{}; }),
      m_value);
  }

  /**
   * Returns the value contained in this result if it is successful. Otherwise, throws
   * `bad_result_access`.
   *
   * @return the value in this result
   *
   * @throw bad_result_access if this result is an error
   */
  value_type&& value() &&
  {
    return std::visit(
      overload(
        [](value_type&& v) -> value_type&& { return std::move(v); },
        [](const auto&) -> value_type&& { throw bad_result_access{}; }),
      std::move(m_value));
  }

  value_type value() const&&
  {
    return std::visit(
      overload(
        [](value_type& v) -> value_type&& { return std::move(v); },
        [](const auto&) -> value_type&& { throw bad_result_access{}; }),
      std::move(m_value));
  }

  value_type value_or(Values... x) const&
  {
    return std::visit(
      overload(
        [](const value_type& v) -> value_type { return v; },
        [&](const auto&) -> value_type { return multi_value{std::move(x)...}; }),
      m_value);
  }

  value_type value_or(Values... x) &&
  {
    return std::visit(
      overload(
        [](value_type&& v) -> value_type { return std::move(v); },
        [&](const auto&) -> value_type { return multi_value{std::move(x)...}; }),
      std::move(m_value));
  }

  /**
   * Returns a the error contained in this result if it not successful. Otherwise,
   * throws `bad_result_access`.
   *
   * @return a std::variant<Errors...> containing a copy of the error in this result
   *
   * @throw bad_result_access if this result is an error
   */
  auto error() const&
  {
    return std::visit(
      overload(
        [](const value_type&) -> std::variant<Errors...> { throw bad_result_access{}; },
        [](const auto& e) -> std::variant<Errors...> { return e; }),
      m_value);
  }

  /**
   * Returns a the error contained in this result if it not successful. Otherwise,
   * throws `bad_result_access`.
   *
   * @return a std::variant<Errors...> containing a copy of the error in this result
   *
   * @throw bad_result_access if this result is an error
   */
  auto error() &
  {
    return std::visit(
      overload(
        [](value_type&) -> std::variant<Errors...> { throw bad_result_access{}; },
        [](auto& e) -> std::variant<Errors...> { return e; }),
      m_value);
  }

  /**
   * Returns a the error contained in this result if it not successful. Otherwise,
   * throws `bad_result_access`.
   *
   * @return a std::variant<Errors...> containing the error in this result
   *
   * @throw bad_result_access if this result is an error
   */
  auto error() &&
  {
    return std::visit(
      overload(
        [](value_type&&) -> std::variant<Errors...> { throw bad_result_access{}; },
        [](auto&& e) -> std::variant<Errors...> { return std::forward<decltype(e)>(e); }),
      std::move(m_value));
  }

  /**
   * Indicates whether the given result contains a value.
   */
  bool is_success() const
  {
    return m_value.index() == 0u;
  }

  /**
   * Indicates whether the given result contains an error.
   */
  bool is_error() const
  {
    return !is_success();
  }

  /**
   * Indicates whether the given result contains the given type of error.
   */
  template <typename E>
  bool is_error_type() const
  {
    static_assert((... || std::is_convertible_v<E, Errors>), "E must be an error type");

    return std::holds_alternative<E>(m_value);
  }

  friend bool operator==(const result& lhs, const result& rhs)
  {
    return lhs.m_value == rhs.m_value;
  }

  friend bool operator!=(const result& lhs, const result& rhs)
  {
    return !(lhs == rhs);
  }
};

namespace detail
{
struct void_success_value_type
{
  friend bool operator==(const void_success_value_type&, const void_success_value_type&)
  {
    return true;
  }
  friend bool operator!=(const void_success_value_type&, const void_success_value_type&)
  {
    return false;
  }
};
} // namespace detail

/**
 * Wrapper class that can contain only nothing.
 *
 * An instance of this class represents an expectation for the result of applying a
 * function if that function returns void.
 *
 * This result is always considered successful.
 */
template <>
class result<void>
{
public:
  using value_type = void;
  static constexpr auto error_count = size_t{0};

  template <typename OtherValue>
  using with_value_type = result<OtherValue>;

public:
  /**
   * Applies the given visitor this result.
   *
   * The given visitor must accept void.
   *
   * @tparam Visitor the type of the visitor
   * @param visitor the visitor to apply
   * @return the value returned by the given visitor or void if the given visitor does
   * not return anything
   */
  template <typename Visitor>
  auto visit(const Visitor& visitor) const
  {
    return visitor();
  }

  /**
   * See result<Value, Errors...>::and_then.
   */
  template <typename F>
  auto and_then(const F& f) const
  {
    using Fn_Result = decltype(f());

    static_assert(is_result_v<Fn_Result>, "Function must return a result type");

    return f();
  }

  /**
   * See result<Value, Errors...>::transform.
   */
  template <typename F>
  auto transform(const F& f) const
  {
    using Fn_Result = decltype(f());
    using Cm_Result = result<Fn_Result>;

    if constexpr (std::is_same_v<typename Cm_Result::value_type, void>)
    {
      f();
      return Cm_Result{};
    }
    else
    {
      return Cm_Result{f()};
    }
  }

  /**
   * Indicates whether this result is empty. Always true.
   */
  bool is_success() const { return true; }

  /**
   * Indicates whether this result contains an error. Always false.
   */
  bool is_error() const { return !is_success(); }

  friend bool operator==(const result&, const result&) { return true; }

  friend bool operator!=(const result& lhs, const result& rhs) { return !(lhs == rhs); }
};


constexpr auto void_success = result<void>{};

/**
 * Wrapper class that can contain either nothing or one of several errors.
 *
 * An instance of this class represents an expectation for the result of applying a
 * function if that function returns void or throws an error.
 *
 * An result is considered successful if it is empty, and a failure if it contains an
 * error.
 *
 * @tparam Errors the types of the possible errors
 */
template <typename... Errors>
class [[nodiscard]] result<void, Errors...>
{
public:
  using value_type = void;
  static constexpr auto error_count = std::tuple_size_v<std::tuple<Errors...>>;

  template <typename OtherValue>
  using with_value_type = result<OtherValue, Errors...>;

private:
  using variant_type = std::variant<detail::void_success_value_type, Errors...>;
  variant_type m_value;

  explicit result(variant_type&& v)
    : m_value{std::move(v)}
  {
  }

public:
  /**
   * Creates a new successful result.
   */
  constexpr result()
    : m_value(detail::void_success_value_type{})
  {
  }

  /**
   * Creates a new result that wraps the given value.
   *
   * v must be convertible to detail::void_success_value_type or one of the error types
   * of this result. If the value is passed by (const) lvalue reference, it is copied
   * into this result, if it s passed by rvalue reference, then it is moved into this
   * result.
   *
   * @tparam T the type of the value, must match detail::void_success_value_type or one
   * of the error types of this result
   * @param v the value
   */
  template <
    typename T,
    typename std::enable_if<std::disjunction_v<
      std::is_convertible<T, value_type>,
      std::is_convertible<T, Errors>...>>::type* = nullptr>
  // NOLINTNEXTLINE
  result(T&& v)
    : m_value{std::forward<T>(v)}
  {
  }

  /**
   * Converting constructor.
   *
   * The given result type must have void as its value type and a subset of the error
   * types of this result type. The value or error wrapped by the given result is moved
   * into this result.
   *
   * @param other the result to convert
   */
  template <typename... ErrorSubset>
  // NOLINTNEXTLINE
  result(result<void, ErrorSubset...> other)
  {
    static_assert(
      meta_is_subset_v<meta_type_list<ErrorSubset...>, meta_type_list<Errors...>>,
      "Error types of result type to convert must be a subset of target result type");
    std::move(other).visit(overload(
      [&]() { m_value = detail::void_success_value_type{}; },
      [&](auto&& e) { m_value = std::forward<decltype(e)>(e); }));
  }

public:
  /**
   * Applies the given visitor this result.
   *
   * The given visitor must accept void and all error types of this result.
   * The error contained in this result is passed to the visitor by const lvalue
   * reference.
   *
   * @tparam Visitor the type of the visitor
   * @param visitor the visitor to apply
   * @return the value returned by the given visitor or void if the given visitor does
   * not return anything
   */
  template <typename Visitor>
  auto visit(const Visitor& visitor) const&
  {
    return std::visit(
      overload(
        [&](const detail::void_success_value_type&) { return visitor(); },
        [&](const auto& e) { return visitor(e); }),
      m_value);
  }

  /**
   * Applies the given visitor this result.
   *
   * The given visitor must accept void and all error types of this result.
   * The error contained in this result is passed to the visitor by non const lvalue
   * reference.
   *
   * @tparam Visitor the type of the visitor
   * @param visitor the visitor to apply
   * @return the value returned by the given visitor or void if the given visitor does
   * not return anything
   */
  template <typename Visitor>
  auto visit(const Visitor& visitor) &
  {
    return std::visit(
      overload(
        [&](detail::void_success_value_type&) { return visitor(); },
        [&](auto& e) { return visitor(e); }),
      m_value);
  }

  /**
   * Visits the value or error contained in this result.
   *
   * The given visitor must accept the value type and all error types of this result.
   * The value or error contained in this result is passed to the visitor by rvalue
   * reference.
   *
   * @tparam Visitor the type of the visitor
   * @param visitor the visitor to apply
   * @return the value returned by the given visitor or void if the given visitor does
   * not return anything
   */
  template <typename Visitor>
  auto visit(const Visitor& visitor) &&
  {
    return std::visit(
      overload(
        [&](detail::void_success_value_type&&) { return visitor(); },
        [&](auto&& e) { return visitor(std::forward<decltype(e)>(e)); }),
      std::move(m_value));
  }

  /**
   * See result<Value, Errors...>::and_then.
   */
  template <typename F>
  auto and_then(const F& f) const&
  {
    using My_Result = result<void, Errors...>;
    using Fn_Result = decltype(f());

    static_assert(is_result_v<Fn_Result>, "Function must return a result type");

    using Cm_Result = typename detail::chain_results<My_Result, Fn_Result>::type;

    return std::visit(
      overload(
        [&](const detail::void_success_value_type&) { return Cm_Result{f()}; },
        [](const auto& e) { return Cm_Result{e}; }),
      m_value);
  }

  /**
   * See result<Value, Errors...>::and_then.
   */
  template <typename F>
  auto and_then(const F& f) &&
  {
    using My_Result = result<void, Errors...>;
    using Fn_Result = decltype(f());

    static_assert(is_result_v<Fn_Result>, "Function must return a result type");

    using Cm_Result = typename detail::chain_results<My_Result, Fn_Result>::type;

    return std::visit(
      overload(
        [&](detail::void_success_value_type&&) { return Cm_Result{f()}; },
        [](auto&& e) { return Cm_Result{std::forward<decltype(e)>(e)}; }),
      std::move(m_value));
  }

  /**
   * See result<Value, Errors...>::or_else.
   */
  template <typename F>
  auto or_else(const F& f) const&
  {
    static_assert(
      sizeof...(Errors) > 0,
      "Cannot apply or_else to a result type with empty error type list");

    using Fn_Result = decltype(f(std::declval<const meta_front_t<Errors...>&>()));

    static_assert(is_result_v<Fn_Result>, "Function must return a result type");
    static_assert(
      std::is_same_v<typename Fn_Result::value_type, void>,
      "Function must return void result");

    return std::visit(
      overload(
        [](const detail::void_success_value_type&) { return Fn_Result{}; },
        [&](const auto& e) { return f(e); }),
      m_value);
  }

  /**
   * See result<Value, Errors...>::or_else.
   */
  template <typename F>
  auto or_else(const F& f) &
  {
    static_assert(
      sizeof...(Errors) > 0,
      "Cannot apply or_else to a result type with empty error type list");

    using Fn_Result = decltype(f(std::declval<meta_front_t<Errors...>&>()));

    static_assert(is_result_v<Fn_Result>, "Function must return a result type");
    static_assert(
      std::is_same_v<typename Fn_Result::value_type, void>,
      "Function must return void result");

    return std::visit(
      overload(
        [](detail::void_success_value_type&) { return Fn_Result{}; },
        [&](auto& e) { return f(e); }),
      m_value);
  }

  /**
   * See result<Value, Errors...>::or_else.
   */
  template <typename F>
  auto or_else(const F& f) &&
  {
    static_assert(
      sizeof...(Errors) > 0,
      "Cannot apply or_else to a result type with empty error type list");

    using Fn_Result = decltype(f(std::declval<meta_front_t<Errors...>&&>()));

    static_assert(is_result_v<Fn_Result>, "Function must return a result type");
    static_assert(
      std::is_same_v<typename Fn_Result::value_type, void>,
      "Function must return void result");

    return std::visit(
      overload(
        [](detail::void_success_value_type&&) { return Fn_Result{}; },
        [&](auto&& e) { return f(std::forward<decltype(e)>(e)); }),
      std::move(m_value));
  }

  /**
   * See result<Value, Errors...>::transform.
   */
  template <typename F>
  auto transform(const F& f) const&
  {
    using Fn_Result = decltype(f());
    using Cm_Result = result<Fn_Result, Errors...>;

    return std::visit(
      overload(
        [&](const detail::void_success_value_type&) {
          if constexpr (std::is_same_v<Fn_Result, void>)
          {
            f();
            return Cm_Result{};
          }
          else
          {
            return Cm_Result{f()};
          }
        },
        [](const auto& e) { return Cm_Result{e}; }),
      m_value);
  }

  /**
   * See result<Value, Errors...>::transform.
   */
  template <typename F>
  auto transform(const F& f) &&
  {
    using Fn_Result = decltype(f());
    using Cm_Result = result<Fn_Result, Errors...>;

    return std::visit(
      overload(
        [&](detail::void_success_value_type&&) {
          if constexpr (std::is_same_v<Fn_Result, void>)
          {
            f();
            return Cm_Result{};
          }
          else
          {
            return Cm_Result{f()};
          }
        },
        [](auto&& e) { return Cm_Result{std::forward<decltype(e)>(e)}; }),
      std::move(m_value));
  }

#ifdef _MSC_VER
// MSVC issues an unreachable code warning if the given function throws.
#pragma warning(push)
#pragma warning(disable : 4702)
#endif
  template <typename F>
  auto transform_error(const F& f) const&
  {
    using Cm_Result = result<void>;

    return std::visit(
      overload(
        [](const detail::void_success_value_type&) { return Cm_Result{}; },
        [&](const auto& e) {
          f(e);
          return Cm_Result{};
        }),
      m_value);
  }

  template <typename F>
  auto transform_error(const F& f) &
  {
    using Cm_Result = result<void>;

    return std::visit(
      overload(
        [](detail::void_success_value_type&) { return Cm_Result{}; },
        [&](auto& e) {
          f(e);
          return Cm_Result{};
        }),
      m_value);
  }

  template <typename F>
  auto transform_error(const F& f) &&
  {
    using Cm_Result = result<void>;

    return std::visit(
      overload(
        [](detail::void_success_value_type&&) { return Cm_Result{}; },
        [&](auto&& e) {
          f(std::forward<decltype(e)>(e));
          return Cm_Result{};
        }),
      std::move(m_value));
  }
#ifdef _MSC_VER
#pragma warning(pop)
#endif

  /**
   * See result<Value, Errors...>::if_error.
   */
  template <typename F>
  auto if_error(const F& f) const&
  {
    std::visit(
      overload(
        [](const detail::void_success_value_type&) { return true; },
        [&](const auto& e) {
          f(e);
          return false;
        }),
      m_value);
    return *this;
  }

  /**
   * See result<Value, Errors...>::if_error.
   */
  template <typename F>
  auto if_error(const F& f) &&
  {
    std::visit(
      overload(
        [](detail::void_success_value_type&&) {},
        [&](auto&& e) { f(std::forward<decltype(e)>(e)); }),
      std::move(m_value));
    return *this;
  }

  /**
   * Returns a the error contained in this result if it not successful. Otherwise,
   * throws `bad_result_access`.
   *
   * @return a std::variant<Errors...> containing a copy of the error in this result
   *
   * @throw bad_result_access if this result is an error
   */
  auto error() const&
  {
    return std::visit(
      overload(
        [](const detail::void_success_value_type&) -> std::variant<Errors...> {
          throw bad_result_access{};
        },
        [](const auto& e) -> std::variant<Errors...> { return e; }),
      m_value);
  }

  /**
   * Returns a the error contained in this result if it not successful. Otherwise,
   * throws `bad_result_access`.
   *
   * @return a std::variant<Errors...> containing a copy of the error in this result
   *
   * @throw bad_result_access if this result is an error
   */
  auto error() &
  {
    return std::visit(
      overload(
        [](detail::void_success_value_type&) -> std::variant<Errors...> {
          throw bad_result_access{};
        },
        [](auto& e) -> std::variant<Errors...> { return e; }),
      m_value);
  }

  /**
   * Returns a the error contained in this result if it not successful. Otherwise,
   * throws `bad_result_access`.
   *
   * @return a std::variant<Errors...> containing the error in this result
   *
   * @throw bad_result_access if this result is an error
   */
  auto error() &&
  {
    return std::visit(
      overload(
        [](detail::void_success_value_type&&) -> std::variant<Errors...> {
          throw bad_result_access{};
        },
        [](auto&& e) -> std::variant<Errors...> { return std::forward<decltype(e)>(e); }),
      std::move(m_value));
  }

  /**
   * Indicates whether this result is empty.
   */
  bool is_success() const
  {
    return std::holds_alternative<detail::void_success_value_type>(m_value);
  }

  /**
   * Indicates whether this result contains an error.
   */
  bool is_error() const
  {
    return !is_success();
  }

  /**
   * Indicates whether the given result contains the given type of error.
   */
  template <typename E>
  bool is_error_type() const
  {
    static_assert((... || std::is_convertible_v<E, Errors>), "E must be an error type");

    return std::holds_alternative<E>(m_value);
  }

  friend bool operator==(const result& lhs, const result& rhs)
  {
    return lhs.m_value == rhs.m_value;
  }

  friend bool operator!=(const result& lhs, const result& rhs)
  {
    return !(lhs == rhs);
  }
};
} // namespace kdl
