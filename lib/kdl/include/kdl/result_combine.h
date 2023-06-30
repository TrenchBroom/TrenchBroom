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
#include "kdl/result.h"

#include <tuple>

namespace kdl
{
namespace detail
{
template <typename Result1, typename Result2>
struct combine_tuple_results
{
};

template <typename Value1, typename... Errors1, typename... Values2, typename... Errors2>
struct combine_tuple_results<
  result<Value1, Errors1...>,
  result<std::tuple<Values2...>, Errors2...>>
{
  using result = typename make_result_type<
    std::tuple<Value1, Values2...>,
    typename meta_remove_duplicates<Errors1..., Errors2...>::result>::type;
};

template <typename Result>
struct tuple_wrap
{
};

template <typename Value, typename... Errors>
struct tuple_wrap<result<Value, Errors...>>
{
  using result =
    typename make_result_type<std::tuple<Value>, meta_type_list<Errors...>>::type;
};
} // namespace detail

/**
 * Base case for combine_results.
 *
 * Given a result of `result<Value, Errors...>`, returns a result of type
 * `result<std::tuple<Value>, Errors...>` that wraps the success value or error contained
 * in the given result.
 */
template <typename Result>
auto combine_results(Result&& result)
{
  using result_type = typename detail::tuple_wrap<
    std::remove_cv_t<std::remove_reference_t<Result>>>::result;
  using value_type =
    typename std::remove_cv_t<std::remove_reference_t<Result>>::value_type;

  return std::forward<Result>(result).visit(overload(
    [](value_type&& v) { return result_type{std::make_tuple(std::move(v))}; },
    [](value_type& v) { return result_type{std::make_tuple(v)}; },
    [](const value_type& v) { return result_type{std::make_tuple(v)}; },
    [](auto&& e) { return result_type{std::forward<decltype(e)>(e)}; }));
}

/**
 * Combines all of the given results into a single result that has a tuple of the given
 * results' value types as its own value type, and the union of the given results' error
 * types as its error types.
 *
 * Given three results of types
 * - result<int, err1, err2>
 * - result<float, err1, err3>
 * - result<bool, err4>,
 *
 * this function returns a result of type `result<std::tuple<int, float, bool>, err1,
 * err2, err3, err4>`. If all of the given results are successful, then the returned
 * result contains a tuple of the given results's success values as its own success value.
 * If any of the given results is a failure, then the returned result contains the error
 * of the first failure result.
 *
 * Results passed as lvalue references will have their value or error copied into the
 * returned result, while results passed as rvalue references will have their value or
 * error moved into the returned result.
 *
 * @tparam FirstResult the type of the first result
 * @tparam MoreResults the types of the other results
 * @param firstResult the first result
 * @param moreResults the other results
 * @return a result wrapping either a tuple of the success values of the given results, or
 * an error otherwise
 */
template <typename FirstResult, typename... MoreResults>
auto combine_results(FirstResult&& firstResult, MoreResults&&... moreResults)
{
  using first_value_type =
    typename std::remove_cv_t<std::remove_reference_t<FirstResult>>::value_type;
  using combined_more_result_type =
    decltype(combine_results(std::forward<MoreResults>(moreResults)...));
  using result_type = typename detail::combine_tuple_results<
    std::remove_cv_t<std::remove_reference_t<FirstResult>>,
    combined_more_result_type>::result;

  return std::forward<FirstResult>(firstResult)
    .visit(overload(
      [&](first_value_type&& firstValue) {
        return combine_results(std::forward<MoreResults>(moreResults)...)
          .visit(overload(
            [&](typename combined_more_result_type::value_type&& remainingValues) {
              return result_type{std::tuple_cat(
                std::tuple{std::move(firstValue)}, std::move(remainingValues))};
            },
            [&](const typename combined_more_result_type::value_type& remainingValues) {
              return result_type{
                std::tuple_cat(std::tuple{std::move(firstValue)}, remainingValues)};
            },
            [](auto&& combinedError) {
              return result_type{std::forward<decltype(combinedError)>(combinedError)};
            }));
      },
      [&](const first_value_type& firstValue) {
        return combine_results(std::forward<MoreResults>(moreResults)...)
          .visit(overload(
            [&](typename combined_more_result_type::value_type&& remainingValues) {
              return result_type{
                std::tuple_cat(std::tuple{firstValue}, std::move(remainingValues))};
            },
            [&](const typename combined_more_result_type::value_type& remainingValues) {
              return result_type{std::tuple_cat(std::tuple{firstValue}, remainingValues)};
            },
            [](auto&& combinedError) {
              return result_type{std::forward<decltype(combinedError)>(combinedError)};
            }));
      },
      [&](auto&& firstError) {
        return result_type{std::forward<decltype(firstError)>(firstError)};
      }));
}
} // namespace kdl
