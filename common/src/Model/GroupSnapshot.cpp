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

#include "GroupSnapshot.h"

#include "Exceptions.h"
#include "Model/GroupNode.h"
#include "Model/Node.h"

#include <kdl/overload.h>
#include <kdl/result.h>
#include <kdl/vector_utils.h>

namespace TrenchBroom {
    namespace Model {
        static void takeSnapshotsOfChildren(Node* node, std::vector<NodeSnapshot*>& snapshots) {
            for (auto* child : node->children()) {
                if (auto* snapshot = child->takeSnapshot()) {
                    snapshots.push_back(snapshot);
                }
                takeSnapshotsOfChildren(child, snapshots);
            }
        }

        GroupSnapshot::GroupSnapshot(GroupNode* group) {
            takeSnapshotsOfChildren(group, m_snapshots);
        }

        GroupSnapshot::~GroupSnapshot() {
            kdl::vec_clear_and_delete(m_snapshots);
        }

        kdl::result<void, SnapshotErrors> GroupSnapshot::doRestore(const vm::bbox3& worldBounds) {
            SnapshotErrors errors;
            for (NodeSnapshot* snapshot : m_snapshots) {
                snapshot->restore(worldBounds)
                    .visit(kdl::overload(
                        []() {},
                        [&](const SnapshotErrors& e) { kdl::vec_append(errors, e); }
                    ));
            }
            return errors.empty()
                ? kdl::result<void, SnapshotErrors>::success()
                : kdl::result<void, SnapshotErrors>::error(std::move(errors));
        }
    }
}
