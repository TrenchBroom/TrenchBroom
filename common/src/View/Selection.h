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

#ifndef TrenchBroom_Selection
#define TrenchBroom_Selection

#include "Model/Model_Forward.h"

#include <vector>

namespace TrenchBroom {
    namespace View {
        class Selection {
        private:
            std::vector<Model::Node*> m_partiallySelectedNodes;
            std::vector<Model::Node*> m_partiallyDeselectedNodes;
            std::vector<Model::Node*> m_recursivelySelectedNodes;
            std::vector<Model::Node*> m_recursivelyDeselectedNodes;
            std::vector<Model::Node*> m_selectedNodes;
            std::vector<Model::Node*> m_deselectedNodes;
            std::vector<Model::BrushFace*> m_selectedBrushFaces;
            std::vector<Model::BrushFace*> m_deselectedBrushFaces;
        public:
            const std::vector<Model::Node*>& partiallySelectedNodes() const;
            const std::vector<Model::Node*>& partiallyDeselectedNodes() const;
            const std::vector<Model::Node*>& recursivelySelectedNodes() const;
            const std::vector<Model::Node*>& recursivelyDeselectedNodes() const;
            const std::vector<Model::Node*>& selectedNodes() const;
            const std::vector<Model::Node*>& deselectedNodes() const;
            const std::vector<Model::BrushFace*>& selectedBrushFaces() const;
            const std::vector<Model::BrushFace*>& deselectedBrushFaces() const;

            void addPartiallySelectedNodes(const std::vector<Model::Node*>& nodes);
            void addPartiallyDeselectedNodes(const std::vector<Model::Node*>& nodes);
            void addRecursivelySelectedNodes(const std::vector<Model::Node*>& nodes);
            void addRecursivelyDeselectedNodes(const std::vector<Model::Node*>& nodes);
            void addSelectedNodes(const std::vector<Model::Node*>& nodes);
            void addDeselectedNodes(const std::vector<Model::Node*>& nodes);
            void addSelectedBrushFaces(const std::vector<Model::BrushFace*>& faces);
            void addDeselectedBrushFaces(const std::vector<Model::BrushFace*>& faces);
        };
    }
}

#endif /* defined(TrenchBroom_Selection) */
