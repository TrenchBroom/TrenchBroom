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

#include "MapWriter.h"

#include "Exceptions.h"
#include "IO/DiskFileSystem.h"
#include "IO/MapSerializer.h"
#include "IO/MapFileSerializer.h"
#include "IO/MapStreamSerializer.h"
#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/Group.h"
#include "Model/Layer.h"
#include "Model/World.h"

namespace TrenchBroom {
    namespace IO {
        BrushWriter::BrushWriter(MapSerializer& serializer) : m_serializer(serializer) {}
        
        void BrushWriter::doVisit(Model::World* world)   { stopRecursion(); }
        void BrushWriter::doVisit(Model::Layer* layer)   { stopRecursion(); }
        void BrushWriter::doVisit(Model::Group* group)   { stopRecursion(); }
        void BrushWriter::doVisit(Model::Entity* entity) { stopRecursion(); }
        
        void BrushWriter::doVisit(Model::Brush* brush)   {
            const Model::BrushFaceList& faces = brush->faces();
            Model::BrushFaceList::const_iterator it, end;
            
            m_serializer.beginBrush(brush);
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                Model::BrushFace* face = *it;
                m_serializer.brushFace(face);
            }
            m_serializer.endBrush(brush);
        }

        Model::EntityAttribute::List getParentAttributes(const Model::Node* node);
        
        MapWriter::MapWriter(MapSerializer& serializer, const Model::Node* currentParent) :
        m_serializer(serializer),
        m_brushWriter(m_serializer),
        m_parentAttributes(getParentAttributes(currentParent)) {}
        
        void MapWriter::doVisit(Model::World* world)   {
            Model::Layer* defaultLayer = world->defaultLayer();
            writeDefaultLayer(world->attributes(), Model::EntityAttribute::EmptyList, defaultLayer);
            
            const Model::LayerList customLayers = world->customLayers();
            Model::LayerList::const_iterator it, end;
            for (it = customLayers.begin(), end = customLayers.end(); it != end; ++it) {
                Model::Layer* layer = *it;
                writeContainer(Model::EntityAttribute::EmptyList, layerAttributes(layer), layer);
            }
            
            stopRecursion();
        }
        
        void MapWriter::doVisit(Model::Layer* layer)   { stopRecursion(); }
        
        void MapWriter::doVisit(Model::Group* group)   {
            writeContainer(groupAttributes(group), m_parentAttributes, group);
            stopRecursion();
        }
        
        void MapWriter::doVisit(Model::Entity* entity) {
            writeEntity(entity->attributes(), m_parentAttributes, entity);
            stopRecursion();
        }
        
        void MapWriter::doVisit(Model::Brush* brush)   { brush->accept(m_brushWriter); stopRecursion(); }
        
        void MapWriter::writeDefaultLayer(const Model::EntityAttribute::List& attrs, const Model::EntityAttribute::List& extra, Model::Node* node) {
            writeEntity(attrs, extra, node);
            
            MapWriter writer(m_serializer, NULL);
            node->iterate(writer);
        }
        
        void MapWriter::writeContainer(const Model::EntityAttribute::List& attrs, const Model::EntityAttribute::List& extra, Model::Node* node) {
            writeEntity(attrs, extra, node);
            
            MapWriter writer(m_serializer, node);
            node->iterate(writer);
        }
        
        void MapWriter::writeEntity(const Model::EntityAttribute::List& attrs, const Model::EntityAttribute::List& extra, Model::Node* node) {
            m_serializer.beginEntity(node);
            writeAttributes(attrs);
            writeAttributes(extra);
            
            node->iterate(m_brushWriter);
            m_serializer.endEntity(node);
        }
        
        Model::EntityAttribute::List MapWriter::layerAttributes(const Model::Layer* layer) {
            Model::EntityAttribute::List attrs;
            attrs.reserve(3);
            attrs.push_back(Model::EntityAttribute(Model::AttributeNames::Classname, Model::AttributeValues::LayerClassname));
            attrs.push_back(Model::EntityAttribute(Model::AttributeNames::GroupType, Model::AttributeValues::GroupTypeLayer));
            attrs.push_back(Model::EntityAttribute(Model::AttributeNames::LayerName, layer->name()));
            return attrs;
        }
        
        Model::EntityAttribute::List MapWriter::groupAttributes(const Model::Group* group) {
            Model::EntityAttribute::List attrs;
            attrs.reserve(3);
            attrs.push_back(Model::EntityAttribute(Model::AttributeNames::Classname, Model::AttributeValues::GroupClassname));
            attrs.push_back(Model::EntityAttribute(Model::AttributeNames::GroupType, Model::AttributeValues::GroupTypeGroup));
            attrs.push_back(Model::EntityAttribute(Model::AttributeNames::GroupName, group->name()));
            return attrs;
        }
        
        void MapWriter::writeAttributes(const Model::EntityAttribute::List& attrs) {
            Model::EntityAttribute::List::const_iterator it, end;
            for (it = attrs.begin(), end = attrs.end(); it != end; ++it)
                m_serializer.entityAttribute(*it);
        }
        
        class GetParentAttributes : public Model::ConstNodeVisitor {
        private:
            Model::EntityAttribute::List m_attributes;
        public:
            const Model::EntityAttribute::List& attributes() const {
                return m_attributes;
            }
        private:
            void doVisit(const Model::World* world)   {}
            void doVisit(const Model::Layer* layer)   { m_attributes.push_back(Model::EntityAttribute(Model::AttributeNames::Layer, layer->name()));}
            void doVisit(const Model::Group* group)   { m_attributes.push_back(Model::EntityAttribute(Model::AttributeNames::Group, group->name())); }
            void doVisit(const Model::Entity* entity) {}
            void doVisit(const Model::Brush* brush)   {}
        };
        
        Model::EntityAttribute::List getParentAttributes(const Model::Node* node) {
            if (node == NULL)
                return Model::EntityAttribute::List(0);

            GetParentAttributes visitor;
            node->accept(visitor);
            return visitor.attributes();
        }

        void MapWriter::writeToFile(Model::World* map, const Path& path, const bool overwrite) {
            if (IO::Disk::fileExists(IO::Disk::fixPath(path)) && !overwrite)
                throw FileSystemException("File already exists: " + path.asString());

            MapSerializer::Ptr serializer = MapFileSerializer::create(map->format(), path);
            MapWriter writer(*serializer);
            map->accept(writer);
        }

        void MapWriter::writeToStream(const Model::MapFormat::Type format, const Model::NodeList& nodes, std::ostream& stream) {
            MapSerializer::Ptr serializer = MapStreamSerializer::create(format, stream);
            MapWriter writer(*serializer);
            Model::Node::accept(nodes.begin(), nodes.end(), writer);
        }

        void MapWriter::writeToStream(Model::MapFormat::Type format, const Model::BrushFaceList& faces, std::ostream& stream) {
            MapSerializer::Ptr serializer = MapStreamSerializer::create(format, stream);
            Model::BrushFaceList::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it)
                serializer->brushFace(*it);
        }
    }
}
