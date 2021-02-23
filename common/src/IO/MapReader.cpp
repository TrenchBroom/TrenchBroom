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

#include "MapReader.h"

#include "IO/ParserStatus.h"
#include "Model/BrushError.h"
#include "Model/BrushFace.h"
#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/EntityProperties.h"
#include "Model/MapFormat.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/LockState.h"
#include "Model/VisibilityState.h"

#include <vecmath/mat.h>
#include <vecmath/mat_io.h>

#include <kdl/map_utils.h>
#include <kdl/parallel.h>
#include <kdl/result.h>
#include <kdl/string_format.h>
#include <kdl/string_utils.h>
#include <kdl/vector_utils.h>

#include <cassert>
#include <map>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace IO {
        MapReader::ParentInfo MapReader::ParentInfo::layer(const Model::IdType layerId) {
            return ParentInfo(Type_Layer, layerId);
        }

        MapReader::ParentInfo MapReader::ParentInfo::group(const Model::IdType groupId) {
            return ParentInfo(Type_Group, groupId);
        }

        MapReader::ParentInfo::ParentInfo(const Type type, const Model::IdType id) :
        m_type(type),
        m_id(id) {}

        bool MapReader::ParentInfo::layer() const {
            return m_type == Type_Layer;
        }

        bool MapReader::ParentInfo::group() const {
            return m_type == Type_Group;
        }

        Model::IdType MapReader::ParentInfo::id() const {
            return m_id;
        }

        MapReader::MapReader(std::string_view str, const Model::MapFormat sourceMapFormat, const Model::MapFormat targetMapFormat) :
        StandardMapParser(std::move(str), sourceMapFormat, targetMapFormat),
        m_brushParent(nullptr),
        m_currentNode(nullptr) {}


        void MapReader::readEntities(const vm::bbox3& worldBounds, ParserStatus& status) {
            m_worldBounds = worldBounds;
            parseEntities(status);
            createNodes(status);
            resolveNodes(status);
        }

        void MapReader::readBrushes(const vm::bbox3& worldBounds, ParserStatus& status) {
            m_worldBounds = worldBounds;
            parseBrushes(status);
            createNodes(status);
        }

        void MapReader::readBrushFaces(const vm::bbox3& worldBounds, ParserStatus& status) {
            m_worldBounds = worldBounds;
            parseBrushFaces(status);
        }

        // implement MapParser interface

        void MapReader::onBeginEntity(const size_t /* line */, const std::vector<Model::EntityProperty>& properties, const ExtraAttributes& extraAttributes, ParserStatus& /* status */) {
            const size_t brushesBegin = m_brushInfos.size();

            m_entityInfos.push_back(EntityInfo{ 0, 0, properties, extraAttributes, brushesBegin, brushesBegin});
        }

        void MapReader::onEndEntity(const size_t startLine, const size_t lineCount, ParserStatus& /* status */) {
            EntityInfo& entity = m_entityInfos.back();
            entity.startLine = startLine;
            entity.lineCount = lineCount;
        }

        void MapReader::onBeginBrush(const size_t /* line */, ParserStatus& /* status */) {
            m_brushInfos.push_back(BrushInfo{{}, 0, 0, {}});
        }

        void MapReader::onEndBrush(const size_t startLine, const size_t lineCount, const ExtraAttributes& extraAttributes, ParserStatus& /* status */) {
            BrushInfo& brush = m_brushInfos.back();
            brush.startLine = startLine;
            brush.lineCount = lineCount;
            brush.extraAttributes = extraAttributes;

            // if there is an open entity, extend its brushes end
            if (!m_entityInfos.empty()) {
                EntityInfo& entity = m_entityInfos.back();
                ++entity.brushesEnd;
                assert(entity.brushesEnd == m_brushInfos.size());
            }
        }

        void MapReader::onStandardBrushFace(const size_t line, const Model::MapFormat targetMapFormat, const vm::vec3& point1, const vm::vec3& point2, const vm::vec3& point3, const Model::BrushFaceAttributes& attribs, ParserStatus& status) {
            Model::BrushFace::createFromStandard(point1, point2, point3, attribs, targetMapFormat)
                .and_then([&](Model::BrushFace&& face) {
                    face.setFilePosition(line, 1u);
                    onBrushFace(std::move(face), status);
                }).handle_errors([&](const Model::BrushError e) {
                    status.error(line, kdl::str_to_string("Skipping face: ", e));
                });
        }

        void MapReader::onValveBrushFace(const size_t line, const Model::MapFormat targetMapFormat, const vm::vec3& point1, const vm::vec3& point2, const vm::vec3& point3, const Model::BrushFaceAttributes& attribs, const vm::vec3& texAxisX, const vm::vec3& texAxisY, ParserStatus& status) {
           Model::BrushFace::createFromValve(point1, point2, point3, attribs, texAxisX, texAxisY, targetMapFormat)
            .and_then([&](Model::BrushFace&& face) {
                face.setFilePosition(line, 1u);
                onBrushFace(std::move(face), status);
            }).handle_errors([&](const Model::BrushError e) {
                status.error(line, kdl::str_to_string("Skipping face: ", e));
            });
        }

        // helper methods

        void MapReader::createNodes(ParserStatus& status) {
            auto loadedBrushes = loadBrushes(status);

            for (EntityInfo& info : m_entityInfos) {
                createNode(info, loadedBrushes, status);
            }

            // handle the case of parsing no entities, but a list of brushes (NodeReader)
            if (m_entityInfos.empty()) {
                for (LoadedBrush& loadedBrush : loadedBrushes) {
                    createBrush(std::move(*loadedBrush.brush), nullptr, loadedBrush.startLine, loadedBrush.lineCount, std::move(loadedBrush.extraAttributes), status);
                }
            }
        }

        void MapReader::createNode(EntityInfo& info, std::vector<LoadedBrush>& loadedBrushes, ParserStatus& status) {
            const auto& properties = info.properties;
            const auto& extraAttributes = info.extraAttributes;
            const size_t line = info.startLine;
            const size_t startLine = info.startLine;
            const size_t lineCount = info.lineCount;

            const EntityType type = entityType(properties);
            switch (type) {
                case EntityType_Layer:
                    createLayer(line, properties, extraAttributes, status);
                    break;
                case EntityType_Group:
                    createGroup(line, properties, extraAttributes, status);
                    break;
                case EntityType_Worldspawn:
                    m_brushParent = onWorldspawn(properties, extraAttributes, status);
                    break;
                case EntityType_Default:
                    createEntity(line, properties, extraAttributes, status);
                    break;
            }

            // add brushes
            for (size_t i = info.brushesBegin; i < info.brushesEnd; ++i) {
                LoadedBrush& loadedBrush = loadedBrushes.at(i);
                createBrush(std::move(*loadedBrush.brush), m_brushParent, loadedBrush.startLine, loadedBrush.lineCount, std::move(loadedBrush.extraAttributes), status);
            }

            // cleanup
            if (m_currentNode != nullptr)
                setFilePosition(m_currentNode, startLine, lineCount);
            else
                onWorldspawnFilePosition(startLine, lineCount, status);
            m_currentNode = nullptr;
            m_brushParent = nullptr;
        }

        void MapReader::createLayer(const size_t line, const std::vector<Model::EntityProperty>& propeties, const ExtraAttributes& extraAttributes, ParserStatus& status) {
            const std::string& name = findProperty(propeties, Model::PropertyKeys::LayerName);
            if (kdl::str_is_blank(name)) {
                status.error(line, "Skipping layer entity: missing name");
                return;
            }

            const std::string& idStr = findProperty(propeties, Model::PropertyKeys::LayerId);
            if (kdl::str_is_blank(idStr)) {
                status.error(line, "Skipping layer entity: missing id");
                return;
            }

            const auto rawId = kdl::str_to_size(idStr);
            if (!rawId || *rawId <= 0u) {
                status.error(line, kdl::str_to_string("Skipping layer entity: '", idStr, "' is not a valid id"));
                return;
            }

            const Model::IdType layerId = static_cast<Model::IdType>(*rawId);
            if (m_layers.count(layerId) > 0) {
                status.error(line, kdl::str_to_string("Skipping layer entity: layer with id '", idStr, "' already exists"));
                return;
            }

            Model::Layer layer = Model::Layer(name);
            // This is optional (not present on maps saved in TB 2020.1 and earlier)
            if (const auto layerSortIndex = kdl::str_to_int(findProperty(propeties, Model::PropertyKeys::LayerSortIndex))) {
                layer.setSortIndex(*layerSortIndex);
            }

            if (findProperty(propeties, Model::PropertyKeys::LayerOmitFromExport) == Model::PropertyValues::LayerOmitFromExportValue) {
                layer.setOmitFromExport(true);
            }

            Model::LayerNode* layerNode = new Model::LayerNode(std::move(layer));
            layerNode->setPersistentId(layerId);

            if (findProperty(propeties, Model::PropertyKeys::LayerLocked) == Model::PropertyValues::LayerLockedValue) {
                layerNode->setLockState(Model::LockState::Lock_Locked);
            }
            if (findProperty(propeties, Model::PropertyKeys::LayerHidden) == Model::PropertyValues::LayerHiddenValue) {
                layerNode->setVisibilityState(Model::VisibilityState::Visibility_Hidden);
            }

            setExtraAttributes(layerNode, extraAttributes);
            m_layers.insert(std::make_pair(layerId, layerNode));

            onLayer(layerNode, status);

            m_currentNode = layerNode;
            m_brushParent = layerNode;
        }

        void MapReader::createGroup(const size_t line, const std::vector<Model::EntityProperty>& properties, const ExtraAttributes& extraAttributes, ParserStatus& status) {
            const std::string& name = findProperty(properties, Model::PropertyKeys::GroupName);
            if (kdl::str_is_blank(name)) {
                status.error(line, "Skipping group entity: missing name");
                return;
            }

            const std::string& idStr = findProperty(properties, Model::PropertyKeys::GroupId);
            if (kdl::str_is_blank(idStr)) {
                status.error(line, "Skipping group entity: missing id");
                return;
            }

            const auto rawId = kdl::str_to_size(idStr);
            if (!rawId || *rawId <= 0) {
                status.error(line, kdl::str_to_string("Skipping group entity: '", idStr, "' is not a valid id"));
                return;
            }

            const Model::IdType groupId = static_cast<Model::IdType>(*rawId);
            if (m_groups.count(groupId) > 0) {
                status.error(line, kdl::str_to_string("Skipping group entity: group with id '", idStr, "' already exists"));
                return;
            }

            Model::GroupNode* groupNode = new Model::GroupNode(Model::Group(name));
            groupNode->setPersistentId(groupId);
            setExtraAttributes(groupNode, extraAttributes);

            storeNode(groupNode, properties, status);
            m_groups.insert(std::make_pair(groupId, groupNode));

            const std::string& linkedGroupId = findProperty(properties, Model::PropertyKeys::LinkedGroupId);
            if (!linkedGroupId.empty()) {
                const std::string& transformationStr = findProperty(properties, Model::PropertyKeys::GroupTransformation);
                if (const auto transformation = vm::parse<FloatType, 4u, 4u>(transformationStr)) {
                    auto group = groupNode->group();
                    group.setLinkedGroupId(linkedGroupId);
                    group.setTransformation(*transformation);
                    groupNode->setGroup(std::move(group));

                    status.debug(line, kdl::str_to_string("Setting linked group ID '", linkedGroupId, "' to group with ID  '", idStr + "'"));
                } else {
                    status.error(line, kdl::str_to_string("Not linking group entity: Could not parse transformation '", transformationStr + "'"));
                }
            }

            m_currentNode = groupNode;
            m_brushParent = groupNode;
        }

        void MapReader::createEntity(const size_t /* line */, const std::vector<Model::EntityProperty>& properties, const ExtraAttributes& extraAttributes, ParserStatus& status) {
            auto entity = Model::Entity{properties};
            if (const auto* protectedPropertiesStr = entity.property(Model::PropertyKeys::ProtectedEntityProperties)) {
                auto protectedProperties = kdl::str_split(*protectedPropertiesStr, ";");
                entity.setProtectedProperties(std::move(protectedProperties));
                entity.removeProperty(Model::PropertyKeys::ProtectedEntityProperties);
            }

            Model::EntityNode* entityNode = new Model::EntityNode{std::move(entity)};
            setExtraAttributes(entityNode, extraAttributes);

            const ParentInfo::Type parentType = storeNode(entityNode, properties, status);
            stripParentProperties(entityNode, parentType);

            m_currentNode = entityNode;
            m_brushParent = entityNode;
        }

        void MapReader::createBrush(kdl::result<Model::Brush, Model::BrushError> brush, Model::Node* parent, const size_t startLine, const size_t lineCount, const ExtraAttributes& extraAttributes, ParserStatus& status) {
            std::move(brush)
                .and_then(
                    [&](Model::Brush&& b) {
                        Model::BrushNode* brushNode = new Model::BrushNode(std::move(b));
                        setFilePosition(brushNode, startLine, lineCount);
                        setExtraAttributes(brushNode, extraAttributes);

                        onBrush(parent, brushNode, status);
                    }
                ).handle_errors(
                    [&](const Model::BrushError e) {
                        status.error(startLine, kdl::str_to_string("Skipping brush: ", e));
                    }
                );
        }

        MapReader::ParentInfo::Type MapReader::storeNode(Model::Node* node, const std::vector<Model::EntityProperty>& properties, ParserStatus& status) {
            const std::string& layerIdStr = findProperty(properties, Model::PropertyKeys::Layer);
            if (!kdl::str_is_blank(layerIdStr)) {
                if (const auto rawId = kdl::str_to_size(layerIdStr)) {
                    const Model::IdType layerId = static_cast<Model::IdType>(*rawId);
                    const auto it = m_layers.find(layerId);
                    if (it != std::end(m_layers)) {
                        onNode(it->second, node, status);
                    } else {
                        m_unresolvedNodes.emplace_back(node, ParentInfo::layer(layerId));
                    }
                    return ParentInfo::Type_Layer;
                }

                status.warn(node->lineNumber(), kdl::str_to_string("Entity has invalid parent id '", layerIdStr, "'"));
            } else {
                const std::string& groupIdStr = findProperty(properties, Model::PropertyKeys::Group);
                if (!kdl::str_is_blank(groupIdStr)) {
                    if (const auto rawId = kdl::str_to_size(groupIdStr)) {
                        const Model::IdType groupId = static_cast<Model::IdType>(*rawId);
                        const auto it = m_groups.find(groupId);
                        if (it != std::end(m_groups)) {
                            onNode(it->second, node, status);
                        } else {
                            m_unresolvedNodes.emplace_back(node, ParentInfo::group(groupId));
                        }
                        return ParentInfo::Type_Group;
                    }

                    status.warn(node->lineNumber(), kdl::str_to_string("Entity has invalid parent id '", groupIdStr, "'"));
                }
            }

            onNode(nullptr, node, status);
            return ParentInfo::Type_None;
        }

        void MapReader::stripParentProperties(Model::EntityNodeBase* node, ParentInfo::Type parentType) {
            auto entity = node->entity();
            switch (parentType) {
                case ParentInfo::Type_Layer:
                    entity.removeProperty(Model::PropertyKeys::Layer);
                    break;
                case ParentInfo::Type_Group:
                    entity.removeProperty(Model::PropertyKeys::Group);
                    break;
                case ParentInfo::Type_None:
                    break;
                switchDefault();
            }
            node->setEntity(std::move(entity));
        }

        /**
         * Resolves cases when a child is parsed before its parent; called after the whole map is parsed.
         */
        void MapReader::resolveNodes(ParserStatus& status) {
            for (const auto& [node, info] : m_unresolvedNodes) {
                Model::Node* parent = resolveParent(info);
                if (parent == nullptr)
                    onUnresolvedNode(info, node, status);
                else
                    onNode(parent, node, status);
            }
        }

        /**
         * Transforms m_brushInfos into a vector of LoadedBrush (leaving m_brushInfos empty).
         */
        std::vector<MapReader::LoadedBrush> MapReader::loadBrushes(ParserStatus& /* status */) {
            // In parallel, create Brush objects (moving faces out of m_brushInfos)
            auto loadedBrushes = kdl::vec_parallel_transform(std::move(m_brushInfos), [&](BrushInfo&& brushInfo) {
                LoadedBrush result;
                result.brush = std::make_optional(Model::Brush::create(m_worldBounds, std::move(brushInfo.faces)));
                result.extraAttributes = std::move(brushInfo.extraAttributes);
                result.startLine = brushInfo.startLine;
                result.lineCount = brushInfo.lineCount;
                return result;
            });

            assert(m_brushInfos.empty());

            return loadedBrushes;
        }

        Model::Node* MapReader::resolveParent(const ParentInfo& parentInfo) const {
            if (parentInfo.layer()) {
                const Model::IdType layerId = parentInfo.id();
                const auto it = m_layers.find(layerId);
                return it != std::end(m_layers) ? it->second : nullptr;
            }
            const Model::IdType groupId = parentInfo.id();
            const auto it = m_groups.find(groupId);
            return it != std::end(m_groups) ? it->second : nullptr;
        }

        MapReader::EntityType MapReader::entityType(const std::vector<Model::EntityProperty>& properties) const {
            const std::string& classname = findProperty(properties, Model::PropertyKeys::Classname);
            if (isLayer(classname, properties))
                return EntityType_Layer;
            if (isGroup(classname, properties))
                return EntityType_Group;
            if (isWorldspawn(classname, properties))
                return EntityType_Worldspawn;
            return EntityType_Default;
        }

        void MapReader::setFilePosition(Model::Node* node, const size_t startLine, const size_t lineCount) {
            node->setFilePosition(startLine, lineCount);
        }

        void MapReader::setExtraAttributes(Model::Node* /* node */, const ExtraAttributes& extraAttributes) {
            ExtraAttributes::const_iterator it;
            it = extraAttributes.find("hideIssues");
            if (it != std::end(extraAttributes)) {
                const ExtraAttribute& attribute = it->second;
                attribute.assertType(ExtraAttribute::Type_Integer);
                // const Model::IssueType mask = attribute.intValue<Model::IssueType>();
                // object->setHiddenIssues(mask);
            }
        }

        /**
         * Default implementation adds it to the current BrushInfo
         * Overridden in BrushFaceReader (which doesn't use m_brushInfos) to collect the faces directly
         */
        void MapReader::onBrushFace(Model::BrushFace face, ParserStatus& /* status */) {
            assert(!m_brushInfos.empty());
            BrushInfo& brush = m_brushInfos.back();
            brush.faces.push_back(std::move(face));
        }
    }
}
