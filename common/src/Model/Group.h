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
            const String& doGetName() const;
            const BBox3& doGetBounds() const;
            
            Node* doClone(const BBox3& worldBounds) const;
            NodeSnapshot* doTakeSnapshot();

            bool doCanAddChild(const Node* child) const;
            bool doCanRemoveChild(const Node* child) const;
            bool doRemoveIfEmpty() const;

            void doChildWasAdded(Node* node);
            void doChildWasRemoved(Node* node);

            void doNodeBoundsDidChange();
            void doChildBoundsDidChange(Node* node);
            bool doShouldPropagateDescendantEvents() const;
            
            bool doSelectable() const;
            
            void doPick(const Ray3& ray, PickResult& pickResult) const;
            FloatType doIntersectWithRay(const Ray3& ray) const;

            void doGenerateIssues(const IssueGenerator* generator, IssueList& issues);
            void doAccept(NodeVisitor& visitor);
            void doAccept(ConstNodeVisitor& visitor) const;
        private: // implement methods inherited from Object
            Node* doGetContainer() const;
            Layer* doGetLayer() const;
            Group* doGetGroup() const;
            
            void doTransform(const Mat4x4& transformation, bool lockTextures, const BBox3& worldBounds);
            bool doContains(const Node* node) const;
            bool doIntersects(const Node* node) const;
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
