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

#ifndef TrenchBroom_Selection
#define TrenchBroom_Selection

#include "Model/ModelTypes.h"

namespace TrenchBroom {
    namespace View {
        class Selection {
        private:
            Model::NodeList m_partiallySelectedNodes;
            Model::NodeList m_partiallyDeselectedNodes;
            Model::NodeList m_recursivelySelectedNodes;
            Model::NodeList m_recursivelyDeselectedNodes;
            Model::NodeList m_selectedNodes;
            Model::NodeList m_deselectedNodes;
            Model::BrushFaceList m_selectedBrushFaces;
            Model::BrushFaceList m_deselectedBrushFaces;
        public:
            const Model::NodeList& partiallySelectedNodes() const;
            const Model::NodeList& partiallyDeselectedNodes() const;
            const Model::NodeList& recursivelySelectedNodes() const;
            const Model::NodeList& recursivelyDeselectedNodes() const;
            const Model::NodeList& selectedNodes() const;
            const Model::NodeList& deselectedNodes() const;
            const Model::BrushFaceList& selectedBrushFaces() const;
            const Model::BrushFaceList& deselectedBrushFaces() const;
            
            void addPartiallySelectedNodes(const Model::NodeList& nodes);
            void addPartiallyDeselectedNodes(const Model::NodeList& nodes);
            void addRecursivelySelectedNodes(const Model::NodeList& nodes);
            void addRecursivelyDeselectedNodes(const Model::NodeList& nodes);
            void addSelectedNodes(const Model::NodeList& nodes);
            void addDeselectedNodes(const Model::NodeList& nodes);
            void addSelectedBrushFaces(const Model::BrushFaceList& faces);
            void addDeselectedBrushFaces(const Model::BrushFaceList& faces);
        };
    }
}

#endif /* defined(TrenchBroom_Selection) */
