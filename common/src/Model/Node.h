/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#ifndef __TrenchBroom__Node__
#define __TrenchBroom__Node__

#include "Model/ModelTypes.h"

namespace TrenchBroom {
    namespace Model {
        class Node {
        private:
            Node* m_parent;
            NodeList m_children;
        protected:
            Node();
        private:
            Node(const Node&);
            Node& operator=(const Node&);
        public:
            virtual ~Node();
        public: // tree management
            Node* parent() const;
            bool hasChildren() const;
            const NodeList& children() const;
            
            template <typename I>
            void addChildren(I cur, I end, size_t count = 0) {
                m_children.reserve(m_children.size() + count);
                while (cur != end) {
                    addChild(*cur);
                    ++cur;
                }
            }
            
            void addChild(Node* child);
            template <typename I>
            void removeChildren(I cur, I end) {
                NodeList::iterator rem = m_children.end();
                while (cur != end) {
                    rem = doRemoveChild(*cur);
                    ++cur;
                }
                m_children.erase(rem, m_children.end());
            }
            
            void removeChild(Node* child);
        private:
            bool canAddChild(Node* child) const;
            bool canRemoveChild(Node* child) const;
            
            NodeList::iterator doRemoveChild(Node* child);
            
            void attachChild(Node* child);
            void detachChild(Node* child);
            
            void setParent(Node* parent);
            void ancestorDidChange();
        public: // visitors
            template <class V>
            void acceptAndRecurse(V& visitor) {
                accept(visitor);
                recurse(visitor);
            }
            
            template <class V>
            void acceptAndRecurse(V& visitor) const {
                accept(visitor);
                recurse(visitor);
            }
            
            template <class V>
            void accept(V& visitor) {
                doAccept(visitor);
            }
            
            template <class V>
            void accept(V& visitor) const {
                doAccept(visitor);
            }

            template <class V>
            void recurse(V& visitor) {
                NodeList::const_iterator it, end;
                for (it = m_children.begin(), end = m_children.end(); it != end && !visitor.cancelled(); ++it) {
                    Node* node = *it;
                    node->acceptAndRecurse(visitor);
                }
            }

            template <class V>
            void recurse(V& visitor) const {
                NodeList::const_iterator it, end;
                for (it = m_children.begin(), end = m_children.end(); it != end && !visitor.cancelled(); ++it) {
                    Node* node = *it;
                    node->acceptAndRecurse(visitor);
                }
            }

            template <class V>
            void iterate(V& visitor) {
                NodeList::const_iterator it, end;
                for (it = m_children.begin(), end = m_children.end(); it != end && !visitor.cancelled(); ++it) {
                    Node* node = *it;
                    node->accept(visitor);
                }
            }
            
            template <class V>
            void iterate(V& visitor) const {
                NodeList::const_iterator it, end;
                for (it = m_children.begin(), end = m_children.end(); it != end && !visitor.cancelled(); ++it) {
                    Node* node = *it;
                    node->accept(visitor);
                }
            }
        protected: // index management
            void findAttributablesWithAttribute(const AttributeName& name, const AttributeValue& value, AttributableList& result) const;
            void findAttributablesWithNumberedAttribute(const AttributeName& prefix, const AttributeValue& value, AttributableList& result) const;
            
            void addToIndex(Attributable* attributable, const AttributeName& name, const AttributeValue& value);
            void removeFromIndex(Attributable* attributable, const AttributeName& name, const AttributeValue& value);
        private: // subclassing interface
            virtual bool doCanAddChild(Node* child) const = 0;
            virtual bool doCanRemoveChild(Node* child) const = 0;
            
            virtual void doAncestorDidChange() = 0;
            
            virtual void doAccept(NodeVisitor& visitor) = 0;
            virtual void doAccept(ConstNodeVisitor& visitor) const = 0;
            
            virtual void doFindAttributablesWithAttribute(const AttributeName& name, const AttributeValue& value, AttributableList& result) const;
            virtual void doFindAttributablesWithNumberedAttribute(const AttributeName& prefix, const AttributeValue& value, AttributableList& result) const;
            
            virtual void doAddToIndex(Attributable* attributable, const AttributeName& name, const AttributeValue& value);
            virtual void doRemoveFromIndex(Attributable* attributable, const AttributeName& name, const AttributeValue& value);
        };
    }
}

#endif /* defined(__TrenchBroom__Node__) */
