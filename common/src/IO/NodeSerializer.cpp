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

#include "NodeSerializer.h"

#include "Model/BrushFace.h"
#include "Model/BrushNode.h"
#include "Model/GroupNode.h"
#include "Model/EntityAttributes.h"
#include "Model/LayerNode.h"
#include "Model/NodeVisitor.h"
#include "Model/WorldNode.h"

#include <kdl/string_format.h>
#include <kdl/string_utils.h>

#include <string>

namespace TrenchBroom {
    namespace IO {
        class NodeSerializer::BrushSerializer : public Model::ConstNodeVisitor {
        private:
            NodeSerializer& m_serializer;
        public:
            explicit BrushSerializer(NodeSerializer& serializer) : m_serializer(serializer) {}

            void doVisit(const Model::WorldNode* /* world */) override   {}
            void doVisit(const Model::LayerNode* /* layer */) override   {}
            void doVisit(const Model::GroupNode* /* group */) override   {}
            void doVisit(const Model::EntityNode* /* entity */) override {}
            void doVisit(const Model::BrushNode* brush) override   { m_serializer.brush(brush); }
        };

        const std::string& NodeSerializer::IdManager::getId(const Model::Node* t) const {
            auto it = m_ids.find(t);
            if (it == std::end(m_ids)) {
                it = m_ids.insert(std::make_pair(t, idToString(makeId()))).first;
            }
            return it->second;
        }

        Model::IdType NodeSerializer::IdManager::makeId() const {
            static Model::IdType currentId = 1;
            return currentId++;
        }

        std::string NodeSerializer::IdManager::idToString(const Model::IdType nodeId) const {
            return kdl::str_to_string(nodeId);
        }

        NodeSerializer::NodeSerializer() :
        m_entityNo(0),
        m_brushNo(0) {}

        NodeSerializer::~NodeSerializer() = default;

        NodeSerializer::ObjectNo NodeSerializer::entityNo() const {
            return m_entityNo;
        }

        NodeSerializer::ObjectNo NodeSerializer::brushNo() const {
            return m_brushNo;
        }

        void NodeSerializer::beginFile() {
            m_entityNo = 0;
            m_brushNo = 0;
            doBeginFile();
        }

        void NodeSerializer::endFile() {
            doEndFile();
        }

        /**
         * Writes the worldspawn entity.
         */
        void NodeSerializer::defaultLayer(const Model::WorldNode& world) {
            auto worldAttribs = Model::EntityAttributes(world.attributes());

            // Transfer the color, locked state, and hidden state from the default layer Layer object to worldspawn
            Model::LayerNode* defaultLayer = world.defaultLayer();
            if (defaultLayer->hasAttribute(Model::AttributeNames::LayerColor)) {
                worldAttribs.addOrUpdateAttribute(Model::AttributeNames::LayerColor, defaultLayer->attribute(Model::AttributeNames::LayerColor), nullptr);
            } else {
                worldAttribs.removeAttribute(Model::AttributeNames::LayerColor);
            }

            if (defaultLayer->locked()) {
                worldAttribs.addOrUpdateAttribute(Model::AttributeNames::LayerLocked, Model::AttributeValues::LayerLockedValue, nullptr);
            } else {
                worldAttribs.removeAttribute(Model::AttributeNames::LayerLocked);
            }

            if (defaultLayer->hidden()) {
                worldAttribs.addOrUpdateAttribute(Model::AttributeNames::LayerHidden, Model::AttributeValues::LayerHiddenValue, nullptr);
            } else {
                worldAttribs.removeAttribute(Model::AttributeNames::LayerHidden);
            }

            entity(&world, worldAttribs.releaseAttributes(), {}, world.defaultLayer());
        }

        void NodeSerializer::customLayer(const Model::LayerNode* layer) {
            entity(layer, layerAttributes(layer), {}, layer);
        }

        void NodeSerializer::group(const Model::GroupNode* group, const std::vector<Model::EntityAttribute>& parentAttributes) {
            entity(group, groupAttributes(group), parentAttributes, group);
        }

        void NodeSerializer::entity(const Model::Node* node, const std::vector<Model::EntityAttribute>& attributes, const std::vector<Model::EntityAttribute>& parentAttributes, const Model::Node* brushParent) {
            beginEntity(node, attributes, parentAttributes);

            BrushSerializer brushSerializer(*this);
            brushParent->iterate(brushSerializer);

            endEntity(node);
        }

        void NodeSerializer::entity(const Model::Node* node, const std::vector<Model::EntityAttribute>& attributes, const std::vector<Model::EntityAttribute>& parentAttributes, const std::vector<Model::BrushNode*>& entityBrushes) {
            beginEntity(node, attributes, parentAttributes);
            brushes(entityBrushes);
            endEntity(node);
        }

        void NodeSerializer::beginEntity(const Model::Node* node, const std::vector<Model::EntityAttribute>& attributes, const std::vector<Model::EntityAttribute>& extraAttributes) {
            beginEntity(node);
            entityAttributes(attributes);
            entityAttributes(extraAttributes);
        }

        void NodeSerializer::beginEntity(const Model::Node* node) {
            m_brushNo = 0;
            doBeginEntity(node);
        }

        void NodeSerializer::endEntity(const Model::Node* node) {
            doEndEntity(node);
            ++m_entityNo;
        }

        void NodeSerializer::entityAttributes(const std::vector<Model::EntityAttribute>& attributes) {
            for (const auto& attribute : attributes) {
                entityAttribute(attribute);
            }
        }

        void NodeSerializer::entityAttribute(const Model::EntityAttribute& attribute) {
            doEntityAttribute(attribute);
        }

        void NodeSerializer::brushes(const std::vector<Model::BrushNode*>& brushNodes) {
            for (auto* brush : brushNodes) {
                this->brush(brush);
            }
        }

        void NodeSerializer::brush(const Model::BrushNode* brushNode) {
            beginBrush(brushNode);
            brushFaces(brushNode->brush().faces());
            endBrush(brushNode);
        }

        void NodeSerializer::beginBrush(const Model::BrushNode* brushNode) {
            doBeginBrush(brushNode);
        }

        void NodeSerializer::endBrush(const Model::BrushNode* brushNode) {
            doEndBrush(brushNode);
            ++m_brushNo;
        }

        void NodeSerializer::brushFaces(const std::vector<Model::BrushFace>& faces) {
            for (const auto& face : faces) {
                brushFace(face);
            }
        }

        void NodeSerializer::brushFace(const Model::BrushFace& face) {
            doBrushFace(face);
        }

        class NodeSerializer::GetParentAttributes : public Model::ConstNodeVisitor {
        private:
            const IdManager& m_layerIds;
            const IdManager& m_groupIds;
            std::vector<Model::EntityAttribute> m_attributes;
        public:
            GetParentAttributes(const IdManager& layerIds, const IdManager& groupIds) :
            m_layerIds(layerIds),
            m_groupIds(groupIds) {}

            const std::vector<Model::EntityAttribute>& attributes() const {
                return m_attributes;
            }
        private:
            void doVisit(const Model::WorldNode* /* world */) override   {}
            void doVisit(const Model::LayerNode* layer) override   { m_attributes.push_back(Model::EntityAttribute(Model::AttributeNames::Layer, m_layerIds.getId(layer)));}
            void doVisit(const Model::GroupNode* group) override   { m_attributes.push_back(Model::EntityAttribute(Model::AttributeNames::Group, m_groupIds.getId(group))); }
            void doVisit(const Model::EntityNode* /* entity */) override {}
            void doVisit(const Model::BrushNode* /* brush */) override   {}
        };

        std::vector<Model::EntityAttribute> NodeSerializer::parentAttributes(const Model::Node* node) {
            if (node == nullptr) {
                return std::vector<Model::EntityAttribute>(0);
            }

            GetParentAttributes visitor(m_layerIds, m_groupIds);
            node->accept(visitor);
            return visitor.attributes();
        }

        std::vector<Model::EntityAttribute> NodeSerializer::layerAttributes(const Model::LayerNode* layer) {
            std::vector<Model::EntityAttribute> result = {
                Model::EntityAttribute(Model::AttributeNames::Classname, Model::AttributeValues::LayerClassname),
                Model::EntityAttribute(Model::AttributeNames::GroupType, Model::AttributeValues::GroupTypeLayer),
                Model::EntityAttribute(Model::AttributeNames::LayerName, layer->name()),
                Model::EntityAttribute(Model::AttributeNames::LayerId, m_layerIds.getId(layer)),
            };
            if (layer->hasAttribute(Model::AttributeNames::LayerSortIndex)) {
                result.push_back(Model::EntityAttribute(Model::AttributeNames::LayerSortIndex, layer->attribute(Model::AttributeNames::LayerSortIndex)));
            }
            if (layer->locked()) {
                result.push_back(Model::EntityAttribute(Model::AttributeNames::LayerLocked, Model::AttributeValues::LayerLockedValue));
            }
            if (layer->hidden()) {
                result.push_back(Model::EntityAttribute(Model::AttributeNames::LayerHidden, Model::AttributeValues::LayerHiddenValue));
            }
            return result;
        }

        std::vector<Model::EntityAttribute> NodeSerializer::groupAttributes(const Model::GroupNode* group) {
            return {
                Model::EntityAttribute(Model::AttributeNames::Classname, Model::AttributeValues::GroupClassname),
                Model::EntityAttribute(Model::AttributeNames::GroupType, Model::AttributeValues::GroupTypeGroup),
                Model::EntityAttribute(Model::AttributeNames::GroupName, group->name()),
                Model::EntityAttribute(Model::AttributeNames::GroupId, m_groupIds.getId(group)),
            };
        }

        std::string NodeSerializer::escapeEntityAttribute(const std::string& str) const {
            // Remove a trailing unescaped backslash, as this will choke the parser.
            const auto l = str.size();
            if (l > 0 && str[l-1] == '\\') {
                const auto p = str.find_last_not_of('\\');
                if ((l - p) % 2 == 0) {
                    // Only remove a trailing backslash if there is an uneven number of trailing backslashes.
                    return kdl::str_escape_if_necessary(str.substr(0, l-1), "\"");
                }
            }
            return kdl::str_escape_if_necessary(str, "\"");
        }
    }
}
