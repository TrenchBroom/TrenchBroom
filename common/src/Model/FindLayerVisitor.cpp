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

#include "FindLayerVisitor.h"

#include "Model/Node.h"

#include <kdl/vector_set.h>

namespace TrenchBroom {
    namespace Model {
        void FindLayerVisitor::doVisit(WorldNode*) {}

        void FindLayerVisitor::doVisit(LayerNode* layer) {
            setResult(layer);
            cancel();
        }

        void FindLayerVisitor::doVisit(GroupNode*) {}
        void FindLayerVisitor::doVisit(EntityNode*) {}
        void FindLayerVisitor::doVisit(BrushNode*) {}

        Model::LayerNode* findLayer(Model::Node* node) {
            FindLayerVisitor visitor;
            node->acceptAndEscalate(visitor);
            return visitor.result();
        }

        std::vector<Model::LayerNode*> findLayers(const std::vector<Model::Node*>& nodes) {
            kdl::vector_set<Model::LayerNode*> layersSet;
            for (Model::Node* node : nodes) {
                Model::LayerNode* layer = findLayer(node);
                if (layer != nullptr) {
                    layersSet.insert(layer);
                }
            }

            std::vector<Model::LayerNode*> layers = layersSet.release_data();
            LayerNode::sortLayers(layers);
            return layers;
        }
    }
}
