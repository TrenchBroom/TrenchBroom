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
        MapReader::ParentInfo MapReader::ParentInfo::layer(const String& name) {
            return ParentInfo(Type_Layer, name);
        }
        
        MapReader::ParentInfo MapReader::ParentInfo::group(const String& name) {
            return ParentInfo(Type_Group, name);
        }

        MapReader::ParentInfo::ParentInfo(Type type, const String& name) :
        m_type(type),
        m_name(name) {}

        bool MapReader::ParentInfo::layer() const {
            return m_type == Type_Layer;
        }
        
        bool MapReader::ParentInfo::group() const {
            return m_type == Type_Group;
        }
        
        const String& MapReader::ParentInfo::name() const {
            return m_name;
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
            parseEntities(format);
        }

        void MapReader::onFormatSet(const Model::MapFormat::Type format) {
            m_factory = initialize(format);
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
            Model::BrushFace* face = m_factory->createFace(point1, point2, point3, attribs.textureName(), texAxisX, texAxisY);
            face->setAttribs(attribs);
            onBrushFace(face);
        }

        void MapReader::createLayer(const size_t line, const Model::EntityAttribute::List& attributes, const ExtraAttributes& extraAttributes) {
            const String& name = findAttribute(attributes, Model::AttributeNames::LayerName);
            if (StringUtils::isBlank(name)) {
                if (logger() != NULL)
                    logger()->warn("Skipping layer entity at line %u: name is blank", static_cast<unsigned int>(line));
            } else if (m_layers.count(name) > 0) {
                if (logger() != NULL)
                    logger()->warn("Skipping layer entity at line %u: a layer with name '%s' already exists", static_cast<unsigned int>(line), name.c_str());
            } else {
                Model::Layer* layer = m_factory->createLayer(name);
                setExtraAttributes(layer, extraAttributes);
                m_layers.insert(std::make_pair(name, layer));

                onLayer(layer);

                m_currentNode = layer;
                m_brushParent = layer;
            }
        }
        
        void MapReader::createGroup(const size_t line, const Model::EntityAttribute::List& attributes, const ExtraAttributes& extraAttributes) {
            const String& name = findAttribute(attributes, Model::AttributeNames::GroupName);
            if (StringUtils::isBlank(name)) {
                if (logger() != NULL)
                    logger()->warn("Skipping group entity at line %u: name is blank", static_cast<unsigned int>(line));
            } else if (m_layers.count(name) > 0) {
                if (logger() != NULL)
                    logger()->warn("Skipping group entity at line %u: a group with name '%s' already exists", static_cast<unsigned int>(line), name.c_str());
            } else {
                Model::Group* group = m_factory->createGroup(name);
                setExtraAttributes(group, extraAttributes);

                storeNode(group, attributes);
                m_groups.insert(std::make_pair(name, group));
                
                m_currentNode = group;
                m_brushParent = group;
            }
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
            }

        }

        MapReader::ParentInfo::Type MapReader::storeNode(Model::Node* node, const Model::EntityAttribute::List& attributes) {
            const String& groupName = findAttribute(attributes, Model::AttributeNames::Group);
            if (!groupName.empty()) {
                Model::Group* group = MapUtils::find(m_groups, groupName, static_cast<Model::Group*>(NULL));
                if (group != NULL)
                    onNode(group, node);
                else
                    m_unresolvedNodes.push_back(std::make_pair(node, ParentInfo::group(groupName)));
                return ParentInfo::Type_Group;
            }

            const String& layerName = findAttribute(attributes, Model::AttributeNames::Layer);
            if (!layerName.empty()) {
                Model::Layer* layer = MapUtils::find(m_layers, layerName, static_cast<Model::Layer*>(NULL));
                if (layer != NULL)
                    onNode(layer, node);
                else
                    m_unresolvedNodes.push_back(std::make_pair(node, ParentInfo::layer(layerName)));
                return ParentInfo::Type_Layer;
            }
            
            onNode(NULL, node);
            return ParentInfo::Type_None;
        }
        
        void MapReader::stripParentAttributes(Model::Attributable* attributable, const ParentInfo::Type parentType) {
            switch (parentType) {
                case ParentInfo::Type_Layer:
                    attributable->removeAttribute(Model::AttributeNames::Layer);
                    break;
                case ParentInfo::Type_Group:
                    attributable->removeAttribute(Model::AttributeNames::Group);
                    break;
                case ParentInfo::Type_None:
                    break;
                DEFAULT_SWITCH();
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
                const String& layerName = parentInfo.name();
                return MapUtils::find(m_layers, layerName, static_cast<Model::Layer*>(NULL));
            }
            const String& groupName = parentInfo.name();
            return MapUtils::find(m_groups, groupName, static_cast<Model::Group*>(NULL));
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
