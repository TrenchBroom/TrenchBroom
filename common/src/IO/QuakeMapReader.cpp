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

#include "Logger.h"
#include "Model/Brush.h"
#include "Model/Layer.h"
#include "Model/World.h"

namespace TrenchBroom {
    namespace IO {
        QuakeMapReader::QuakeMapReader(const char* begin, const char* end, const Model::BrushContentTypeBuilder* brushContentTypeBuilder, Logger* logger) :
        QuakeReader(begin, end, logger),
        m_brushContentTypeBuilder(brushContentTypeBuilder),
        m_world(NULL) {}
        
        QuakeMapReader::QuakeMapReader(const String& str, const Model::BrushContentTypeBuilder* brushContentTypeBuilder, Logger* logger) :
        QuakeReader(str, logger),
        m_brushContentTypeBuilder(brushContentTypeBuilder),
        m_world(NULL) {}
        
        Model::World* QuakeMapReader::readMap(const BBox3& worldBounds) {
            read(worldBounds);
            return m_world;
        }

        Model::ModelFactory* QuakeMapReader::initialize(const Model::MapFormat::Type format) {
            assert(m_world == NULL);
            m_world = new Model::World(format, m_brushContentTypeBuilder);
            return m_world;
        }
        
        void QuakeMapReader::onWorldspawn(const Model::EntityAttribute::List& attributes, const ExtraAttributes& extraAttributes) {
            m_world->setAttributes(attributes);
            setExtraAttributes(m_world, extraAttributes);
        }

        void QuakeMapReader::onWorldspawnFilePosition(const size_t lineNumber, const size_t lineCount) {
            m_world->setFilePosition(lineNumber, lineCount);
        }

        void QuakeMapReader::onLayer(Model::Layer* layer) {
            m_world->addChild(layer);
        }
        
        void QuakeMapReader::onNode(Model::Node* parent, Model::Node* node) {
            if (parent != NULL)
                parent->addChild(node);
            else
                m_world->defaultLayer()->addChild(node);
        }
        
        void QuakeMapReader::onUnresolvedNode(const ParentInfo& parentInfo, Model::Node* node) {
            if (logger() != NULL) {
                if (parentInfo.layer())
                    logger()->warn("Entity at line %u references missing layer '%s', adding to default layer", node->lineNumber(), parentInfo.name().c_str());
                else
                    logger()->warn("Entity at line %u references missing group '%s', adding to default layer", node->lineNumber(), parentInfo.name().c_str());
            }
            m_world->defaultLayer()->addChild(node);
        }
        
        void QuakeMapReader::onBrush(Model::Node* parent, Model::Brush* brush) {
            if (parent != NULL)
                parent->addChild(brush);
            else
                m_world->defaultLayer()->addChild(brush);
        }
        
        void QuakeMapReader::onBrushFace(Model::Brush* brush, Model::BrushFace* face) {
            assert(false);
        }
    }
}
