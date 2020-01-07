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

#include <cassert>
#include <exception>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace kdl {
    /**
     * Maps string keys to values, but with more efficient storage characteristics than a regular std::map. Another
     * difference is that values can be stored multiple times in each node.
     *
     * A trie is implemented as a search tree with ordered nodes. Each node in the trie is associated with a string key,
     * and it stores all of the values which were inserted with that key. However, a node `n` only stores a suffix of the
     * key it is associated with, and the full key can be restored by concatenating all partial keys stored at the nodes
     * on the path from the root to node `n` itself. This implies that the keys of all of the children of node `n`
     * contain `n`'s key as their prefix.
     *
     * Consider an example of a trie where the following keys and values have been inserted:
     * - key: "key",     value: "value"
     * - key: "key",     value: "again"
     * - key: "key_1",   value: "some value"
     * - key: "key_2",   value: "another value"
     * - key: "key_223", value: "value"
     * - key: "test"     value: "test value"
     * - key: "testing"  value: "testing testing"
     *
     * The trie then has the following structure:
     * - { key: "", values: {} }
     *   - { key: "key", values: { "value", "again" } }
     *     - { key: "_1, values: { "some value" } }
     *     - { key: "_2, values: { "another value" } }
     *       - { key: "23, values: { "value" } }
     * - { key: "test, values: { "test value" } }
     *   - { key: "ing, values: { "testing testing" } }
     *
     * @tparam V the type of the values associated with each node
     */
    template <typename V>
    class compact_trie {
    private:
        struct node_cmp;
        class node;

        /**
         * To avoid matching the same node multiple times using different partial patterns, we store tsome state for
         * each node that is encountered during matching. For each node, we remember its parent node, whether or not the
         * node was previously matched by a partial pattern, and whether or not all children of the node are already
         * fully matched.
         *
         * A node is fully matched if the node itself was matched and each of its children is fully matched.
         */
        class match_state {
        private:
            struct node_match_state {
                /**
                 * The parent of a node.
                 */
                const node* parent;

                /**
                 * Indicates whether a node was matched by a pattern.
                 */
                bool node_matched;

                /**
                 * The number of fully matched children.
                 */
                std::size_t fully_matched_children;
            public:
                /**
                 * Creates a new state with the given parent. `node_matched` is initialized to `false` and
                 * `fully_matched_children` to 0.
                 *
                 * @param i_parent the parent, can be null
                 */
                explicit node_match_state(const node* i_parent) :
                parent(i_parent),
                node_matched(false),
                fully_matched_children(0u) {}
            };

            std::unordered_map<const node*, node_match_state> m_state;
        public:
            /**
             * Inserts a match state for the given node and its parent.
             *
             * @param n the node, must not be null
             * @param parent the parent, may be null
             */
            void insert(const node* n, const node* parent) {
                assert(n != nullptr);
                m_state.try_emplace(n, parent);
            }

            /**
             * Indicates whether the given node is fully matched.
             *
             * Precondition: the node must have an associated state (insert was previously called for the node)
             *
             * @param n the node to check
             * @return true if the given node is fully matched and false otherwise
             */
            bool is_fully_matched(const node* n) {
                auto it = m_state.find(n);
                assert(it != std::end(m_state));
                const auto& state = it->second;
                return state.node_matched && state.fully_matched_children == n->m_children.size();
            }

            /**
             * Sets the given node to be fully matched. Note that none of the node's descendents are set to be fully
             * matched, but this is not necessary as the match algorithm will not traverse into the given node's subtree
             * anymore.
             *
             * Precondition: the node must have an associated state (insert was previously called for the node)
             *
             * @param n the node to set to fully matched
             */
            void set_fully_matched(const node* n) {
                auto it = m_state.find(n);
                assert(it != std::end(m_state));

                auto& state = it->second;
                state.node_matched = true;
                state.fully_matched_children = n->m_children.size();
                update_parent_states(state.parent);
            }

            /**
             * Sets the given node to be matched if it isn't already. If the given node is already matched, then the
             * function immediately returns `false`. Otherwise, the given node's state is set to matched, and if the node
             * is now fully matched, its parents are updated recursively in case they are now fully matched. In this
             * case, the function returns `true`.
             *
             * Precondition: the node must have an associated state (insert was previously called for the node)
             *
             * @param n the node to set to matched
             * @return `false` if the given node is already matched, and `true` otherwise
             */
            bool set_matched(const node* n) {
                auto it = m_state.find(n);
                assert(it != std::end(m_state));

                auto& state = it->second;
                if (state.node_matched) {
                    return false;
                }

                state.node_matched = true;
                if (state.fully_matched_children == n->m_children.size()) {
                    // update the subtree match counts of all nodes on the path to the given node
                    update_parent_states(state.parent);
                }

                return true;
            }
        private:
            void update_parent_states(const node* n) {
                while (n != nullptr) {
                    auto it = m_state.find(n);
                    assert(it != std::end(m_state));

                    auto& state = it->second;
                    state.fully_matched_children += 1u;
                    if (!state.node_matched || state.fully_matched_children < n->m_children.size()) {
                        // parent is not fully matched, so it cannot contribute to its parents' subtree match count yet
                        break;
                    }

                    n = it->second.parent;
                }
            }
        };

        /**
         * A trie node. Children are stored in a set ordered by `node_cmp`. Each node can store a given value multiple
         * times.
         *
         * A trie node has only const methods and therefore appears to be immutable, but this is not the case. All of
         * its members are actually declared as mutable and may be modified by the `insert` or `remove` member
         * functions. The reason for this peculiar design choice is that trie nodes are stored by value in a `std::set`,
         * and `std::set` does not allow its contents to be modified, since that might affect the position of the value
         * in the set.
         *
         * But in this particular case, the position of a node in the containing set depends only on the node's key, and
         * the node's key is changed only in ways that does not affect its position. The key can only become shorter
         * or longer, that is, key "abc" might be changed to "ab" or "abcd", and neither of these changes affects how
         * the key compares to other keys due to the order introduce by `node_cmp`, which consideres two keys equivalent
         * if they share a non-empty prefix.
         *
         * This implies that the following two conditions hold for a node with key "abc":
         * - There is no sibling node with a key that is a prefix of "abc".
         * - There is no sibling node with a key that that has "abc" as a prefix.
         *
         * Therefore, shortening or extending a node's key does not affect its position in the containing set, and we
         * can update a node's key without violating any invariants of the containing set.
         */
        class node {
        private:
            friend struct node_cmp;
            friend class match_state;

            using value_container = std::unordered_map<V, std::size_t>;
            using node_set = std::set<node, node_cmp>;

            /**
             * The partical key of this node.
             */
            mutable std::string m_key;

            /**
             * Maps a value to the number of times it was stored in this node.
             */
            mutable value_container m_values;

            /**
             * The children of this node.
             */
            mutable node_set m_children;
        public:
            /**
             * Creates a new node with the given key.
             *
             * @param key the key
             */
            explicit node(std::string key) :
            m_key(std::move(key)) {}

            /**
             * Inserts the given value into this node's subtree. If this node's key is empty, then it is the root node.
             *
             * Precondition: Unless this node is the root node, the given key must share a non-empty prefix with this
             * node's key.
             *
             * @param key the key to insert
             * @param value the value to insert
             */
            void insert(const std::string_view key, const V& value) const {
                /*
                 Possible cases for insertion:
                  index: 01234567 |   | #m_key: 6
                  m_key: target   | ^ | #key | conditions              | todo
                 =================|===|======|=========================|======
                  case:  key:     |   |      |                         |
                     0:  blah     | 0 | 4    | ^ = 0                   | this is the root node, find or create child 'blah' and insert there;
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
                assert(mismatch > 0u || m_key.empty());

                if (mismatch < key.size()) {
                    // cases 0, 1, 2: key and m_key have a common prefix, or m_key is a prefix of key
                    if (mismatch == m_key.size()) {
                        // case 0, 1: m_key is a prefix of key, find or create a child that has a common prefix with
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

            /**
             * Removes the given value from this node's subtree.
             *
             * @param key the key to remove
             * @param value the value to remove
             * @return true if the given key and value were removed from this node's subtree
             */
            bool remove(const std::string_view key, const V& value) const {
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

            /**
             * Finds every node in this node's subtree whose keys match a pattern, and adds the values to the given
             * output iterator.
             *
             * The keys are matched against a suffix of the given pattern starting at the given position. The matching
             * algorithm uses an auxiliary `match_State` to prevent matching unnecessarily matching nodes. This state
             * is updated in the following situations:
             *
             * - a node is visited for the first time
             * - a node is matches the given pattern (this might also update the node's parent's states)
             * - an entire subtree matches the given pattern (due to a trailing wildcard in the pattern)
             *
             * Using this information, the algorithm will stop matching a node if every node in its subtree was already
             * matched against the pattern. Furthermore, it will not add a node's values multiple times if the node's key
             * matches the pattern in more than one way. The latter situation can arise due to wildcards in the pattern.
             *
             * @tparam O the type of the given output iterator
             * @param pattern the pattern to match
             * @param pattern_position where to start matching the pattern
             * @param parent this node's parent (used to update the match_state)
             * @param match_state the match state
             * @param out the output iterator to which the values of matched nodes are added
             *
             * @throws std::invalid_argument if the given pattern contains an invalid escape sequence
             */
            template <typename O>
            void find_matches(const std::string_view pattern, const std::size_t pattern_position, const node* parent, match_state& match_state, O out) const {
                using match_task = std::pair<std::size_t, std::size_t>;

                match_state.insert(this, parent);

                std::vector<match_task> match_tasks({{ 0u, pattern_position }});
                while (!match_tasks.empty()) {
                    if (match_state.is_fully_matched(this)) {
                        // this node and all of its subtrees have been fully matched, so we are done here
                        return;
                    }

                    const auto [k_i, p_i] = match_tasks.back();
                    match_tasks.pop_back();

                    if (k_i == m_key.length() && p_i == pattern.length()) {
                        if (match_state.set_matched(this)) {
                            // this node was not matched yet, so fetch the results
                            get_values(out);
                        }

                        // there might still be children of this node that could be matched by a pending match task,
                        // so continue matching
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
                                    match_tasks.emplace_back(k_i + 1u, p_i + 2u);
                                }
                            } else {
                                throw std::invalid_argument("invalid escape sequence in pattern");
                            }
                        } else {
                            // the key is consumed, so continue matching at the children
                            for (const auto& c : { "*", "?", "%", "\\" }) {
                                const auto it = m_children.find(c);
                                if (it != std::end(m_children)) {
                                    it->find_matches(pattern, p_i, this, match_state, out);
                                }
                            }
                        }
                    } else if (pattern[p_i] == '*') {
                        // handle '*' in the pattern
                        if (p_i == pattern.length() - 1u) {
                            // the pattern is consumed after the '*', so it matches all keys in this node's subtree
                            match_state.set_fully_matched(this);
                            get_values_and_recurse(out);
                            return;
                        }

                        if (k_i < m_key.length()) {
                            // '*' matches any character
                            // consume the '*' and continue matching at the current character of the key
                            match_tasks.emplace_back(k_i, p_i + 1u);
                            // consume the current character of the key and continue matching at '*'
                            match_tasks.emplace_back(k_i + 1u, p_i);
                        } else {
                            // the key is consumed, so continue matching at the children
                            for (const auto& child : m_children) {
                                child.find_matches(pattern, p_i, this, match_state, out);
                            }
                        }
                    } else if (pattern[p_i] == '?') {
                        // handle '?' in the pattern
                        if (k_i < m_key.length()) {
                            // '?' matches any character, continue at the next chars in both the pattern and the key
                            match_tasks.emplace_back(k_i + 1u, p_i + 1u);
                        } else {
                            // the key is consumed, so continue matching at the children
                            for (const auto& child : m_children) {
                                child.find_matches(pattern, p_i, this, match_state, out);
                            }
                        }
                    } else if (pattern[p_i] == '%') {
                        // handle '%' in the pattern
                        if (p_i < pattern.length() - 1u && pattern[p_i + 1u] == '*') {
                            // handle "%*" in the pattern
                            // try to continue matching after "%*"
                            match_tasks.emplace_back(k_i, p_i + 2u);
                            if (k_i < m_key.length()) {
                                if (m_key[k_i] >= '0' && m_key[k_i] <= '9') {
                                    // try to match more digits
                                    match_tasks.emplace_back(k_i + 1u, p_i);
                                }
                            } else {
                                // the key is consumed, so continue matching at the children
                                for (auto it = m_children.lower_bound("0"), end = m_children.upper_bound("9"); it != end; ++it) {
                                    it->find_matches(pattern, p_i, this, match_state, out);
                                }
                            }
                        } else {
                            if (k_i < m_key.length()) {
                                // handle '%' in the pattern (not followed by '*')
                                if (m_key[k_i] >= '0' && m_key[k_i] <= '9') {
                                    // continue matching after the digit
                                    match_tasks.emplace_back(k_i + 1u, p_i + 1u);
                                }
                            } else {
                                // the key is consumed, so continue matching at the children
                                for (auto it = m_children.lower_bound("0"), end = m_children.upper_bound("9"); it != end; ++it) {
                                    it->find_matches(pattern, p_i, this, match_state, out);
                                }
                            }
                        }
                    } else {
                        if (k_i < m_key.length()) {
                            if (pattern[p_i] == m_key[k_i]) {
                                // handle a regular character in the pattern
                                match_tasks.emplace_back(k_i + 1u, p_i + 1u);
                            }
                        } else {
                            // the key is consumed, so continue matching at the children
                            for (auto [it, end] = m_children.equal_range(pattern.substr(p_i, 1u)); it != end; ++it) {
                                it->find_matches(pattern, p_i, this, match_state, out);
                            }
                        }
                    }
                }
            }

            /**
             * Adds the keys of all nodes in this subtree to the given output iterator.
             *
             * @tparam O the type of the output iterator
             * @param prefix the prefix of all keys in this subtree
             * @param out the output iterator
             */
            template <typename O>
            void get_keys(const std::string& prefix, O out) const {
                const auto key = prefix + m_key;
                if (!m_values.empty()) {
                    out++ = key;
                }

                for (const auto& child : m_children) {
                    child.get_keys(key, out);
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

            /**
             * Splits this node into two nodes at the given index of its key. For example, given a node n with key
             * "abcd" and index 2, the following will happen:
             * - n's key will be shortened to "ab"
             * - a new node c will be created to n with key "cd"
             * - all of n's children and values will be moved to c
             * - c will be added to n's children
             *
             * Precondition: This node's key has at least two characters, and the index is chosen in such a way that
             * neither of the resulting keys is empty.
             *
             * @param index the index at which to split the node's key
             */
            void split_node(const std::size_t index) const {
                assert(m_key.length() > 1u);

                auto new_key = m_key.substr(0u, index);
                auto remainder = m_key.substr(index);

                assert(!new_key.empty());
                assert(!remainder.empty());

                using std::swap;
                node_set new_children;
                swap(new_children, m_children);

                const node& new_child = *m_children.insert(node(std::move(remainder))).first;
                swap(new_child.m_children, new_children);
                swap(new_child.m_values, m_values);

                m_key = std::move(new_key);
            }

            /**
             * Merges this node with its only child. Thereby, this child node's key is appended to this node's key, the
             * child's children and values are moved to this node, and the child is removed.
             *
             * Precondition: This node has only one child, and this node has no values of its own.
             */
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

        /**
         * Compares nodes against each other or nodes against strings.
         *
         * Two nodes are compared by their keys.
         * A node is compared to a string by comparing its key to the string.
         *
         * Two strings are compared by their first character only, soif two nodes share a non-empty prefix, they are
         * considered equivalent. Therefore, the keys "ab" and "abc" are equivalent. Conversely, the keys "ab" and "bc"
         * are not equivalent because they do not share a prefix, more specifically, "ab" is considered less than "bc".
         *
         * Precondition: the strings to copmare are not empty.
         */
        struct node_cmp {
            using is_transparent = void;

            bool operator()(const node& lhs, const node& rhs) const {
                return compare(lhs.m_key, rhs.m_key);
            }

            bool operator()(const std::string_view lhs, const node& rhs) const {
                return compare(lhs, rhs.m_key);
            }

            bool operator()(const node& lhs, const std::string_view rhs) const {
                return compare(lhs.m_key, rhs);
            }

            bool compare(const std::string_view lhs, const std::string_view& rhs) const {
                assert(!lhs.empty() && !rhs.empty());
                return lhs[0] < rhs[0];
            }
        };
    private:
        node m_root;
    public:
        /**
         * Creates a new empty trie.
         */
        compact_trie() :
        m_root(node("")) {}

        /**
         * Inserts the given value under the given key.
         *
         * @param key the key to insert
         * @param value the value to insert
         */
        void insert(const std::string_view key, const V& value) {
            m_root.insert(key, value);
        }

        /**
         * Removes the given value using the given key.
         *
         * @param key the key to remove
         * @param value the value to remove
         * @return `true` if the given value was found under the given key, and `false` otherwise
         */
        bool remove(const std::string_view key, const V& value) {
            return m_root.remove(key, value);
        }

        /**
         * Clears this trie.
         */
        void clear() {
            m_root = node("");
        }

        /**
         * Finds all values whose keys match the given glob pattern. See `kdl::str_matches_glob` for the definition and
         * semantics of glob patterns and adds the values to the given output iterator.
         *
         * @tparam O the type of the output iterator
         * @param pattern the pattern to match
         * @param out the output iterator
         */
        template <typename O>
        void find_matches(const std::string_view pattern, O out) const {
            match_state match_state;
            m_root.find_matches(pattern, { 0u }, nullptr, match_state, out);
        }

        /**
         * Adds the keys of all nodes in this trie to the give output iterator.
         *
         * @tparam O the type of the output iterator
         * @param out the output iterator
         */
        template <typename O>
        void get_keys(O out) const {
            m_root.get_keys("", out);
        }
    };
}

#endif //KDL_GLOB_INDEX_H
