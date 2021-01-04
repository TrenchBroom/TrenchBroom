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

#pragma once

#include "IO/MapReader.h"

#include <string>
#include <string_view>
#include <vector>

namespace TrenchBroom {
    namespace IO {
        class ParserStatus;

        /**
         * MapReader subclass for loading the clipboard contents, rather than an entire .map
         */
        class NodeReader : public MapReader {
        private:
            std::vector<Model::Node*> m_nodes;
        public:
            /**
             * Creates a new parser where the given string is expected to be formatted in the given source map format,
             * and the created objects are converted to the given target format.
             *
             * @param str the string to parse
             * @param sourceMapFormat the expected format of the given string
             * @param targetMapFormat the format to convert the created objects to
             */
            NodeReader(std::string_view str, Model::MapFormat sourceMapFormat, Model::MapFormat targetMapFormat);

            static std::vector<Model::Node*> read(const std::string& str, Model::MapFormat preferredMapFormat, const vm::bbox3& worldBounds, ParserStatus& status);
        private:
            static std::vector<Model::Node*> readAsFormat(Model::MapFormat sourceMapFormat, Model::MapFormat targetMapFormat, const std::string& str, const vm::bbox3& worldBounds, ParserStatus& status);
        private: // implement MapReader interface
            Model::Node* onWorldspawn(const std::vector<Model::EntityProperty>& properties, const ExtraAttributes& extraAttributes, ParserStatus& status) override;
            void onWorldspawnFilePosition(size_t lineNumber, size_t lineCount, ParserStatus& status) override;
            void onLayer(Model::LayerNode* layer, ParserStatus& status) override;
            void onNode(Model::Node* parent, Model::Node* node, ParserStatus& status) override;
            void onUnresolvedNode(const ParentInfo& parentInfo, Model::Node* node, ParserStatus& status) override;
            void onBrush(Model::Node* parent, Model::BrushNode* brush, ParserStatus& status) override;
        };
    }
}

