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

#include "NodeReader.h"

#include "Logger.h"
#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/Layer.h"
#include "Model/ModelFactory.h"
#include "Model/World.h"

namespace TrenchBroom {
    namespace IO {
        NodeReader::NodeReader(const String& str, Model::ModelFactory* factory, Logger* logger) :
        MapReader(str, logger),
        m_factory(factory) {
            assert(m_factory != NULL);
        }
        
        const Model::NodeList& NodeReader::read(const BBox3& worldBounds) {
            try {
                readEntities(m_factory->format(), worldBounds);
            } catch (const ParserException&) {
                VectorUtils::clearAndDelete(m_nodes);

                try {
                    reset();
                    readBrushes(m_factory->format(), worldBounds);
                } catch (const ParserException&) {
                    VectorUtils::clearAndDelete(m_nodes);
                    throw;
                }
            }
            return m_nodes;
        }
        
        Model::ModelFactory* NodeReader::initialize(const Model::MapFormat::Type format, const BBox3& worldBounds) {
            assert(format == m_factory->format());
            return m_factory;
        }
        
        Model::Node* NodeReader::onWorldspawn(const Model::EntityAttribute::List& attributes, const ExtraAttributes& extraAttributes) {
            Model::Entity* worldspawn = m_factory->createEntity();
            worldspawn->setAttributes(attributes);
            setExtraAttributes(worldspawn, extraAttributes);
            
            m_nodes.insert(m_nodes.begin(), worldspawn);
            return worldspawn;
        }
        
        void NodeReader::onWorldspawnFilePosition(const size_t lineNumber, const size_t lineCount) {
            assert(!m_nodes.empty());
            m_nodes.front()->setFilePosition(lineNumber, lineCount);
        }
        
        void NodeReader::onLayer(Model::Layer* layer) {
            m_nodes.push_back(layer);
        }
        
        void NodeReader::onNode(Model::Node* parent, Model::Node* node) {
            if (parent != NULL)
                parent->addChild(node);
            else
                m_nodes.push_back(node);
        }
        
        void NodeReader::onUnresolvedNode(const ParentInfo& parentInfo, Model::Node* node) {
            if (parentInfo.layer()) {
                logger()->warn("Could not resolve parent layer for object at line %u, adding to default layer", static_cast<unsigned int>(node->lineNumber()));
            } else if (parentInfo.group()) {
                logger()->warn("Could not resolve parent group for object at line %u, adding to default layer", static_cast<unsigned int>(node->lineNumber()));
            }
            m_nodes.push_back(node);
        }
        
        void NodeReader::onBrush(Model::Node* parent, Model::Brush* brush) {
            if (parent != NULL)
                parent->addChild(brush);
            else
                m_nodes.push_back(brush);
        }
    }
}
