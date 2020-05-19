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

#ifndef TrenchBroom_ClipTool
#define TrenchBroom_ClipTool

#include "FloatType.h"
#include "Model/HitType.h"
#include "View/Tool.h"

#include <map>
#include <memory>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class BrushFace;
        class BrushFaceHandle;
        class Node;
        class PickResult;
    }

    namespace Renderer {
        class BrushRenderer;
        class Camera;
        class RenderBatch;
        class RenderContext;
    }

    namespace View {
        class Grid;
        class MapDocument;
        class Selection;

        class ClipTool : public Tool {
        public:
            static const Model::HitType::Type PointHitType;
        private:
            enum ClipSide {
                ClipSide_Front,
                ClipSide_Both,
                ClipSide_Back
            };

            class ClipStrategy {
            public:
                virtual ~ClipStrategy();
                void pick(const vm::ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) const;
                void render(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Model::PickResult& pickResult);
                void renderFeedback(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const vm::vec3& point) const;

                bool computeThirdPoint(vm::vec3& point) const;

                bool canClip() const;
                bool hasPoints() const;
                bool canAddPoint(const vm::vec3& point) const;
                void addPoint(const vm::vec3& point, const std::vector<vm::vec3>& helpVectors);
                bool canRemoveLastPoint() const;
                void removeLastPoint();

                bool canDragPoint(const Model::PickResult& pickResult, vm::vec3& initialPosition) const;
                void beginDragPoint(const Model::PickResult& pickResult);
                void beginDragLastPoint();
                bool dragPoint(const vm::vec3& newPosition, const std::vector<vm::vec3>& helpVectors);
                void endDragPoint();
                void cancelDragPoint();

                bool setFace(const Model::BrushFaceHandle& faceHandle);
                void reset();
                size_t getPoints(vm::vec3& point1, vm::vec3& point2, vm::vec3& point3) const;
            private:
                virtual void doPick(const vm::ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) const = 0;
                virtual void doRender(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Model::PickResult& pickResult) = 0;

                virtual void doRenderFeedback(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const vm::vec3& point) const = 0;

                virtual bool doComputeThirdPoint(vm::vec3& point) const = 0;

                virtual bool doCanClip() const = 0;
                virtual bool doHasPoints() const = 0;
                virtual bool doCanAddPoint(const vm::vec3& point) const = 0;
                virtual void doAddPoint(const vm::vec3& point, const std::vector<vm::vec3>& helpVectors) = 0;
                virtual bool doCanRemoveLastPoint() const = 0;
                virtual void doRemoveLastPoint() = 0;

                virtual bool doCanDragPoint(const Model::PickResult& pickResult, vm::vec3& initialPosition) const = 0;
                virtual void doBeginDragPoint(const Model::PickResult& pickResult) = 0;
                virtual void doBeginDragLastPoint() = 0;
                virtual bool doDragPoint(const vm::vec3& newPosition, const std::vector<vm::vec3>& helpVectors) = 0;
                virtual void doEndDragPoint() = 0;
                virtual void doCancelDragPoint() = 0;

                virtual bool doSetFace(const Model::BrushFaceHandle& face) = 0;
                virtual void doReset() = 0;
                virtual size_t doGetPoints(vm::vec3& point1, vm::vec3& point2, vm::vec3& point3) const = 0;
            };

            class PointClipStrategy;
            class FaceClipStrategy;
        private:
            std::weak_ptr<MapDocument> m_document;

            ClipSide m_clipSide;
            std::unique_ptr<ClipStrategy> m_strategy;

            std::map<Model::Node*, std::vector<Model::Node*>> m_frontBrushes;
            std::map<Model::Node*, std::vector<Model::Node*>> m_backBrushes;

            std::unique_ptr<Renderer::BrushRenderer> m_remainingBrushRenderer;
            std::unique_ptr<Renderer::BrushRenderer> m_clippedBrushRenderer;

            bool m_ignoreNotifications;
            bool m_dragging;
        public:
            explicit ClipTool(std::weak_ptr<MapDocument> document);
            ~ClipTool() override;

            const Grid& grid() const;

            void toggleSide();

            void pick(const vm::ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult);

            void render(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Model::PickResult& pickResult);
        private:
            void renderBrushes(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            void renderStrategy(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const Model::PickResult& pickResult);
        public:
            void renderFeedback(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const vm::vec3& point) const;
        public:
            bool hasBrushes() const;
            bool canClip() const;
            void performClip();
        private:
            std::map<Model::Node*, std::vector<Model::Node*>> clipBrushes();
        public:

            vm::vec3 defaultClipPointPos() const;

            bool canAddPoint(const vm::vec3& point) const;
            bool hasPoints() const;
            void addPoint(const vm::vec3& point, const std::vector<vm::vec3>& helpVectors);
            bool canRemoveLastPoint() const;
            bool removeLastPoint();

            bool beginDragPoint(const Model::PickResult& pickResult, vm::vec3& initialPosition);
            void beginDragLastPoint();
            bool dragPoint(const vm::vec3& newPosition, const std::vector<vm::vec3>& helpVectors);
            void endDragPoint();
            void cancelDragPoint();

            void setFace(const Model::BrushFaceHandle& face);
            bool reset();
        private:
            void resetStrategy();
            void update();

            void clearBrushes();
            void updateBrushes();

            void setFaceAttributes(const std::vector<Model::BrushFace>& faces, Model::BrushFace& frontFace, Model::BrushFace& backFace) const;

            void clearRenderers();
            void updateRenderers();
            void addBrushesToRenderer(const std::map<Model::Node*, std::vector<Model::Node*>>& map, Renderer::BrushRenderer& renderer);

            bool keepFrontBrushes() const;
            bool keepBackBrushes() const;
        private:
            bool doActivate() override;
            bool doDeactivate() override;

            bool doRemove();

            void bindObservers();
            void unbindObservers();
            void selectionDidChange(const Selection& selection);
            void nodesWillChange(const std::vector<Model::Node*>& nodes);
            void nodesDidChange(const std::vector<Model::Node*>& nodes);
            void brushFacesDidChange(const std::vector<Model::BrushFaceHandle>& nodes);
        };
    }
}

#endif /* defined(TrenchBroom_ClipTool) */
