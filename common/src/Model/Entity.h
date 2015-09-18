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

#ifndef TrenchBroom_Entity
#define TrenchBroom_Entity

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Hit.h"
#include "Assets/AssetTypes.h"
#include "Model/AttributableNode.h"
#include "Model/EntityRotationPolicy.h"
#include "Model/Object.h"

namespace TrenchBroom {
    namespace Model {
        class PickResult;
        
        class Entity : public AttributableNode, public Object, private EntityRotationPolicy {
        public:
            static const Hit::HitType EntityHit;
        private:
            static const BBox3 DefaultBounds;
            mutable BBox3 m_bounds;
            mutable bool m_boundsValid;
            Assets::EntityModel* m_model;
        public:
            Entity();
            
            bool pointEntity() const;
            Vec3 origin() const;
            Mat4x4 rotation() const;
            FloatType area(Math::Axis::Type axis) const;
        private:
            void setOrigin(const Vec3& origin);
            void applyRotation(const Mat4x4& transformation);
        public: // entity model
            Assets::ModelSpecification modelSpecification() const;
            Assets::EntityModel* model() const;
            void setModel(Assets::EntityModel* model);
        private: // implement Node interface
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

            bool doSelectable() const;
            
            void doPick(const Ray3& ray, PickResult& pickResult) const;
            FloatType doIntersectWithRay(const Ray3& ray) const;

            void doGenerateIssues(const IssueGenerator* generator, IssueList& issues);
            void doAccept(NodeVisitor& visitor);
            void doAccept(ConstNodeVisitor& visitor) const;
        private: // implement AttributableNode interface
            void doAttributesDidChange();
            bool doIsAttributeNameMutable(const AttributeName& name) const;
            bool doIsAttributeValueMutable(const AttributeName& name) const;
            Vec3 doGetLinkSourceAnchor() const;
            Vec3 doGetLinkTargetAnchor() const;
        private: // implement Object interface
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
            Entity(const Entity&);
            Entity& operator=(const Entity&);
        };
    }
}

#endif /* defined(TrenchBroom_Entity) */
