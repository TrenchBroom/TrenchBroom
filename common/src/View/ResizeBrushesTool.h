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

#include "FloatType.h"
#include "NotifierConnection.h"
#include "Model/Brush.h"
#include "Model/HitType.h"
#include "View/Tool.h"

#include <vecmath/forward.h>
#include <vecmath/polygon.h>
#include <vecmath/vec.h>

#include <memory>
#include <tuple>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class Brush;
        class BrushFace;
        class BrushFaceHandle;
        class BrushNode;
        class Hit;
        class Node;
        class PickResult;
    }

    namespace Renderer {
        class Camera;
    }

    namespace View {
        class MapDocument;
        class Selection;

        /**
         * Similar to Model::BrushFaceHandle but caches the Brush state at the beginning of the drag.
         * We need this to be able to make decisions about the drag before reverting the transaction.
         */
        struct DragHandle {
            Model::BrushNode* node;
            Model::Brush brushAtDragStart;
            size_t faceIndex;

            explicit DragHandle(const Model::BrushFaceHandle& handle);

            const Model::BrushFace& faceAtDragStart() const;
            vm::vec3 faceNormal() const;

            bool operator==(const DragHandle& other) const;
            bool operator!=(const DragHandle& other) const;
        };

        /**
         * Tool for extruding faces along their normals (Shift+LMB Drag).
         * 
         * Also:
         *  - split brushes outward/inward (Ctrl+Shift+LMB Drag)
         *  - move faces (Alt+Shift+LMB Drag, 2D views only)
         * 
         * Terminology:
         *  - "Drag handle": for calculating the brush modifications.
         *    Instance of DragHandle. Always based on the brush
         *    state at the start of the drag.
         * 
         *  - "Visual handle": specifies which faces to render highlighted.
         *    Based on the current document state, so needs to be updated
         *    after each modification or roll back. May be fewer than the number
         *    of Drag handles if faces are clipped away by the drag.
         */
        class ResizeBrushesTool : public Tool {
        public:
            static const Model::HitType::Type Resize3DHitType;
            static const Model::HitType::Type Resize2DHitType;

            using Resize2DHitData = std::vector<Model::BrushFaceHandle>;
            using Resize3DHitData = Model::BrushFaceHandle;
        private:
            std::weak_ptr<MapDocument> m_document;
            /**
             * Propsed drag handles for the next drag. Should only be accessed when m_dragging is false.
             */
            std::vector<DragHandle> m_proposedDragHandles;
            bool m_dragging;

            NotifierConnection m_notifierConnection;
        private: // drag state - should only be accessed when m_dragging is true
            std::vector<Model::BrushFaceHandle> m_currentDragVisualHandles;
            std::vector<DragHandle> m_dragHandlesAtDragStart;
            vm::vec3 m_dragOrigin;
            /**
             * This is temporarily set to true when a drag is started with Ctrl,
             * to signal that new brushes need to be split off. After the split brushes have been
             * created, it's set back to false, in `resize()`.
             */
            bool m_splitBrushes;
            /**
             * How much drag is currently applied to the document.
             */
            vm::vec3 m_totalDelta;
        public:
            explicit ResizeBrushesTool(std::weak_ptr<MapDocument> document);

            bool applies() const;

            Model::Hit pick2D(const vm::ray3& pickRay, const Model::PickResult& pickResult);
            Model::Hit pick3D(const vm::ray3& pickRay, const Model::PickResult& pickResult);
        private:
            Model::Hit pickProximateFace(Model::HitType::Type hitType, const vm::ray3& pickRay) const;
        public:
            bool hasVisualHandles() const;
            std::vector<Model::BrushFaceHandle> visualHandles() const;
            void updateProposedDragHandles(const Model::PickResult& pickResult);
        private:
            std::vector<DragHandle> getDragHandles(const Model::Hit& hit) const;
            std::vector<DragHandle> collectDragHandles(const Model::Hit& hit) const;
            std::vector<Model::BrushFaceHandle> collectDragFaces(const Model::BrushFaceHandle& faceHandle) const;
        public:
            bool beginResize(const Model::PickResult& pickResult, bool split);
            bool resize(const vm::ray3& pickRay, const Renderer::Camera& camera);

            bool beginMove(const Model::PickResult& pickResult);
            bool move(const vm::ray3& pickRay, const Renderer::Camera& camera);

            void commit();
            void cancel();
        private:
            bool splitBrushesOutward(const vm::vec3& delta);
            bool splitBrushesInward(const vm::vec3& delta);
            std::vector<vm::polygon3> polygonsAtDragStart() const;
            void updateCurrentDragVisualHandles();
        private:
            void connectObservers();
            void nodesDidChange(const std::vector<Model::Node*>& nodes);
            void selectionDidChange(const Selection& selection);
        };
    }
}

