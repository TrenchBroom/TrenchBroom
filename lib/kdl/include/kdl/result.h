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
#include <iosfwd> // users must include <ostream> to use stream operators
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
         * Visits the value or error contained in this result.
         *
         * The given visitor must accept the value type and all error types of this result.
         * The value or error contained in this result is passed to the visitor by const lvalue reference.
         *
         * @tparam Visitor the type of the visitor
         * @param visitor the visitor to apply
         * @return the value returned by the given visitor or void if the given visitor does not return anything
         */
        template <typename Visitor>
        auto visit(Visitor&& visitor) const & {
            return std::visit(std::forward<Visitor>(visitor), m_value);
        }
        
        /**
         * Visits the value or error contained in this result.
         *
         * The given visitor must accept the value type and all error types of this result.
         * The value or error contained in this result is passed to the visitor by rvalue reference.
         *
         * @tparam Visitor the type of the visitor
         * @param visitor the visitor to apply
         * @return the value returned by the given visitor or void if the given visitor does not return anything
         */
        template <typename Visitor>
        auto visit(Visitor&& visitor) && {
            return std::visit(std::forward<Visitor>(visitor), std::move(m_value));
        }
        
        /**
         * Applies the given function to the value contained in this result.
         *
         * If this result is a success, the given function is invoked and passed the contained value by const lvalue
         * reference. Then f's return value is wrapped in a new result and returned.
         * If this result is an error, then the given function is not invoked, the error is wrapped in a new result and
         * returned.
         * The returned result value has the return type of the given function as its value type, and the error types of
         * this result as its error types.
         *
         * To illustrate, consider a function with the signature `std::string to_string(int)` and a result `r` of type
         * `result<int, Error1, Error2>`. Then calling `r.map(to_string)` returns a result of type `result<std::string,
         * Error1, Error2>`.
         *
         * Note that it's also permissible that the given function returns nothing. In that case, a void result is
         * returned.
         *
         * @tparam F the type of the function to apply
         * @param f the function to apply
         * @return a new result value containing the transformed success value or the error contained in this result
         */
        template <typename F>
        auto map(F&& f) const & {
            using R = std::invoke_result_t<F, Value>;
            return visit(kdl::overload {
                [&](const value_type& v) { return result<R, Errors...>::success(f(v)); },
                [] (const auto& e)       { return result<R, Errors...>::error(e); }
            });
        }
        
        /**
         * Applies the given function to the value contained in this result.
         *
         * If this result is a success, the given function is invoked and passed the contained value by rvalue
         * reference. Then f's return value is wrapped in a new result and returned.
         * If this result is an error, then the given function is not invoked, the error is wrapped in a new result and
         * returned.
         * The returned result value has the return type of the given function as its value type, and the error types of
         * this result as its error types.
         *
         * To illustrate, consider a function with the signature `std::string to_string(int)` and a result `r` of type
         * `result<int, Error1, Error2>`. Then calling `r.map(to_string)` returns a result of type `result<std::string,
         * Error1, Error2>`.
         *
         * Note that it's also permissible that the given function returns nothing. In that case, a void result is
         * returned.
         *
         * @tparam F the type of the function to apply
         * @param f the function to apply
         * @return a new result value containing the transformed success value or the error contained in this result
         */
        template <typename F>
        auto map(F&& f) && {
            using R = std::invoke_result_t<F, Value>;
            return std::move(*this).visit(kdl::overload {
                [&](value_type&& v) { return result<R, Errors...>::success(f(std::move(v))); },
                [] (auto&& e)       { return result<R, Errors...>::error(std::move(e)); }
            });
        }

        /**
         * Returns the value contained in this result if it is successful. Otherwise, throws `bad_result_access`.
         *
         * @return a copy of the value in this result
         *
         * @throw bad_result_access if this result is an error
         */
        auto value() const & {
            return visit(kdl::overload {
                [](const value_type& v) -> value_type { return v; },
                [](const auto&)         -> value_type { throw bad_result_access(); }
            });
        }

        /**
         * Returns the value contained in this result if it is successful. Otherwise, throws `bad_result_access`.
         *
         * @return the value in this result
         *
         * @throw bad_result_access if this result is an error
         */
        auto value() && {
            return std::move(*this).visit(kdl::overload {
                [](value_type&& v) -> value_type { return std::move(v); },
                [](const auto&)    -> value_type { throw bad_result_access(); }
            });
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

        friend std::ostream& operator<<(std::ostream& str, const result& result_) {
            std::visit([&](const auto& v){ str << v; }, result_.m_value);
            return str;
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
         * Visits the value or error contained in this result.
         *
         * The given visitor must accept the value type and all error types of this result.
         * The value or error contained in this result is passed to the visitor by const lvalue reference.
         *
         * @tparam Visitor the type of the visitor
         * @param visitor the visitor to apply
         * @return the value returned by the given visitor or void if the given visitor does not return anything
         */
        template <typename Visitor>
        auto visit(Visitor&& visitor) const & {
            return std::visit(kdl::overload {
                [&](const wrapper_type& v) { return visitor(v.get()); },
                [&](const auto& e)         { return visitor(e); }
            }, m_value);
        }
        
        /**
         * Visits the value or error contained in this result.
         *
         * The given visitor must accept the value type and all error types of this result.
         * The value or error contained in this result is passed to the visitor by rvalue reference.
         *
         * @tparam Visitor the type of the visitor
         * @param visitor the visitor to apply
         * @return the value returned by the given visitor or void if the given visitor does not return anything
         */
        template <typename Visitor>
        auto visit(Visitor&& visitor) && {
            return std::visit(kdl::overload {
                [&](wrapper_type&& v) { return visitor(std::move(v.get())); },
                [&](auto&& e)         { return visitor(std::move(e)); }
            }, std::move(m_value));
        }

        /**
         * Applies the given function to the value contained in this result.
         *
         * If this result is a success, the given function is invoked and passed the contained value by const lvalue
         * reference. Then f's return value is wrapped in a new result and returned.
         * If this result is an error, then the given function is not invoked, the error is wrapped in a new result and
         * returned.
         * The returned result value has the return type of the given function as its value type, and the error types of
         * this result as its error types.
         *
         * To illustrate, consider a function with the signature `std::string to_string(int)` and a result `r` of type
         * `result<int&, Error1, Error2>`. Then calling `r.map(to_string)` returns a result of type `result<std::string,
         * Error1, Error2>`.
         *
         * Note that it's also permissible that the given function returns nothing. In that case, a void result is
         * returned.
         *
         * @tparam F the type of the function to apply
         * @param f the function to apply
         * @return a new result value containing the transformed success value or the error contained in this result
         */
        template <typename F>
        auto map(F&& f) const & {
            using R = std::invoke_result_t<F, Value>;
            return visit(kdl::overload {
                [&](const value_type& v) { return result<R, Errors...>::success(f(v)); },
                [] (const auto& e)       { return result<R, Errors...>::error(e); }
            });
        }
        
        /**
         * Applies the given function to the value contained in this result.
         *
         * If this result is a success, the given function is invoked and passed the contained value by rvalue
         * reference. Then f's return value is wrapped in a new result and returned.
         * If this result is an error, then the given function is not invoked, the error is wrapped in a new result and
         * returned.
         * The returned result value has the return type of the given function as its value type, and the error types of
         * this result as its error types.
         *
         * To illustrate, consider a function with the signature `std::string to_string(int)` and a result `r` of type
         * `result<int&, Error1, Error2>`. Then calling `r.map(to_string)` returns a result of type `result<std::string,
         * Error1, Error2>`.
         *
         * Note that it's also permissible that the given function returns nothing. In that case, a void result is
         * returned.
         *
         * @tparam F the type of the function to apply
         * @param f the function to apply
         * @return a new result value containing the transformed success value or the error contained in this result
         */
        template <typename F>
        auto map(F&& f) && {
            using R = std::invoke_result_t<F, Value>;
            return std::move(*this).visit(kdl::overload {
                [&](value_type&& v) { return result<R, Errors...>::success(f(std::move(v))); },
                [] (auto&& e)       { return result<R, Errors...>::error(std::move(e)); }
            });
        }

        /**
         * Returns the value contained this result if it is successful. Otherwise, throws `bad_result_access`.
         *
         * @return a copy of the value in this result
         *
         * @throw bad_result_access if this result is an error
         */
        auto value() const &{
            return visit(kdl::overload {
                [](const value_type& v) -> value_type& { return v; },
                [](const auto&)         -> value_type& { throw bad_result_access(); }
            });
        }

        /**
         * Returns the value contained this result if it is successful. Otherwise, throws `bad_result_access`.
         *
         * @return the value in the given result
         *
         * @throw bad_result_access if this result is an error
         */
        auto value() && {
            return std::move(*this).visit(kdl::overload {
                [](value_type&& v) -> value_type&& { return std::move(v); },
                [](const auto&)    -> value_type&& { throw bad_result_access(); }
            });
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

        friend std::ostream& operator<<(std::ostream& str, const result& result_) {
            std::visit([&](const auto& v){ str << v; }, result_.m_value);
            return str;
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
         * Applies the given visitor this result.
         *
         * The given visitor must accept void and all error types of this result.
         * The error contained in this result is passed to the visitor by const lvalue reference.
         *
         * @tparam Visitor the type of the visitor
         * @param visitor the visitor to apply
         * @return the value returned by the given visitor or void if the given visitor does not return anything
         */
        template <typename Visitor>
        auto visit(Visitor&& visitor) const & {
            if (m_error.has_value()) {
                return std::visit(
                    std::forward<Visitor>(visitor),
                    m_error.value());
            } else {
                return visitor();
            }
        }
        
        /**
         * Visits the value or error contained in this result.
         *
         * The given visitor must accept the value type and all error types of this result.
         * The value or error contained in this result is passed to the visitor by rvalue reference.
         *
         * @tparam Visitor the type of the visitor
         * @param visitor the visitor to apply
         * @return the value returned by the given visitor or void if the given visitor does not return anything
         */
        template <typename Visitor>
        auto visit(Visitor&& visitor) && {
            if (m_error.has_value()) {
                return std::visit(
                    std::forward<Visitor>(visitor),
                    std::move(m_error.value()));
            } else {
                return visitor();
            }
        }
        
        /**
         * Applies the given function if this result is a success.
         *
         * If this result is a success, the given function is invoked reference. Then f's return value is wrapped in a
         * new result and returned.
         * If this result is an error, then the given function is not invoked, the error is wrapped in a new result and
         * returned.
         * The returned result value has the return type of the given function as its value type, and the error types of
         * this result as its error types.
         *
         * To illustrate, consider a function with the signature `std::string to_string()` and a result `r` of type
         * `result<void, Error1, Error2>`. Then calling `r.map(to_string)` returns a result of type `result<std::string,
         * Error1, Error2>`.
         *
         * Note that it's also permissible that the given function returns nothing. In that case, a void result is
         * returned.
         *
         * @tparam F the type of the function to apply
         * @param f the function to apply
         * @return a new result value containing the transformed success value or the error contained in this result
         */
        template <typename F>
        auto map(F&& f) const & {
            using R = std::invoke_result_t<F>;
            if constexpr (std::is_same_v<R, void>) {
                return visit(kdl::overload {
                    [&]()              { f(); return result<R, Errors...>::success(); },
                    [] (const auto& e) { return result<R, Errors...>::error(e); }
                });
            } else {
                return visit(kdl::overload {
                    [&]()              { return result<R, Errors...>::success(f()); },
                    [] (const auto& e) { return result<R, Errors...>::error(e); }
                });
            }
        }
        
        /**
         * Applies the given function to the value contained in this result.
         *
         * If this result is a success, the given function is invoked and passed the contained value by rvalue
         * reference. Then f's return value is wrapped in a new result and returned.
         * If this result is an error, then the given function is not invoked, the error is wrapped in a new result and
         * returned.
         * The returned result value has the return type of the given function as its value type, and the error types of
         * this result as its error types.
         *
         * To illustrate, consider a function with the signature `std::string to_string(int)` and a result `r` of type
         * `result<int, Error1, Error2>`. Then calling `r.map(to_string)` returns a result of type `result<std::string,
         * Error1, Error2>`.
         *
         * Note that it's also permissible that the given function returns nothing. In that case, a void result is
         * returned.
         *
         * @tparam F the type of the function to apply
         * @param f the function to apply
         * @return a new result value containing the transformed success value or the error contained in this result
         */
        template <typename F>
        auto map(F&& f) && {
            using R = std::invoke_result_t<F>;
            if constexpr (std::is_same_v<R, void>) {
                return std::move(*this).visit(kdl::overload {
                    [&]()         { f(); return result<R, Errors...>::success(); },
                    [] (auto&& e) { return result<R, Errors...>::error(std::move(e)); }
                });
            } else {
                return std::move(*this).visit(kdl::overload {
                    [&]()         { return result<R, Errors...>::success(f()); },
                    [] (auto&& e) { return result<R, Errors...>::error(std::move(e)); }
                });
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

        friend std::ostream& operator<<(std::ostream& str, const result& result_) {
            if (result_.m_error.has_value()) {
                std::visit([&](const auto& v){ str << v; }, result_.m_error.value());
            }
            return str;
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
         * Visits the value or error contained in this result.
         *
         * The given visitor must accept the value type and all error types of this result.
         * The value or error contained in this result is passed to the visitor by const lvalue reference.
         *
         * @tparam Visitor the type of the visitor
         * @param visitor the visitor to apply
         * @return the value returned by the given visitor or void if the given visitor does not return anything
         */
        template <typename Visitor>
        auto visit(Visitor&& visitor) const & {
            if (m_value.has_value()) {
                return std::visit(
                    std::forward<Visitor>(visitor),
                    m_value.value());
            } else {
                return visitor();
            }
        }
        
        /**
         * Visits the value or error contained in this result.
         *
         * The given visitor must accept the value type and all error types of this result.
         * The value or error contained in this result is passed to the visitor by rvalue reference.
         *
         * @tparam Visitor the type of the visitor
         * @param visitor the visitor to apply
         * @return the value returned by the given visitor or void if the given visitor does not return anything
         */
        template <typename Visitor>
        auto visit(Visitor&& visitor) && {
            if (m_value.has_value()) {
                return std::visit(
                    std::forward<Visitor>(visitor),
                    std::move(m_value.value()));
            } else {
                return visitor();
            }
        }

        /**
         * Returns the value contained this result if it is successful. Otherwise, throws `bad_result_access`.
         *
         * @return a copy of the value in this result
         *
         * @throw bad_result_access if this result is an error or if this result does not contain a value
         */
        auto get() const & {
            return visit(kdl::overload {
                [](const value_type& v) -> value_type { return v; },
                []()                    -> value_type { throw bad_result_access(); },
                [](const auto&)         -> value_type { throw bad_result_access(); }
            });
        }

        /**
         * Returns the value contained this result if it is successful. Otherwise, throws `bad_result_access`.
         *
         * @return the value in the given result
         *
         * @throw bad_result_access if this result is an error or if this result does not contain a value
         */
        auto get() && {
            return std::move(*this).visit(kdl::overload {
                [](value_type&& v) -> value_type { return std::move(v); },
                []()               -> value_type { throw bad_result_access(); },
                [](const auto&)    -> value_type { throw bad_result_access(); }
            });
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

        friend std::ostream& operator<<(std::ostream& str, const result& result_) {
            if (result_.m_value.has_value()) {
                std::visit([&](const auto& v){ str << v; }, result_.m_value.value());
            }
            return str;
        }
    };
}

#endif /* result_h */
