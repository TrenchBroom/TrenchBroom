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
            const String& doGetName() const override;
            const BBox3& doGetBounds() const override;
            
            Node* doClone(const BBox3& worldBounds) const override;
            bool doCanAddChild(const Node* child) const override;
            bool doCanRemoveChild(const Node* child) const override;
            bool doRemoveIfEmpty() const override;
            
            class AddNodeToOctree;
            class RemoveNodeFromOctree;
            class UpdateNodeInOctree;
            
            void doChildWasAdded(Node* node) override;
            void doChildWillBeRemoved(Node* node) override;
            void doChildBoundsDidChange(Node* node) override;
            
            bool doSelectable() const override;
            
            void doGenerateIssues(const IssueGenerator* generator, IssueList& issues) override;
            void doAccept(NodeVisitor& visitor) override;
            void doAccept(ConstNodeVisitor& visitor) const override;

            void doPick(const Ray3& ray, PickResult& pickResult) const override;
            void doFindNodesContaining(const Vec3& point, NodeList& result) override;
            FloatType doIntersectWithRay(const Ray3& ray) const override;
        private:
            Layer(const Layer&);
            Layer& operator=(const Layer&);
        };
    }
}

#endif /* defined(TrenchBroom_Layer) */
