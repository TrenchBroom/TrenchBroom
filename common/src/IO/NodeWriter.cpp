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
#include "IO/MapStreamSerializer.h"
#include "IO/NodeSerializer.h"
#include "Model/BrushNode.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/Node.h"
#include "Model/WorldNode.h"

#include <kdl/overload.h>

#include <vector>

namespace TrenchBroom {
    namespace IO {
        class NodeWriter::WriteNode : public Model::ConstNodeVisitor {
        private:
            NodeSerializer& m_serializer;
            const std::vector<Model::EntityAttribute> m_parentAttributes;
        public:
            explicit WriteNode(NodeSerializer& serializer, const Model::Node* parent = nullptr) :
            m_serializer(serializer),
            m_parentAttributes(m_serializer.parentAttributes(parent)) {}

            void doVisit(const Model::WorldNode* /* world */) override   { stopRecursion(); }
            void doVisit(const Model::LayerNode* /* layer */) override   { stopRecursion(); }

            void doVisit(const Model::GroupNode* group) override   {
                m_serializer.group(group, m_parentAttributes);
                WriteNode visitor(m_serializer, group);
                group->iterate(visitor);
                stopRecursion();
            }

            void doVisit(const Model::EntityNode* entity) override {
                m_serializer.entity(entity, entity->attributes(), m_parentAttributes, entity);
                stopRecursion();
            }

            void doVisit(const Model::BrushNode* /* brush */) override   { stopRecursion();  }
        };

        NodeWriter::NodeWriter(const Model::WorldNode& world, FILE* stream) :
        m_world(world),
        m_serializer(MapFileSerializer::create(m_world.format(), stream)) {}

        NodeWriter::NodeWriter(const Model::WorldNode& world, std::ostream& stream) :
        m_world(world),
        m_serializer(MapStreamSerializer::create(m_world.format(), stream)) {}

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
                const std::vector<Model::Node *> &children = m_world.defaultLayer()->children();
                WriteNode visitor(*m_serializer);
                Model::Node::accept(std::begin(children), std::end(children), visitor);
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

                const std::vector<Model::Node *> &children = layer->children();
                WriteNode visitor(*m_serializer, layer);
                Model::Node::accept(std::begin(children), std::end(children), visitor);
            }
        }

        void NodeWriter::writeNodes(const std::vector<Model::Node*>& nodes) {
            m_serializer->beginFile();

            // Assort nodes according to their type and, in case of brushes, whether they are entity or world brushes.
            std::vector<Model::GroupNode*> groups;
            std::vector<Model::EntityNode*> entities;
            std::vector<Model::BrushNode*> worldBrushes;
            EntityBrushesMap entityBrushes;

            for (auto* node : nodes) {
                node->acceptLambda(kdl::overload(
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
            
            WriteNode visitor(*m_serializer);
            Model::Node::accept(std::begin(groups), std::end(groups), visitor);
            Model::Node::accept(std::begin(entities), std::end(entities), visitor);

            m_serializer->endFile();
        }

        void NodeWriter::writeWorldBrushes(const std::vector<Model::BrushNode*>& brushes) {
            if (!brushes.empty()) {
                m_serializer->entity(&m_world, m_world.attributes(), {}, brushes);
            }
        }

        void NodeWriter::writeEntityBrushes(const EntityBrushesMap& entityBrushes) {
            for (const auto& [entity, brushes] : entityBrushes) {
                m_serializer->entity(entity, entity->attributes(), {}, brushes);
            }
        }

        void NodeWriter::writeBrushFaces(const std::vector<Model::BrushFace>& faces) {
            m_serializer->beginFile();
            m_serializer->brushFaces(faces);
            m_serializer->endFile();
        }
    }
}
