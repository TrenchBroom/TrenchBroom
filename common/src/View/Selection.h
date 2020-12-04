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

#include "Model/BrushFaceHandle.h"

#include <vector>

namespace TrenchBroom {
    namespace Model {
        class Node;
    }

    namespace View {
        class Selection {
        private:
            std::vector<Model::Node*> m_selectedNodes;
            std::vector<Model::Node*> m_deselectedNodes;
            std::vector<Model::BrushFaceHandle> m_selectedBrushFaces;
            std::vector<Model::BrushFaceHandle> m_deselectedBrushFaces;
        public:
            const std::vector<Model::Node*>& selectedNodes() const;
            const std::vector<Model::Node*>& deselectedNodes() const;
            const std::vector<Model::BrushFaceHandle>& selectedBrushFaces() const;
            const std::vector<Model::BrushFaceHandle>& deselectedBrushFaces() const;

            void addSelectedNodes(const std::vector<Model::Node*>& nodes);
            void addDeselectedNodes(const std::vector<Model::Node*>& nodes);
            void addSelectedBrushFaces(const std::vector<Model::BrushFaceHandle>& faces);
            void addDeselectedBrushFaces(const std::vector<Model::BrushFaceHandle>& faces);
        };
    }
}


