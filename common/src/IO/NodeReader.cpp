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
#include "Model/EntityNode.h"
#include "Model/EntityProperties.h"
#include "Model/LayerNode.h"
#include "Model/WorldNode.h"

#include <kdl/vector_utils.h>

#include <sstream>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace IO {
        NodeReader::NodeReader(std::string_view str, const Model::MapFormat sourceMapFormat, const Model::MapFormat targetMapFormat) :
        MapReader(str, sourceMapFormat, targetMapFormat) {}

        std::vector<Model::Node*> NodeReader::read(const std::string& str, const Model::MapFormat preferredMapFormat, const vm::bbox3& worldBounds, ParserStatus& status) {
            // Try preferred format first
            for (const auto compatibleMapFormat : Model::compatibleFormats(preferredMapFormat)) {
                if (auto result = readAsFormat(compatibleMapFormat, preferredMapFormat, str, worldBounds, status); !result.empty()) {
                    return result;
                }
            }

            // All formats failed
            return {};
        }

        /**
         * Attempts to parse the string as one or more entities (in the given source format), and if that fails,
         * as one or more brushes.
         *
         * Does not throw upon parsing failure, but instead logs the failure to `status` and returns {}.
         *
         * @returns the parsed nodes; caller is responsible for freeing them.
         */
        std::vector<Model::Node*> NodeReader::readAsFormat(const Model::MapFormat sourceMapFormat, const Model::MapFormat targetMapFormat, const std::string& str, const vm::bbox3& worldBounds, ParserStatus& status) {
            {
                NodeReader reader(str, sourceMapFormat, targetMapFormat);
                try {
                    reader.readEntities(worldBounds, status);
                    status.info("Parsed successfully as " + Model::formatName(sourceMapFormat) + " entities");
                    return reader.m_nodes;
                } catch (const ParserException& e) {
                    status.info("Couldn't parse as " + Model::formatName(sourceMapFormat) + " entities: " + e.what());
                    kdl::vec_clear_and_delete(reader.m_nodes);
                }
            }

            {
                NodeReader reader(str, sourceMapFormat, targetMapFormat);
                try {
                    reader.readBrushes(worldBounds, status);
                    status.info("Parsed successfully as " + Model::formatName(sourceMapFormat) + " brushes");
                    return reader.m_nodes;
                } catch (const ParserException& e) {
                    status.info("Couldn't parse as " + Model::formatName(sourceMapFormat) + " brushes: " + e.what());
                    kdl::vec_clear_and_delete(reader.m_nodes);
                }
            }
            return {};
        }

        Model::Node* NodeReader::onWorldspawn(std::vector<Model::EntityProperty> properties, ParserStatus& /* status */) {
            Model::EntityNode* worldspawn = new Model::EntityNode{Model::Entity{std::move(properties)}};
            m_nodes.insert(std::begin(m_nodes), worldspawn);
            return worldspawn;
        }

        void NodeReader::onWorldspawnFilePosition(const size_t lineNumber, const size_t lineCount, ParserStatus& /* status */) {
            assert(!m_nodes.empty());
            m_nodes.front()->setFilePosition(lineNumber, lineCount);
        }

        void NodeReader::onLayer(Model::LayerNode* layer, ParserStatus& /* status */) {
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
            if (parentInfo.type == ParentType::Layer) {
                std::stringstream msg;
                msg << "Could not resolve parent layer '" << parentInfo.id << "', adding to default layer";
                status.warn(node->lineNumber(), msg.str());
            } else {
                std::stringstream msg;
                msg << "Could not resolve parent group '" << parentInfo.id << "', adding to default layer";
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
