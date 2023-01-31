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

namespace detail
{
template <typename Value, typename Errors>
struct make_result_type
{
};

template <typename Value, typename... Errors>
struct make_result_type<Value, kdl::meta_type_list<Errors...>>
{
  using type = kdl::result<Value, Errors...>;
};

template <typename Result>
struct is_result : public std::false_type
{
};

template <typename Value, typename... Errors>
struct is_result<kdl::result<Value, Errors...>> : public std::true_type
{
};

template <typename Result1, typename Result2>
struct chain_results
{
};

template <typename Value1, typename... Errors1, typename Value2, typename... Errors2>
struct chain_results<kdl::result<Value1, Errors1...>, kdl::result<Value2, Errors2...>>
{
  using result = typename make_result_type<
    Value2,
    typename meta_remove_duplicates<Errors1..., Errors2...>::result>::type;
};
} // namespace detail

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
  {
    static_assert(
      meta_is_subset<meta_type_list<ErrorSubset...>, meta_type_list<Errors...>>::value,
      "Error types of result type to convert must be a subset of target result type");
    std::move(other).visit(overload(
      [&](Value&& v) { m_value = std::move(v); },
      [&](auto&& e) { m_value = std::forward<decltype(e)>(e); }));
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
  auto visit(Visitor&& visitor) const&
  {
    return std::visit(std::forward<Visitor>(visitor), m_value);
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
  auto visit(Visitor&& visitor) &&
  {
    return std::visit(std::forward<Visitor>(visitor), std::move(m_value));
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
  auto and_then(F&& f) const&
  {
    using My_Result = result<Value, Errors...>;
    using Fn_Result = std::invoke_result_t<F, Value>;

    if constexpr (detail::is_result<Fn_Result>::value)
    {
      using Cm_Result = typename detail::chain_results<My_Result, Fn_Result>::result;
      using Fn_Value = typename Fn_Result::value_type;

      if constexpr (std::is_same_v<Fn_Value, void>)
      {
        return visit(kdl::overload(
          [&](const value_type& v) {
            return f(v).visit(kdl::overload(
              []() { return Cm_Result{}; },
              [](auto&& fn_e) { return Cm_Result{std::forward<decltype(fn_e)>(fn_e)}; }));
          },
          [](const auto& e) { return Cm_Result{e}; }));
      }
      else
      {
        return visit(kdl::overload(
          [&](const value_type& v) {
            return f(v).visit(kdl::overload(
              [](Fn_Value&& fn_v) { return Cm_Result{std::move(fn_v)}; },
              [](auto&& fn_e) { return Cm_Result{std::forward<decltype(fn_e)>(fn_e)}; }));
          },
          [](const auto& e) { return Cm_Result{e}; }));
      }
    }
    else
    {
      using Cm_Result = kdl::result<Fn_Result, Errors...>;

      if constexpr (std::is_same_v<Fn_Result, void>)
      {
        return visit(kdl::overload(
          [&](const value_type& v) {
            f(v);
            return Cm_Result{};
          },
          [](const auto& e) { return Cm_Result{e}; }));
      }
      else
      {
        return visit(kdl::overload(
          [&](const value_type& v) { return Cm_Result{f(v)}; },
          [](const auto& e) { return Cm_Result{e}; }));
      }
    }
  }

  /**
   * See the previous function. The only difference is that the value contained in this
   * result is passed to `f` by rvalue reference to allow moving.
   */
  template <typename F>
  auto and_then(F&& f) &&
  {
    using My_Result = result<Value, Errors...>;
    using Fn_Result = std::invoke_result_t<F, Value>;

    if constexpr (detail::is_result<Fn_Result>::value)
    {
      using Cm_Result = typename detail::chain_results<My_Result, Fn_Result>::result;
      using Fn_Value = typename Fn_Result::value_type;

      if constexpr (std::is_same_v<Fn_Value, void>)
      {
        return std::move(*this).visit(kdl::overload(
          [&](value_type&& v) {
            return f(std::move(v))
              .visit(kdl::overload(
                []() { return Cm_Result{}; },
                [](auto&& fn_e) {
                  return Cm_Result{std::forward<decltype(fn_e)>(fn_e)};
                }));
          },
          [](auto&& e) { return Cm_Result{e}; }));
      }
      else
      {
        return std::move(*this).visit(kdl::overload(
          [&](value_type&& v) {
            return f(std::move(v))
              .visit(kdl::overload(
                [](Fn_Value&& fn_v) { return Cm_Result{std::move(fn_v)}; },
                [](auto&& fn_e) {
                  return Cm_Result{std::forward<decltype(fn_e)>(fn_e)};
                }));
          },
          [](auto&& e) { return Cm_Result{e}; }));
      }
    }
    else
    {
      using Cm_Result = kdl::result<Fn_Result, Errors...>;

      if constexpr (std::is_same_v<Fn_Result, void>)
      {
        return std::move(*this).visit(kdl::overload(
          [&](value_type&& v) {
            f(std::move(v));
            return Cm_Result{};
          },
          [](auto&& e) { return Cm_Result{std::forward<decltype(e)>(e)}; }));
      }
      else
      {
        return std::move(*this).visit(kdl::overload(
          [&](value_type&& v) { return Cm_Result{f(std::move(v))}; },
          [](auto&& e) { return Cm_Result{std::forward<decltype(e)>(e)}; }));
      }
    }
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
  auto or_else(F&& f) const&
  {
    static_assert(
      std::tuple_size_v<std::tuple<Errors...>> > 0,
      "Cannot apply or_else to a result type with empty error type list");

    using Fn_Result = std::invoke_result_t<F, meta_front_v<Errors...>>;

    if constexpr (detail::is_result<Fn_Result>::value)
    {
      using Cm_Result = Fn_Result;

      static_assert(
        std::is_same_v<typename Cm_Result::value_type, Value>,
        "Function must return result with same value type");

      return visit(kdl::overload(
        [](const value_type& v) { return Cm_Result{v}; },
        [&](const auto& e) { return f(e); }));
    }
    else
    {
      static_assert(std::is_same_v<Fn_Result, Value>, "Function must return value type");

      using Cm_Result = kdl::result<Fn_Result>;

      return visit(kdl::overload(
        [](const value_type& v) { return Cm_Result{v}; },
        [&](const auto& e) { return Cm_Result{f(e)}; }));
    }
  }

  /**
   * See the previous function. The only difference is that the value contained in this
   * result is passed to `f` by rvalue reference to allow moving.
   */
  template <typename F>
  auto or_else(F&& f) &&
  {
    static_assert(
      std::tuple_size_v<std::tuple<Errors...>> > 0,
      "Cannot apply or_else to a result type with empty error type list");

    using Fn_Result = std::invoke_result_t<F, meta_front_v<Errors...>>;

    if constexpr (detail::is_result<Fn_Result>::value)
    {
      using Cm_Result = Fn_Result;

      static_assert(
        std::is_same_v<typename Cm_Result::value_type, Value>,
        "Function must return value type");

      return std::move(*this).visit(kdl::overload(
        [](value_type&& v) { return Cm_Result{std::move(v)}; },
        [&](auto&& e) { return f(std::forward<decltype(e)>(e)); }));
    }
    else
    {
      static_assert(std::is_same_v<Fn_Result, Value>, "Function must return value type");

      using Cm_Result = kdl::result<Fn_Result>;

      return std::move(*this).visit(kdl::overload(
        [](value_type&& v) { return Cm_Result{std::move(v)}; },
        [&](auto&& e) { return Cm_Result{f(std::forward<decltype(e)>(e))}; }));
    }
  }

  /**
   * Maps the error types of this result type to different error types using the given
   * function.
   *
   * The given function must return a result type with the same (success) value type as
   * this result. Let f_result_type be the result type returned by the given function.
   *
   * - If this result is successful, then its value is moved into in an instance of
   * f_result_type and returned.
   * - If this result is not successful, then its error is passed by rvalue reference to
   * the given function and the result returned by the function is returned by this
   * function.
   *
   * Example:
   *
   * auto r = result<int, std::string>{"error"};
   * auto x = std::move(r).map_errors([](std::string&& e) { return result<int,
   * float>{7.0f}; });
   *
   * Then x is a failure result of type result<int, float> that holds 7.0f as its error
   * value.
   *
   * @tparam F the type of the given function
   * @param f the error mapping function
   * @return a result wrapping either the success value or the mapped error value
   */
  template <typename F>
  auto map_errors(F&& f) &&
  {
    using first_error_type = typename meta_front<Errors...>::front;
    using f_result_type = std::invoke_result_t<F, first_error_type>;

    static_assert(
      detail::is_result<f_result_type>::value, "Function must return result type");
    static_assert(
      std::is_same_v<typename f_result_type::value_type, value_type>,
      "Function must return a result type with matching value type");

    return std::move(*this).visit(kdl::overload(
      [&](value_type&& v) { return f_result_type{std::move(v)}; },
      [&](auto&& e) { return f(std::forward<decltype(e)>(e)); }));
  }

  /**
   * Maps the error types of this result type to different error types using the given
   * function.
   *
   * The given function must return a result type with the same (success) value type as
   * this result. Let f_result_type be the result type returned by the given function.
   *
   * - If this result is successful, then its value is copied into in an instance of
   * f_result_type and returned.
   * - If this result is not successful, then its error is passed by const lvalue
   * reference to the given function and the result returned by the function is returned
   * by this function.
   *
   * Example:
   *
   * auto r = result<int, std::string>{"error"};
   * auto x = std::move(r).map_errors([](std::string&& e) { return result<int,
   * float>{7.0f}; });
   *
   * Then x is a failure result of type result<int, float> that holds 7.0f as its error
   * value.
   *
   * @tparam F the type of the given function
   * @param f the error mapping function
   * @return a result wrapping either the success value or the mapped error value
   */
  template <typename F>
  auto map_errors(F&& f) const&
  {
    using first_error_type = typename meta_front<Errors...>::front;
    using f_result_type = std::invoke_result_t<F, first_error_type>;

    static_assert(
      detail::is_result<f_result_type>::value, "Function must return result type");
    static_assert(
      std::is_same_v<typename f_result_type::value_type, value_type>,
      "Function must return a result type with matching value type");

    return visit(kdl::overload(
      [&](const value_type& v) { return f_result_type{v}; },
      [&](const auto& e) { return f(e); }));
  }

  /**
   * Applies the given function to the error contained in this result.
   *
   * Does nothing if this result does not contain an error.
   *
   * Passes the error contained in this result by rvalue reference to the given function.
   *
   * @tparam F the type of the function
   * @param f the function
   * @return true if this result is successful and false otherwise
   */
  template <typename F>
  bool handle_errors(F&& f) &&
  {
    return std::move(*this).visit(kdl::overload(
      [](const value_type&) { return true; },
      [&](auto&& error) {
        f(std::forward<decltype(error)>(error));
        return false;
      }));
  }

  /**
   * Applies the given function to the error contained in this result.
   *
   * Does nothing if this result does not contain an error.
   *
   * Passes the error contained in this result by const lvalue reference to the given
   * function.
   *
   * @tparam F the type of the function
   * @param f the function
   * @return true if this result is successful and false otherwise
   */
  template <typename F>
  bool handle_errors(F&& f) const&
  {
    return visit(kdl::overload(
      [](const value_type&) { return true; },
      [&](const auto& error) {
        f(error);
        return false;
      }));
  }

  /**
   * Returns the value contained in this result if it is successful. Otherwise, throws
   * `bad_result_access`.
   *
   * @return a copy of the value in this result
   *
   * @throw bad_result_access if this result is an error
   */
  auto value() const&
  {
    return visit(kdl::overload(
      [](const value_type& v) -> value_type { return v; },
      [](const auto&) -> value_type { throw bad_result_access{}; }));
  }

  /**
   * Returns the value contained in this result if it is successful. Otherwise, throws
   * `bad_result_access`.
   *
   * @return the value in this result
   *
   * @throw bad_result_access if this result is an error
   */
  auto value() &&
  {
    return std::move(*this).visit(kdl::overload(
      [](value_type&& v) -> value_type { return std::move(v); },
      [](const auto&) -> value_type { throw bad_result_access{}; }));
  }

  /**
   * Returns a the error contained in this result if it not successful. Otherwise, throws
   * `bad_result_access`.
   *
   * @return a std::variant<Errors...> containing a copy of the error in this result
   *
   * @throw bad_result_access if this result is an error
   */
  auto error() const&
  {
    return visit(kdl::overload(
      [](const value_type&) -> std::variant<Errors...> { throw bad_result_access{}; },
      [](const auto& e) -> std::variant<Errors...> { return e; }));
  }

  /**
   * Returns a the error contained in this result if it not successful. Otherwise, throws
   * `bad_result_access`.
   *
   * @return a std::variant<Errors...> containing the error in this result
   *
   * @throw bad_result_access if this result is an error
   */
  auto error() &&
  {
    return visit(kdl::overload(
      [](const value_type&) -> std::variant<Errors...> { throw bad_result_access{}; },
      [](auto&& e) -> std::variant<Errors...> { return std::forward<decltype(e)>(e); }));
  }

  /**
   * Indicates whether the given result contains a value.
   */
  bool is_success() const { return m_value.index() == 0u; }

  /**
   * Indicates whether the given result contains an error.
   */
  bool is_error() const { return !is_success(); }

  /**
   * Indicates whether the given result contains the given type of error.
   */
  template <typename E>
  bool is_error_type() const
  {
    static_assert((... || std::is_convertible_v<E, Errors>), "E must be an error type");

    return std::holds_alternative<E>(m_value);
  }

  /**
   * Indicates whether the given result contains a value.
   */
  // NOLINTNEXTLINE
  operator bool() const { return is_success(); }

  friend bool operator==(const result& lhs, const result& rhs)
  {
    return lhs.m_value == rhs.m_value;
  }

  friend bool operator!=(const result& lhs, const result& rhs) { return !(lhs == rhs); }
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
   * @return the value returned by the given visitor or void if the given visitor does not
   * return anything
   */
  template <typename Visitor>
  auto visit(Visitor&& visitor) const&
  {
    return visitor();
  }

  /**
   * Applies the given visitor this result.
   *
   * See above.
   */
  template <typename Visitor>
  auto visit(Visitor&& visitor) &&
  {
    return visitor();
  }

  /**
   * See result<Value, Errors...>::and_then.
   */
  template <typename F>
  auto and_then(F&& f) const&
  {
    return f();
  }

  /**
   * See result<Value, Errors...>::and_then.
   */
  template <typename F>
  auto and_then(F&& f) &&
  {
    return f();
  }

  /**
   * Indicates whether this result is empty. Always true.
   */
  bool is_success() const { return true; }

  /**
   * Indicates whether this result contains an error. Always false.
   */
  bool is_error() const { return !is_success(); }

  /**
   * Indicates whether this result is empty.
   */
  // NOLINTNEXTLINE
  operator bool() const { return is_success(); }

  friend bool operator==(const result&, const result&) { return true; }

  friend bool operator!=(const result& lhs, const result& rhs) { return !(lhs == rhs); }
};


constexpr auto void_success = kdl::result<void>{};

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
   * v must be convertible to detail::void_success_value_type or one of the error types of
   * this result. If the value is passed by (const) lvalue reference, it is copied into
   * this result, if it s passed by rvalue reference, then it is moved into this result.
   *
   * @tparam T the type of the value, must match detail::void_success_value_type or one of
   * the error types of this result
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
      meta_is_subset<meta_type_list<ErrorSubset...>, meta_type_list<Errors...>>::value,
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
   * @return the value returned by the given visitor or void if the given visitor does not
   * return anything
   */
  template <typename Visitor>
  auto visit(Visitor&& visitor) const&
  {
    return std::visit(
      kdl::overload(
        [&](const detail::void_success_value_type&) { return visitor(); },
        [&](const auto& e) { return visitor(e); }),
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
   * @return the value returned by the given visitor or void if the given visitor does not
   * return anything
   */
  template <typename Visitor>
  auto visit(Visitor&& visitor) &&
  {
    return std::visit(
      kdl::overload(
        [&](detail::void_success_value_type&&) { return visitor(); },
        [&](auto&& e) { return visitor(std::forward<decltype(e)>(e)); }),
      std::move(m_value));
  }

  /**
   * See result<Value, Errors...>::and_then.
   */
  template <typename F>
  auto and_then(F&& f) const&
  {
    using My_Result = result<void, Errors...>;
    using Fn_Result = std::invoke_result_t<F>;

    if constexpr (detail::is_result<Fn_Result>::value)
    {
      using Cm_Result = typename detail::chain_results<My_Result, Fn_Result>::result;
      using Fn_Value = typename Fn_Result::value_type;

      if constexpr (std::is_same_v<Fn_Value, void>)
      {
        return visit(kdl::overload(
          [&]() {
            return f().visit(kdl::overload(
              []() { return Cm_Result{}; },
              [](auto&& fn_e) { return Cm_Result{std::forward<decltype(fn_e)>(fn_e)}; }));
          },
          [](const auto& e) { return Cm_Result{e}; }));
      }
      else
      {
        return visit(kdl::overload(
          [&]() {
            return f().visit(kdl::overload(
              [](Fn_Value&& fn_v) { return Cm_Result{std::move(fn_v)}; },
              [](auto&& fn_e) { return Cm_Result(std::forward<decltype(fn_e)>(fn_e)); }));
          },
          [](const auto& e) { return Cm_Result{e}; }));
      }
    }
    else
    {
      using Cm_Result = kdl::result<Fn_Result, Errors...>;

      if constexpr (std::is_same_v<Fn_Result, void>)
      {
        return visit(kdl::overload(
          [&]() {
            f();
            return Cm_Result{};
          },
          [](const auto& e) { return Cm_Result{e}; }));
      }
      else
      {
        return visit(kdl::overload(
          [&]() { return Cm_Result{f()}; }, [](const auto& e) { return Cm_Result{e}; }));
      }
    }
  }

  /**
   * See result<Value, Errors...>::and_then.
   */
  template <typename F>
  auto and_then(F&& f) &&
  {
    using My_Result = result<void, Errors...>;
    using Fn_Result = std::invoke_result_t<F>;

    if constexpr (detail::is_result<Fn_Result>::value)
    {
      using Cm_Result = typename detail::chain_results<My_Result, Fn_Result>::result;
      using Fn_Value = typename Fn_Result::value_type;

      if constexpr (std::is_same_v<Fn_Value, void>)
      {
        return std::move(*this).visit(kdl::overload(
          [&]() {
            return f().visit(kdl::overload(
              []() { return Cm_Result{}; },
              [](auto&& fn_e) { return Cm_Result{std::forward<decltype(fn_e)>(fn_e)}; }));
          },
          [](auto&& e) { return Cm_Result{e}; }));
      }
      else
      {
        return std::move(*this).visit(kdl::overload(
          [&]() {
            return f().visit(kdl::overload(
              [](Fn_Value&& fn_v) { return Cm_Result{std::move(fn_v)}; },
              [](auto&& fn_e) { return Cm_Result{std::forward<decltype(fn_e)>(fn_e)}; }));
          },
          [](auto&& e) { return Cm_Result{e}; }));
      }
    }
    else
    {
      using Cm_Result = kdl::result<Fn_Result, Errors...>;

      if constexpr (std::is_same_v<Fn_Result, void>)
      {
        return std::move(*this).visit(kdl::overload(
          [&]() {
            f();
            return Cm_Result{};
          },
          [](auto&& e) { return Cm_Result{std::forward<decltype(e)>(e)}; }));
      }
      else
      {
        return std::move(*this).visit(kdl::overload(
          [&]() { return Cm_Result{f()}; },
          [](auto&& e) { return Cm_Result{std::forward<decltype(e)>(e)}; }));
      }
    }
  }

  /**
   * See result<Value, Errors...>::or_else.
   */
  template <typename F>
  auto or_else(F&& f) const&
  {
    static_assert(
      std::tuple_size_v<std::tuple<Errors...>> > 0,
      "Cannot apply or_else to a result type with empty error type list");

    using Fn_Result = std::invoke_result_t<F, meta_front_v<Errors...>>;

    if constexpr (detail::is_result<Fn_Result>::value)
    {
      using Cm_Result = Fn_Result;

      static_assert(
        std::is_same_v<typename Cm_Result::value_type, void>,
        "Function must return void result");

      return visit(
        kdl::overload([]() { return Cm_Result{}; }, [&](const auto& e) { return f(e); }));
    }
    else
    {
      static_assert(std::is_same_v<Fn_Result, void>, "Function must return void");

      visit(kdl::overload([]() {}, [&](const auto& e) { f(e); }));
      return result<void>{};
    }
  }

  /**
   * See result<Value, Errors...>::or_else.
   */
  template <typename F>
  auto or_else(F&& f) &&
  {
    static_assert(
      std::tuple_size_v<std::tuple<Errors...>> > 0,
      "Cannot apply or_else to a result type with empty error type list");

    using Fn_Result = std::invoke_result_t<F, meta_front_v<Errors...>>;

    if constexpr (detail::is_result<Fn_Result>::value)
    {
      using Cm_Result = Fn_Result;

      return std::move(*this).visit(kdl::overload(
        []() { return Cm_Result{}; },
        [&](auto&& e) { return f(std::forward<decltype(e)>(e)); }));
    }
    else
    {
      static_assert(std::is_same_v<Fn_Result, void>, "Function must return void");

      std::move(*this).visit(
        kdl::overload([]() {}, [&](auto&& e) { f(std::forward<decltype(e)>(e)); }));
      return result<void>{};
    }
  }

  /**
   * See result<Value, Errors...>::map_errors.
   */
  template <typename F>
  auto map_errors(F&& f) &&
  {
    using first_error_type = typename meta_front<Errors...>::front;
    using f_result_type = std::invoke_result_t<F, first_error_type>;

    static_assert(
      detail::is_result<f_result_type>::value, "Function must return result type");
    static_assert(
      std::is_same_v<typename f_result_type::value_type, value_type>,
      "Function must return a result type with matching value type");

    return std::move(*this).visit(kdl::overload(
      [&]() { return f_result_type{}; },
      [&](auto&& e) { return f(std::forward<decltype(e)>(e)); }));
  }

  /**
   * See result<Value, Errors...>::map_errors.
   */
  template <typename F>
  auto map_errors(F&& f) const&
  {
    using first_error_type = typename meta_front<Errors...>::front;
    using f_result_type = std::invoke_result_t<F, first_error_type>;

    static_assert(
      detail::is_result<f_result_type>::value, "Function must return result type");
    static_assert(
      std::is_same_v<typename f_result_type::value_type, value_type>,
      "Function must return a result type with matching value type");

    return visit(kdl::overload(
      [&]() { return f_result_type{}; }, [&](const auto& e) { return f(e); }));
  }

  /**
   * See result<Value, Errors...>::handle_errors.
   */
  template <typename F>
  bool handle_errors(F&& f) &&
  {
    return std::move(*this).visit(kdl::overload(
      []() { return true; },
      [&](auto&& error) {
        f(std::forward<decltype(error)>(error));
        return false;
      }));
  }

  /**
   * See result<Value, Errors...>::handle_errors.
   */
  template <typename F>
  bool handle_errors(F&& f) const&
  {
    return visit(kdl::overload(
      []() { return true; },
      [&](const auto& error) {
        f(error);
        return false;
      }));
  }

  /**
   * Returns a the error contained in this result if it not successful. Otherwise, throws
   * `bad_result_access`.
   *
   * @return a std::variant<Errors...> containing a copy of the error in this result
   *
   * @throw bad_result_access if this result is an error
   */
  auto error() const&
  {
    return visit(kdl::overload(
      []() -> std::variant<Errors...> { throw bad_result_access{}; },
      [](const auto& e) -> std::variant<Errors...> { return e; }));
  }

  /**
   * Returns a the error contained in this result if it not successful. Otherwise, throws
   * `bad_result_access`.
   *
   * @return a std::variant<Errors...> containing the error in this result
   *
   * @throw bad_result_access if this result is an error
   */
  auto error() &&
  {
    return visit(kdl::overload(
      []() -> std::variant<Errors...> { throw bad_result_access{}; },
      [](auto&& e) -> std::variant<Errors...> { return std::forward<decltype(e)>(e); }));
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
  bool is_error() const { return !is_success(); }

  /**
   * Indicates whether the given result contains the given type of error.
   */
  template <typename E>
  bool is_error_type() const
  {
    static_assert((... || std::is_convertible_v<E, Errors>), "E must be an error type");

    return std::holds_alternative<E>(m_value);
  }

  /**
   * Indicates whether this result is empty.
   */
  // NOLINTNEXTLINE
  operator bool() const { return is_success(); }

  friend bool operator==(const result& lhs, const result& rhs)
  {
    return lhs.m_value == rhs.m_value;
  }

  friend bool operator!=(const result& lhs, const result& rhs) { return !(lhs == rhs); }
};
} // namespace kdl
