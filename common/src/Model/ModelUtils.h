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

#ifndef TrenchBroom_ModelUtils
#define TrenchBroom_ModelUtils

#include "FloatType.h"
#include "Model/Node.h"

#include <vecmath/bbox.h>

#include <map>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class BrushFaceHandle;
        class EditorContext;
        class LayerNode;
        class Node;

        LayerNode* findContainingLayer(Node* node);

        std::vector<LayerNode*> findContainingLayersUserSorted(const std::vector<Node*>& nodes);

        GroupNode* findContainingGroup(Node* node);

        /**
         * Searches the ancestor chain of `node` for the outermost closed group and returns
         * it if one is found, otherwise returns nullptr.
         */
        GroupNode* findOutermostClosedGroup(Node* node);

        std::vector<Node*> collectParents(const std::vector<Node*>& nodes);
        std::vector<Node*> collectParents(const std::map<Node*, std::vector<Node*>>& nodes);

        std::vector<Node*> collectChildren(const std::map<Node*, std::vector<Node*>>& nodes);
        std::vector<Node*> collectDescendants(const std::vector<Node*>& nodes);
        std::map<Node*, std::vector<Node*>> parentChildrenMap(const std::vector<Node*>& nodes);

        std::vector<Node*> collectNodes(const std::vector<Node*>& nodes);

        std::vector<Node*> collectTouchingNodes(const std::vector<Node*>& nodes, const std::vector<BrushNode*>& brushes);
        std::vector<Node*> collectContainedNodes(const std::vector<Node*>& nodes, const std::vector<BrushNode*>& brushes);

        std::vector<Node*> collectSelectedNodes(const std::vector<Node*>& nodes);

        std::vector<Node*> collectSelectableNodes(const std::vector<Node*>& nodes, const EditorContext& editorContext);
        
        std::vector<BrushFaceHandle> collectBrushFaces(const std::vector<Node*>& nodes);
        std::vector<BrushFaceHandle> collectSelectableBrushFaces(const std::vector<Node*>& nodes, const EditorContext& editorContext);

        vm::bbox3 computeLogicalBounds(const std::vector<Node*>& nodes, const vm::bbox3& defaultBounds = vm::bbox3());
        vm::bbox3 computePhysicalBounds(const std::vector<Node*>& nodes, const vm::bbox3& defaultBounds = vm::bbox3());

        bool boundsContainNode(const vm::bbox3& bounds, const Node* node);
        bool boundsIntersectNode(const vm::bbox3& bounds, const Node* node);

    }
}

#endif /* defined(TrenchBroom_ModelUtils) */
