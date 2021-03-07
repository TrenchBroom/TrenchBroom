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
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class BrushNode;
        class BrushFace;
        class EntityProperty;
        class LayerNode;
        enum class MapFormat;
        class Node;
    }

    namespace IO {
        class ParserStatus;

        /**
         * Used for pasting brush faces (i.e. their texture alignment only)
         */
        class BrushFaceReader : public MapReader {
        private:
            std::vector<Model::BrushFace> m_brushFaces;
        public:
            BrushFaceReader(const std::string& str, Model::MapFormat sourceAndTargetMapFormat);

            std::vector<Model::BrushFace> read(const vm::bbox3& worldBounds, ParserStatus& status);
        private: // implement MapReader interface
            Model::Node* onWorldNode(std::unique_ptr<Model::WorldNode> worldNode, ParserStatus& status) override;
            void onLayerNode(std::unique_ptr<Model::Node> layerNode, ParserStatus& status) override;
            void onNode(Model::Node* parentNode, std::unique_ptr<Model::Node> node, ParserStatus& status) override;
            void onBrushFace(Model::BrushFace face, ParserStatus& status) override;
        };
    }
}

