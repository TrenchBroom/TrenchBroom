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

#ifndef TrenchBroom_WorldReader
#define TrenchBroom_WorldReader

#include "IO/MapReader.h"

#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class World;
    }

    namespace IO {
        class ParserStatus;

        class WorldReader : public MapReader {
            std::unique_ptr<Model::World> m_world;
        public:
            WorldReader(const char* begin, const char* end);
            explicit WorldReader(const std::string& str);

            std::unique_ptr<Model::World> read(Model::MapFormat format, const vm::bbox3& worldBounds, ParserStatus& status);
        private: // implement MapReader interface
            Model::ModelFactory& initialize(Model::MapFormat format) override;
            Model::Node* onWorldspawn(const std::vector<Model::EntityAttribute>& attributes, const ExtraAttributes& extraAttributes, ParserStatus& status) override;
            void onWorldspawnFilePosition(size_t lineNumber, size_t lineCount, ParserStatus& status) override;
            void onLayer(Model::Layer* layer, ParserStatus& status) override;
            void onNode(Model::Node* parent, Model::Node* node, ParserStatus& status) override;
            void onUnresolvedNode(const ParentInfo& parentInfo, Model::Node* node, ParserStatus& status) override;
            void onBrush(Model::Node* parent, Model::BrushNode* brush, ParserStatus& status) override;
        };
    }
}

#endif /* defined(TrenchBroom_WorldReader) */
