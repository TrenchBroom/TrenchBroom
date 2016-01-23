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

#include "WorldReader.h"

#include "Logger.h"
#include "Model/Brush.h"
#include "Model/Layer.h"
#include "Model/World.h"

namespace TrenchBroom {
    namespace IO {
        WorldReader::WorldReader(const char* begin, const char* end, const Model::BrushContentTypeBuilder* brushContentTypeBuilder, Logger* logger) :
        MapReader(begin, end, logger),
        m_brushContentTypeBuilder(brushContentTypeBuilder),
        m_world(NULL) {}
        
        WorldReader::WorldReader(const String& str, const Model::BrushContentTypeBuilder* brushContentTypeBuilder, Logger* logger) :
        MapReader(str, logger),
        m_brushContentTypeBuilder(brushContentTypeBuilder),
        m_world(NULL) {}
        
        Model::World* WorldReader::read(Model::MapFormat::Type format, const BBox3& worldBounds) {
            readEntities(format, worldBounds);
            return m_world;
        }

        Model::ModelFactory* WorldReader::initialize(const Model::MapFormat::Type format, const BBox3& worldBounds) {
            assert(m_world == NULL);
            m_world = new Model::World(format, m_brushContentTypeBuilder, worldBounds);
            return m_world;
        }
        
        Model::Node* WorldReader::onWorldspawn(const Model::EntityAttribute::List& attributes, const ExtraAttributes& extraAttributes) {
            m_world->setAttributes(attributes);
            setExtraAttributes(m_world, extraAttributes);
            return m_world->defaultLayer();
        }

        void WorldReader::onWorldspawnFilePosition(const size_t lineNumber, const size_t lineCount) {
            m_world->setFilePosition(lineNumber, lineCount);
        }

        void WorldReader::onLayer(Model::Layer* layer) {
            m_world->addChild(layer);
        }
        
        void WorldReader::onNode(Model::Node* parent, Model::Node* node) {
            if (parent != NULL)
                parent->addChild(node);
            else
                m_world->defaultLayer()->addChild(node);
        }
        
        void WorldReader::onUnresolvedNode(const ParentInfo& parentInfo, Model::Node* node) {
            if (logger() != NULL) {
                if (parentInfo.layer())
                    logger()->warn("Entity at line %u references missing layer '%u', adding to default layer", node->lineNumber(), parentInfo.id());
                else
                    logger()->warn("Entity at line %u references missing group '%u', adding to default layer", node->lineNumber(), parentInfo.id());
            }
            m_world->defaultLayer()->addChild(node);
        }
        
        void WorldReader::onBrush(Model::Node* parent, Model::Brush* brush) {
            if (parent != NULL)
                parent->addChild(brush);
            else
                m_world->defaultLayer()->addChild(brush);
        }
    }
}
