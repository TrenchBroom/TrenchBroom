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

#include "QuakeMapReader.h"

#include "CollectionUtils.h"
#include "Logger.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/Entity.h"
#include "Model/EntityAttributes.h"
#include "Model/Group.h"
#include "Model/Layer.h"
#include "Model/World.h"

namespace TrenchBroom {
    namespace IO {
        QuakeMapReader::QuakeMapReader(const char* begin, const char* end, Model::BrushContentTypeBuilder* brushContentTypeBuilder, Logger* logger) :
        QuakeMapParser(begin, end, logger),
        m_brushContentTypeBuilder(brushContentTypeBuilder),
        m_world(NULL),
        m_parent(NULL),
        m_currentNode(NULL) {}
        
        QuakeMapReader::QuakeMapReader(const String& str, Model::BrushContentTypeBuilder* brushContentTypeBuilder, Logger* logger) :
        QuakeMapParser(str, logger),
        m_brushContentTypeBuilder(brushContentTypeBuilder),
        m_world(NULL),
        m_parent(NULL),
        m_currentNode(NULL) {}
        
        QuakeMapReader::~QuakeMapReader() {
            VectorUtils::clearAndDelete(m_faces);
        }

        Model::World* QuakeMapReader::read(const BBox3& worldBounds) {
            m_worldBounds = worldBounds;
            doParse();
            return m_world;
        }

        void QuakeMapReader::onFormatDetected(const Model::MapFormat::Type format) {
            assert(m_world == NULL);
            m_world = new Model::World(format, m_brushContentTypeBuilder);
            m_parent = m_world->defaultLayer();
        }
        
        void QuakeMapReader::onBeginEntity(const size_t line, const Model::EntityAttribute::List& attributes, const ExtraAttributes& extraAttributes) {
            const EntityType type = entityType(attributes);
            switch (type) {
                case EntityType_Layer:
                    createLayer(line, attributes, extraAttributes);
                    break;
                case EntityType_Group:
                    createGroup(line, attributes, extraAttributes);
                    break;
                case EntityType_Worldspawn:
                    createWorldspawn(line, attributes, extraAttributes);
                    break;
                case EntityType_Default:
                    createEntity(line, attributes, extraAttributes);
                    break;
            }
        }
        
        void QuakeMapReader::onEndEntity(const size_t startLine, const size_t lineCount) {
            assert(m_currentNode != NULL);
            setFilePosition(m_currentNode, startLine, lineCount);
            m_currentNode = NULL;
            m_parent = m_world->defaultLayer();
        }
        
        void QuakeMapReader::onBeginBrush(const size_t line) {
            assert(m_faces.empty());
        }
        
        void QuakeMapReader::onEndBrush(const size_t startLine, const size_t lineCount, const ExtraAttributes& extraAttributes) {
            createBrush(startLine, lineCount, extraAttributes);
        }
        
        void QuakeMapReader::onBrushFace(const size_t line, const Vec3& point1, const Vec3& point2, const Vec3& point3, const Model::BrushFaceAttribs& attribs, const Vec3& texAxisX, const Vec3& texAxisY) {
            Model::BrushFace* face = m_world->createFace(point1, point2, point3, attribs.textureName(), texAxisX, texAxisY);
            face->setAttribs(attribs);
            m_faces.push_back(face);
        }

        void QuakeMapReader::createLayer(const size_t line, const Model::EntityAttribute::List& attributes, const ExtraAttributes& extraAttributes) {
            const String& name = findAttribute(attributes, Model::AttributeNames::LayerName);
            if (StringUtils::isBlank(name)) {
                if (logger() != NULL)
                    logger()->warn("Skipping layer entity at line %u: name is blank", static_cast<unsigned int>(line));
            } else if (m_layers.count(name) > 0) {
                if (logger() != NULL)
                    logger()->warn("Skipping layer entity at line %u: a layer with name '%s' already exists", static_cast<unsigned int>(line), name.c_str());
            } else {
                Model::Layer* layer = m_world->createLayer(name);
                setExtraAttributes(layer, extraAttributes);
                m_layers.insert(std::make_pair(name, layer));

                m_world->addChild(layer);
                m_currentNode = layer;
                m_parent = layer;
            }
        }
        
        void QuakeMapReader::createGroup(const size_t line, const Model::EntityAttribute::List& attributes, const ExtraAttributes& extraAttributes) {
            const String& name = findAttribute(attributes, Model::AttributeNames::GroupName);
            if (StringUtils::isBlank(name)) {
                if (logger() != NULL)
                    logger()->warn("Skipping group entity at line %u: name is blank", static_cast<unsigned int>(line));
            } else if (m_layers.count(name) > 0) {
                if (logger() != NULL)
                    logger()->warn("Skipping group entity at line %u: a group with name '%s' already exists", static_cast<unsigned int>(line), name.c_str());
            } else {
                Model::Group* group = m_world->createGroup(name);
                setExtraAttributes(group, extraAttributes);
                m_groups.insert(std::make_pair(name, group));
                
                Model::Node* parent = findParentForEntity(line, attributes);
                parent->addChild(group);
                
                m_currentNode = group;
                m_parent = group;
            }
        }
        
        void QuakeMapReader::createWorldspawn(const size_t line, const Model::EntityAttribute::List& attributes, const ExtraAttributes& extraAttributes) {
            m_world->setAttributes(attributes);
            setExtraAttributes(m_world, extraAttributes);
            m_currentNode = m_world;
        }

        void QuakeMapReader::createEntity(const size_t line, const Model::EntityAttribute::List& attributes, const ExtraAttributes& extraAttributes) {
            Model::Entity* entity = m_world->createEntity();
            entity->setAttributes(attributes);
            setExtraAttributes(entity, extraAttributes);
            
            Model::Node* parent = findParentForEntity(line, attributes);
            parent->addChild(entity);

            m_currentNode = entity;
            m_parent = entity;
        }

        Model::Node* QuakeMapReader::findParentForEntity(const size_t line, const Model::EntityAttribute::List& attributes) const {
            const String& groupName = findAttribute(attributes, Model::AttributeNames::Group);
            if (!groupName.empty()) {
                Model::Group* group = MapUtils::find(m_groups, groupName, static_cast<Model::Group*>(NULL));
                if (group != NULL)
                    return group;
                if (logger() != NULL)
                    logger()->warn("Entity at line %u references missing group '%s'", static_cast<unsigned int>(line), groupName.c_str());
                return m_world->defaultLayer();
            }

            const String& layerName = findAttribute(attributes, Model::AttributeNames::Layer);
            if (!layerName.empty()) {
                Model::Layer* layer = MapUtils::find(m_layers, layerName, static_cast<Model::Layer*>(NULL));
                if (layer != NULL)
                    return layer;
                if (logger() != NULL)
                    logger()->warn("Entity at line %u references missing layer '%s', adding it to the default layer instead", static_cast<unsigned int>(line), layerName.c_str());
                return m_world->defaultLayer();
            }
            
            return m_world->defaultLayer();
        }

        void QuakeMapReader::createBrush(const size_t startLine, const size_t lineCount, const ExtraAttributes& extraAttributes) {
            try {
                // sort the faces by the weight of their plane normals like QBSP does
                Model::BrushFace::sortFaces(m_faces);
                
                Model::Brush* brush = m_world->createBrush(m_worldBounds, m_faces);
                setFilePosition(brush, startLine, lineCount);
                setExtraAttributes(brush, extraAttributes);
                
                m_parent->addChild(brush);
                m_faces.clear();
            } catch (GeometryException& e) {
                if (logger() != NULL)
                    logger()->error("Error parsing brush at line %u: %s", startLine, e.what());
            }

        }

        QuakeMapReader::EntityType QuakeMapReader::entityType(const Model::EntityAttribute::List& attributes) const {
            const String& classname = findAttribute(attributes, Model::AttributeNames::Classname);
            if (isLayer(classname, attributes))
                return EntityType_Layer;
            if (isGroup(classname, attributes))
                return EntityType_Group;
            if (isWorldspawn(classname, attributes))
                return EntityType_Worldspawn;
            return EntityType_Default;
        }

        bool QuakeMapReader::isLayer(const String& classname, const Model::EntityAttribute::List& attributes) const {
            if (classname != Model::AttributeValues::LayerClassname)
                return false;
            const String& groupType = findAttribute(attributes, Model::AttributeNames::GroupType);
            return groupType == Model::AttributeValues::GroupTypeLayer;
        }
        
        bool QuakeMapReader::isGroup(const String& classname, const Model::EntityAttribute::List& attributes) const {
            if (classname != Model::AttributeValues::GroupClassname)
                return false;
            const String& groupType = findAttribute(attributes, Model::AttributeNames::GroupType);
            return groupType == Model::AttributeValues::GroupTypeGroup;
        }
        
        bool QuakeMapReader::isWorldspawn(const String& classname, const Model::EntityAttribute::List& attributes) const {
            return classname == Model::AttributeValues::WorldspawnClassname;
        }

        const String& QuakeMapReader::findAttribute(const Model::EntityAttribute::List& attributes, const String& name, const String& defaultValue) const {
            Model::EntityAttribute::List::const_iterator it, end;
            for (it = attributes.begin(), end = attributes.end(); it != end; ++it) {
                if (name == it->name())
                    return it->value();
            }
            return defaultValue;
        }

        void QuakeMapReader::setFilePosition(Model::Node* node, const size_t startLine, const size_t lineCount) {
            node->setFilePosition(startLine, lineCount);
        }

        void QuakeMapReader::setExtraAttributes(Model::Node* node, const ExtraAttributes& extraAttributes) {
            ExtraAttributes::const_iterator it;
            it = extraAttributes.find("hideIssues");
            if (it != extraAttributes.end()) {
                const ExtraAttribute& attribute = it->second;
                attribute.assertType(ExtraAttribute::Type_Integer);
                // const Model::IssueType mask = attribute.intValue<Model::IssueType>();
                // object->setHiddenIssues(mask);
            }
        }
    }
}
