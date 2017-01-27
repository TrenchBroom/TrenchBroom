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

#include "Model/Brush.h"
#include "Model/Group.h"
#include "Model/Layer.h"
#include "Model/NodeVisitor.h"
#include "Model/World.h"

namespace TrenchBroom {
    namespace IO {
        class NodeSerializer::BrushSerializer : public Model::NodeVisitor {
        private:
            NodeSerializer& m_serializer;
        public:
            BrushSerializer(NodeSerializer& serializer) : m_serializer(serializer) {}
            
            void doVisit(Model::World* world)   {}
            void doVisit(Model::Layer* layer)   {}
            void doVisit(Model::Group* group)   {}
            void doVisit(Model::Entity* entity) {}
            void doVisit(Model::Brush* brush)   { m_serializer.brush(brush); }
        };

        NodeSerializer::NodeSerializer() :
        m_entityNo(0),
        m_brushNo(0) {}
        
        NodeSerializer::~NodeSerializer() {}
        
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

        void NodeSerializer::defaultLayer(Model::World* world) {
            entity(world, world->attributes(), Model::EntityAttribute::EmptyList, world->defaultLayer());
        }

        void NodeSerializer::customLayer(Model::Layer* layer) {
            entity(layer, layerAttributes(layer), Model::EntityAttribute::EmptyList, layer);
        }
        
        void NodeSerializer::group(Model::Group* group, const Model::EntityAttribute::List& parentAttributes) {
            entity(group, groupAttributes(group), parentAttributes, group);
        }

        void NodeSerializer::entity(Model::Node* node, const Model::EntityAttribute::List& attributes, const Model::EntityAttribute::List& parentAttributes, Model::Node* brushParent) {
            beginEntity(node, attributes, parentAttributes);
            
            BrushSerializer brushSerializer(*this);
            brushParent->iterate(brushSerializer);
            
            endEntity(node);
        }

        void NodeSerializer::entity(Model::Node* node, const Model::EntityAttribute::List& attributes, const Model::EntityAttribute::List& parentAttributes, const Model::BrushList& entityBrushes) {
            beginEntity(node, attributes, parentAttributes);
            brushes(entityBrushes);
            endEntity(node);
        }

        void NodeSerializer::beginEntity(const Model::Node* node, const Model::EntityAttribute::List& attributes, const Model::EntityAttribute::List& extraAttributes) {
            beginEntity(node);
            entityAttributes(attributes);
            entityAttributes(extraAttributes);
        }
        
        void NodeSerializer::beginEntity(const Model::Node* node) {
            m_brushNo = 0;
            doBeginEntity(node);
        }
        
        void NodeSerializer::endEntity(Model::Node* node) {
            doEndEntity(node);
            ++m_entityNo;
        }
        
        void NodeSerializer::entityAttributes(const Model::EntityAttribute::List& attributes) {
            std::for_each(std::begin(attributes), std::end(attributes),
                          [this](const Model::EntityAttribute& attribute) { entityAttribute(attribute); });
        }

        void NodeSerializer::entityAttribute(const Model::EntityAttribute& attribute) {
            doEntityAttribute(attribute);
        }
        
        void NodeSerializer::brushes(const Model::BrushList& brushes) {
            std::for_each(std::begin(brushes), std::end(brushes),
                          [this](Model::Brush* brush) { this->brush(brush); });
        }
        
        void NodeSerializer::brush(Model::Brush* brush) {
            beginBrush(brush);
            brushFaces(brush->faces());
            endBrush(brush);
        }

        void NodeSerializer::beginBrush(const Model::Brush* brush) {
            doBeginBrush(brush);
        }
        
        void NodeSerializer::endBrush(Model::Brush* brush) {
            doEndBrush(brush);
            ++m_brushNo;
        }
        
        void NodeSerializer::brushFaces(const Model::BrushFaceList& faces) {
            std::for_each(std::begin(faces), std::end(faces),
                          [this](Model::BrushFace* face) { brushFace(face); });
        }

        void NodeSerializer::brushFace(Model::BrushFace* face) {
            doBrushFace(face);
        }
        
        class NodeSerializer::GetParentAttributes : public Model::ConstNodeVisitor {
        private:
            const LayerIds& m_layerIds;
            const GroupIds& m_groupIds;
            Model::EntityAttribute::List m_attributes;
        public:
            GetParentAttributes(const LayerIds& layerIds, const GroupIds& groupIds) :
            m_layerIds(layerIds),
            m_groupIds(groupIds) {}
            
            const Model::EntityAttribute::List& attributes() const {
                return m_attributes;
            }
        private:
            void doVisit(const Model::World* world)   {}
            void doVisit(const Model::Layer* layer)   { m_attributes.push_back(Model::EntityAttribute(Model::AttributeNames::Layer, m_layerIds.getId(layer)));}
            void doVisit(const Model::Group* group)   { m_attributes.push_back(Model::EntityAttribute(Model::AttributeNames::Group, m_groupIds.getId(group))); }
            void doVisit(const Model::Entity* entity) {}
            void doVisit(const Model::Brush* brush)   {}
        };
        
        Model::EntityAttribute::List NodeSerializer::parentAttributes(const Model::Node* node) {
            if (node == NULL)
                return Model::EntityAttribute::List(0);
            
            GetParentAttributes visitor(m_layerIds, m_groupIds);
            node->accept(visitor);
            return visitor.attributes();
        }
        
        Model::EntityAttribute::List NodeSerializer::layerAttributes(const Model::Layer* layer) {
            Model::EntityAttribute::List attrs;
            attrs.push_back(Model::EntityAttribute(Model::AttributeNames::Classname, Model::AttributeValues::LayerClassname));
            attrs.push_back(Model::EntityAttribute(Model::AttributeNames::GroupType, Model::AttributeValues::GroupTypeLayer));
            attrs.push_back(Model::EntityAttribute(Model::AttributeNames::LayerName, layer->name()));
            attrs.push_back(Model::EntityAttribute(Model::AttributeNames::LayerId, m_layerIds.getId(layer)));
            return attrs;
        }
        
        Model::EntityAttribute::List NodeSerializer::groupAttributes(const Model::Group* group) {
            Model::EntityAttribute::List attrs;
            attrs.push_back(Model::EntityAttribute(Model::AttributeNames::Classname, Model::AttributeValues::GroupClassname));
            attrs.push_back(Model::EntityAttribute(Model::AttributeNames::GroupType, Model::AttributeValues::GroupTypeGroup));
            attrs.push_back(Model::EntityAttribute(Model::AttributeNames::GroupName, group->name()));
            attrs.push_back(Model::EntityAttribute(Model::AttributeNames::GroupId, m_groupIds.getId(group)));
            return attrs;
        }

        String NodeSerializer::escapeEntityAttribute(const String& str) const {
            return StringUtils::escape(str, "\"", '\\');
        }
    }
}
