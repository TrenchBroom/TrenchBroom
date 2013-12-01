/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#ifndef __TrenchBroom__StringIndex__
#define __TrenchBroom__StringIndex__

#include "CollectionUtils.h"
#include "Exceptions.h"
#include "StringUtils.h"

#include <cassert>
#include <map>
#include <set>
#include <vector>

namespace TrenchBroom {
    template <typename V>
    class StringIndex {
    public:
        typedef std::vector<V> ValueList;
    private:
        class Node {
        private:
            typedef std::set<Node> NodeSet;
            typedef std::map<V, size_t> ValueMap;
            
            // The key is declared mutable because we must change in splitNode and mergeNode, but the resulting new key
            // will still compare equal to the old key.
            mutable String m_key;
            mutable ValueMap m_partialValues;
            mutable ValueMap m_exactValues;
            mutable NodeSet m_children;
        public:
            Node(const String& key) :
            m_key(key) {}
            
            bool operator<(const Node& rhs) const {
                const size_t firstDiff = StringUtils::findFirstDifference(m_key, rhs.m_key);
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
            void insert(const String& key, const V& value) const {
                const size_t firstDiff = StringUtils::findFirstDifference(key, m_key);
                if (firstDiff == 0)
                    // no common prefix
                    return;
                if (firstDiff < key.size()) {
                    if (firstDiff < m_key.size()) {
                        // key and m_key share a common prefix, split this node and insert again
                        splitNode(firstDiff);
                        insert(key, value);
                    } else if (firstDiff == m_key.size()) {
                        // m_key is a prefix of key, find or create a child that shares a common prefix with remainder, and insert there, and insert here
                        const String remainder = key.substr(firstDiff);
                        const Node& child = findOrCreateChild(remainder);
                        child.insert(remainder, value);
                        insertValue(value, false);
                    }
                } else if (firstDiff == key.size()) {
                    if (firstDiff < m_key.size()) {
                        // key is prefix of m_key, split this node and insert here
                        splitNode(firstDiff);
                        insertValue(value, true);
                    } else if (firstDiff == m_key.size()) {
                        // keys are equal, insert here
                        insertValue(value, true);
                    }
                }
            }
            
            bool remove(const String& key, const V& value) const {
                const size_t firstDiff = StringUtils::findFirstDifference(key, m_key);
                if (m_key.size() <= key.size() && firstDiff == m_key.size()) {
                    // this node's key is a prefix of the given key
                    if (firstDiff < key.size()) {
                        // the given key is longer than this node's key, so we must continue at the appropriate child node
                        const String remainder(key.substr(firstDiff));
                        typename NodeSet::iterator it = m_children.find(remainder);
                        assert(it != m_children.end());
                        const Node& child = *it;
                        if (child.remove(remainder, value))
                            m_children.erase(it);
                        removeValue(value, false);
                    } else {
                        removeValue(value, true);
                    }
                    
                    if (m_children.size() == 1) {
                        const Node& child = *m_children.begin();
                        if (m_partialValues.size() == child.m_partialValues.size())
                            mergeNode();
                    }
                }
                return m_partialValues.empty() && m_children.empty();
            }
            
            void queryExact(const String& key, ValueList& result) const {
                const size_t firstDiff = StringUtils::findFirstDifference(key, m_key);
                if (firstDiff == 0)
                    // no common prefix
                    return;
                if (firstDiff == key.size() && firstDiff <= m_key.size()) {
                    // this node represents the given (remaining) prefix
                    if (firstDiff == m_key.size())
                        getExactValues(result);
                } else if (firstDiff < key.size() && firstDiff == m_key.size()) {
                    // this node is only a partial match, try to find a child to continue searching
                    const String remainder(key.substr(firstDiff));
                    typename NodeSet::iterator it = m_children.find(remainder);
                    if (it != m_children.end()) {
                        const Node& child = *it;
                        child.queryExact(remainder, result);
                    }
                }
            }
            
            void queryPrefix(const String& prefix, ValueList& result) const {
                const size_t firstDiff = StringUtils::findFirstDifference(prefix, m_key);
                if (firstDiff == 0)
                    // no common prefix
                    return;
                if (firstDiff == prefix.size() && firstDiff <= m_key.size()) {
                    // this node represents the given (remaining) prefix
                    getPartialValues(result);
                } else if (firstDiff < prefix.size() && firstDiff == m_key.size()) {
                    // this node is only a partial match, try to find a child to continue searching
                    const String remainder(prefix.substr(firstDiff));
                    typename NodeSet::iterator it = m_children.find(remainder);
                    if (it != m_children.end()) {
                        const Node& child = *it;
                        child.queryPrefix(remainder, result);
                    }
                }
            }
            
            void queryNumbered(const String& prefix, ValueList& result) const {
                const size_t firstDiff = StringUtils::findFirstDifference(prefix, m_key);
                if (firstDiff == prefix.size() && firstDiff <= m_key.size()) {
                    if (firstDiff < m_key.size()) {
                        const String remainder(m_key.substr(firstDiff));
                        if (StringUtils::isNumber(remainder)) {
                            getExactValues(result);
                            queryAllNumberedChildren(result);
                        }
                    } else {
                        queryAllNumberedChildren(result);
                    }
                } else if (firstDiff < prefix.size() && firstDiff == m_key.size()) {
                    // if the surplus of the given key is a number, we must also include this node's values
                    const String remainder(prefix.substr(firstDiff));
                    if (StringUtils::isNumber(remainder))
                        getExactValues(result);
                    
                    // this node is only a partial match, try to find a child to continue searching
                    typename NodeSet::iterator it = m_children.find(remainder);
                    if (it != m_children.end()) {
                        const Node& child = *it;
                        child.queryNumbered(remainder, result);
                    }
                }
            }
            
            void queryAllNumberedChildren(ValueList& result) const {
                typename NodeSet::const_iterator it, end;
                for (it = m_children.begin(), end = m_children.end(); it != end; ++it) {
                    const Node& child = *it;
                    child.queryNumbered("", result);
                }
            }
        private:
            void insertValue(const V& value, const bool endshere) const {
                insertValue(value, m_partialValues);
                if (endshere)
                    insertValue(value, m_exactValues);
            }
            
            void insertValue(const V& value, ValueMap& values) const {
                typename ValueMap::iterator it = MapUtils::findOrInsert(values, value, 0u);
                ++it->second;
            }
            
            void removeValue(const V& value, const bool endshere) const {
                removeValue(value, m_partialValues);
                if (endshere)
                    removeValue(value, m_exactValues);
            }
            
            void removeValue(const V& value, ValueMap& values) const {
                typename ValueMap::iterator it = values.find(value);
                if (it == values.end())
                    throw Exception("Cannot remove value (does not belong to this node)");
                if (it->second == 1)
                    values.erase(it);
                else
                    --it->second;
            }
            
            const Node& findOrCreateChild(const String& key) const {
                std::pair<typename NodeSet::iterator, bool> result = m_children.insert(Node(key));
                typename NodeSet::iterator it = result.first;
                return *it;
            }
            
            void splitNode(const size_t index) const {
                assert(m_key.size() > 1);
                assert(index < m_key.size());
                const String newKey = m_key.substr(0, index);
                const String remainder = m_key.substr(index);

                // We want to avoid copying the children of this node to the new child, therefore we swap with an empty
                // node set. Afterwards this node's children are empty.
                NodeSet newChildren;
                std::swap(newChildren, m_children);

                const Node& newChild = findOrCreateChild(remainder);
                newChild.m_partialValues = m_partialValues;
                std::swap(newChild.m_children, newChildren);
                std::swap(newChild.m_exactValues, m_exactValues);
                
                m_key = newKey;
            }
            
            void mergeNode() const {
                assert(m_children.size() == 1);
                assert(m_exactValues.empty());
                
                NodeSet oldChildren;
                std::swap(oldChildren, m_children);
                
                const Node& child = *oldChildren.begin();
                assert(m_partialValues == child.m_partialValues);
                std::swap(m_children, child.m_children);
                m_exactValues = child.m_exactValues;
                
                m_key += child.m_key;
            }
            
            void getPartialValues(ValueList& result) const {
                typename ValueMap::const_iterator it, end;
                for (it = m_partialValues.begin(), end = m_partialValues.end(); it != end; ++it)
                    result.push_back(it->first);
            }
            
            void getExactValues(ValueList& result) const {
                typename ValueMap::const_iterator it, end;
                for (it = m_exactValues.begin(), end = m_exactValues.end(); it != end; ++it)
                    result.push_back(it->first);
            }
        };
        
        Node* m_root;
    public:
        StringIndex() :
        m_root(NULL) {}
        
        ~StringIndex() {
            delete m_root;
            m_root = NULL;
        }
        
        void insert(const String& key, const V& value) {
            if (m_root == NULL)
                m_root = new Node(key);
            m_root->insert(key, value);
        }
        
        void remove(const String& key, const V& value) {
            if (m_root != NULL) {
                if (m_root->remove(key, value)) {
                    delete m_root;
                    m_root = NULL;
                }
            }
        }
        
        ValueList queryPrefixMatches(const String& prefix) const {
            ValueList result;
            if (m_root != NULL)
                m_root->queryPrefix(prefix, result);
            return result;
        }
        
        ValueList queryNumberedMatches(const String& prefix) const {
            ValueList result;
            if (m_root != NULL)
                m_root->queryNumbered(prefix, result);
            return result;
        }
        
        ValueList queryExactMatches(const String& prefix) const {
            ValueList result;
            if (m_root != NULL)
                m_root->queryExact(prefix, result);
            return result;
        }
    };
}

#endif /* defined(__TrenchBroom__StringIndex__) */
