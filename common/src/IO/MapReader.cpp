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
#include "Model/BrushNode.h"
#include "Model/BrushFace.h"
#include "Model/Entity.h"
#include "Model/EntityAttributes.h"
#include "Model/Group.h"
#include "Model/LayerNode.h"
#include "Model/ModelFactory.h"

#include <kdl/map_utils.h>
#include <kdl/string_format.h>
#include <kdl/string_utils.h>
#include <kdl/vector_utils.h>

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

        MapReader::MapReader(const char* begin, const char* end) :
        StandardMapParser(begin, end),
        m_factory(nullptr),
        m_brushParent(nullptr),
        m_currentNode(nullptr) {}

        MapReader::MapReader(const std::string& str) :
        StandardMapParser(str),
        m_factory(nullptr),
        m_brushParent(nullptr),
        m_currentNode(nullptr) {}

        MapReader::~MapReader() {
            kdl::vec_clear_and_delete(m_faces);
        }

        void MapReader::readEntities(Model::MapFormat format, const vm::bbox3& worldBounds, ParserStatus& status) {
            m_worldBounds = worldBounds;
            parseEntities(format, status);
            resolveNodes(status);
        }

        void MapReader::readBrushes(Model::MapFormat format, const vm::bbox3& worldBounds, ParserStatus& status) {
            m_worldBounds = worldBounds;
            parseBrushes(format, status);
        }

        void MapReader::readBrushFaces(Model::MapFormat format, const vm::bbox3& worldBounds, ParserStatus& status) {
            m_worldBounds = worldBounds;
            parseBrushFaces(format, status);
        }

        void MapReader::onFormatSet(const Model::MapFormat format) {
            m_factory = &initialize(format);
        }

        void MapReader::onBeginEntity(const size_t line, const std::vector<Model::EntityAttribute>& attributes, const ExtraAttributes& extraAttributes, ParserStatus& status) {
            const EntityType type = entityType(attributes);
            switch (type) {
                case EntityType_Layer:
                    createLayer(line, attributes, extraAttributes, status);
                    break;
                case EntityType_Group:
                    createGroup(line, attributes, extraAttributes, status);
                    break;
                case EntityType_Worldspawn:
                    m_brushParent = onWorldspawn(attributes, extraAttributes, status);
                    break;
                case EntityType_Default:
                    createEntity(line, attributes, extraAttributes, status);
                    break;
            }
        }

        void MapReader::onEndEntity(const size_t startLine, const size_t lineCount, ParserStatus& status) {
            if (m_currentNode != nullptr)
                setFilePosition(m_currentNode, startLine, lineCount);
            else
                onWorldspawnFilePosition(startLine, lineCount, status);
            m_currentNode = nullptr;
            m_brushParent = nullptr;
        }

        void MapReader::onBeginBrush(const size_t /* line */, ParserStatus& /* status */) {
            assert(m_faces.empty());
        }

        void MapReader::onEndBrush(const size_t startLine, const size_t lineCount, const ExtraAttributes& extraAttributes, ParserStatus& status) {
            createBrush(startLine, lineCount, extraAttributes, status);
        }

        void MapReader::onBrushFace(const size_t line, const vm::vec3& point1, const vm::vec3& point2, const vm::vec3& point3, const Model::BrushFaceAttributes& attribs, const vm::vec3& texAxisX, const vm::vec3& texAxisY, ParserStatus& status) {
            Model::BrushFace* face = m_factory->createFace(point1, point2, point3, attribs, texAxisX, texAxisY);
            face->setFilePosition(line, 1);
            onBrushFace(face, status);
        }

        void MapReader::createLayer(const size_t line, const std::vector<Model::EntityAttribute>& attributes, const ExtraAttributes& extraAttributes, ParserStatus& status) {
            const std::string& name = findAttribute(attributes, Model::AttributeNames::LayerName);
            if (kdl::str_is_blank(name)) {
                status.error(line, "Skipping layer entity: missing name");
                return;
            }

            const std::string& idStr = findAttribute(attributes, Model::AttributeNames::LayerId);
            if (kdl::str_is_blank(idStr)) {
                status.error(line, "Skipping layer entity: missing id");
                return;
            }

            const long rawId = std::atol(idStr.c_str());
            if (rawId <= 0) {
                status.error(line, kdl::str_to_string("Skipping layer entity: '", idStr, "' is not a valid id"));
                return;
            }

            const Model::IdType layerId = static_cast<Model::IdType>(rawId);
            if (m_layers.count(layerId) > 0) {
                status.error(line, kdl::str_to_string("Skipping layer entity: layer with id '", idStr, "' already exists"));
                return;
            }

            Model::LayerNode* layer = m_factory->createLayer(name);
            setExtraAttributes(layer, extraAttributes);
            m_layers.insert(std::make_pair(layerId, layer));

            onLayer(layer, status);

            m_currentNode = layer;
            m_brushParent = layer;
        }

        void MapReader::createGroup(const size_t line, const std::vector<Model::EntityAttribute>& attributes, const ExtraAttributes& extraAttributes, ParserStatus& status) {
            const std::string& name = findAttribute(attributes, Model::AttributeNames::GroupName);
            if (kdl::str_is_blank(name)) {
                status.error(line, "Skipping group entity: missing name");
                return;
            }

            const std::string& idStr = findAttribute(attributes, Model::AttributeNames::GroupId);
            if (kdl::str_is_blank(idStr)) {
                status.error(line, "Skipping group entity: missing id");
                return;
            }

            const long rawId = std::atol(idStr.c_str());
            if (rawId <= 0) {
                status.error(line, kdl::str_to_string("Skipping group entity: '", idStr, "' is not a valid id"));
                return;
            }

            const Model::IdType groupId = static_cast<Model::IdType>(rawId);
            if (m_groups.count(groupId) > 0) {
                status.error(line, kdl::str_to_string("Skipping group entity: group with id '", idStr, "' already exists"));
                return;
            }

            Model::Group* group = m_factory->createGroup(name);
            setExtraAttributes(group, extraAttributes);

            storeNode(group, attributes, status);
            m_groups.insert(std::make_pair(groupId, group));

            m_currentNode = group;
            m_brushParent = group;
        }

        void MapReader::createEntity(const size_t /* line */, const std::vector<Model::EntityAttribute>& attributes, const ExtraAttributes& extraAttributes, ParserStatus& status) {
            Model::Entity* entity = m_factory->createEntity();
            entity->setAttributes(attributes);
            setExtraAttributes(entity, extraAttributes);

            const ParentInfo::Type parentType = storeNode(entity, attributes, status);
            stripParentAttributes(entity, parentType);

            m_currentNode = entity;
            m_brushParent = entity;
        }

        void MapReader::createBrush(const size_t startLine, const size_t lineCount, const ExtraAttributes& extraAttributes, ParserStatus& status) {
            try {
                Model::BrushNode* brush = m_factory->createBrush(m_worldBounds, m_faces);
                setFilePosition(brush, startLine, lineCount);
                setExtraAttributes(brush, extraAttributes);

                onBrush(m_brushParent, brush, status);
                m_faces.clear();
            } catch (GeometryException& e) {
                status.error(startLine, kdl::str_to_string("Skipping brush: ", e.what()));
                m_faces.clear(); // the faces will have been deleted by the brush's constructor
            }

        }

        MapReader::ParentInfo::Type MapReader::storeNode(Model::Node* node, const std::vector<Model::EntityAttribute>& attributes, ParserStatus& status) {
            const std::string& layerIdStr = findAttribute(attributes, Model::AttributeNames::Layer);
            if (!kdl::str_is_blank(layerIdStr)) {
                const long rawId = std::atol(layerIdStr.c_str());
                if (rawId > 0) {
                    const Model::IdType layerId = static_cast<Model::IdType>(rawId);
                    Model::LayerNode* layer = kdl::map_find_or_default(m_layers, layerId,
                        static_cast<Model::LayerNode*>(nullptr));
                    if (layer != nullptr)
                        onNode(layer, node, status);
                    else
                        m_unresolvedNodes.push_back(std::make_pair(node, ParentInfo::layer(layerId)));
                    return ParentInfo::Type_Layer;
                }

                status.warn(node->lineNumber(), kdl::str_to_string("Entity has invalid parent id '", layerIdStr, "'"));
            } else {
                const std::string& groupIdStr = findAttribute(attributes, Model::AttributeNames::Group);
                if (!kdl::str_is_blank(groupIdStr)) {
                    const long rawId = std::atol(groupIdStr.c_str());
                    if (rawId > 0) {
                        const Model::IdType groupId = static_cast<Model::IdType>(rawId);
                        Model::Group* group = kdl::map_find_or_default(m_groups, groupId,
                            static_cast<Model::Group*>(nullptr));
                        if (group != nullptr)
                            onNode(group, node, status);
                        else
                            m_unresolvedNodes.push_back(std::make_pair(node, ParentInfo::group(groupId)));
                        return ParentInfo::Type_Group;
                    }

                    status.warn(node->lineNumber(), kdl::str_to_string("Entity has invalid parent id '", groupIdStr, "'"));
                }
            }

            onNode(nullptr, node, status);
            return ParentInfo::Type_None;
        }

        void MapReader::stripParentAttributes(Model::AttributableNode* attributable, const ParentInfo::Type parentType) {
            switch (parentType) {
                case ParentInfo::Type_Layer:
                    attributable->removeAttribute(Model::AttributeNames::Layer);
                    break;
                case ParentInfo::Type_Group:
                    attributable->removeAttribute(Model::AttributeNames::Group);
                    break;
                case ParentInfo::Type_None:
                    break;
                switchDefault();
            }
        }

        void MapReader::resolveNodes(ParserStatus& status) {
            for (const auto& entry : m_unresolvedNodes) {
                Model::Node* node = entry.first;
                const ParentInfo& info = entry.second;

                Model::Node* parent = resolveParent(info);
                if (parent == nullptr)
                    onUnresolvedNode(info, node, status);
                else
                    onNode(parent, node, status);
            }
        }

        Model::Node* MapReader::resolveParent(const ParentInfo& parentInfo) const {
            if (parentInfo.layer()) {
                const Model::IdType layerId = parentInfo.id();
                return kdl::map_find_or_default(m_layers, layerId, static_cast<Model::LayerNode*>(nullptr));
            }
            const Model::IdType groupId = parentInfo.id();
            return kdl::map_find_or_default(m_groups, groupId, static_cast<Model::Group*>(nullptr));
        }

        MapReader::EntityType MapReader::entityType(const std::vector<Model::EntityAttribute>& attributes) const {
            const std::string& classname = findAttribute(attributes, Model::AttributeNames::Classname);
            if (isLayer(classname, attributes))
                return EntityType_Layer;
            if (isGroup(classname, attributes))
                return EntityType_Group;
            if (isWorldspawn(classname, attributes))
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

        void MapReader::onBrushFace(Model::BrushFace* face, ParserStatus& /* status */) {
            m_faces.push_back(face);
        }
    }
}
