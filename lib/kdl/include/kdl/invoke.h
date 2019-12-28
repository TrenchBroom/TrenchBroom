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

#ifndef KDL_INVOKE_H
#define KDL_INVOKE_H

namespace kdl {
    /**
     * Immediately invokes a lambda.
     *
     * @tparam L the type of the lambda to invoke
     */
    template <typename L>
    class invoke_now {
    public:
        /**
         * Invokes the given lambda.
         *
         * @param lambda the lambda to invoke
         */
        explicit invoke_now(const L& lambda) {
            lambda();
        }
    };

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
         * Creates an instance that invokes the given lambda when going out of scope. The given lambda is copied.
         *
         * @param lambda the lambda to invoke
         */
        explicit invoke_later(const L& lambda) :
        m_lambda(lambda) {}

        /**
         * Creates an instance that invokes the given lambda when going out of scope. The given lambda is moved.
         *
         * @param lambda the lambda to invoke
         */
        explicit invoke_later(L&& lambda) :
        m_lambda(std::move(lambda)) {}

        /**
         * Invokes the lambda.
         */
        ~invoke_later() {
            m_lambda();
        }
    };

    /**
     * Invokes a lambda immediately and another lambda when going out of scope.
     *
     * @tparam L_N the type of the lambda to invoke immediately
     * @tparam L_L the type of the lambda to invoke later
     */
    template <typename L_N, typename L_L>
    class invoke_now_and_later : private invoke_now<L_N>, private invoke_later<L_L> {
    public:
        /**
         * Creates an instance that immediately invokes the first lambda, and invokes the second lambda when going out
         * of scope. The second lambda is a forwarding reference and is copied or moved depending on the actual type of
         * the reference given.
         *
         * @tparam LL_L the type of the lambda to invoke later
         * @param lambda_now the lambda to invoke immediately
         * @param lambda_later the lambda to invoke later
         */
        template <typename LL_L>
        invoke_now_and_later(const L_N& lambda_now, LL_L&& lambda_later) :
        invoke_now<L_N>(lambda_now),
        invoke_later<L_L>(std::forward<LL_L>(lambda_later)) {}
    };

    template <typename L_N, typename LL_L>
    invoke_now_and_later(const L_N& lambda_now, LL_L&& lambda_later) -> invoke_now_and_later<L_N, typename std::remove_reference<LL_L>::type>;
}

#endif //KDL_INVOKE_H
