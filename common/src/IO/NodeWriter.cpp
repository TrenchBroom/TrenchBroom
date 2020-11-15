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

#include "NodeWriter.h"

#include "IO/MapFileSerializer.h"
#include "IO/NodeSerializer.h"
#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/Node.h"
#include "Model/WorldNode.h"

#include <kdl/overload.h>

#include <vector>

namespace TrenchBroom {
    namespace IO {
        static void doWriteNodes(NodeSerializer& serializer, const std::vector<Model::Node*>& nodes, const Model::Node* parent = nullptr) {
            auto parentStack = std::vector<const Model::Node*>{ parent };
            const auto parentAttributes = [&]() {
                assert(!parentStack.empty());
                return serializer.parentAttributes(parentStack.back());
            };

            for (const auto* node : nodes) {
                node->accept(kdl::overload(
                    [] (const Model::WorldNode*) {},
                    [] (const Model::LayerNode*) {},
                    [&](auto&& thisLambda, const Model::GroupNode* group) {
                        serializer.group(group, parentAttributes());

                        parentStack.push_back(group);
                        group->visitChildren(thisLambda);
                        parentStack.pop_back();
                    },
                    [&](const Model::EntityNode* entityNode) {
                        serializer.entity(entityNode, entityNode->entity().attributes(), parentAttributes(), entityNode);
                    },
                    [] (const Model::BrushNode*) {}
                ));
            }
        }

        NodeWriter::NodeWriter(const Model::WorldNode& world, std::ostream& stream) :
        m_world(world),
        m_serializer(MapFileSerializer::create(m_world.format(), stream)) {}

        NodeWriter::NodeWriter(const Model::WorldNode& world, std::unique_ptr<NodeSerializer> serializer) :
        m_world(world),
        m_serializer(std::move(serializer)) {}

        NodeWriter::~NodeWriter() = default;

        void NodeWriter::setExporting(const bool exporting) {
            m_serializer->setExporting(exporting);
        }

        void NodeWriter::writeMap() {
            m_serializer->beginFile();
            writeDefaultLayer();
            writeCustomLayers();
            m_serializer->endFile();
        }

        void NodeWriter::writeDefaultLayer() {
            m_serializer->defaultLayer(m_world);

            if (!(m_serializer->exporting() && m_world.defaultLayer()->omitFromExport())) {
                doWriteNodes(*m_serializer, m_world.defaultLayer()->children());
            }
        }

        void NodeWriter::writeCustomLayers() {
            const std::vector<const Model::LayerNode*> customLayers = m_world.customLayers();
            for (auto* layer : customLayers) {
                writeCustomLayer(layer);
            }
        }

        void NodeWriter::writeCustomLayer(const Model::LayerNode* layer) {
            if (!(m_serializer->exporting() && layer->omitFromExport())) {
                m_serializer->customLayer(layer);
                doWriteNodes(*m_serializer, layer->children(), layer);
            }
        }

        void NodeWriter::writeNodes(const std::vector<Model::Node*>& nodes) {
            m_serializer->beginFile();

            // Assort nodes according to their type and, in case of brushes, whether they are entity or world brushes.
            std::vector<Model::Node*> groups;
            std::vector<Model::Node*> entities;
            std::vector<Model::BrushNode*> worldBrushes;
            EntityBrushesMap entityBrushes;

            for (auto* node : nodes) {
                node->accept(kdl::overload(
                    [] (Model::WorldNode*) {},
                    [] (Model::LayerNode*) {},
                    [&](Model::GroupNode* group)   { groups.push_back(group); },
                    [&](Model::EntityNode* entity) { entities.push_back(entity); },
                    [&](Model::BrushNode* brush)   {
                        if (auto* entity = dynamic_cast<Model::EntityNode*>(brush->parent())) {
                            entityBrushes[entity].push_back(brush);
                        } else {
                            worldBrushes.push_back(brush);
                        }
                    }
                ));
            }

            writeWorldBrushes(worldBrushes);
            writeEntityBrushes(entityBrushes);
            
            doWriteNodes(*m_serializer, groups);
            doWriteNodes(*m_serializer, entities);

            m_serializer->endFile();
        }

        void NodeWriter::writeWorldBrushes(const std::vector<Model::BrushNode*>& brushes) {
            if (!brushes.empty()) {
                m_serializer->entity(&m_world, m_world.entity().attributes(), {}, brushes);
            }
        }

        void NodeWriter::writeEntityBrushes(const EntityBrushesMap& entityBrushes) {
            for (const auto& [entityNode, brushes] : entityBrushes) {
                m_serializer->entity(entityNode, entityNode->entity().attributes(), {}, brushes);
            }
        }

        void NodeWriter::writeBrushFaces(const std::vector<Model::BrushFace>& faces) {
            m_serializer->beginFile();
            m_serializer->brushFaces(faces);
            m_serializer->endFile();
        }
    }
}
