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

#include <kdl/vector_utils.h>

#include <vector>

namespace TrenchBroom {
    namespace View {
        const std::vector<Model::Node*>& Selection::partiallySelectedNodes() const {
            return m_partiallySelectedNodes;
        }

        const std::vector<Model::Node*>& Selection::partiallyDeselectedNodes() const {
            return m_partiallyDeselectedNodes;
        }

        const std::vector<Model::Node*>& Selection::recursivelySelectedNodes() const {
            return m_recursivelySelectedNodes;
        }

        const std::vector<Model::Node*>& Selection::recursivelyDeselectedNodes() const {
            return m_recursivelyDeselectedNodes;
        }

        const std::vector<Model::Node*>& Selection::selectedNodes() const {
            return m_selectedNodes;
        }

        const std::vector<Model::Node*>& Selection::deselectedNodes() const {
            return m_deselectedNodes;
        }

        const std::vector<Model::BrushFace*>& Selection::selectedBrushFaces() const {
            return m_selectedBrushFaces;
        }

        const std::vector<Model::BrushFace*>& Selection::deselectedBrushFaces() const {
            return m_deselectedBrushFaces;
        }

        void Selection::addPartiallySelectedNodes(const std::vector<Model::Node*>& nodes) {
            kdl::vec_append(m_partiallySelectedNodes, nodes);
        }

        void Selection::addPartiallyDeselectedNodes(const std::vector<Model::Node*>& nodes) {
            kdl::vec_append(m_partiallyDeselectedNodes, nodes);
        }

        void Selection::addRecursivelySelectedNodes(const std::vector<Model::Node*>& nodes) {
            kdl::vec_append(m_recursivelySelectedNodes, nodes);
        }

        void Selection::addRecursivelyDeselectedNodes(const std::vector<Model::Node*>& nodes) {
            kdl::vec_append(m_recursivelyDeselectedNodes, nodes);
        }

        void Selection::addSelectedNodes(const std::vector<Model::Node*>& nodes) {
            kdl::vec_append(m_selectedNodes, nodes);
        }

        void Selection::addDeselectedNodes(const std::vector<Model::Node*>& nodes) {
            kdl::vec_append(m_deselectedNodes, nodes);
        }

        void Selection::addSelectedBrushFaces(const std::vector<Model::BrushFace*>& faces) {
            kdl::vec_append(m_selectedBrushFaces, faces);
        }

        void Selection::addDeselectedBrushFaces(const std::vector<Model::BrushFace*>& faces) {
            kdl::vec_append(m_deselectedBrushFaces, faces);
        }
    }
}
