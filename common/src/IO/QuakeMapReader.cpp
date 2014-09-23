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
        
        void QuakeMapReader::onBeginEntity(const Model::EntityAttribute::List& attributes, const ExtraAttributes& extraAttributes) {
            const EntityType type = entityType(attributes);
            switch (type) {
                case EntityType_Layer:
                    createLayer(attributes, extraAttributes);
                    break;
                case EntityType_Group:
                    createGroup(attributes, extraAttributes);
                    break;
                case EntityType_Worldspawn:
                    createWorldspawn(attributes, extraAttributes);
                    break;
                case EntityType_Default:
                    createEntity(attributes, extraAttributes);
                    break;
            }
        }
        
        void QuakeMapReader::onEndEntity(const size_t startLine, const size_t lineCount) {
            assert(m_currentNode != NULL);
            setFilePosition(m_currentNode, startLine, lineCount);
            m_parent = m_world->defaultLayer();
        }
        
        void QuakeMapReader::onBeginBrush() {
            assert(m_faces.empty());
        }
        
        void QuakeMapReader::onEndBrush(const size_t startLine, const size_t lineCount, const ExtraAttributes& extraAttributes) {
            createBrush(startLine, lineCount, extraAttributes);
        }
        
        void QuakeMapReader::onBrushFace(const Vec3& point1, const Vec3& point2, const Vec3& point3, const Model::BrushFaceAttribs& attribs, const Vec3& texAxisX, const Vec3& texAxisY) {
            Model::BrushFace* face = m_world->createFace(point1, point2, point3, attribs.textureName(), texAxisX, texAxisY);
            face->setAttribs(attribs);
            m_faces.push_back(face);
        }

        void QuakeMapReader::createLayer(const Model::EntityAttribute::List& attributes, const ExtraAttributes& extraAttributes) {
            const String& name = findAttribute(attributes, Model::AttributeNames::LayerName);
            if (StringUtils::isBlank(name)) {
                // show some error
            } else {
                m_currentNode = m_world->createLayer(name);
                m_world->addChild(m_currentNode);
                m_parent = m_currentNode;
            }
        }
        
        void QuakeMapReader::createGroup(const Model::EntityAttribute::List& attributes, const ExtraAttributes& extraAttributes) {
            const String& name = findAttribute(attributes, Model::AttributeNames::GroupName);
            if (StringUtils::isBlank(name)) {
                // show some error
            } else {
                m_currentNode = m_world->createGroup(name);
                addChild(m_currentNode);
                m_parent = m_currentNode;
            }
        }
        
        void QuakeMapReader::createWorldspawn(const Model::EntityAttribute::List& attributes, const ExtraAttributes& extraAttributes) {
            m_world->setAttributes(attributes);
            setExtraAttributes(m_world, extraAttributes);
            m_currentNode = m_world;
        }

        void QuakeMapReader::createEntity(const Model::EntityAttribute::List& attributes, const ExtraAttributes& extraAttributes) {
            Model::Entity* entity = m_world->createEntity();
            entity->setAttributes(attributes);
            setExtraAttributes(entity, extraAttributes);
            m_currentNode = entity;
            addChild(m_currentNode);
            m_parent = m_currentNode;
        }

        void QuakeMapReader::createBrush(const size_t startLine, const size_t lineCount, const ExtraAttributes& extraAttributes) {
            try {
                // sort the faces by the weight of their plane normals like QBSP does
                Model::BrushFace::sortFaces(m_faces);
                
                Model::Brush* brush = m_world->createBrush(m_worldBounds, m_faces);
                setFilePosition(brush, startLine, lineCount);
                setExtraAttributes(brush, extraAttributes);
                addChild(brush);
                m_faces.clear();
            } catch (GeometryException& e) {
                if (logger() != NULL)
                    logger()->error("Error parsing brush at line %u: %s", startLine, e.what());
            }

        }

        void QuakeMapReader::addChild(Model::Node* node) {
            m_parent->addChild(node);
        }

        QuakeMapReader::EntityType QuakeMapReader::entityType(const Model::EntityAttribute::List& attributes) const {
            const String& classname = findAttribute(attributes, Model::AttributeNames::Classname);
            if (classname == Model::AttributeValues::LayerClassname) {
                const String& groupType = findAttribute(attributes, Model::AttributeNames::GroupType);
                if (groupType == Model::AttributeValues::GroupTypeLayer)
                    return EntityType_Layer;
            } else if (classname == Model::AttributeValues::GroupClassname) {
                const String& groupType = findAttribute(attributes, Model::AttributeNames::GroupType);
                if (groupType == Model::AttributeValues::GroupTypeGroup)
                    return EntityType_Group;
            } else if (classname == Model::AttributeValues::WorldspawnClassname) {
                return EntityType_Worldspawn;
            }
            
            return EntityType_Default;
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
