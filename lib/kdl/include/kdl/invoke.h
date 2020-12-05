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

#pragma once

#include <utility>

namespace kdl {
    /**
     * Invokes a lambda when going out of scope.
     *
     * @tparam L the type of the lambda to invoke
     */
    template <typename L>
    class invoke_later {
    private:
        L m_lambda;
    public:
        /**
         * Creates an instance that invokes the given lambda when going out of scope.
         *
         * @param lambda the lambda to invoke
         */
        template <typename LL>
        explicit invoke_later(LL&& lambda) :
        m_lambda(std::forward<LL>(lambda)) {}

        /**
         * Invokes the lambda.
         */
        ~invoke_later() {
            m_lambda();
        }
    };

    /**
     * Deduction guide.
     */
    template <typename LL>
    invoke_later(const LL& lambda_now) -> invoke_later<LL>;
}

