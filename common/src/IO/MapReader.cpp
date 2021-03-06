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

#include <kdl/parallel.h>
#include <kdl/result.h>
#include <kdl/string_format.h>
#include <kdl/string_utils.h>
#include <kdl/vector_utils.h>

#include <cassert>
#include <optional>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace IO {
        MapReader::MapReader(std::string_view str, const Model::MapFormat sourceMapFormat, const Model::MapFormat targetMapFormat) :
        StandardMapParser(std::move(str), sourceMapFormat, targetMapFormat) {}

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

        void MapReader::onBeginEntity(const size_t /* line */, std::vector<Model::EntityProperty> properties, ParserStatus& /* status */) {
            const size_t brushesBegin = m_brushInfos.size();

            m_entityInfos.push_back(EntityInfo{ 0, 0, std::move(properties), brushesBegin, brushesBegin});
        }

        void MapReader::onEndEntity(const size_t startLine, const size_t lineCount, ParserStatus& /* status */) {
            EntityInfo& entity = m_entityInfos.back();
            entity.startLine = startLine;
            entity.lineCount = lineCount;
        }

        void MapReader::onBeginBrush(const size_t /* line */, ParserStatus& /* status */) {
            m_brushInfos.push_back(BrushInfo{{}, 0, 0});
        }

        void MapReader::onEndBrush(const size_t startLine, const size_t lineCount, ParserStatus& /* status */) {
            BrushInfo& brush = m_brushInfos.back();
            brush.startLine = startLine;
            brush.lineCount = lineCount;

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

        /**
        * Transforms brush infos into a vector of brush nodes. The resulting vector can contain null if a brush
        * cannot be created! For such brush infos, this function logs an error using the given parser status.
        */
        std::vector<std::unique_ptr<Model::BrushNode>> createBrushes(const vm::bbox3& worldBounds, std::vector<MapReader::BrushInfo> brushInfos, ParserStatus& status) {
            struct LoadedBrush {
                // optional wrapper is just to let this struct be default-constructible
                std::optional<kdl::result<Model::Brush, Model::BrushError>> brush;
                size_t startLine;
                size_t lineCount;
            };

            // In parallel, create Brush objects (moving faces out of m_brushInfos)
            auto loadedBrushes = kdl::vec_parallel_transform(std::move(brushInfos), [&](auto&& brushInfo) {
                return LoadedBrush{
                    std::make_optional(Model::Brush::create(worldBounds, std::move(brushInfo.faces))),
                    brushInfo.startLine,
                    brushInfo.lineCount
                };
            });

            return kdl::vec_transform(std::move(loadedBrushes), [&](auto&& loadedBrush) -> std::unique_ptr<Model::BrushNode> {
                return std::move(*loadedBrush.brush)
                    .visit(kdl::overload(
                        [&](Model::Brush&& b) -> std::unique_ptr<Model::BrushNode> {
                            auto brushNode = std::make_unique<Model::BrushNode>(std::move(b));
                            brushNode->setFilePosition(loadedBrush.startLine, loadedBrush.lineCount);
                            return brushNode;
                        },
                        [&](const Model::BrushError e) -> std::unique_ptr<Model::BrushNode> {
                            status.error(loadedBrush.startLine, kdl::str_to_string("Skipping brush: ", e));
                            return {};
                        }
                    ));
            });
        }

        void MapReader::createNodes(ParserStatus& status) {
            auto brushNodes = createBrushes(m_worldBounds, std::move(m_brushInfos), status);

            if (!m_entityInfos.empty()) {
                Model::Node* currentParent = nullptr;
                for (EntityInfo& info : m_entityInfos) {
                    createLayerGroupOrEntity(currentParent, info, brushNodes, status);
                }
            } else {
                // handle the case of parsing no entities, but a list of brushes (NodeReader)
                for (auto& brushNode : brushNodes) {
                    if (brushNode) {
                        onBrush(nullptr, brushNode.release(), status);
                    }
                }
            }
        }

        enum class EntityType {
            Layer,
            Group,
            Worldspawn,
            Default
        };

        static EntityType entityType(const std::vector<Model::EntityProperty>& properties) {
            const std::string& classname = findProperty(properties, Model::PropertyKeys::Classname);
            if (isLayer(classname, properties)) {
                return EntityType::Layer;
            } else if (isGroup(classname, properties)) {
                return EntityType::Group;
            } else if (isWorldspawn(classname, properties)) {
                return EntityType::Worldspawn;
            } else {
                return EntityType::Default;
            }
        }

        static void stripParentProperties(Model::EntityNodeBase* node) {
            auto entity = node->entity();
            entity.removeProperty(Model::PropertyKeys::Layer);
            entity.removeProperty(Model::PropertyKeys::Group);
            node->setEntity(std::move(entity));
        }

        static std::unique_ptr<Model::LayerNode> createLayer(const size_t line, const std::vector<Model::EntityProperty>& propeties, ParserStatus& status) {
            const std::string& name = findProperty(propeties, Model::PropertyKeys::LayerName);
            if (kdl::str_is_blank(name)) {
                status.error(line, "Skipping layer entity: missing name");
                return {};
            }

            const std::string& idStr = findProperty(propeties, Model::PropertyKeys::LayerId);
            if (kdl::str_is_blank(idStr)) {
                status.error(line, "Skipping layer entity: missing id");
                return {};
            }

            const auto rawId = kdl::str_to_size(idStr);
            if (!rawId || *rawId <= 0u) {
                status.error(line, kdl::str_to_string("Skipping layer entity: '", idStr, "' is not a valid id"));
                return {};
            }

            Model::Layer layer = Model::Layer(name);
            // This is optional (not present on maps saved in TB 2020.1 and earlier)
            if (const auto layerSortIndex = kdl::str_to_int(findProperty(propeties, Model::PropertyKeys::LayerSortIndex))) {
                layer.setSortIndex(*layerSortIndex);
            }

            if (findProperty(propeties, Model::PropertyKeys::LayerOmitFromExport) == Model::PropertyValues::LayerOmitFromExportValue) {
                layer.setOmitFromExport(true);
            }

            auto layerNode = std::make_unique<Model::LayerNode>(std::move(layer));

            const Model::IdType layerId = static_cast<Model::IdType>(*rawId);
            layerNode->setPersistentId(layerId);

            if (findProperty(propeties, Model::PropertyKeys::LayerLocked) == Model::PropertyValues::LayerLockedValue) {
                layerNode->setLockState(Model::LockState::Lock_Locked);
            }
            if (findProperty(propeties, Model::PropertyKeys::LayerHidden) == Model::PropertyValues::LayerHiddenValue) {
                layerNode->setVisibilityState(Model::VisibilityState::Visibility_Hidden);
            }

            return layerNode;
        }

        static std::unique_ptr<Model::GroupNode> createGroup(const size_t line, const std::vector<Model::EntityProperty>& properties, ParserStatus& status) {
            const std::string& name = findProperty(properties, Model::PropertyKeys::GroupName);
            if (kdl::str_is_blank(name)) {
                status.error(line, "Skipping group entity: missing name");
                return {};
            }

            const std::string& idStr = findProperty(properties, Model::PropertyKeys::GroupId);
            if (kdl::str_is_blank(idStr)) {
                status.error(line, "Skipping group entity: missing id");
                return {};
            }

            const auto rawId = kdl::str_to_size(idStr);
            if (!rawId || *rawId <= 0) {
                status.error(line, kdl::str_to_string("Skipping group entity: '", idStr, "' is not a valid id"));
                return {};
            }

            auto groupNode = std::make_unique<Model::GroupNode>(Model::Group(name));

            const Model::IdType groupId = static_cast<Model::IdType>(*rawId);
            groupNode->setPersistentId(groupId);

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

            return groupNode;
        }

        static std::unique_ptr<Model::EntityNode> createEntity(std::vector<Model::EntityProperty> properties) {
            auto entity = Model::Entity{std::move(properties)};
            if (const auto* protectedPropertiesStr = entity.property(Model::PropertyKeys::ProtectedEntityProperties)) {
                auto protectedProperties = kdl::str_split(*protectedPropertiesStr, ";");
                entity.setProtectedProperties(std::move(protectedProperties));
                entity.removeProperty(Model::PropertyKeys::ProtectedEntityProperties);
            }

            return std::make_unique<Model::EntityNode>(std::move(entity));
        }

        void MapReader::createLayerGroupOrEntity(Model::Node*& currentParent, EntityInfo& info, std::vector<std::unique_ptr<Model::BrushNode>>& brushNodes, ParserStatus& status) {
            auto& properties = info.properties;
            const size_t startLine = info.startLine;
            const size_t lineCount = info.lineCount;

            const EntityType type = entityType(properties);
            switch (type) {
                case EntityType::Layer:
                    if (auto layerNode = createLayer(startLine, properties, status)) {
                        const auto layerId = *layerNode->persistentId();
                        if (!m_layers.insert(std::make_pair(layerId, layerNode.get())).second) {
                            status.error(startLine, kdl::str_to_string("Skipping layer entity: layer with id '", layerId, "' already exists"));
                        } else {
                            layerNode->setFilePosition(startLine, lineCount);
                            onLayer(layerNode.get(), status);
                            currentParent = layerNode.release();
                        }
                    }
                    break;
                case EntityType::Group: {
                    if (auto groupNode = createGroup(startLine, properties, status)) {
                        const auto groupId = *groupNode->persistentId();
                        if (!m_groups.insert(std::make_pair(*groupNode->persistentId(), groupNode.get())).second) {
                            status.error(startLine, kdl::str_to_string("Skipping group entity: group with id '", groupId, "' already exists"));
                        } else {
                            groupNode->setFilePosition(startLine, lineCount);
                            storeNode(groupNode.get(), properties, status);
                            currentParent = groupNode.release();
                        }
                    }
                    break;
                }
                case EntityType::Worldspawn: {
                    if (auto* worldNode = onWorldspawn(std::move(properties), status)) {
                        onWorldspawnFilePosition(startLine, lineCount, status);
                        currentParent = worldNode;
                    }
                    break;
                }
                case EntityType::Default: {
                    if (auto entityNode = createEntity(std::move(properties))) {
                        entityNode->setFilePosition(startLine, lineCount);
                        
                        storeNode(entityNode.get(), entityNode->entity().properties(), status);
                        stripParentProperties(entityNode.get());
                        currentParent = entityNode.release();
                    }
                    break;
                }
            }

            // add brushes
            for (size_t i = info.brushesBegin; i < info.brushesEnd; ++i) {
                if (auto& brushNode = brushNodes.at(i)) {
                    onBrush(currentParent, brushNode.release(), status);
                }
            }
        }

        void MapReader::storeNode(Model::Node* node, const std::vector<Model::EntityProperty>& properties, ParserStatus& status) {
            const std::string& layerIdStr = findProperty(properties, Model::PropertyKeys::Layer);
            if (!kdl::str_is_blank(layerIdStr)) {
                if (const auto rawId = kdl::str_to_size(layerIdStr)) {
                    const Model::IdType layerId = static_cast<Model::IdType>(*rawId);
                    const auto it = m_layers.find(layerId);
                    if (it != std::end(m_layers)) {
                        onNode(it->second, node, status);
                    } else {
                        m_unresolvedNodes.emplace_back(node, ParentInfo{ParentType::Layer, layerId});
                    }
                } else {
                    status.warn(node->lineNumber(), kdl::str_to_string("Entity has invalid parent id '", layerIdStr, "'"));
                }
            } else {
                const std::string& groupIdStr = findProperty(properties, Model::PropertyKeys::Group);
                if (!kdl::str_is_blank(groupIdStr)) {
                    if (const auto rawId = kdl::str_to_size(groupIdStr)) {
                        const Model::IdType groupId = static_cast<Model::IdType>(*rawId);
                        const auto it = m_groups.find(groupId);
                        if (it != std::end(m_groups)) {
                            onNode(it->second, node, status);
                        } else {
                            m_unresolvedNodes.emplace_back(node, ParentInfo{ParentType::Group, groupId});
                        }
                    } else {
                        status.warn(node->lineNumber(), kdl::str_to_string("Entity has invalid parent id '", groupIdStr, "'"));
                    }
                } else {
                    onNode(nullptr, node, status);
                }
            }
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

        Model::Node* MapReader::resolveParent(const ParentInfo& parentInfo) const {
            if (parentInfo.type == ParentType::Layer) {
                const Model::IdType layerId = parentInfo.id;
                const auto it = m_layers.find(layerId);
                return it != std::end(m_layers) ? it->second : nullptr;
            } else {
                const Model::IdType groupId = parentInfo.id;
                const auto it = m_groups.find(groupId);
                return it != std::end(m_groups) ? it->second : nullptr;
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
