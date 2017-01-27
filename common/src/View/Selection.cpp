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

#include "Selection.h"

#include "CollectionUtils.h"

namespace TrenchBroom {
    namespace View {
        const Model::NodeList& Selection::partiallySelectedNodes() const {
            return m_partiallySelectedNodes;
        }
        
        const Model::NodeList& Selection::partiallyDeselectedNodes() const {
            return m_partiallyDeselectedNodes;
        }
        
        const Model::NodeList& Selection::recursivelySelectedNodes() const {
            return m_recursivelySelectedNodes;
        }
        
        const Model::NodeList& Selection::recursivelyDeselectedNodes() const {
            return m_recursivelyDeselectedNodes;
        }
        
        const Model::NodeList& Selection::selectedNodes() const {
            return m_selectedNodes;
        }
        
        const Model::NodeList& Selection::deselectedNodes() const {
            return m_deselectedNodes;
        }
        
        const Model::BrushFaceList& Selection::selectedBrushFaces() const {
            return m_selectedBrushFaces;
        }
        
        const Model::BrushFaceList& Selection::deselectedBrushFaces() const {
            return m_deselectedBrushFaces;
        }

        void Selection::addPartiallySelectedNodes(const Model::NodeList& nodes) {
            VectorUtils::append(m_partiallySelectedNodes, nodes);
        }
        
        void Selection::addPartiallyDeselectedNodes(const Model::NodeList& nodes) {
            VectorUtils::append(m_partiallyDeselectedNodes, nodes);
        }
        
        void Selection::addRecursivelySelectedNodes(const Model::NodeList& nodes) {
            VectorUtils::append(m_recursivelySelectedNodes, nodes);
        }
        
        void Selection::addRecursivelyDeselectedNodes(const Model::NodeList& nodes) {
            VectorUtils::append(m_recursivelyDeselectedNodes, nodes);
        }
        
        void Selection::addSelectedNodes(const Model::NodeList& nodes) {
            VectorUtils::append(m_selectedNodes, nodes);
        }
        
        void Selection::addDeselectedNodes(const Model::NodeList& nodes) {
            VectorUtils::append(m_deselectedNodes, nodes);
        }
        
        void Selection::addSelectedBrushFaces(const Model::BrushFaceList& faces) {
            VectorUtils::append(m_selectedBrushFaces, faces);
        }
        
        void Selection::addDeselectedBrushFaces(const Model::BrushFaceList& faces) {
            VectorUtils::append(m_deselectedBrushFaces, faces);
        }
    }
}
