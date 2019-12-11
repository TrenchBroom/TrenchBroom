/*
 Copyright (C) 2010-2017 Kristian Duske

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TrenchBroom_StringMap
#define TrenchBroom_StringMap

#include "Exceptions.h"

#include <kdl/string_compare.h>
#include <kdl/string_format.h>
#include <kdl/vector_utils.h>

#include <cassert>
#include <map>
#include <set> // FIXME: use vector_set
#include <string>
#include <vector>

namespace TrenchBroom {
    template <typename V>
    class StringMapValueContainer {
    public:
        using ValueContainer = std::vector<V>;
        using QueryResult = std::vector<V>;

        static void insertValue(ValueContainer& values, const V& value) {
            values.push_back(value);
        }

        static void removeValue(ValueContainer& values, const V& value) {
            typename ValueContainer::iterator it = std::find(std::begin(values), std::end(values), value);
            if (it == std::end(values))
                throw Exception("Cannot remove value (does not belong to this node)");
            values.erase(it);
        }

        static void getValues(const ValueContainer& values, QueryResult& result) {
            kdl::vec_append(result, values);
        }
    };

    template <typename V>
    class StringMultiMapValueContainer {
    public:
        using ValueContainer = std::map<V, size_t>;
        using QueryResult = std::set<V>;

        static void insertValue(ValueContainer& values, const V& value) {
            values[value]++; // unknown map values are value constructed, which initializes to 0 for size_t
        }

        static void removeValue(ValueContainer& values, const V& value) {
            typename ValueContainer::iterator it = values.find(value);
            if (it == std::end(values))
                throw Exception("Cannot remove value from string map.");
            if (it->second == 1)
                values.erase(it);
            else
                --it->second;
        }

        static void getValues(const ValueContainer& values, QueryResult& result) {
            for (const auto& entry : values)
                result.insert(entry.first);
        }
    };

    template <typename V, typename P>
    class StringMap {
    public:
        using QueryResult = typename P::QueryResult;
    private:
        class Node {
        private:
            using NodeSet = std::set<Node>;
            using ValueContainer = typename P::ValueContainer;

            // The key is declared mutable because we must change it in splitNode and mergeNode, but the resulting new key
            // will still compare equal to the old key.
            mutable std::string m_key;
            mutable ValueContainer m_values;
            mutable NodeSet m_children;
        public:
            explicit Node(const std::string& key) :
            m_key(key) {}

            bool operator<(const Node& rhs) const {
                const size_t firstDiff = kdl::cs::str_mismatch(m_key, rhs.m_key);
                if (firstDiff == 0)
                    return m_key[0] < rhs.m_key[0];
                // both keys share a common prefix and are thus treated as the same
                return false;
            }

            /*
             Possible cases for insertion:
              index: 01234567 |   | #m_key: 6
              m_key: target   | ^ | #key | conditions              | todo
             =================|===|======|=========================|======
              case:  key:     |   |      |                         |
                 1:  targetli | 6 | 8    | ^ < #key AND ^ = #m_key | try insert in all children, if none match, create child 'li' and insert there;
                           ^  |   |      |                         |
                 2:  target   | 6 | 6    | ^ = #key AND ^ = #m_key | insert here; return true;
                           ^  |   |      |                         |
                 3:  tarus    | 3 | 5    | ^ < #key AND ^ < #m_key | split this node in 'tar' and 'get'; create child 'us' and insert there;
                        ^     |   |      |                         |
                 4:  tar      | 3 | 3    | ^ = #key AND ^ < #m_key | split this node in 'tar' and 'get'; insert here; return true;
                        ^     |   |      |                         |
                 5:  blah     | 0 | 4    | ^ = 0                   | return false;
                     ^        |   |      |                         |
             ==================================================================================
              ^ indicates where key and m_key first differ
             */
            void insert(const std::string& key, const V& value) const {
                const size_t firstDiff = kdl::cs::str_mismatch(key, m_key);
                if (firstDiff == 0 && !m_key.empty())
                    // no common prefix
                    return;
                if (firstDiff < key.size()) {
                    if (firstDiff < m_key.size()) {
                        // key and m_key share a common prefix, split this node and insert again
                        splitNode(firstDiff);
                        insert(key, value);
                    } else if (firstDiff == m_key.size()) {
                        // m_key is a prefix of key, find or create a child that shares a common prefix with remainder, and insert there, and insert here
                        const std::string remainder = key.substr(firstDiff);
                        const Node& child = findOrCreateChild(remainder);
                        child.insert(remainder, value);
                    }
                } else if (firstDiff == key.size()) {
                    if (firstDiff < m_key.size()) {
                        // key is prefix of m_key, split this node and insert here
                        splitNode(firstDiff);
                        insertValue(value);
                    } else if (firstDiff == m_key.size()) {
                        // keys are equal, insert here
                        insertValue(value);
                    }
                }
            }

            bool remove(const std::string& key, const V& value) const {
                const size_t firstDiff = kdl::cs::str_mismatch(key, m_key);
                if (m_key.size() <= key.size() && firstDiff == m_key.size()) {
                    // this node's key is a prefix of the given key
                    if (firstDiff < key.size()) {
                        // the given key is longer than this node's key, so we must continue at the appropriate child node
                        const std::string remainder(key.substr(firstDiff));
                        const Node query(remainder);
                        typename NodeSet::iterator it = m_children.find(query);
                        assert(it != std::end(m_children));
                        const Node& child = *it;
                        if (child.remove(remainder, value))
                            m_children.erase(it);
                    } else {
                        removeValue(value);
                    }

                    if (!m_key.empty() && m_values.empty() && m_children.size() == 1)
                        mergeNode();
                }
                return !m_key.empty() && m_values.empty() && m_children.empty();
            }

            void queryExact(const std::string& key, QueryResult& result) const {
                const size_t firstDiff = kdl::cs::str_mismatch(key, m_key);
                if (firstDiff == 0 && !m_key.empty())
                    // no common prefix
                    return;
                if (firstDiff == key.size() && firstDiff <= m_key.size()) {
                    // this node represents the given (remaining) prefix
                    if (firstDiff == m_key.size())
                        getValues(result);
                } else if (firstDiff < key.size() && firstDiff == m_key.size()) {
                    // this node is only a partial match, try to find a child to continue searching
                    const std::string remainder(key.substr(firstDiff));
                    const Node query(remainder);
                    typename NodeSet::iterator it = m_children.find(query);
                    if (it != std::end(m_children)) {
                        const Node& child = *it;
                        child.queryExact(remainder, result);
                    }
                }
            }

            void queryPrefix(const std::string& prefix, QueryResult& result) const {
                const size_t firstDiff = kdl::cs::str_mismatch(prefix, m_key);
                if (firstDiff == 0 && !m_key.empty())
                    // no common prefix
                    return;
                if (firstDiff == prefix.size() && firstDiff <= m_key.size()) {
                    // the given prefix is a prefix of this node's key, collect all values in the subtree starting at
                    // this node
                    collectValues(result);
                } else if (firstDiff < prefix.size() && firstDiff == m_key.size()) {
                    // this node is only a partial match, try to find a child to continue searching
                    const std::string remainder(prefix.substr(firstDiff));
                    const Node query(remainder);
                    typename NodeSet::iterator it = m_children.find(query);
                    if (it != std::end(m_children)) {
                        const Node& child = *it;
                        child.queryPrefix(remainder, result);
                    }
                }
            }

            void collectValues(QueryResult& result) const {
                getValues(result);
                for (const Node& child : m_children)
                    child.collectValues(result);
            }

            void queryNumbered(const std::string& prefix, QueryResult& result) const {
                const size_t firstDiff = kdl::cs::str_mismatch(prefix, m_key);
                if (firstDiff == 0 && !m_key.empty())
                    // no common prefix
                    return;
                if (firstDiff == prefix.size() && firstDiff <= m_key.size()) {
                    // the given prefix is a prefix of this node's key
                    // if the remainder of this node's key is a number, add this node's values and continue searching
                    // the entire subtree starting at this node
                    const std::string remainder(m_key.substr(firstDiff));
                    if (kdl::str_is_numeric(remainder)) {
                        getValues(result);
                        for (const Node& child : m_children)
                            child.collectIfNumbered(result);
                    }
                } else if (firstDiff < prefix.size() && firstDiff == m_key.size()) {
                    // this node is only a partial match, try to find a child to continue searching
                    const std::string remainder(prefix.substr(firstDiff));
                    const Node query(remainder);
                    for (const Node& child : m_children)
                        child.queryNumbered(remainder, result);
                }
            }

            void collectIfNumbered(QueryResult& result) const {
                if (kdl::str_is_numeric(m_key)) {
                    getValues(result);
                    for (const Node& child : m_children)
                        child.collectIfNumbered(result);
                }
            }

            void getKeys(const std::string& prefix, std::vector<std::string>& result) const {
                const std::string prefixAndKey = prefix + m_key;
                if (!m_values.empty()) {
                    result.push_back(prefixAndKey);
                }
                for (const auto& child : m_children) {
                    child.getKeys(prefixAndKey, result);
                }
            }
        private:
            void insertValue(const V& value) const {
                P::insertValue(m_values, value);
            }

            void removeValue(const V& value) const {
                P::removeValue(m_values, value);
            }

            const Node& findOrCreateChild(const std::string& key) const {
                std::pair<typename NodeSet::iterator, bool> result = m_children.insert(Node(key));
                typename NodeSet::iterator it = result.first;
                return *it;
            }

            void splitNode(const size_t index) const {
                using std::swap;

                assert(m_key.size() > 1);
                assert(index < m_key.size());

                const std::string newKey = m_key.substr(0, index);
                const std::string remainder = m_key.substr(index);

                // We want to avoid copying the children of this node to the new child, therefore we swap with an empty
                // node set. Afterwards this node's children are empty.
                NodeSet newChildren;
                swap(newChildren, m_children);

                const Node& newChild = findOrCreateChild(remainder);
                swap(newChild.m_children, newChildren);
                swap(newChild.m_values, m_values);

                m_key = newKey;
            }

            void mergeNode() const {
                using std::swap;

                assert(m_children.size() == 1);
                assert(m_values.empty());

                NodeSet oldChildren;
                swap(oldChildren, m_children);

                const Node& child = *std::begin(oldChildren);
                swap(m_children, child.m_children);
                swap(m_values, child.m_values);

                m_key += child.m_key;
            }

            void getValues(QueryResult& result) const {
                P::getValues(m_values, result);
            }
        };

        Node* m_root;
    public:
        StringMap() :
        m_root(new Node("")) {}

        ~StringMap() {
            delete m_root;
            m_root = nullptr;
        }

        void insert(const std::string& key, const V& value) {
            assert(m_root != nullptr);
            m_root->insert(key, value);
        }

        void remove(const std::string& key, const V& value) {
            assert(m_root != nullptr);
            m_root->remove(key, value);
        }

        void clear() {
            delete m_root;
            m_root = new Node("");
        }

        QueryResult queryPrefixMatches(const std::string& prefix) const {
            assert(m_root != nullptr);
            QueryResult result;
            m_root->queryPrefix(prefix, result);
            return result;
        }

        QueryResult queryNumberedMatches(const std::string& prefix) const {
            assert(m_root != nullptr);
            QueryResult result;
            m_root->queryNumbered(prefix, result);
            return result;
        }

        QueryResult queryExactMatches(const std::string& prefix) const {
            assert(m_root != nullptr);
            QueryResult result;
            m_root->queryExact(prefix, result);
            return result;
        }

        std::vector<std::string> getKeys() const {
            assert(m_root != nullptr);
            std::vector<std::string> result;
            m_root->getKeys("", result);
            return result;
        }
    private:
        StringMap(const StringMap& other);
        StringMap& operator=(const StringMap& other);
    };
}

#endif /* defined(TrenchBroom_StringMap) */
