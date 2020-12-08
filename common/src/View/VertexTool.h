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

#include "Renderer/PointGuideRenderer.h"
#include "View/VertexToolBase.h"
#include "View/VertexHandleManager.h"

#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class PickResult;
    }

    namespace Renderer {
        class Camera;
        class RenderContext;
        class RenderBatch;
        class RenderService;
    }

    namespace View {
        class BrushVertexCommandBase;
        class Grid;
        class Lasso;
        class Selection;
        class VertexCommand;

        class VertexTool : public VertexToolBase<vm::vec3> {
        private:
            typedef enum {
                Mode_Move,
                Mode_Split_Edge,
                Mode_Split_Face
            } Mode;

            Mode m_mode;

            std::unique_ptr<VertexHandleManager> m_vertexHandles;
            std::unique_ptr<EdgeHandleManager> m_edgeHandles;
            std::unique_ptr<FaceHandleManager> m_faceHandles;

            mutable Renderer::PointGuideRenderer m_guideRenderer;
        public:
            explicit VertexTool(const std::weak_ptr<MapDocument>& document);
        public:
            std::vector<Model::BrushNode*> findIncidentBrushes(const vm::vec3& handle) const;
            std::vector<Model::BrushNode*> findIncidentBrushes(const vm::segment3& handle) const;
            std::vector<Model::BrushNode*> findIncidentBrushes(const vm::polygon3& handle) const;
        private:
            using VertexToolBase::findIncidentBrushes;
        public:
            void pick(const vm::ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) const override;
        public: // Handle selection
            bool deselectAll() override;
        public:
            VertexHandleManager& handleManager() override;
            const VertexHandleManager& handleManager() const override;
        public: // Vertex moving
            bool startMove(const std::vector<Model::Hit>& hits) override;
            MoveResult move(const vm::vec3& delta) override;
            void endMove() override;
            void cancelMove() override;

            vm::vec3 getHandlePosition(const Model::Hit& hit) const override;
            std::string actionName() const override;

            void removeSelection();
        public: // Rendering
            void renderGuide(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const vm::vec3& position) const override;
        private: // Tool interface
            bool doActivate() override;
            bool doDeactivate() override;
        private:
            void addHandles(const std::vector<Model::Node*>& nodes) override;
            void removeHandles(const std::vector<Model::Node*>& nodes) override;

            void addHandles(VertexCommand* command) override;
            void removeHandles(VertexCommand* command) override;

            void addHandles(BrushVertexCommandBase* command) override;
            void removeHandles(BrushVertexCommandBase* command) override;
        private: // General helper methods
            void resetModeAfterDeselection();
        };
    }
}

