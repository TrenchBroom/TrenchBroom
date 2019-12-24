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

#ifndef KDL_GLOB_INDEX_H
#define KDL_GLOB_INDEX_H

#include <kdl/string_compare.h>
#include <kdl/string_format.h>
#include <kdl/vector_utils.h>

#include <exception>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace kdl {
    template <typename V>
    class glob_index {
    private:
        struct node_cmp;

        class node {
        private:
            friend struct node_cmp;
            using value_container = std::unordered_map<V, std::size_t>;
            using node_set = std::set<node, node_cmp>;

            mutable std::string m_key;
            mutable value_container m_values;
            mutable node_set m_children;
        public:
            explicit node(std::string key) :
            m_key(std::move(key)) {}

            void insert(const std::string_view& key, const V& value) const {
                /*
                 Possible cases for insertion:
                  index: 01234567 |   | #m_key: 6
                  m_key: target   | ^ | #key | conditions              | todo
                 =================|===|======|=========================|======
                  case:  key:     |   |      |                         |
                     0:  blah     | 0 | 4    | ^ = 0                   | do nothing
                         ^        |   |      |                         |
                     1:  targetli | 6 | 8    | ^ < #key AND ^ = #m_key | try insert in all children, if none match, create child 'li' and insert there;
                               ^  |   |      |                         |
                     2:  tarus    | 3 | 5    | ^ < #key AND ^ < #m_key | split this node in 'tar' and 'get'; create child 'us' and insert there;
                            ^     |   |      |                         |
                     3:  tar      | 3 | 3    | ^ = #key AND ^ < #m_key | split this node in 'tar' and 'get'; insert here; return true;
                            ^     |   |      |                         |
                     4:  target   | 6 | 6    | ^ = #key AND ^ = #m_key | insert here; return true;
                               ^  |   |      |                         |
                 ==================================================================================
                  ^ indicates where key and m_key first differ
                 */

                // find the index of the first character where the given key and this node's key differ
                const std::size_t mismatch = kdl::cs::str_mismatch(key, m_key);
                if (mismatch == 0u && !m_key.empty()) {
                    // case 0: there is no common prefix, we cannot insert here
                    return;
                } else if (mismatch < key.size()) {
                    // cases 1, 2: key and m_key have a common prefix, or m_key is a prefix of key
                    if (mismatch == m_key.size()) {
                        // case 1: m_key is a prefix of key, find or create a child that has a common prefix with
                        // the remainder of key and insert there
                        const auto remainder = key.substr(mismatch);
                        const auto& child = *m_children.insert(node(std::string(remainder))).first;
                        child.insert(remainder, value);
                    } else { // mismatch == m_key.size()
                        // case 2: key and m_key have a common prefix, split this node and insert again
                        split_node(mismatch);
                        insert(key, value);
                    }
                } else if (mismatch == key.size()) {
                    // cases 3, 4: key is a prefix of m_key, or key == m_key
                    if (mismatch < m_key.size()) {
                        // case 3: key is a prefix of m_key, split this node
                        split_node(mismatch);
                    }
                    insert_value(value);
                }
            }

            bool remove(const std::string_view& key, const V& value) const {
                bool result = false;

                const std::size_t mismatch = kdl::cs::str_mismatch(key, m_key);
                if (m_key.size() <= key.length() && mismatch == m_key.length()) {
                    // m_key is a prefix of key or m_key == key
                    if (mismatch < key.length()) {
                        // m_key is a true prefix of key, continue at the corresponding child node
                        const auto remainder = key.substr(mismatch);
                        const auto it = m_children.find(remainder);
                        assert(it != std::end(m_children));

                        result = it->remove(remainder, value);
                        if (!it->m_key.empty() && it->m_values.empty() && it->m_children.empty()) {
                            m_children.erase(it);
                        }
                    } else {
                        // m_key == key
                        result = remove_value(value);
                    }

                    if (!m_key.empty() && m_values.empty() && m_children.size() == 1u) {
                        merge_node();
                    }
                }

                return result;
            }

            template <typename O>
            void query(const std::string_view& pattern, const std::size_t pattern_position, O out) const {
                using match_state = std::pair<std::size_t, std::size_t>;

                std::vector<match_state> match_states({{ 0u, pattern_position }});
                while (!match_states.empty()) {
                    const auto match_state = match_states.back();
                    match_states.pop_back();
                    const auto k_i = match_state.first;
                    const auto p_i = match_state.second;

                    if (k_i == m_key.length() && p_i == pattern.length()) {
                        // both the key and the pattern are consumed, so we have a match
                        get_values(out);
                        continue;
                    }

                    if (p_i == pattern.length()) {
                        // the pattern is consumed by the key isn't, we cannot have a match here
                        continue;
                    }

                    // after this point, we can assume that the pattern is not consumed, but the key might be
                    if (pattern[p_i] == '\\' && p_i < pattern.length() - 1u) {
                        // handle escaped characters in the pattern
                        const auto& n = pattern[p_i + 1u];

                        if (k_i < m_key.length()) {
                            // check the next character in the pattern against the next character in the key
                            if (n == '*' || n == '?' || n == '%' || n == '\\') {
                                if (m_key[k_i] == n) {
                                    // the key matches the escaped character, continue
                                    match_states.emplace_back(k_i + 1u, p_i + 2u);
                                }
                            } else {
                                throw std::invalid_argument("invalid escape sequence in pattern");
                            }
                        } else {
                            // the key is consumed, so continue matching at the children
                            for (const auto& c : { "*", "?", "%", "\\" }) {
                                const auto it = m_children.find(c);
                                if (it != std::end(m_children)) {
                                    it->query(pattern, p_i, out);
                                }
                            }
                        }
                    } else if (pattern[p_i] == '*') {
                        // handle '*' in the pattern
                        if (p_i == pattern.length() - 1u) {
                            // the pattern is consumed after the '*', so it matches all keys in this node's subtree
                            get_values_and_recurse(out);
                            return;
                        }

                        if (k_i < m_key.length()) {
                            // '*' matches any character
                            // consume the '*' and continue matching at the current character of the key
                            match_states.emplace_back(k_i, p_i + 1u);
                            // consume the current character of the key and continue matching at '*'
                            match_states.emplace_back(k_i + 1u, p_i);
                        } else {
                            // the key is consumed, so continue matching at the children
                            for (const auto& child : m_children) {
                                child.query(pattern, p_i, out);
                            }
                        }
                    } else if (pattern[p_i] == '?') {
                        // handle '?' in the pattern
                        if (k_i < m_key.length()) {
                            // '?' matches any character, continue at the next chars in both the pattern and the key
                            match_states.emplace_back(k_i + 1u, p_i + 1u);
                        } else {
                            // the key is consumed, so continue matching at the children
                            for (const auto& child : m_children) {
                                child.query(pattern, p_i, out);
                            }
                        }
                    } else if (pattern[p_i] == '%') {
                        // handle '%' in the pattern
                        if (p_i < pattern.length() - 1u && pattern[p_i + 1u] == '*') {
                            // handle "%*" in the pattern
                            // try to continue matching after "%*"
                            match_states.emplace_back(k_i, p_i + 2u);
                            if (k_i < m_key.length()) {
                                if (m_key[k_i] >= '0' && m_key[k_i] <= '9') {
                                    // try to match more digits
                                    match_states.emplace_back(k_i + 1u, p_i);
                                }
                            } else {
                                // the key is consumed, so continue matching at the children
                                for (auto it = m_children.lower_bound("0"), end = m_children.upper_bound("9"); it != end; ++it) {
                                    it->query(pattern, p_i, out);
                                }
                            }
                        } else {
                            if (k_i < m_key.length()) {
                                // handle '%' in the pattern (not followed by '*')
                                if (m_key[k_i] >= '0' && m_key[k_i] <= '9') {
                                    // continue matching after the digit
                                    match_states.emplace_back(k_i + 1u, p_i + 1u);
                                }
                            } else {
                                // the key is consumed, so continue matching at the children
                                for (auto it = m_children.lower_bound("0"), end = m_children.upper_bound("9"); it != end; ++it) {
                                    it->query(pattern, p_i, out);
                                }
                            }
                        }
                    } else {
                        if (k_i < m_key.length()) {
                            if (pattern[p_i] == m_key[k_i]) {
                                // handle a regular character in the pattern
                                match_states.emplace_back(k_i + 1u, p_i + 1u);
                            }
                        } else {
                            // the key is consumed, so continue matching at the children
                            for (auto [it, end] = m_children.equal_range(pattern.substr(p_i, 1u)); it != end; ++it) {
                                it->query(pattern, p_i, out);
                            }
                        }
                    }
                }
            }
        private:
            void insert_value(const V& value) const {
                m_values[value]++;
            }

            bool remove_value(const V& value) const {
                auto it = m_values.find(value);
                if (it == std::end(m_values)) {
                    return false;
                } else {
                    if (--(it->second) == 0u) {
                        m_values.erase(it);
                    }
                    return true;
                }
            }

            void split_node(const std::size_t index) const {
                assert(m_key.length() > 1u);
                assert(index < m_key.length());

                auto new_key = m_key.substr(0u, index);
                auto remainder = m_key.substr(index);

                using std::swap;
                node_set new_children;
                swap(new_children, m_children);

                const node& new_child = *m_children.insert(node(std::move(remainder))).first;
                swap(new_child.m_children, new_children);
                swap(new_child.m_values, m_values);

                m_key = std::move(new_key);
            }

            void merge_node() const {
                assert(m_children.size() == 1u);
                assert(m_values.empty());

                using std::swap;
                node_set old_children;
                swap(old_children, m_children);

                const node& child = *std::begin(old_children);
                swap(m_children, child.m_children);
                swap(m_values, child.m_values);

                m_key += child.m_key;
            }

            template <typename O>
            void get_values(O out) const {
                for (const auto& [value, count] : m_values) {
                    for (std::size_t i = 0u; i < count; ++i) {
                        out++ = value;
                    }
                }
            }

            template <typename O>
            void get_values_and_recurse(O out) const {
                get_values(out);
                for (const auto& child : m_children) {
                    child.get_values_and_recurse(out);
                }
            }
        };

        struct node_cmp {
            using is_transparent = void;

            bool operator()(const node& lhs, const node& rhs) const {
                return compare(lhs.m_key, rhs.m_key);
            }

            bool operator()(const std::string_view& lhs, const node& rhs) const {
                return compare(lhs, rhs.m_key);
            }

            bool operator()(const node& lhs, const std::string_view& rhs) const {
                return compare(lhs.m_key, rhs);
            }

            template <typename T1, typename T2>
            bool is_equivalent(const T1& lhs, const T2& rhs) const {
                return !(*this)(lhs, rhs) && !(*this)(rhs, lhs);
            }

            bool compare(const std::string_view& lhs, const std::string_view& rhs) const {
                const std::size_t mismatch = kdl::cs::str_mismatch(lhs, rhs);
                if (mismatch == 0u) {
                    return lhs[0] < rhs[0];
                } else {
                    // both keys share a common prefix and are considered equivalent
                    return false;
                }
            }
        };
    private:
        node m_root;
    public:
        glob_index() :
        m_root(node("")) {}

        void insert(const std::string_view& key, const V& value) {
            m_root.insert(key, value);
        }

        bool remove(const std::string_view& key, const V& value) {
            return m_root.remove(key, value);
        }

        void clear() {
            m_root = node("");
        }

        template <typename O>
        void query(const std::string_view& pattern, O out) const {
            m_root.query(pattern, { 0u }, out);
        }
    };
}

#endif //KDL_GLOB_INDEX_H
