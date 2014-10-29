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

#include "NodeSerializer.h"

#include "Model/Brush.h"
#include "Model/Group.h"
#include "Model/Layer.h"

namespace TrenchBroom {
    namespace IO {
        NodeSerializer::~NodeSerializer() {}
        
        void NodeSerializer::layer(Model::Layer* layer) {
            entity(layer, layerAttributes(layer), Model::EntityAttribute::EmptyList);
        }
        
        void NodeSerializer::group(Model::Group* group, const Model::EntityAttribute::List& parentAttributes) {
            entity(group, groupAttributes(group), parentAttributes);
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
            doBeginEntity(node);
        }
        
        void NodeSerializer::endEntity(Model::Node* node) {
            doEndEntity(node);
        }
        
        void NodeSerializer::entityAttributes(const Model::EntityAttribute::List& attributes) {
            Model::EntityAttribute::List::const_iterator it, end;
            for (it = attributes.begin(), end = attributes.end(); it != end; ++it)
                entityAttribute(*it);
        }

        void NodeSerializer::entityAttribute(const Model::EntityAttribute& attribute) {
            doEntityAttribute(attribute);
        }
        
        void NodeSerializer::brushes(const Model::BrushList& brushes) {
            Model::BrushList::const_iterator it, end;
            for (it = brushes.begin(), end = brushes.end(); it != end; ++it)
                brush(*it);
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
        }
        
        void NodeSerializer::brushFaces(const Model::BrushFaceList& faces) {
            Model::BrushFaceList::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it)
                brushFace(*it);
        }

        void NodeSerializer::brushFace(Model::BrushFace* face) {
            doBrushFace(face);
        }
        
        Model::EntityAttribute::List NodeSerializer::layerAttributes(const Model::Layer* layer) {
            Model::EntityAttribute::List attrs;
            attrs.reserve(3);
            attrs.push_back(Model::EntityAttribute(Model::AttributeNames::Classname, Model::AttributeValues::LayerClassname));
            attrs.push_back(Model::EntityAttribute(Model::AttributeNames::GroupType, Model::AttributeValues::GroupTypeLayer));
            attrs.push_back(Model::EntityAttribute(Model::AttributeNames::LayerName, layer->name()));
            return attrs;
        }
        
        Model::EntityAttribute::List NodeSerializer::groupAttributes(const Model::Group* group) {
            Model::EntityAttribute::List attrs;
            attrs.reserve(3);
            attrs.push_back(Model::EntityAttribute(Model::AttributeNames::Classname, Model::AttributeValues::GroupClassname));
            attrs.push_back(Model::EntityAttribute(Model::AttributeNames::GroupType, Model::AttributeValues::GroupTypeGroup));
            attrs.push_back(Model::EntityAttribute(Model::AttributeNames::GroupName, group->name()));
            return attrs;
        }
    }
}
