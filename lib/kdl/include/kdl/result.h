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

#ifndef result_h
#define result_h

#include "kdl/overload.h"
#include "kdl/result_forward.h"

#include <functional> // for std::ref
#include <optional>
#include <variant>

namespace kdl {
    /**
     * Thrown when attempting to access a result success value on a failed result.
     */
    class bad_result_access : public std::exception {
    public:
        bad_result_access() noexcept = default;
        
        const char* what() const noexcept override {
            return "access success value on a failed result";
        }
    };
    
    /**
     * Wrapper class that can contain either a value or one of several errors.
     *
     * An instance of this class represents an expectation for the result of applying a function if that function
     * returns either a value or throws an error.
     *
     * A result is considered successful if it contains a value, and a failure if it contains an error.
     *
     * @tparam Value the type of the value
     * @tparam Errors the types of the possible errors
     */
    template <typename Value, typename... Errors>
    class [[nodiscard]] result {
    public:
        using value_type = Value;
    private:
        using variant_type = std::variant<value_type, Errors...>;
        variant_type m_value;
    private:
        result(variant_type&& v)
        : m_value(std::move(v)) {}
    public:
        /**
         * Creates a new successful result that wraps the given value.
         * If the value is passed by (const) lvalue reference, it is copied into the given result, if it is passed by
         * rvalue reference, then it is moved into the given result.
         *
         * @tparam V the type of the value, must be convertible to value_type
         * @param v the value
         * @return a successful result that wraps the given value
         */
        template <typename V, typename std::enable_if_t<std::is_convertible_v<V, Value>>* = nullptr>
        static result success(V&& v) {
            return result(variant_type(std::in_place_index_t<0>(), std::forward<V>(v)));
        }
        
        /**
         * Creates a new failure result that wraps the given error.
         * If the error is passed by (const) lvalue reference, it is copied into the given result, if it s passed by rvalue
         * reference, then it is moved into the given result.
         *
         * @tparam E the type of the error, must match one of the error types of the given result
         * @param e the error
         * @return a failure result that wraps the given error
         */
        template <typename E, typename std::enable_if_t<std::disjunction_v<std::is_convertible<E, Errors>...>>* = nullptr>
        static result error(E&& e) {
            return result(variant_type(std::forward<E>(e)));
        }
        
        /**
         * Applies the given visitor to the given result and returns the result returned by the visitor.
         * The value or error contained in the given result is passed to the visitor by const lvalue reference.
         *
         * @tparam Visitor the type of the visitor
         * @param visitor the visitor to apply
         * @param result the result to visit
         * @return the result of applying the given visitor
         */
        template <typename Visitor>
        friend auto visit_result(Visitor&& visitor, const result& result) {
            return std::visit(
                std::forward<Visitor>(visitor),
                result.m_value);
        }
        
        /**
         * Applies the given visitor to the given result and returns the result returned by the visitor.
         * The value or error contained in the given result is passed to the visitor by rvalue reference.
         *
         * The given visitor can move the value or error out of the given result, so care must be taken not to
         * use the result afterwards, as it will be in a moved-from state.
         *
         * @tparam Visitor the type of the visitor
         * @param visitor the visitor to apply
         * @param result the result to visit
         * @return the result of applying the given visitor
         */
        template <typename Visitor>
        friend auto visit_result(Visitor&& visitor, result&& result) {
            return std::visit(
                std::forward<Visitor>(visitor),
                std::move(result.m_value));
        }
        
        /**
         * Applies the given function to the given result and returns a new result with the result type of the
         * given function as its value type.
         *
         * If the given result contains a value, the function is applied to it and the result of applying the function
         * is returned wrapped in a result.
         * If the given result contains an error, that error is returned as is.
         *
         * The value is passed to the mapping function by const lvalue reference.

         * @tparam F the type of the function to apply
         * @param f the function to apply
         * @param result_ the result to map
         */
        template <typename F>
        friend auto map_result(F&& f, const result& result_) {
            using R = std::invoke_result_t<F, Value>;
            return visit_result(kdl::overload {
                [&](const value_type& v) { return result<R, Errors...>::success(f(v)); },
                [] (const auto& e)       { return result<R, Errors...>::error(e); }
            }, result_);
        }
        
        /**
         * Applies the given function to the given result and returns a new result with the result type of the
         * given function as its value type.
         *
         * If the given result contains a value, the function is applied to it and the result of applying the function
         * is returned wrapped in a result.
         * If the given result contains an error, that error is returned as is.
         *
         * The value is passed to the mapping function by rvalue reference.
         *
         * The given function can move the value out of the given result, so care must be taken not to
         * use the given result afterwards, as it will be in a moved-from state.

         * @tparam F the type of the function to apply
         * @param f the function to apply
         * @param result_ the result to map
         */
        template <typename F>
        friend auto map_result(F&& f, result&& result_) {
            using R = std::invoke_result_t<F, Value>;
            return visit_result(kdl::overload {
                [&](value_type&& v) { return result<R, Errors...>::success(f(std::move(v))); },
                [] (const auto& e)  { return result<R, Errors...>::error(e); }
            }, std::move(result_));
        }

        /**
         * Returns the value contained in the given result if it is successful. Otherwise, throws `bad_result_access`.
         *
         * @param result_ the result to access
         * @return a copy of the value in the given result
         *
         * @throw bad_result_access if the given result is an error
         */
        friend auto get_success(const result& result_) {
            return visit_result(kdl::overload {
                [](const value_type& v) -> value_type { return v; },
                [](const auto&)         -> value_type { throw bad_result_access(); }
            }, result_);
        }

        /**
         * Returns the value contained in the given result if it is successful. Otherwise, throws `bad_result_access`.
         *
         * @param result_ the result to access
         * @return the value in the given result
         *
         * @throw bad_result_access if the given result is an error
         */
        friend auto get_success(result&& result_) {
            return visit_result(kdl::overload {
                [](value_type&& v) -> value_type { return std::move(v); },
                [](const auto&)    -> value_type { throw bad_result_access(); }
            }, std::move(result_));
        }

        /**
         * Indicates whether the given result contains a value.
         */
        bool is_success() const {
            return m_value.index() == 0u;
        }
        
        /**
         * Indicates whether the given result contains an error.
         */
        bool is_error() const {
            return !is_success();
        }

        /**
         * Indicates whether the given result contains the given type of error.
         */
        template <class E, typename std::enable_if_t<std::disjunction_v<std::is_convertible<E, Errors>...>>* = nullptr>
        bool is_error_type() const {
            return std::holds_alternative<E>(m_value);
        }
        
        /**
         * Indicates whether the given result contains a value.
         */
        operator bool() const {
            return is_success();
        }
        
        friend bool operator==(const result& lhs, const result& rhs) {
            return lhs.m_value == rhs.m_value;
        }
        
        friend bool operator!=(const result& lhs, const result& rhs) {
            return !(lhs == rhs);
        }
    };

    /**
     * Wrapper class that can contain either a reference or one of several errors.
     *
     * An instance of this class represents an expectation for the result of applying a function if that function
     * returns either a reference or throws an error.
     *
     * A result is considered successful if it contains a reference, and a failure if it contains an error.
     *
     * @tparam Value the type of the value being referenced
     * @tparam Errors the types of the possible errors
     */
    template <typename Value, typename... Errors>
    class [[nodiscard]] result<Value&, Errors...> {
    public:
        using value_type = Value;
    private:
        using wrapper_type = std::reference_wrapper<value_type>;
        using variant_type = std::variant<wrapper_type, Errors...>;
        variant_type m_value;
    private:
        result(variant_type&& v)
        : m_value(std::move(v)) {}
    public:
        /**
         * Creates a result that wraps the given lvalue reference.
         *
         * @param v the reference to wrap
         * @return a successful result that wraps the given reference
         */
        static result success(value_type& v) {
            return result(variant_type(std::in_place_index_t<0>(), std::ref(v)));
        }
        
        /**
         * Creates a new failure result that wraps the given error.
         * If the error is passed by (const) lvalue reference, it is copied into the given result, if it s passed by rvalue
         * reference, then it is moved into the given result.
         *
         * @tparam E the type of the error, must match one of the error types of the given result
         * @param e the error
         * @return a failure result that wraps the given error
         */
        template <typename E, typename std::enable_if_t<std::disjunction_v<std::is_convertible<E, Errors>...>>* = nullptr>
        static result error(E&& e) {
            return result(variant_type(std::forward<E>(e)));
        }

        /**
         * Applies the given visitor to the given result and returns the result returned by the visitor.
         * The reference or error contained in the given result is passed to the visitor as a const lvalue
         * reference.
         *
         * @tparam Visitor the type of the visitor
         * @param visitor the visitor to apply
         * @param result the result to visit
         * @return the result of applying the given visitor
         */
        template <typename Visitor>
        friend auto visit_result(Visitor&& visitor, const result& result) {
            return std::visit(kdl::overload {
                [&](const wrapper_type& v) { return visitor(v.get()); },
                [&](const auto& e)         { return visitor(e); }
            }, result.m_value);
        }
        
        /**
         * Applies the given visitor to the given result and returns the result returned by the visitor.
         * The reference or error contained in the given result is passed to the visitor as a rvalue reference.
         *
         * The given visitor can move the value or error out of the given result, so care must be taken not to
         * use the result afterwards, as it will be in a moved-from state.
         *
         * Note that the visitor can only move the value of the contained reference if it is not const.
         *
         * @tparam Visitor the type of the visitor
         * @param visitor the visitor to apply
         * @param result the result to visit
         * @return the result of applying the given visitor
         */
        template <typename Visitor>
        friend auto visit_result(Visitor&& visitor, result&& result) {
            return std::visit(kdl::overload {
                [&](wrapper_type&& v) { return visitor(std::move(v.get())); },
                [&](auto&& e)         { return visitor(std::move(e)); }
            }, std::move(result.m_value));
        }
        
        /**
         * Applies the given function to given result and returns a new result with the result type of
         * the given function as its value type.
         *
         * If the given result contains a reference, the function is applied to it and the result of applying the
         * function is returned wrapped in a result.
         * If the given result contains an error, that error is returned as is.
         *
         * The reference is passed to the mapping function by const lvalue reference.

         * @tparam F the type of the function to apply
         * @param f the function to apply
         * @param result_ the result to map
         */
        template <typename F>
        friend auto map_result(F&& f, const result& result_) {
            using R = std::invoke_result_t<F, Value>;
            return visit_result(kdl::overload {
                [&](const value_type& v) { return result<R, Errors...>::success(f(v)); },
                [] (const auto& e)       { return result<R, Errors...>::error(e); }
            }, result_);
        }
        
        /**
         * Applies the given function to the given result and returns a new result with the result type of
         * the given function as its value type.
         *
         * If the given result contains a reference, the function is applied to it and the result of applying the
         * function is returned wrapped in a result.
         * If the given result contains an error, that error is returned as is.
         *
         * The reference is passed to the mapping function by rvalue reference.
         *
         * The given function can move the value out of the given result, so care must be taken not to
         * use the given result afterwards, as it will be in a moved-from state.
         *
         * Note that the visitor can only move the value of the contained reference if it is not const.
         *
         * @tparam F the type of the function to apply
         * @param f the function to apply
         * @param result_ the result to map
         */
        template <typename F>
        friend auto map_result(F&& f, result&& result_) {
            using R = std::invoke_result_t<F, Value>;
            return visit_result(kdl::overload {
                [&](value_type&& v) { return result<R, Errors...>::success(f(std::move(v))); },
                [] (const auto& e)  { return result<R, Errors...>::error(e); }
            }, std::move(result_));
        }

        /**
         * Returns the value contained in the given result if it is successful. Otherwise, throws `bad_result_access`.
         *
         * @param result_ the result to access
         * @return a copy of the value in the given result
         *
         * @throw bad_result_access if the given result is an error
         */
        friend auto get_success(const result& result_) {
            return visit_result(kdl::overload {
                [](const value_type& v) -> value_type& { return v; },
                [](const auto&)         -> value_type& { throw bad_result_access(); }
            }, result_);
        }

        /**
         * Returns the value contained in the given result if it is successful. Otherwise, throws `bad_result_access`.
         *
         * @param result_ the result to access
         * @return the value in the given result
         *
         * @throw bad_result_access if the given result is an error
         */
        friend auto get_success(result&& result_) {
            return visit_result(kdl::overload {
                [](value_type&& v) -> value_type&& { return std::move(v); },
                [](const auto&)    -> value_type&& { throw bad_result_access(); }
            }, std::move(result_));
        }

        /**
         * Indicates whether this result contains a reference.
         */
        bool is_success() const {
            return m_value.index() == 0u;
        }
        
        /**
         * Indicates whether this result contains an error.
         */
        bool is_error() const {
            return !is_success();
        }

        /**
         * Indicates whether the given result contains the given type of error.
         */
        template <class E, typename std::enable_if_t<std::disjunction_v<std::is_convertible<E, Errors>...>>* = nullptr>
        bool is_error_type() const {
            return std::holds_alternative<E>(m_value);
        }

        /**
         * Indicates whether this result contains a reference.
         */
        operator bool() const {
            return is_success();
        }
        
        friend bool operator==(const result& lhs, const result& rhs) {
            return lhs.m_value == rhs.m_value;
        }
        
        friend bool operator!=(const result& lhs, const result& rhs) {
            return !(lhs == rhs);
        }
    };
    
    /**
     * Wrapper class that can contain either nothing or one of several errors.
     *
     * An instance of this class represents an expectation for the result of applying a function if that function
     * returns void or throws an error.
     *
     * An result is considered successful if it is empty, and a failure if it contains an error.
     *
     * @tparam Errors the types of the possible errors
     */
    template <typename... Errors>
    class [[nodiscard]] result<void, Errors...> {
    private:
        using variant_type = std::variant<Errors...>;
        std::optional<variant_type> m_error;
    private:
        result() {}
        
        result(variant_type&& v)
        : m_error(std::move(v)) {}
    public:
        /**
         * Creates a new successful result.
         *
         * @return a successful result
         */
        static result success() {
            return result();
        }
        
        /**
         * Creates a new failure result that wraps the given error.
         * If the error is passed by (const) lvalue reference, it is copied into this result, if it s passed by rvalue
         * reference, then it is moved into this result.
         *
         * @tparam E the type of the error, must match one of the error types of this result
         * @param e the error
         * @return a failure result that wraps the given error
         */
        template <typename E, typename std::enable_if_t<std::disjunction_v<std::is_convertible<E, Errors>...>>* = nullptr>
        static result error(E&& e) {
            return result(variant_type(std::forward<E>(e)));
        }

        /**
         * Applies the given visitor to the given result and returns the result returned by the visitor.
         * If the given result is successful, then the visitor is called without any arguments.
         * Otherwise, the error contained in the given result is passed to the visitor by const lvalue reference.
         *
         * @tparam Visitor the type of the visitor
         * @param visitor the visitor to apply
         * @param result the result to visit
         * @return the result of applying the given visitor
         */
        template <typename Visitor>
        friend auto visit_result(Visitor&& visitor, const result& result) {
            if (result.m_error.has_value()) {
                return std::visit(
                    std::forward<Visitor>(visitor),
                    result.m_error.value());
            } else {
                return visitor();
            }
        }
        
        /**
         * Applies the given visitor to the given result and returns the result returned by the visitor.
         * If the given result is successful, then the visitor is called without any arguments.
         * Otherwise, the error contained in the given result is passed to the visitor by rvalue reference.
         *
         * The given visitor can move the value or error out of the given result, so care must be taken not to
         * use the result afterwards, as it will be in a moved-from state.
         *
         * @tparam Visitor the type of the visitor
         * @param visitor the visitor to apply
         * @param result the result to visit
         * @return the result of applying the given visitor
         */
        template <typename Visitor>
        friend auto visit_result(Visitor&& visitor, result&& result) {
            if (result.m_error.has_value()) {
                return std::visit(
                    std::forward<Visitor>(visitor),
                    std::move(result.m_error.value()));
            } else {
                return visitor();
            }
        }
        
        /**
         * Applies the given function to given result and returns a new result with the result type of
         * the given function as its value type.
         *
         * If the given result is a success, the function is applied and the result of applying the
         * function is returned wrapped in a result.
         * If the given result contains an error, that error is returned as is.
         *
         * @tparam F the type of the function to apply
         * @param f the function to apply
         * @param result_ the result to map
         */
        template <typename F>
        friend auto map_result(F&& f, const result& result_) {
            using R = std::invoke_result_t<F>;
            if constexpr (std::is_same_v<R, void>) {
                return visit_result(kdl::overload {
                    [&]()              { f(); return result<R, Errors...>::success(); },
                    [] (const auto& e) { return result<R, Errors...>::error(e); }
                }, result_);
            } else {
                return visit_result(kdl::overload {
                    [&]()              { return result<R, Errors...>::success(f()); },
                    [] (const auto& e) { return result<R, Errors...>::error(e); }
                }, result_);
            }
        }
        
        /**
         * Applies the given function to given result and returns a new result with the result type of
         * the given function as its value type.
         *
         * If the given result is a success, the function is applied and the result of applying the
         * function is returned wrapped in a result.
         * If the given result contains an error, that error is returned as is.
         *
         * @tparam F the type of the function to apply
         * @param f the function to apply
         * @param result_ the result to map
         */
        template <typename F>
        friend auto map_result(F&& f, result&& result_) {
            using R = std::invoke_result_t<F>;
            if constexpr (std::is_same_v<R, void>) {
                return visit_result(kdl::overload {
                    [&]()              { f(); return result<R, Errors...>::success(); },
                    [] (const auto& e) { return result<R, Errors...>::error(e); }
                }, std::move(result_));
            } else {
                return visit_result(kdl::overload {
                    [&]()              { return result<R, Errors...>::success(f()); },
                    [] (const auto& e) { return result<R, Errors...>::error(e); }
                }, std::move(result_));
            }
        }

        /**
         * Indicates whether this result is empty.
         */
        bool is_success() const {
            return !m_error.has_value();
        }
        
        /**
         * Indicates whether this result contains an error.
         */
        bool is_error() const {
            return !is_success();
        }
    
        /**
         * Indicates whether the given result contains the given type of error.
         */
        template <class E, typename std::enable_if_t<std::disjunction_v<std::is_convertible<E, Errors>...>>* = nullptr>
        bool is_error_type() const {
            if (m_error.has_value()) { 
                return std::holds_alternative<E>(m_error.value());
            } else {
                return false;
            }
        }

        /**
         * Indicates whether this result is empty.
         */
        operator bool() const {
            return is_success();
        }
        
        friend bool operator==(const result& lhs, const result& rhs) {
            return lhs.m_value == rhs.m_value;
        }
        
        friend bool operator!=(const result& lhs, const result& rhs) {
            return !(lhs == rhs);
        }
    };
    
    /**
     * Wrapper class that can contain a value, one of several errors, or nothing at all.
     *
     * An instance of this class represents an expectation for the result of applying a function if that function
     * returns an optional value or throws an error.
     *
     * An result is considered successful if it is empty or contains a value, and a failure if it contains an
     * error.
     *
     * To create an result of this type, the value type parameter must be wrapped in a type tag called `opt`, i.e.
     *
     * ```
     * result<opt<std::string>, std::exception>
     * ```
     *
     * @tparam Value the type of the value
     * @tparam Errors the types of the possible errors
     */
    template <typename Value, typename... Errors>
    class [[nodiscard]] result<opt<Value>, Errors...> {
    public:
        using value_type = Value;
    private:
        using variant_type = std::variant<value_type, Errors...>;
        std::optional<variant_type> m_value;
    private:
        result() {}
        
        result(variant_type&& v)
        : m_value(std::move(v)) {}
    public:
        /**
         * Creates an empty successful result.
         */
        static result success() {
            return result();
        }

        /**
         * Creates a new successful result that wraps the given value.
         * If the value is passed by (const) lvalue reference, it is copied into this result, if it is passed by
         * rvalue reference, then it is moved into this result.
         *
         * @tparam V the type of the value, must be convertible to value_type
         * @param v the value
         * @return a successful result that wraps the given value
         */
        template <typename V, typename std::enable_if_t<std::is_convertible_v<V, Value>>* = nullptr>
        static result success(V&& v) {
            return result(variant_type(std::in_place_index_t<0>(), std::forward<V>(v)));
        }

        /**
         * Creates a new failure result that wraps the given error.
         * If the error is passed by (const) lvalue reference, it is copied into this result, if it s passed by rvalue
         * reference, then it is moved into this result.
         *
         * @tparam E the type of the error, must match one of the error types of this result
         * @param e the error
         * @return a failure result that wraps the given error
         */
        template <typename E, typename std::enable_if_t<std::disjunction_v<std::is_convertible<E, Errors>...>>* = nullptr>
        static result error(E&& e) {
            return result(variant_type(std::forward<E>(e)));
        }
        
        /**
         * Applies the given visitor to the given result and returns the result returned by the visitor.
         * The value or error contained in the given result is passed to the visitor by const lvalue reference.
         * If the given result is successful but does not contain a value, the given visitor is called without any
         * arguments.
         *
         * @tparam Visitor the type of the visitor
         * @param visitor the visitor to apply
         * @param result the result to visit
         * @return the result of applying the given visitor
         */
        template <typename Visitor>
        friend auto visit_result(Visitor&& visitor, const result& result) {
            if (result.m_value.has_value()) {
                return std::visit(
                    std::forward<Visitor>(visitor),
                    result.m_value.value());
            } else {
                return visitor();
            }
        }
        
        /**
         * Applies the given visitor to the given result and returns the result returned by the visitor.
         * The value or error contained in the given result is passed to the visitor by rvalue reference.
         * If the given result is successful but does not contain a value, the given visitor is called without any
         * arguments.
         *
         * The given visitor can move the value or error out of the given result, so care must be taken not to
         * use the result afterwards, as it will be in a moved-from state.
         *
         * @tparam Visitor the type of the visitor
         * @param visitor the visitor to apply
         * @param result the result to visit
         * @return the result of applying the given visitor
         */
        template <typename Visitor>
        friend auto visit_result(Visitor&& visitor, result&& result) {
            if (result.m_value.has_value()) {
                return std::visit(
                    std::forward<Visitor>(visitor),
                    std::move(result.m_value.value()));
            } else {
                return visitor();
            }
        }

        /**
         * Returns the value contained in the given result if it is successful. Otherwise, throws `bad_result_access`.
         *
         * @param result_ the result to access
         * @return a copy of the value in the given result
         *
         * @throw bad_result_access if the given result is an error or if the given result does not contain a value
         */
        friend auto get_success(const result& result_) {
            return visit_result(kdl::overload {
                [](const value_type& v) -> value_type { return v; },
                []()                    -> value_type { throw bad_result_access(); },
                [](const auto&)         -> value_type { throw bad_result_access(); }
            }, result_);
        }

        /**
         * Returns the value contained in the given result if it is successful. Otherwise, throws `bad_result_access`.
         *
         * @param result_ the result to access
         * @return the value in the given result
         *
         * @throw bad_result_access if the given result is an error or if the given result does not contain a value
         */
        friend auto get_success(result&& result_) {
            return visit_result(kdl::overload {
                [](value_type&& v) -> value_type { return std::move(v); },
                []()               -> value_type { throw bad_result_access(); },
                [](const auto&)    -> value_type { throw bad_result_access(); }
            }, std::move(result_));
        }

        /**
         * Indicates whether the given result has a value.
         */
        bool has_value() const {
            return m_value.has_value() && m_value->index() == 0u;
        }
        
        /**
         * Indicates whether the given result is a success, i.e. whether it is empty or contains a value.
         */
        bool is_success() const {
            return !m_value.has_value() || m_value->index() == 0u;
        }
        
        /**
         * Indicates whether the given result contains an error.
         */
        bool is_error() const {
            return !is_success();
        }

        /**
         * Indicates whether the given result contains the given type of error.
         */
        template <class E, typename std::enable_if_t<std::disjunction_v<std::is_convertible<E, Errors>...>>* = nullptr>
        bool is_error_type() const {
            return std::holds_alternative<E>(m_value);
        }

        /**
         * Indicates whether the given result is a success, i.e. whether it is empty or contains a value.
         */
        operator bool() const {
            return is_success();
        }
        
        friend bool operator==(const result& lhs, const result& rhs) {
            return lhs.m_value == rhs.m_value;
        }
        
        friend bool operator!=(const result& lhs, const result& rhs) {
            return !(lhs == rhs);
        }
    };

    
    template <typename Visitor, typename Value, typename... Errors>
    auto visit_result(Visitor&& visitor, const result<Value, Errors...>& result) {
        return visit_result(std::forward<Visitor>(visitor), result);
    }

    template <typename Visitor, typename Value, typename... Errors>
    auto visit_result(Visitor&& visitor, result<Value, Errors...>&& result) {
        return visit_result(std::forward<Visitor>(visitor), std::move(result));
    }

    template <typename F, typename Value, typename... Errors>
    auto map_result(F&& f, const result<Value, Errors...>& result_) {
        return map_result(std::forward<F>(f), result_);
    }
    
    template <typename F, typename Value, typename... Errors>
    auto map_result(F&& f, result<Value, Errors...>&& result_) {
        return map_result(std::forward<F>(f), std::move(result_));
    }
    
    template <typename Value, typename... Errors>
    auto get_success(const result<Value, Errors...>& result_) {
        return get_success(result_);
    }
    
    template <typename Value, typename... Errors>
    auto get_success(result<Value, Errors...>&& result_) {
        return get_success(std::move(result_));
    }
}

#endif /* result_h */
