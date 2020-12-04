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

#include <iterator>
#include <tuple>

namespace kdl {
    /**
     * Wraps several iterators and offers their current values as a tuple of references.
     * 
     * The iterators are incremented simultaneously. Comparison operators only consider the first iterator, which
     * implies the requirement that the iterated ranges have the same number of elements. Violating this expectation
     * yields undefined behavior.
     * 
     * Dereferencing a zip iterator returns a tuple of references to the current values of the iterators.
     * 
     * @tparam I the types of the iterators
     */
    template <typename... I>
    class zip_iterator {
    public:
        static_assert(sizeof...(I) > 0, "At least one iterator is required.");

        using iterator_category = std::forward_iterator_tag;
        using difference_type = long;
        using value_type = std::tuple<typename I::reference...>;
        using pointer = value_type*;
        using reference = value_type&;
    private:
        using tuple_type = std::tuple<I...>;
        tuple_type m_iters;
    public:
        /**
         * Creates a zip iterator of the given iterators
         * 
         * @param iters the iterators
         */
        zip_iterator(I... iters) :
        m_iters(std::forward<I>(iters)...) {}

        friend bool operator<(const zip_iterator& lhs, const zip_iterator& rhs) { return std::get<0>(lhs.m_iters) < std::get<0>(rhs.m_iters); }
        friend bool operator>(const zip_iterator& lhs, const zip_iterator& rhs) { return std::get<0>(lhs.m_iters) > std::get<0>(rhs.m_iters); }

        friend bool operator==(const zip_iterator& lhs, const zip_iterator& rhs) { return std::get<0>(lhs.m_iters) == std::get<0>(rhs.m_iters); }
        friend bool operator!=(const zip_iterator& lhs, const zip_iterator& rhs) { return std::get<0>(lhs.m_iters) != std::get<0>(rhs.m_iters); }

        zip_iterator& operator++() {
            advance();
            return *this;
        }

        zip_iterator operator++(int) {
            auto result = zip_iterator(*this);
            advance();
            return result;
        }

        value_type operator*()  const { return dereference(); }
        value_type operator->() const { return dereference(); }
    private:
        template <size_t... Idx>
        void advance(std::index_sequence<Idx...>) {
            (..., ++std::get<Idx>(m_iters));
        }

        void advance() {
            advance(std::make_index_sequence<std::tuple_size_v<tuple_type>>{});
        }

        template <size_t... Idx>
        value_type dereference(std::index_sequence<Idx...>) const {
            return std::forward_as_tuple(*std::get<Idx>(m_iters)...);
        }

        value_type dereference() const {
            return dereference(std::make_index_sequence<std::tuple_size_v<tuple_type>>{});
        }
    };

    /**
     * Deduction guide.
     */
    template <typename... I>
    zip_iterator(I... iters) -> zip_iterator<I...>;

    /**
     * Creates a zip iterator with iterators pointing to the beginning of each of the given ranges.
     * 
     * @tparam C the types of the ranges
     * @param c the ranges to iterate
     */
    template <typename... C>
    auto make_zip_begin(C&&... c) {
        return zip_iterator(std::begin(std::forward<C>(c))...);
    }

    /**
     * Creates a zip iterator with iterators pointing to the end of each of the given ranges.
     * 
     * @tparam C the types of the ranges
     * @param c the ranges to iterate
     */
    template <typename... C>
    auto make_zip_end(C&&... c) {
        return zip_iterator(std::end(std::forward<C>(c))...);
    }

    /**
     * A zip range returns zip iterators for begin() and end().
     * 
     * @tparam I the types of the individual iterators
     */
    template <typename I>
    struct zip_range {
        I m_begin;
        I m_end;

        I begin() const {
            return m_begin;
        }

        I end() const {
            return m_end;
        }
    };

    /**
     * Creates a zip range of the given ranges.
     * 
     * @tparam C the types of the ranges
     * @param c the ranges to iterate
     */
    template <typename... C>
    auto make_zip_range(C&&... c) {
        using I = decltype(make_zip_begin(std::forward<C>(c)...));
        return zip_range<I>{make_zip_begin(std::forward<C>(c)...), make_zip_end(std::forward<C>(c)...)};
    }
}

#endif //KDL_ZIP_ITERATOR_H
