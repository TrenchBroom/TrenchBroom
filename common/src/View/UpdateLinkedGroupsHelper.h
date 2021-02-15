/*
 Copyright (C) 2020 Kristian Duske

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

#include <kdl/result_forward.h>

#include <memory>
#include <utility>
#include <variant>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class GroupNode;
        class Node;
        enum class UpdateLinkedGroupsError;
    }

    namespace View {
        class MapDocumentCommandFacade;

        /**
         * Checks whether the given vector of linked group can be updated consistently.
         *
         * The given linked groups can be updated consistently if no two of them are in the same
         * linked set.
         */
        bool checkLinkedGroupsToUpdate(const std::vector<const Model::GroupNode*>& linkedGroupsToUpdate);

        /**
         * A helper class to add support for updating linked groups to commands.
         *
         * The class is initialized with a vector of group nodes whose changes should be propagated
         * to the members of their respective link sets. When applyLinkedGroupUpdates is first called,
         * a replacement node is created for each linked group that needs to be updated, and these
         * linked groups are replaced with their replacements. Calling applyLinkedGroupUpdates replaces
         * the replacement nodes with their original corresponding groups again, effectively undoing the change.
         */
        class UpdateLinkedGroupsHelper {
        private:
            using LinkedGroupsToUpdate = std::vector<std::pair<const Model::GroupNode*, std::vector<Model::GroupNode*>>>;
            using LinkedGroupUpdates = std::vector<std::pair<Model::Node*, std::vector<std::unique_ptr<Model::Node>>>>;
            std::variant<LinkedGroupsToUpdate, LinkedGroupUpdates> m_state;
        public:
            explicit UpdateLinkedGroupsHelper(LinkedGroupsToUpdate linkedGroupsToUpdate);
            ~UpdateLinkedGroupsHelper();

            kdl::result<void, Model::UpdateLinkedGroupsError> applyLinkedGroupUpdates(MapDocumentCommandFacade& document);
            void undoLinkedGroupUpdates(MapDocumentCommandFacade& document);
            void collateWith(UpdateLinkedGroupsHelper& other);
        private:
            kdl::result<void, Model::UpdateLinkedGroupsError> computeLinkedGroupUpdates(MapDocumentCommandFacade& document);
            static kdl::result<LinkedGroupUpdates, Model::UpdateLinkedGroupsError> computeLinkedGroupUpdates(const LinkedGroupsToUpdate& linkedGroupsToUpdate, const vm::bbox3& worldBounds);

            void doApplyOrUndoLinkedGroupUpdates(MapDocumentCommandFacade& document);
        };
    }
}
