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

#include "IO/ParserStatus.h"
#include "Model/BrushNode.h"
#include "Model/EntityAttributes.h"
#include "Model/LayerNode.h"
#include "Model/World.h"

#include <kdl/string_utils.h>

#include <string>

namespace TrenchBroom {
    namespace IO {
        WorldReader::WorldReader(const char* begin, const char* end) :
        MapReader(begin, end) {}

        WorldReader::WorldReader(const std::string& str) :
        MapReader(str) {}

        std::unique_ptr<Model::World> WorldReader::read(Model::MapFormat format, const vm::bbox3& worldBounds, ParserStatus& status) {
            readEntities(format, worldBounds, status);
            m_world->rebuildNodeTree();
            m_world->enableNodeTreeUpdates();
            return std::move(m_world);
        }

        Model::ModelFactory& WorldReader::initialize(const Model::MapFormat format) {
            m_world = std::make_unique<Model::World>(format);
            m_world->disableNodeTreeUpdates();
            return *m_world;
        }

        Model::Node* WorldReader::onWorldspawn(const std::vector<Model::EntityAttribute>& attributes, const ExtraAttributes& extraAttributes, ParserStatus& /* status */) {
            m_world->setAttributes(attributes);
            setExtraAttributes(m_world.get(), extraAttributes);
            return m_world->defaultLayer();
        }

        void WorldReader::onWorldspawnFilePosition(const size_t lineNumber, const size_t lineCount, ParserStatus& /* status */) {
            m_world->setFilePosition(lineNumber, lineCount);
        }

        void WorldReader::onLayer(Model::LayerNode* layer, ParserStatus& /* status */) {
            m_world->addChild(layer);
        }

        void WorldReader::onNode(Model::Node* parent, Model::Node* node, ParserStatus& /* status */) {
            if (parent != nullptr) {
                parent->addChild(node);
            } else {
                m_world->defaultLayer()->addChild(node);
            }
        }

        void WorldReader::onUnresolvedNode(const ParentInfo& parentInfo, Model::Node* node, ParserStatus& status) {
            if (parentInfo.layer()) {
                status.warn(node->lineNumber(), kdl::str_to_string("Entity references missing layer '", parentInfo.id(), "', adding to default layer"));
            } else {
                status.warn(node->lineNumber(), kdl::str_to_string("Entity references missing group '", parentInfo.id(), "', adding to default layer"));
            }
            m_world->defaultLayer()->addChild(node);
        }

        void WorldReader::onBrush(Model::Node* parent, Model::BrushNode* brush, ParserStatus& /* status */) {
            if (parent != nullptr) {
                parent->addChild(brush);
            } else {
                m_world->defaultLayer()->addChild(brush);
            }
        }
    }
}
