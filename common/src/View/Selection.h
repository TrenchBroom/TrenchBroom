/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "Model/ModelTypes.h"

namespace TrenchBroom {
    namespace View {
        class Selection {
        private:
            Model::NodeArray m_partiallySelectedNodes;
            Model::NodeArray m_partiallyDeselectedNodes;
            Model::NodeArray m_recursivelySelectedNodes;
            Model::NodeArray m_recursivelyDeselectedNodes;
            Model::NodeArray m_selectedNodes;
            Model::NodeArray m_deselectedNodes;
            Model::BrushFaceArray m_selectedBrushFaces;
            Model::BrushFaceArray m_deselectedBrushFaces;
        public:
            const Model::NodeArray& partiallySelectedNodes() const;
            const Model::NodeArray& partiallyDeselectedNodes() const;
            const Model::NodeArray& recursivelySelectedNodes() const;
            const Model::NodeArray& recursivelyDeselectedNodes() const;
            const Model::NodeArray& selectedNodes() const;
            const Model::NodeArray& deselectedNodes() const;
            const Model::BrushFaceArray& selectedBrushFaces() const;
            const Model::BrushFaceArray& deselectedBrushFaces() const;
            
            void addPartiallySelectedNodes(const Model::NodeArray& nodes);
            void addPartiallyDeselectedNodes(const Model::NodeArray& nodes);
            void addRecursivelySelectedNodes(const Model::NodeArray& nodes);
            void addRecursivelyDeselectedNodes(const Model::NodeArray& nodes);
            void addSelectedNodes(const Model::NodeArray& nodes);
            void addDeselectedNodes(const Model::NodeArray& nodes);
            void addSelectedBrushFaces(const Model::BrushFaceArray& faces);
            void addDeselectedBrushFaces(const Model::BrushFaceArray& faces);
        };
    }
}

#endif /* defined(TrenchBroom_Selection) */
