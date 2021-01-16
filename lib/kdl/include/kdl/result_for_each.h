/*
 Copyright 2020 Kristian Duske

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

#pragma once

#include "kdl/meta_utils.h"
#include "kdl/overload.h"
#include "kdl/result.h"

#include <iterator>
#include <vector>

namespace kdl {

    /**
     * Applies the given lambda to each element in the given range and returns the result.
     *
     * The given lambda must return a result type.
     *
     * - If the lambda returns a void result type, this function returns result<void, error_type>.
     *   The void (success) case is returned if *all* invocations of the given lambda return void (success).
     *
     * - If the given lambda returns a non void result type, returns result<std::vector<value_type>, error_type>.
     *   The vector is returned if *all* invocations of the given lambda return success values, and contains
     *   those success values.
     *
     * If any invocation of the given lambda fails, processing stops and that failure result is returned.
     */
    template <typename I, typename F>
    auto for_each_result(I cur, I end, F f) {
        using i_value_type = typename std::iterator_traits<I>::value_type;
        using f_result_type = std::invoke_result_t<F, i_value_type>;
        using f_result_value_type = typename f_result_type::value_type;

        if constexpr(std::is_same_v<f_result_value_type, void>) {
            using result_type = typename f_result_type::template with_value_type<void>;

            while (cur != end) {
                auto f_result = f(*cur);
                if (f_result.is_error()) {
                    return f_result;
                }
                ++cur;
            }

            return result_type{};
        } else {
            using vector_type = std::vector<f_result_value_type>;
            using result_type = typename f_result_type::template with_value_type<vector_type>;
            using i_category = typename std::iterator_traits<I>::iterator_category;

            auto result_vector = vector_type{};
            if constexpr(std::is_same_v<i_category, std::random_access_iterator_tag>) {
                result_vector.reserve(static_cast<std::size_t>(end - cur));
            }

            while (cur != end) {
                auto f_result = f(*cur);
                if (f_result.is_error()) {
                    return std::visit([](auto&& e) { 
                        return result_type{std::move(e)};
                    }, std::move(f_result).error());
                }

                result_vector.push_back(std::move(f_result).value());
                ++cur;
            }

            return result_type{std::move(result_vector)};
        }
    }

    template <typename C, typename F>
    auto for_each_result(C&& c, F f) {
        return for_each_result(std::begin(c), std::end(c), std::move(f));
    }
}
