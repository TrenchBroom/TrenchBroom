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

#include <cassert>
#include <map>
#include <set>

namespace kdl {
    template <typename L, typename R, typename Cmp_L = std::less<L>, typename Cmp_R = std::less<R>>
    class binary_relation {
    private:
        using left_set = std::set<L, Cmp_L>;
        using right_set = std::set<R, Cmp_R>;

        using left_right_map = std::map<L, right_set, Cmp_L>;
        using right_left_map = std::map<R, left_set, Cmp_R>;

        using const_left_iterator = typename left_set::const_iterator;
        using const_right_iterator = typename right_set::const_iterator;
        using const_left_range = std::pair<const_left_iterator, const_left_iterator>;
        using const_right_range = std::pair<const_right_iterator, const_right_iterator>;
        using pair_type = std::pair<L, R>;

        left_right_map m_left_right_map;
        right_left_map m_right_left_map;
        std::size_t m_size;
    public:
        /**
         * Iterates over all pairs in a binary relation.
         */
        template <typename left_iter, typename right_iter>
        class iterator_base {
        public:
            using iterator_category = std::forward_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = pair_type;
            using pointer = pair_type*;
            using reference = pair_type&;
        private:
            left_iter m_left;
            left_iter m_left_end;
            right_iter m_right;
        public:
            iterator_base(left_iter i_left, left_iter i_left_end) :
            m_left(i_left),
            m_left_end(i_left_end) {
                if (has_more()) {
                    m_right = right_begin();
                    assert(!is_at_right_end());
                }
            }

            bool operator==(const iterator_base& other) const {
                return equals(other);
            }

            bool operator!=(const iterator_base& other) const {
                return !equals(other);
            }

            // prefix increment
            iterator_base& operator++() {
                advance();
                return *this;
            }

            // postfix increment
            iterator_base operator++(int) {
                iterator_base result(*this);
                advance();
                return result;
            }

            pair_type operator*() const {
                assert(has_more() && !is_at_right_end());

                const auto& left = m_left->first;
                const auto& right = *m_right;

                return std::make_pair(left, right);
            }
        private:
            bool equals(const iterator_base& other) const {
                if (!has_more() && !other.has_more()) {
                    return true;
                } else {
                    return m_left == other.m_left && m_right == other.m_right;
                }
            }

            /*
             Advance until we found the next valid value.
             */
            void advance() {
                if (!has_more()) {
                    return;
                }

                ++m_right;

                if (is_at_right_end()) {
                    ++m_left;
                    if (has_more()) {
                        m_right = right_begin();
                        assert(!is_at_right_end());
                    }
                }
            }

            bool has_more() const {
                return m_left != m_left_end;
            }

            bool is_at_right_end() const {
                assert(has_more());
                return m_right == right_end();
            }

            right_iter right_begin() const {
                assert(has_more());
                return std::begin(m_left->second);
            }

            right_iter right_end() const {
                assert(has_more());
                return std::end(m_left->second);
            }
        };

        using const_iterator = iterator_base<typename left_right_map::const_iterator, const_right_iterator>;
    public:
        /**
         * Creates an empty binary relation.
         */
        binary_relation() :
        m_size(0u) {}

        /**
         * Creates a binary relation with the given entries.
         *
         * @param list the entries for the relation
         */
        binary_relation(std::initializer_list<std::pair<L, R>> list) :
        m_size(0u) {
            for (const auto& pair : list) {
                insert(pair.first, pair.second);
            }
        }


        /**
         * Indicates whether this binary relation is empty.
         */
        bool empty() {
            return size() == 0u;
        }

        /**
         * Returns the number of pairs in this binary relation.
         */
        std::size_t size() const {
            return m_size;
        }

        /**
         * Indicates whether this binary relation contains the given pair.
         *
         * @param l the left value
         * @param r the right value
         * @return true if this binary relation contains the given pair, and false otherwise
         */
        bool contains(const L& l, const R& r) const {
            return find_right(l).count(r) > 0u;
        }

        /**
         * Returns the number of pairs in this binary relation which have the given right value.
         *
         * @param r the right value
         * @return the number of pairs with the given right value
         */
        std::size_t count_left(const R& r) const {
            return find_left(r).size();
        }

        /**
         * Returns the number of pairs in this binary relation which have the given left value.
         *
         * @param l the left value
         * @return the number of pairs with the given left value
         */
        std::size_t count_right(const L& l) const {
            return find_right(l).size();
        }

        /**
         * Returns an iterator to iterate over all pairs of values stored in this binary relation.
         */
        auto begin() const {
            return cbegin();
        }

        /**
         * Returns an iterator to iterate over all pairs of values stored in this binary relation.
         */
        auto cbegin() const {
            return const_iterator(std::begin(m_left_right_map), std::end(m_left_right_map));
        }

        /**
         * Returns an iterator to the pair following the last pair of values stored in this binary relation.
         */
        auto end() const {
            return cend();
        }

        /**
         * Returns an iterator to the pair following the last pair of values stored in this binary relation.
         */
        auto cend() const {
            return const_iterator(std::end(m_left_right_map), std::end(m_left_right_map));
        }

        /**
         * Returns a pair of iterators for the range of left values associated with the given right value.
         *
         * @param r the right value
         * @return a pair of iterators for the range of left values associated with r
         */
        const_left_range left_range(const R& r) const {
            const auto& l_set = find_left(r);
            return std::make_pair(std::begin(l_set), std::end(l_set));
        }

        /**
         * Returns an iterator to the first left value associated with the given right value.
         *
         * @param r the right value
         * @return an iterator to the first left value associated with r
         */
        const_left_iterator left_begin(const R& r) const {
            return std::begin(find_left(r));
        }

        /**
         * Returns an iterator to the value following the last left value associated with the given right value.
         * @param r the right value
         * @return an iterator to the value following the last left value associated with r
         */
        const_left_iterator left_end(const R& r) const {
            return std::end(find_left(r));
        }

        /**
         * Returns a pair of iterators for the range of right values associated with the given left value.
         *
         * @param l the left value
         * @return a pair of iterators for the range of right values associated with l
         */
        const_right_range right_range(const L& l) const {
            const auto& r_set = find_right(l);
            return std::make_pair(std::begin(r_set), std::end(r_set));
        }

        /**
         * Returns an iterator to the first right value associated with the given left value.
         *
         * @param l the left value
         * @return an iterator to the first right value associated with l
         */
        const_right_iterator right_begin(const L& l) const {
            return std::begin(find_right(l));
        }

        /**
         * Returns an iterator to the value following the last right value associated with the given left value.
         * @param l the left value
         * @return an iterator to the value following the last right value associated with l
         */
        const_right_iterator right_end(const L& l) const {
            return std::end(find_right(l));
        }

        /**
         * Inserts all related pairs of elements from the given binary relation.
         *
         * @param other the binary relation to insert into this binary relation
         */
        void insert(const binary_relation& other) {
            for (const auto& [l, o_r] : other.m_left_right_map) {
                auto& m_r = find_or_insert_right(l);
                const auto m_r_s = m_r.size();

                m_r.insert(std::begin(o_r), std::end(o_r));
                m_size += (m_r.size() - m_r_s);
            }

            for (const auto& [r, o_l] : other.m_right_left_map) {
                auto& m_l = find_or_insert_left(r);
                m_l.insert(std::begin(o_l), std::end(o_l));
            }
        }

        /**
         * For each value r in range [r_range.first, r_range.second), inserts a pair (l, r) into this map.
         *
         * @tparam I the range iterator type
         * @param l the left value to insert
         * @param r_range the range of right values to insert
         */
        template <typename I>
        void insert(const L& l, const std::pair<I, I> r_range) {
            insert(l, r_range.first, r_range.second);
        }

        /**
         * For each value r in range [r_cur, r_end), inserts a pair (l, r) into this map.
         *
         * @tparam I the range iterator type
         * @param l the left value to insert
         * @param r_cur the start of the range of right values to insert
         * @param r_end the end of the range of right values to insert
         */
        template <typename I>
        void insert(const L& l, I r_cur, I r_end) {
            auto& r_set = find_or_insert_right(l);

            while (r_cur != r_end) {
                const auto& r = *r_cur;
                if (r_set.insert(r).second) {
                    insert_right_to_left(l, r);
                    ++m_size;
                }
                ++r_cur;
            }
        }

        /**
         * For each value l in range [l_range.first, l_range.second), inserts a pair (l, r) into this map.
         *
         * @tparam I the range iterator type
         * @param l_range the range of left values to insert
         * @param r the right value to insert
         */
        template <typename I>
        void insert(const std::pair<I, I>& l_range, const R& r) {
            insert(l_range.first, l_range.second, r);
        }

        /**
         * For each value l in range [l_cur, l_end), inserts a pair (l, r) into this map.
         *
         * @tparam I the range iterator type
         * @param l_cur the start of the range of left values to insert
         * @param l_end the end of the range of left values to insert
         * @param r the right value to insert
         */
        template <typename I>
        void insert(I l_cur, I l_end, const R& r) {
            auto& l_set = find_or_insert_left(r);

            while (l_cur != l_end) {
                const auto& l = *l_cur;
                if (l_set.insert(l).second) {
                    insert_left_to_right(l, r);
                    ++m_size;
                }
                ++l_cur;
            }
        }

        /**
         * Inserts the given pair of values. If the given pair is already contained in this relation, nothing
         * happens.
         *
         * @param l the left value to insert
         * @param r the right value to insert
         * @return true if the given pair was inserted and false otherwise
         */
        bool insert(const L& l, const R& r) {
            if (!insert_left_to_right(l, r)) {
                return false;
            } else {
                const auto result = insert_right_to_left(l, r);
                assert(result);
                ++m_size;
                return result;
            }
        }

        /**
         * Erases the given pair of values.
         *
         * @param l the left value
         * @param r the right value
         * @return true if this binary relation contained the given pair, and false otherwise
         */
        bool erase(const L& l, const R& r) {
            auto lr_it = m_left_right_map.find(l);
            if (lr_it == std::end(m_left_right_map)) {
                return false;
            }

            auto& r_set = lr_it->second;
            if (r_set.erase(r) > 0u) {
                auto rl_it = m_right_left_map.find(r);
                assert(rl_it != std::end(m_right_left_map));

                auto& l_set = rl_it->second;
                const auto c = l_set.erase(l);
                assert(c == 1u);
                m_size -= c;

                if (r_set.empty()) {
                    m_left_right_map.erase(lr_it);
                }
                if (l_set.empty()) {
                    m_right_left_map.erase(rl_it);
                }

                return true;
            } else {
                assert(find_left(r).count(l) == 0u);
                return false;
            }
        }
    private:
        const left_set& find_left(const R& r) const {
            static const left_set empty_left_set;

            auto rl_it = m_right_left_map.find(r);
            if (rl_it == std::end(m_right_left_map)) {
                return empty_left_set;
            } else {
                return rl_it->second;
            }
        }

        const right_set& find_right(const L& l) const {
            static const right_set empty_right_set;

            auto lr_it = m_left_right_map.find(l);
            if (lr_it == std::end(m_left_right_map)) {
                return empty_right_set;
            } else {
                return lr_it->second;
            }
        }

        template <typename RR>
        bool insert_left_to_right(const L& l, RR&& r) {
            return find_or_insert_right(l).insert(std::forward<RR>(r)).second;
        }

        template <typename LL>
        bool insert_right_to_left(LL&& l, const R& r) {
            return find_or_insert_left(r).insert(std::forward<LL>(l)).second;
        }

        left_set& find_or_insert_left(const R& r) {
            return m_right_left_map.try_emplace(r).first->second;
        }

        right_set& find_or_insert_right(const L& l) {
            return m_left_right_map.try_emplace(l).first->second;
        }
    };
}

#endif //KDL_BINARY_RELATION_H
