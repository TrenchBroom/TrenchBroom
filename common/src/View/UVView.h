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

#ifndef TrenchBroom_UVView
#define TrenchBroom_UVView

#include "FloatType.h"
#include "Model/HitType.h"
#include "Model/PickResult.h"
#include "Renderer/OrthographicCamera.h"
#include "View/RenderView.h"
#include "View/ToolBox.h"
#include "View/ToolBoxConnector.h"
#include "View/UVViewHelper.h"

#include <memory>
#include <vector>

class QWidget;

namespace TrenchBroom {
    namespace IO {
        class Path;
    }

    namespace Model {
        class Node;
    }

    namespace Renderer {
        class ActiveShader;
        class RenderBatch;
        class RenderContext;
    }

    namespace View {
        class MapDocument;
        class Selection;
        class UVRotateTool;
        class UVOriginTool;
        class UVScaleTool;
        class UVShearTool;
        class UVOffsetTool;
        class UVCameraTool;

        /**
         A view which allows the user to manipulate the texture projection interactively with the mouse. The user can
         change texture offsets, scaling factors and rotation. If supported by the map format, the user can manipulate
         the texture axes as well.
         */
        class UVView : public RenderView, public ToolBoxConnector {
            Q_OBJECT
        public:
            static const Model::HitType::Type FaceHitType;
        private:
            std::weak_ptr<MapDocument> m_document;

            Renderer::OrthographicCamera m_camera;
            UVViewHelper m_helper;

            ToolBox m_toolBox;
        public:
            UVView(std::weak_ptr<MapDocument> document, GLContextManager& contextManager);
            ~UVView() override;

            void setSubDivisions(const vm::vec2i& subDivisions);
        private:
            void createTools();

            void bindObservers();
            void unbindObservers();

            void selectionDidChange(const Selection& selection);
            void documentWasCleared(MapDocument* document);
            void nodesDidChange(const std::vector<Model::Node*>& nodes);
            void brushFacesDidChange(const std::vector<Model::BrushFace*>& faces);
            void gridDidChange();
            void cameraDidChange(const Renderer::Camera* camera);
            void preferenceDidChange(const IO::Path& path);

            void doUpdateViewport(int x, int y, int width, int height) override;
            void doRender() override;
            bool doShouldRenderFocusIndicator() const override;

            void setupGL(Renderer::RenderContext& renderContext);

            class RenderTexture;
            void renderTexture(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);

            void renderFace(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            void renderTextureAxes(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
            void renderToolBox(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
        public: // implement InputEventProcessor interface
            void processEvent(const KeyEvent& event) override;
            void processEvent(const MouseEvent& event) override;
            void processEvent(const CancelEvent& event) override;
        private:
            PickRequest doGetPickRequest(int x, int y) const override;
            Model::PickResult doPick(const vm::ray3& pickRay) const override;
        };
    }
}

#endif /* defined(TrenchBroom_UVView) */
