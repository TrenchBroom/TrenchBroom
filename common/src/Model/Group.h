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

#ifndef TrenchBroom_Group
#define TrenchBroom_Group

#include "TrenchBroom.h"
#include "VecMath.h"
#include "StringUtils.h"
#include "Hit.h"
#include "Model/ModelTypes.h"
#include "Model/Node.h"
#include "Model/Object.h"

namespace TrenchBroom {
    namespace Model {
        class PickResult;
        
        class Group : public Node, public Object {
        public:
            static const Hit::HitType GroupHit;
        private:
            typedef enum {
                Edit_Open,
                Edit_Closed,
                Edit_DescendantOpen
            } EditState;
            
            String m_name;
            EditState m_editState;
            mutable BBox3 m_bounds;
            mutable bool m_boundsValid;
        public:
            Group(const String& name);
            
            void setName(const String& name);
            
            bool opened() const;
            void open();
            void close();
        private:
            void setEditState(EditState editState);
            
            class SetEditStateVisitor;
            void openAncestors();
            void closeAncestors();
            
            bool hasOpenedDescendant() const;
        private: // implement methods inherited from Node
            const String& doGetName() const override;
            const BBox3& doGetBounds() const override;
            
            Node* doClone(const BBox3& worldBounds) const override;
            NodeSnapshot* doTakeSnapshot() override;

            bool doCanAddChild(const Node* child) const override;
            bool doCanRemoveChild(const Node* child) const override;
            bool doRemoveIfEmpty() const override;

            void doChildWasAdded(Node* node) override;
            void doChildWasRemoved(Node* node) override;

            void doNodeBoundsDidChange(const BBox3& oldBounds) override;
            void doChildBoundsDidChange(Node* node, const BBox3& oldBounds) override;
            bool doShouldPropagateDescendantEvents() const override;
            
            bool doSelectable() const override;
            
            void doPick(const Ray3& ray, PickResult& pickResult) const override;
            void doFindNodesContaining(const Vec3& point, NodeList& result) override;
            FloatType doIntersectWithRay(const Ray3& ray) const override;

            void doGenerateIssues(const IssueGenerator* generator, IssueList& issues) override;
            void doAccept(NodeVisitor& visitor) override;
            void doAccept(ConstNodeVisitor& visitor) const override;
        private: // implement methods inherited from Object
            Node* doGetContainer() const override;
            Layer* doGetLayer() const override;
            Group* doGetGroup() const override;
            
            void doTransform(const Mat4x4& transformation, bool lockTextures, const BBox3& worldBounds) override;
            bool doContains(const Node* node) const override;
            bool doIntersects(const Node* node) const override;
        private:
            void invalidateBounds();
            void validateBounds() const;
        private:
            Group(const Group&);
            Group& operator=(const Group&);
        };
    }
}

#endif /* defined(TrenchBroom_Group) */
