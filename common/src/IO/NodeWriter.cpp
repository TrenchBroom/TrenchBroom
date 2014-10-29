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

#include "NodeWriter.h"

#include "IO/MapStreamSerializer.h"
#include "IO/MapWriter.h"
#include "Model/AssortNodesVisitor.h"
#include "Model/Brush.h"
#include "Model/Node.h"

namespace TrenchBroom {
    namespace IO {
        NodeWriter::NodeWriter(NodeSerializer& serializer) :
        m_serializer(serializer) {}
        
        class NodeWriter::CollectEntityBrushesStrategy {
        public:
            typedef Model::AssortNodesVisitorT<Model::SkipLayersStrategy, Model::CollectGroupsStrategy, Model::CollectEntitiesStrategy, CollectEntityBrushesStrategy> AssortNodesVisitor;
        private:
            EntityBrushesMap m_entityBrushes;
            Model::BrushList m_worldBrushes;
            
            class VisitParent : public Model::NodeVisitor {
            private:
                Model::Brush* m_brush;
                EntityBrushesMap& m_entityBrushes;
                Model::BrushList& m_worldBrushes;
            public:
                VisitParent(Model::Brush* brush, EntityBrushesMap& entityBrushes, Model::BrushList& worldBrushes) :
                m_brush(brush),
                m_entityBrushes(entityBrushes),
                m_worldBrushes(worldBrushes) {}
            private:
                void doVisit(Model::World* world)   { m_worldBrushes.push_back(m_brush);  }
                void doVisit(Model::Layer* layer)   { m_worldBrushes.push_back(m_brush);  }
                void doVisit(Model::Group* group)   { m_worldBrushes.push_back(m_brush);  }
                void doVisit(Model::Entity* entity) { m_entityBrushes[entity].push_back(m_brush); }
                void doVisit(Model::Brush* brush)   {}
            };
        public:
            const EntityBrushesMap& entityBrushes() const {
                return m_entityBrushes;
            }
            
            const Model::BrushList& worldBrushes() const {
                return m_worldBrushes;
            }
            
            void addBrush(Model::Brush* brush) {
                VisitParent visitParent(brush, m_entityBrushes, m_worldBrushes);
                Model::Node* parent = brush->parent();
                parent->accept(visitParent);
            }
        };
        
        
        void NodeWriter::writeNodes(const Model::NodeList& nodes) {
            CollectEntityBrushesStrategy::AssortNodesVisitor assort;
            Model::Node::accept(nodes.begin(), nodes.end(), assort);
            
            writeGroups(assort.groups());
            writeEntities(assort.entities());
            writeWorldBrushes(assort.worldBrushes());
            writeEntityBrushes(assort.entityBrushes());
        }
        
        void NodeWriter::writeNodesToStream(const Model::NodeList& nodes, Model::MapFormat::Type format, std::ostream& stream) {
            NodeSerializer::Ptr serializer = MapStreamSerializer::create(format, stream);
            NodeWriter writer(*serializer);
            writer.writeNodes(nodes);
        }

        void NodeWriter::writeGroups(const Model::GroupList& groups) {
            MapWriter writer(m_serializer);
            Model::Node::accept(groups.begin(), groups.end(), writer);
        }
        
        void NodeWriter::writeEntities(const Model::EntityList& entities) {
            MapWriter writer(m_serializer);
            Model::Node::accept(entities.begin(), entities.end(), writer);
        }
        
        void NodeWriter::writeWorldBrushes(const Model::BrushList& brushes) {
            BrushWriter writer(m_serializer);
            Model::Node::accept(brushes.begin(), brushes.end(), writer);
        }
        
        void NodeWriter::writeEntityBrushes(const EntityBrushesMap& entityBrushes) {
        }
    }
}
