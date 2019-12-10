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

#ifndef TRENCHBROOM_TRANSFORM_RANGE_H
#define TRENCHBROOM_TRANSFORM_RANGE_H

#include <type_traits>

namespace kdl {
    template <typename I, typename L>
    class transform_iterator {
    public:
        using iterator_category = typename I::iterator_category;
        using value_type = std::result_of_t<L(typename I::value_type)>;
        using difference_type = typename I::difference_type;
        using pointer = value_type*;
        using reference = value_type&;
    private:
        I m_iter;
        L m_lambda;
    public:
        transform_iterator(I iter, L lambda) :
        m_iter(iter),
        m_lambda(std::move(lambda)) {}

        bool operator<(const transform_iterator& other) const { return m_iter <  other.m_iter; }
        bool operator>(const transform_iterator& other) const { return m_iter >  other.m_iter; }
        bool operator==(const transform_iterator& other) const { return m_iter == other.m_iter; }
        bool operator!=(const transform_iterator& other) const { return m_iter != other.m_iter; }

        // prefix
        transform_iterator& operator++() { ++m_iter; return *this; }
        transform_iterator& operator--() { --m_iter; return *this; }

        // postfix
        transform_iterator operator++(int) { transform_iterator result(*this); ++m_iter; return result; }
        transform_iterator operator--(int) { transform_iterator result(*this); --m_iter; return result; }

        value_type operator*()  { return m_lambda(*m_iter); }
    };

    template <typename C, typename L>
    class transform_adapter {
    public:
        using const_iterator = transform_iterator<typename C::const_iterator, L>;
        using const_reverse_iterator = transform_iterator<typename C::const_reverse_iterator, L>;
    private:
        const C& m_container;
        const L m_lambda;
    public:
        explicit transform_adapter(const C& container, L lambda) :
        m_container(container),
        m_lambda(std::move(lambda)) {}

        bool empty() const {
            return m_container.empty();
        }

        std::size_t size() const {
            return m_container.size();
        }

        const_iterator begin() const {
            return cbegin();
        }

        const_iterator end() const {
            return cend();
        }

        const_iterator cbegin() const {
            return const_iterator(std::cbegin(m_container), m_lambda);
        }

        const_iterator cend() const {
            return const_iterator(std::cend(m_container), m_lambda);
        }

        const_reverse_iterator rbegin() const {
            return crbegin();
        }

        const_reverse_iterator rend() const {
            return crend();
        }

        const_reverse_iterator crbegin() const {
            return const_reverse_iterator(std::crbegin(m_container), m_lambda);
        }

        const_reverse_iterator crend() const {
            return const_reverse_iterator(std::crend(m_container), m_lambda);
        }
    };

    /**
     * Deduction guide.
     */
    template <typename C, typename L>
    transform_adapter(const C& container, L lambda) -> transform_adapter<C, L>;
}

#endif //TRENCHBROOM_TRANSFORM_RANGE_H
