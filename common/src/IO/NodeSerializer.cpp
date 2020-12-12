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
#include "Model/EntityProperties.h"
#include "Model/LayerNode.h"
#include "Model/LockState.h"
#include "Model/WorldNode.h"

#include <vecmath/vec_io.h> // for Color stream output operator

#include <kdl/overload.h>
#include <kdl/string_format.h>
#include <kdl/string_utils.h>

#include <string>

namespace TrenchBroom {
    namespace IO {
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
        m_brushNo(0),
        m_exporting(false) {}

        NodeSerializer::~NodeSerializer() = default;

        NodeSerializer::ObjectNo NodeSerializer::entityNo() const {
            return m_entityNo;
        }

        NodeSerializer::ObjectNo NodeSerializer::brushNo() const {
            return m_brushNo;
        }

        bool NodeSerializer::exporting() const {
            return m_exporting;
        }

        void NodeSerializer::setExporting(const bool exporting) {
            m_exporting = exporting;
        }

        void NodeSerializer::beginFile(const std::vector<const Model::Node*>& rootNodes) {
            m_entityNo = 0;
            m_brushNo = 0;
            doBeginFile(rootNodes);
        }

        void NodeSerializer::endFile() {
            doEndFile();
        }

        /**
         * Writes the worldspawn entity.
         */
        void NodeSerializer::defaultLayer(const Model::WorldNode& world) {
            auto worldEntity = world.entity();

            // Transfer the color, locked state, and hidden state from the default layer Layer object to worldspawn
            const Model::LayerNode* defaultLayerNode = world.defaultLayer();
            const Model::Layer& defaultLayer = defaultLayerNode->layer();
            if (defaultLayer.color()) {
                worldEntity.addOrUpdateAttribute(Model::PropertyKeys::LayerColor, kdl::str_to_string(*defaultLayer.color()));
            } else {
                worldEntity.removeAttribute(Model::PropertyKeys::LayerColor);
            }

            if (defaultLayerNode->lockState() == Model::LockState::Lock_Locked) {
                worldEntity.addOrUpdateAttribute(Model::PropertyKeys::LayerLocked, Model::PropertyValues::LayerLockedValue);
            } else {
                worldEntity.removeAttribute(Model::PropertyKeys::LayerLocked);
            }

            if (defaultLayerNode->hidden()) {
                worldEntity.addOrUpdateAttribute(Model::PropertyKeys::LayerHidden, Model::PropertyValues::LayerHiddenValue);
            } else {
                worldEntity.removeAttribute(Model::PropertyKeys::LayerHidden);
            }

            if (defaultLayer.omitFromExport()) {
                worldEntity.addOrUpdateAttribute(Model::PropertyKeys::LayerOmitFromExport, Model::PropertyValues::LayerOmitFromExportValue);
            } else {
                worldEntity.removeAttribute(Model::PropertyKeys::LayerOmitFromExport);
            }

            if (m_exporting && defaultLayer.omitFromExport()) {
                beginEntity(&world, worldEntity.attributes(), {});
                endEntity(&world);
            } else {
                entity(&world, worldEntity.attributes(), {}, world.defaultLayer());
            }
        }

        void NodeSerializer::customLayer(const Model::LayerNode* layer) {
            if (!(m_exporting && layer->layer().omitFromExport())) {
                entity(layer, layerAttributes(layer), {}, layer);
            }
        }

        void NodeSerializer::group(const Model::GroupNode* group, const std::vector<Model::EntityAttribute>& parentAttributes) {
            entity(group, groupAttributes(group), parentAttributes, group);
        }

        void NodeSerializer::entity(const Model::Node* node, const std::vector<Model::EntityAttribute>& attributes, const std::vector<Model::EntityAttribute>& parentAttributes, const Model::Node* brushParent) {
            beginEntity(node, attributes, parentAttributes);

            brushParent->visitChildren(kdl::overload(
                [] (const Model::WorldNode*)   {},
                [] (const Model::LayerNode*)   {},
                [] (const Model::GroupNode*)   {},
                [] (const Model::EntityNode*)  {},
                [&](const Model::BrushNode* b) {
                    brush(b);
                }
            ));

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
            doBrush(brushNode);
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

        std::vector<Model::EntityAttribute> NodeSerializer::parentAttributes(const Model::Node* node) {
            if (node == nullptr) {
                return std::vector<Model::EntityAttribute>{};
            }

            auto attributes = std::vector<Model::EntityAttribute>{};
            node->accept(kdl::overload(
                [](const Model::WorldNode*) {},
                [&](const Model::LayerNode* layer) { attributes.push_back(Model::EntityAttribute(Model::PropertyKeys::Layer, m_layerIds.getId(layer))); },
                [&](const Model::GroupNode* group) { attributes.push_back(Model::EntityAttribute(Model::PropertyKeys::Group, m_groupIds.getId(group))); },
                [](const Model::EntityNode*) {},
                [](const Model::BrushNode*) {}
            ));

            return attributes;
        }

        std::vector<Model::EntityAttribute> NodeSerializer::layerAttributes(const Model::LayerNode* layerNode) {
            std::vector<Model::EntityAttribute> result = {
                Model::EntityAttribute(Model::PropertyKeys::Classname, Model::PropertyValues::LayerClassname),
                Model::EntityAttribute(Model::PropertyKeys::GroupType, Model::PropertyValues::GroupTypeLayer),
                Model::EntityAttribute(Model::PropertyKeys::LayerName, layerNode->name()),
                Model::EntityAttribute(Model::PropertyKeys::LayerId, m_layerIds.getId(layerNode)),
            };

            const auto& layer = layerNode->layer();
            if (layer.hasSortIndex()) {
                result.push_back(Model::EntityAttribute(Model::PropertyKeys::LayerSortIndex, kdl::str_to_string(layer.sortIndex())));
            }
            if (layerNode->lockState() == Model::LockState::Lock_Locked) {
                result.push_back(Model::EntityAttribute(Model::PropertyKeys::LayerLocked, Model::PropertyValues::LayerLockedValue));
            }
            if (layerNode->hidden()) {
                result.push_back(Model::EntityAttribute(Model::PropertyKeys::LayerHidden, Model::PropertyValues::LayerHiddenValue));
            }
            if (layer.omitFromExport()) {
                result.push_back(Model::EntityAttribute(Model::PropertyKeys::LayerOmitFromExport, Model::PropertyValues::LayerOmitFromExportValue));
            }
            return result;
        }

        std::vector<Model::EntityAttribute> NodeSerializer::groupAttributes(const Model::GroupNode* group) {
            return {
                Model::EntityAttribute(Model::PropertyKeys::Classname, Model::PropertyValues::GroupClassname),
                Model::EntityAttribute(Model::PropertyKeys::GroupType, Model::PropertyValues::GroupTypeGroup),
                Model::EntityAttribute(Model::PropertyKeys::GroupName, group->name()),
                Model::EntityAttribute(Model::PropertyKeys::GroupId, m_groupIds.getId(group)),
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
