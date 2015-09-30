/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include "CollectionUtils.h"
#include "Logger.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/Entity.h"
#include "Model/EntityAttributes.h"
#include "Model/Group.h"
#include "Model/Layer.h"
#include "Model/ModelFactory.h"

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

        MapReader::MapReader(const char* begin, const char* end, Logger* logger) :
        StandardMapParser(begin, end, logger),
        m_factory(NULL),
        m_brushParent(NULL),
        m_currentNode(NULL) {}
        
        MapReader::MapReader(const String& str, Logger* logger) :
        StandardMapParser(str, logger),
        m_factory(NULL),
        m_brushParent(NULL),
        m_currentNode(NULL) {}
        
        MapReader::~MapReader() {
            VectorUtils::clearAndDelete(m_faces);
        }

        void MapReader::readEntities(Model::MapFormat::Type format, const BBox3& worldBounds) {
            m_worldBounds = worldBounds;
            parseEntities(format);
            resolveNodes();
        }
        
        void MapReader::readBrushes(Model::MapFormat::Type format, const BBox3& worldBounds) {
            m_worldBounds = worldBounds;
            parseBrushes(format);
        }
        
        void MapReader::readBrushFaces(Model::MapFormat::Type format, const BBox3& worldBounds) {
            m_worldBounds = worldBounds;
            parseBrushFaces(format);
        }

        void MapReader::onFormatSet(const Model::MapFormat::Type format) {
            m_factory = initialize(format, m_worldBounds);
            assert(m_factory != NULL);
        }
        
        void MapReader::onBeginEntity(const size_t line, const Model::EntityAttribute::List& attributes, const ExtraAttributes& extraAttributes) {
            const EntityType type = entityType(attributes);
            switch (type) {
                case EntityType_Layer:
                    createLayer(line, attributes, extraAttributes);
                    break;
                case EntityType_Group:
                    createGroup(line, attributes, extraAttributes);
                    break;
                case EntityType_Worldspawn:
                    m_brushParent = onWorldspawn(attributes, extraAttributes);
                    break;
                case EntityType_Default:
                    createEntity(line, attributes, extraAttributes);
                    break;
            }
        }
        
        void MapReader::onEndEntity(const size_t startLine, const size_t lineCount) {
            if (m_currentNode != NULL)
                setFilePosition(m_currentNode, startLine, lineCount);
            else
                onWorldspawnFilePosition(startLine, lineCount);
            m_currentNode = NULL;
            m_brushParent = NULL;
        }
        
        void MapReader::onBeginBrush(const size_t line) {
            assert(m_faces.empty());
        }
        
        void MapReader::onEndBrush(const size_t startLine, const size_t lineCount, const ExtraAttributes& extraAttributes) {
            createBrush(startLine, lineCount, extraAttributes);
        }
        
        void MapReader::onBrushFace(const size_t line, const Vec3& point1, const Vec3& point2, const Vec3& point3, const Model::BrushFaceAttributes& attribs, const Vec3& texAxisX, const Vec3& texAxisY) {
            Model::BrushFace* face = m_factory->createFace(point1, point2, point3, attribs, texAxisX, texAxisY);
            onBrushFace(face);
        }

        void MapReader::createLayer(const size_t line, const Model::EntityAttribute::List& attributes, const ExtraAttributes& extraAttributes) {
            const String& name = findAttribute(attributes, Model::AttributeNames::LayerName);
            if (StringUtils::isBlank(name)) {
                if (logger() != NULL)
                    logger()->warn("Skipping layer entity at line %u: missing name", static_cast<unsigned int>(line));
                return;
            }
            
            const String& idStr = findAttribute(attributes, Model::AttributeNames::LayerId);
            if (StringUtils::isBlank(idStr)) {
                if (logger() != NULL)
                    logger()->warn("Skipping layer entity at line %u: missing id", static_cast<unsigned int>(line));
                return;
            }
            
            const long rawId = std::atol(idStr.c_str());
            if (rawId <= 0) {
                if (logger() != NULL)
                    logger()->warn("Skipping layer entity at line %u: invalid id", static_cast<unsigned int>(line), name.c_str());
                return;
            }
            
            const Model::IdType layerId = static_cast<Model::IdType>(rawId);
            if (m_layers.count(layerId) > 0) {
                if (logger() != NULL)
                    logger()->warn("Skipping layer entity at line %u: a layer with id '%s' already exists", static_cast<unsigned int>(line), idStr.c_str());
                return;
            }
            
            Model::Layer* layer = m_factory->createLayer(name, m_worldBounds);
            setExtraAttributes(layer, extraAttributes);
            m_layers.insert(std::make_pair(layerId, layer));
            
            onLayer(layer);
            
            m_currentNode = layer;
            m_brushParent = layer;
        }
        
        void MapReader::createGroup(const size_t line, const Model::EntityAttribute::List& attributes, const ExtraAttributes& extraAttributes) {
            const String& name = findAttribute(attributes, Model::AttributeNames::GroupName);
            if (StringUtils::isBlank(name)) {
                if (logger() != NULL)
                    logger()->warn("Skipping group entity at line %u: missing name", static_cast<unsigned int>(line));
                return;
            }
            
            const String& idStr = findAttribute(attributes, Model::AttributeNames::GroupId);
            if (StringUtils::isBlank(idStr)) {
                if (logger() != NULL)
                    logger()->warn("Skipping group entity at line %u: missing id", static_cast<unsigned int>(line));
                return;
            }
            
            const long rawId = std::atol(idStr.c_str());
            if (rawId <= 0) {
                if (logger() != NULL)
                    logger()->warn("Skipping group entity at line %u: invalid id", static_cast<unsigned int>(line), name.c_str());
                return;
            }
            
            const Model::IdType groupId = static_cast<Model::IdType>(rawId);
            if (m_groups.count(groupId) > 0) {
                if (logger() != NULL)
                    logger()->warn("Skipping group entity at line %u: a group with id '%s' already exists", static_cast<unsigned int>(line), idStr.c_str());
                return;
            }
            
            Model::Group* group = m_factory->createGroup(name);
            setExtraAttributes(group, extraAttributes);
            
            storeNode(group, attributes);
            m_groups.insert(std::make_pair(groupId, group));
            
            m_currentNode = group;
            m_brushParent = group;
        }

        void MapReader::createEntity(const size_t line, const Model::EntityAttribute::List& attributes, const ExtraAttributes& extraAttributes) {
            Model::Entity* entity = m_factory->createEntity();
            entity->setAttributes(attributes);
            setExtraAttributes(entity, extraAttributes);

            const ParentInfo::Type parentType = storeNode(entity, attributes);
            stripParentAttributes(entity, parentType);
            
            m_currentNode = entity;
            m_brushParent = entity;
        }

        void MapReader::createBrush(const size_t startLine, const size_t lineCount, const ExtraAttributes& extraAttributes) {
            try {
                // sort the faces by the weight of their plane normals like QBSP does
                Model::BrushFace::sortFaces(m_faces);
                
                Model::Brush* brush = m_factory->createBrush(m_worldBounds, m_faces);
                setFilePosition(brush, startLine, lineCount);
                setExtraAttributes(brush, extraAttributes);
                
                onBrush(m_brushParent, brush);
                m_faces.clear();
            } catch (GeometryException& e) {
                if (logger() != NULL)
                    logger()->error("Error parsing brush at line %u: %s", startLine, e.what());
                VectorUtils::clearAndDelete(m_faces);
            }

        }

        MapReader::ParentInfo::Type MapReader::storeNode(Model::Node* node, const Model::EntityAttribute::List& attributes) {
            const String& layerIdStr = findAttribute(attributes, Model::AttributeNames::Layer);
            if (!StringUtils::isBlank(layerIdStr)) {
                const long rawId = std::atol(layerIdStr.c_str());
                if (rawId > 0) {
                    const Model::IdType layerId = static_cast<Model::IdType>(rawId);
                    Model::Layer* layer = MapUtils::find(m_layers, layerId, static_cast<Model::Layer*>(NULL));
                    if (layer != NULL)
                        onNode(layer, node);
                    else
                        m_unresolvedNodes.push_back(std::make_pair(node, ParentInfo::layer(layerId)));
                    return ParentInfo::Type_Layer;
                }
                
                if (logger() != NULL)
                    logger()->warn("Entity at line %u has invalid parent id", static_cast<unsigned int>(node->lineNumber()), layerIdStr.c_str());
            } else {
                const String& groupIdStr = findAttribute(attributes, Model::AttributeNames::Group);
                if (!StringUtils::isBlank(groupIdStr)) {
                    const long rawId = std::atol(groupIdStr.c_str());
                    if (rawId > 0) {
                        const Model::IdType groupId = static_cast<Model::IdType>(rawId);
                        Model::Group* group = MapUtils::find(m_groups, groupId, static_cast<Model::Group*>(NULL));
                        if (group != NULL)
                            onNode(group, node);
                        else
                            m_unresolvedNodes.push_back(std::make_pair(node, ParentInfo::group(groupId)));
                        return ParentInfo::Type_Group;
                    }
                    
                    if (logger() != NULL)
                        logger()->warn("Entity at line %u has invalid parent id", static_cast<unsigned int>(node->lineNumber()), groupIdStr.c_str());
                }
            }
            
            onNode(NULL, node);
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

        void MapReader::resolveNodes() {
            NodeParentList::const_iterator it, end;
            for (it = m_unresolvedNodes.begin(), end = m_unresolvedNodes.end(); it != end; ++it) {
                Model::Node* node = it->first;
                const ParentInfo& info = it->second;
                Model::Node* parent = resolveParent(info);
                if (parent == NULL)
                    onUnresolvedNode(info, node);
                else
                    onNode(parent, node);
            }
        }

        Model::Node* MapReader::resolveParent(const ParentInfo& parentInfo) const {
            if (parentInfo.layer()) {
                const Model::IdType layerId = parentInfo.id();
                return MapUtils::find(m_layers, layerId, static_cast<Model::Layer*>(NULL));
            }
            const Model::IdType groupId = parentInfo.id();
            return MapUtils::find(m_groups, groupId, static_cast<Model::Group*>(NULL));
        }

        MapReader::EntityType MapReader::entityType(const Model::EntityAttribute::List& attributes) const {
            const String& classname = findAttribute(attributes, Model::AttributeNames::Classname);
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

        void MapReader::setExtraAttributes(Model::Node* node, const ExtraAttributes& extraAttributes) {
            ExtraAttributes::const_iterator it;
            it = extraAttributes.find("hideIssues");
            if (it != extraAttributes.end()) {
                const ExtraAttribute& attribute = it->second;
                attribute.assertType(ExtraAttribute::Type_Integer);
                // const Model::IssueType mask = attribute.intValue<Model::IssueType>();
                // object->setHiddenIssues(mask);
            }
        }

        void MapReader::onBrushFace(Model::BrushFace* face) {
            m_faces.push_back(face);
        }
    }
}
