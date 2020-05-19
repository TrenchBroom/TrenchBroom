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

#include "NodeReader.h"

#include "IO/ParserStatus.h"
#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/EntityAttributes.h"
#include "Model/Layer.h"
#include "Model/ModelFactory.h"
#include "Model/World.h"

#include <kdl/vector_utils.h>

#include <sstream>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace IO {
        NodeReader::NodeReader(const std::string& str, Model::ModelFactory& factory) :
        MapReader(str),
        m_factory(factory) {}

        std::vector<Model::Node*> NodeReader::read(const std::string& str, Model::ModelFactory& factory, const vm::bbox3& worldBounds, ParserStatus& status) {
            NodeReader reader(str, factory);
            return reader.read(worldBounds, status);
        }

        const std::vector<Model::Node*>& NodeReader::read(const vm::bbox3& worldBounds, ParserStatus& status) {
            try {
                readEntities(m_factory.format(), worldBounds, status);
            } catch (const ParserException&) {
                kdl::vec_clear_and_delete(m_nodes);

                try {
                    reset();
                    readBrushes(m_factory.format(), worldBounds, status);
                } catch (const ParserException&) {
                    kdl::vec_clear_and_delete(m_nodes);
                    throw;
                }
            }
            return m_nodes;
        }

        Model::ModelFactory& NodeReader::initialize([[maybe_unused]] const Model::MapFormat format) {
            assert(format == m_factory.format());
            return m_factory;
        }

        Model::Node* NodeReader::onWorldspawn(const std::vector<Model::EntityAttribute>& attributes, const ExtraAttributes& extraAttributes, ParserStatus& /* status */) {
            Model::Entity* worldspawn = m_factory.createEntity();
            worldspawn->setAttributes(attributes);
            setExtraAttributes(worldspawn, extraAttributes);

            m_nodes.insert(std::begin(m_nodes), worldspawn);
            return worldspawn;
        }

        void NodeReader::onWorldspawnFilePosition(const size_t lineNumber, const size_t lineCount, ParserStatus& /* status */) {
            assert(!m_nodes.empty());
            m_nodes.front()->setFilePosition(lineNumber, lineCount);
        }

        void NodeReader::onLayer(Model::Layer* layer, ParserStatus& /* status */) {
            m_nodes.push_back(layer);
        }

        void NodeReader::onNode(Model::Node* parent, Model::Node* node, ParserStatus& /* status */) {
            if (parent != nullptr) {
                parent->addChild(node);
            } else {
                m_nodes.push_back(node);
            }
        }

        void NodeReader::onUnresolvedNode(const ParentInfo& parentInfo, Model::Node* node, ParserStatus& status) {
            if (parentInfo.layer()) {
                std::stringstream msg;
                msg << "Could not resolve parent layer '" << parentInfo.id() << "', adding to default layer";
                status.warn(node->lineNumber(), msg.str());
            } else {
                std::stringstream msg;
                msg << "Could not resolve parent group '" << parentInfo.id() << "', adding to default layer";
                status.warn(node->lineNumber(), msg.str());
            }
            m_nodes.push_back(node);
        }

        void NodeReader::onBrush(Model::Node* parent, Model::BrushNode* brush, ParserStatus& /* status */) {
            if (parent != nullptr) {
                parent->addChild(brush);
            } else {
                m_nodes.push_back(brush);
            }
        }
    }
}
