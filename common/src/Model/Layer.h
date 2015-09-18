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

#ifndef TrenchBroom_Layer
#define TrenchBroom_Layer

#include "StringUtils.h"
#include "Model/ModelTypes.h"
#include "Model/Node.h"
#include "Model/Octree.h"

namespace TrenchBroom {
    namespace Model {
        class Layer : public Node {
        private:
            String m_name;
            
            typedef Octree<FloatType, Node*> NodeTree;
            NodeTree m_octree;
        public:
            Layer(const String& name, const BBox3& worldBounds);
            
            void setName(const String& name);
        private: // implement Node interface
            const String& doGetName() const;
            const BBox3& doGetBounds() const;
            
            Node* doClone(const BBox3& worldBounds) const;
            bool doCanAddChild(const Node* child) const;
            bool doCanRemoveChild(const Node* child) const;
            bool doRemoveIfEmpty() const;
            
            class AddNodeToOctree;
            class RemoveNodeFromOctree;
            class UpdateNodeInOctree;
            
            void doChildWasAdded(Node* node);
            void doChildWillBeRemoved(Node* node);
            void doChildBoundsDidChange(Node* node);
            
            bool doSelectable() const;
            
            void doGenerateIssues(const IssueGenerator* generator, IssueList& issues);
            void doAccept(NodeVisitor& visitor);
            void doAccept(ConstNodeVisitor& visitor) const;

            void doPick(const Ray3& ray, PickResult& pickResult) const;
            FloatType doIntersectWithRay(const Ray3& ray) const;
        private:
            Layer(const Layer&);
            Layer& operator=(const Layer&);
        };
    }
}

#endif /* defined(TrenchBroom_Layer) */
