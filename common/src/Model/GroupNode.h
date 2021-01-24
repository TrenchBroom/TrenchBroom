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

#pragma once

#include "FloatType.h"
#include "Macros.h"
#include "Model/Group.h"
#include "Model/IdType.h"
#include "Model/Node.h"
#include "Model/Object.h"

#include <kdl/result_forward.h>

#include <vecmath/bbox.h>

#include <iosfwd>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        enum class UpdateLinkedGroupsError;
        using UpdateLinkedGroupsResult = std::vector<std::pair<Node*, std::vector<std::unique_ptr<Node>>>>;

        /**
         * A node that groups other nodes to make them editable as one. Multiple groups can form a
         * link set; a link set is a set of groups such that changes to the children of of one of the
         * member of the link set are reflected in the other members of the link set.
         *
         * Every group is in a link set, even if it isn't linked to any other groups. In that case
         * the group is the only member in its own link set. A group that is a member of a link set
         * can be either connected or disconnected from the link set. A disconnected group is not
         * updated when any other member of the link set changes. An example for a disconnected member
         * of a link set is a group that has been removed from the map, but is still kept around for
         * purposes of adding it back to the map again later via undo.
         *
         * If two groups A and B are connected to the same link set, we call them "linked groups", 
         * and we say that A is linked to B and vice versa. 
         *
         * When a group node is newly created, it is initially connected from its link set.
         */
        class GroupNode : public Node, public Object {
        private:
            struct LinkSet;

            enum class EditState {
                Open,
                Closed,
                DescendantOpen
            };

            Group m_group;
            std::shared_ptr<LinkSet> m_linkSet;
            EditState m_editState;
            mutable vm::bbox3 m_logicalBounds;
            mutable vm::bbox3 m_physicalBounds;
            mutable bool m_boundsValid;

            /**
             * The ID used to serialize group nodes (see MapReader and NodeSerializer). This is set by MapReader when a
             * layer is read, or by WorldNode when a group is added that doesn't yet have a persistent ID.
             */
            std::optional<IdType> m_persistentId;
        public:
            explicit GroupNode(Group group);

            /**
             * Recursively clones this group, but does not add the clone to this groups' link set.
             */
            std::unique_ptr<GroupNode> cloneRecursivelyWithoutLinking(const vm::bbox3& worldBounds) const;

            const Group& group() const;
            Group setGroup(Group group);

            bool opened() const;
            bool hasOpenedDescendant() const;
            bool closed() const;
            void open();
            void close();

            const std::optional<IdType>& persistentId() const;
            void setPersistentId(IdType persistentId);

            const std::optional<std::string>& sharedPersistentId() const;
            void setSharedPersistentId(std::string sharedPersistentId);

            /**
             * Returns the connected members of the link set. If this group is disconnected from the link set,
             * then it will not be included in the returned vector.
             */
            const std::vector<GroupNode*> linkedGroups() const;

            /**
             * Indicates whether the given groups are in the same link set, either connected or disconnected.
             */
            friend bool inSameLinkSet(const GroupNode& lhs, const GroupNode& rhs);

            /**
             * Adds the given group to this group's link set.
             *
             * The given group node is expected to be disconnected from its own link set. It is removed from its
             * own link set and added to this node's link set. It is not connected to this node's link set afterwards.
             *
             * If the given group is already a member of this group's link set, then nothing happens.
             */
            void addToLinkSet(GroupNode& groupNode);
            
            /**
             * Indicates whether this group node is connected to its link set.
             */
            bool connectedToLinkSet() const;

            /**
             * Connects this group node to its link set.
             *
             * Expects that this group node is not already connected to its link set.
             */
            void connectToLinkSet();

            /**
             * Disconnects this group node from its link set.
             *
             * Expects that this group node is connected to its link set.
             */
            void disconnectFromLinkSet();

            /**
             * Updates all connected members of this groups' link set.
             *
             * The children of this node are cloned (recursively) and transformed into the linked nodes by means of the
             * recorded transformations of this group and the linked groups.
             *
             * Depending on the preserved property keys of the cloned entities and their corresponding entities in the
             * linked groups, some entity property changes may not be propagated from this group to the linked groups.
             * Specifically, if an entity property is preserved in either the cloned entity or its corresponding entity
             * in a linked group, then changes to that entity property incl. removal are not propagated. This also
             * applies to numbered properties, i.e. properties whose names end in a number. So if the entity property
             * "target" is preserved, then changes to the property "target2" are not propagated or overwritten during
             * propagation.
             *
             * If this operation fails for any child and linked group, then an error is returned. The operation can fail
             * if any of the following conditions arises:
             *
             * - the transformation of this group node is not invertible
             * - transforming any of this node's children fails
             * - any of the transformed children is no longer within the world bounds
             *
             * If this operation succeeds, a vector of pairs is returned where each pair consists of the linked node that
             * should be updated, and a replacement group node.
             *
             * Expects that this group is currently connected to its link set.
             */
            kdl::result<UpdateLinkedGroupsResult, UpdateLinkedGroupsError> updateLinkedGroups(const vm::bbox3& worldBounds);
        private:
            void setEditState(EditState editState);
            void setAncestorEditState(EditState editState);

            void openAncestors();
            void closeAncestors();
        private: // implement methods inherited from Node
            const std::string& doGetName() const override;
            const vm::bbox3& doGetLogicalBounds() const override;
            const vm::bbox3& doGetPhysicalBounds() const override;

            Node* doClone(const vm::bbox3& worldBounds) override;

            bool doCanAddChild(const Node* child) const override;
            bool doCanRemoveChild(const Node* child) const override;
            bool doRemoveIfEmpty() const override;

            bool doShouldAddToSpacialIndex() const override;

            void doChildWasAdded(Node* node) override;
            void doChildWasRemoved(Node* node) override;

            void doNodePhysicalBoundsDidChange() override;
            void doChildPhysicalBoundsDidChange() override;

            bool doSelectable() const override;

            void doPick(const vm::ray3& ray, PickResult& pickResult) override;
            void doFindNodesContaining(const vm::vec3& point, std::vector<Node*>& result) override;

            void doGenerateIssues(const IssueGenerator* generator, std::vector<Issue*>& issues) override;
            void doAccept(NodeVisitor& visitor) override;
            void doAccept(ConstNodeVisitor& visitor) const override;
        private: // implement methods inherited from Object
            Node* doGetContainer() override;
            LayerNode* doGetContainingLayer() override;
            GroupNode* doGetContainingGroup() override;

            bool doContains(const Node* node) const override;
            bool doIntersects(const Node* node) const override;
        private:
            void invalidateBounds();
            void validateBounds() const;
        private: // implement Taggable interface
            void doAcceptTagVisitor(TagVisitor& visitor) override;
            void doAcceptTagVisitor(ConstTagVisitor& visitor) const override;
        private:
            deleteCopyAndMove(GroupNode)
        };
    }
}

