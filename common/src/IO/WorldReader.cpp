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

#include "WorldReader.h"

#include "Logger.h"
#include "Model/Brush.h"
#include "Model/Layer.h"
#include "Model/World.h"

namespace TrenchBroom {
    namespace IO {
        WorldReader::WorldReader(const char* begin, const char* end, const Model::BrushContentTypeBuilder* brushContentTypeBuilder) :
        MapReader(begin, end),
        m_brushContentTypeBuilder(brushContentTypeBuilder),
        m_world(nullptr) {}
        
        WorldReader::WorldReader(const String& str, const Model::BrushContentTypeBuilder* brushContentTypeBuilder) :
        MapReader(str),
        m_brushContentTypeBuilder(brushContentTypeBuilder),
        m_world(nullptr) {}
        
        Model::World* WorldReader::read(Model::MapFormat::Type format, const BBox3& worldBounds, ParserStatus& status) {
            readEntities(format, worldBounds, status);
            return m_world;
        }

        Model::ModelFactory* WorldReader::initialize(const Model::MapFormat::Type format, const BBox3& worldBounds) {
            assert(m_world == nullptr);
            m_world = new Model::World(format, m_brushContentTypeBuilder, worldBounds);
            return m_world;
        }
        
        Model::Node* WorldReader::onWorldspawn(const Model::EntityAttribute::List& attributes, const ExtraAttributes& extraAttributes, ParserStatus& status) {
            m_world->setAttributes(attributes);
            setExtraAttributes(m_world, extraAttributes);
            return m_world->defaultLayer();
        }

        void WorldReader::onWorldspawnFilePosition(const size_t lineNumber, const size_t lineCount, ParserStatus& status) {
            m_world->setFilePosition(lineNumber, lineCount);
        }

        void WorldReader::onLayer(Model::Layer* layer, ParserStatus& status) {
            m_world->addChild(layer);
        }
        
        void WorldReader::onNode(Model::Node* parent, Model::Node* node, ParserStatus& status) {
            if (parent != nullptr)
                parent->addChild(node);
            else
                m_world->defaultLayer()->addChild(node);
        }
        
        void WorldReader::onUnresolvedNode(const ParentInfo& parentInfo, Model::Node* node, ParserStatus& status) {
            if (parentInfo.layer()) {
                StringStream msg;
                msg << "Entity references missing layer '" << parentInfo.id() << "', adding to default layer";
                status.warn(node->lineNumber(), msg.str());
            } else {
                StringStream msg;
                msg << "Entity references missing group '" << parentInfo.id() << "', adding to default layer";
                status.warn(node->lineNumber(), msg.str());
            }
            m_world->defaultLayer()->addChild(node);
        }
        
        void WorldReader::onBrush(Model::Node* parent, Model::Brush* brush, ParserStatus& status) {
            if (parent != nullptr)
                parent->addChild(brush);
            else
                m_world->defaultLayer()->addChild(brush);
        }
    }
}
