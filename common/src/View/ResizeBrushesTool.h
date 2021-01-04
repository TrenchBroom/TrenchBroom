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
#include "Model/HitType.h"
#include "View/Tool.h"

#include <vecmath/forward.h>
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

        class ResizeBrushesTool : public Tool {
        public:
            static const Model::HitType::Type Resize3DHitType;
            static const Model::HitType::Type Resize2DHitType;

            using Resize2DHitData = std::vector<Model::BrushFaceHandle>;
            using Resize3DHitData = Model::BrushFaceHandle;
        private:
            /**
             * Brush and face normal pair.
             */
            using FaceHandle = std::tuple<Model::BrushNode*, vm::vec3>;

            std::weak_ptr<MapDocument> m_document;
            std::vector<FaceHandle> m_dragHandles;
            vm::vec3 m_dragOrigin;
            vm::vec3 m_lastPoint;
            /**
             * This is temporarily set to true when a drag is started with Ctrl,
             * to signal that new brushes need to be split off. After the split brushes have been
             * created, it's set back to false, in `resize()`.
             */
            bool m_splitBrushes;
            vm::vec3 m_totalDelta;
            bool m_dragging;
        public:
            explicit ResizeBrushesTool(std::weak_ptr<MapDocument> document);
            ~ResizeBrushesTool() override;

            bool applies() const;

            Model::Hit pick2D(const vm::ray3& pickRay, const Model::PickResult& pickResult);
            Model::Hit pick3D(const vm::ray3& pickRay, const Model::PickResult& pickResult);
        private:
            Model::Hit pickProximateFace(Model::HitType::Type hitType, const vm::ray3& pickRay) const;
        public:
            bool hasDragFaces() const;
            std::vector<Model::BrushFaceHandle> dragFaces() const;
            void updateDragFaces(const Model::PickResult& pickResult);
        private:
            std::vector<FaceHandle> getDragHandles(const Model::Hit& hit) const;
            std::vector<FaceHandle> collectDragHandles(const Model::Hit& hit) const;
            std::vector<Model::BrushFaceHandle> collectDragFaces(const Model::BrushFaceHandle& faceHandle) const;
        public:
            bool beginResize(const Model::PickResult& pickResult, bool split);
            bool resize(const vm::ray3& pickRay, const Renderer::Camera& camera);
            vm::vec3 selectDelta(const vm::vec3& relativeDelta, const vm::vec3& absoluteDelta, FloatType mouseDistance) const;

            bool beginMove(const Model::PickResult& pickResult);
            bool move(const vm::ray3& pickRay, const Renderer::Camera& camera);

            void commit();
            void cancel();
        private:
            bool splitBrushesOutward(const vm::vec3& delta);
            bool splitBrushesInward(const vm::vec3& delta);
            std::vector<vm::polygon3> dragFaceDescriptors() const;
        private:
            void bindObservers();
            void unbindObservers();
            void nodesDidChange(const std::vector<Model::Node*>& nodes);
            void selectionDidChange(const Selection& selection);
        };
    }
}

