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

#include "UVView.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "FloatType.h"
#include "Assets/Texture.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceHandle.h"
#include "Model/Polyhedron.h"
#include "Renderer/ActiveShader.h"
#include "Renderer/Camera.h"
#include "Renderer/EdgeRenderer.h"
#include "Renderer/FaceRenderer.h" // for gridColorForTexture()
#include "Renderer/GLVertexType.h"
#include "Renderer/PrimType.h"
#include "Renderer/Renderable.h"
#include "Renderer/RenderBatch.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/Shaders.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/VboManager.h"
#include "Renderer/VertexArray.h"
#include "View/Grid.h"
#include "View/MapDocument.h"
#include "View/UVCameraTool.h"
#include "View/UVOffsetTool.h"
#include "View/UVRotateTool.h"
#include "View/UVScaleTool.h"
#include "View/UVShearTool.h"
#include "View/UVOriginTool.h"

#include <kdl/memory_utils.h>

#include <cassert>
#include <memory>
#include <vector>

namespace TrenchBroom {
    namespace View {
        const Model::HitType::Type UVView::FaceHitType = Model::HitType::freeType();

        UVView::UVView(std::weak_ptr<MapDocument> document, GLContextManager& contextManager) :
        RenderView(contextManager),
        m_document(document),
        m_helper(m_camera) {
            setToolBox(m_toolBox);
            createTools();
            m_toolBox.disable();
            connectObservers();
        }

        void UVView::setSubDivisions(const vm::vec2i& subDivisions) {
            m_helper.setSubDivisions(subDivisions);
            update();
        }

        void UVView::createTools() {
            addTool(std::make_unique<UVRotateTool>(m_document, m_helper));
            addTool(std::make_unique<UVOriginTool>(m_helper));
            addTool(std::make_unique<UVScaleTool>(m_document, m_helper));
            addTool(std::make_unique<UVShearTool>(m_document, m_helper));
            addTool(std::make_unique<UVOffsetTool>(m_document, m_helper));
            addTool(std::make_unique<UVCameraTool>(m_camera));
        }

        void UVView::connectObservers() {
            auto document = kdl::mem_lock(m_document);
            m_notifierConnection += document->documentWasClearedNotifier.connect(this, &UVView::documentWasCleared);
            m_notifierConnection += document->nodesDidChangeNotifier.connect(this, &UVView::nodesDidChange);
            m_notifierConnection += document->brushFacesDidChangeNotifier.connect(this, &UVView::brushFacesDidChange);
            m_notifierConnection += document->selectionDidChangeNotifier.connect(this, &UVView::selectionDidChange);
            m_notifierConnection += document->grid().gridDidChangeNotifier.connect(this, &UVView::gridDidChange);

            PreferenceManager& prefs = PreferenceManager::instance();
            m_notifierConnection += prefs.preferenceDidChangeNotifier.connect(this, &UVView::preferenceDidChange);

            m_notifierConnection += m_camera.cameraDidChangeNotifier.connect(this, &UVView::cameraDidChange);
        }

        void UVView::selectionDidChange(const Selection&) {
            auto document = kdl::mem_lock(m_document);
            const auto faces = document->selectedBrushFaces();
            if (faces.size() != 1) {
                m_helper.setFaceHandle(std::nullopt);
            } else {
                m_helper.setFaceHandle(faces.back());
            }

            if (m_helper.valid()) {
                m_toolBox.enable();
            } else {
                m_toolBox.disable();
            }

            update();
        }

        void UVView::documentWasCleared(MapDocument*) {
            m_helper.setFaceHandle(std::nullopt);
            m_toolBox.disable();
            update();
        }

        void UVView::nodesDidChange(const std::vector<Model::Node*>&) {
            update();
        }

        void UVView::brushFacesDidChange(const std::vector<Model::BrushFaceHandle>&) {
            update();
        }

        void UVView::gridDidChange() {
            update();
        }

        void UVView::preferenceDidChange(const IO::Path&) {
            update();
        }

        void UVView::cameraDidChange(const Renderer::Camera*) {
            update();
        }

        void UVView::doUpdateViewport(int x, int y, int width, int height) {
            if (m_camera.setViewport(Renderer::Camera::Viewport(x, y, width, height))) {
                m_helper.cameraViewportChanged();
            }
        }

        void UVView::doRender() {
            if (m_helper.valid()) {
                auto document = kdl::mem_lock(m_document);
                document->commitPendingAssets();

                Renderer::RenderContext renderContext(Renderer::RenderMode::Render2D, m_camera, fontManager(), shaderManager());
                Renderer::RenderBatch renderBatch(vboManager());

                setupGL(renderContext);
                renderTexture(renderContext, renderBatch);
                renderFace(renderContext, renderBatch);
                renderToolBox(renderContext, renderBatch);
                renderTextureAxes(renderContext, renderBatch);

                renderBatch.render(renderContext);
            }
        }

        bool UVView::doShouldRenderFocusIndicator() const {
            return false;
        }

        const Color& UVView::getBackgroundColor() {
            return pref(Preferences::BrowserBackgroundColor);
        }

        void UVView::setupGL(Renderer::RenderContext& renderContext) {
            const Renderer::Camera::Viewport& viewport = renderContext.camera().viewport();
            const qreal r = devicePixelRatioF();
            const int x = static_cast<int>(viewport.x * r);
            const int y = static_cast<int>(viewport.y * r);
            const int width = static_cast<int>(viewport.width * r);
            const int height = static_cast<int>(viewport.height * r);

            glAssert(glViewport(x, y, width, height))

            if (pref(Preferences::EnableMSAA)) {
                glAssert(glEnable(GL_MULTISAMPLE))
            } else {
                glAssert(glDisable(GL_MULTISAMPLE))
            }
            glAssert(glEnable(GL_BLEND))
            glAssert(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA))
            glAssert(glShadeModel(GL_SMOOTH))
            glAssert(glDisable(GL_DEPTH_TEST))
        }

        class UVView::RenderTexture : public Renderer::DirectRenderable {
        private:
            using Vertex = Renderer::GLVertexTypes::P3NT2::Vertex;

            const UVViewHelper& m_helper;
            Renderer::VertexArray m_vertexArray;
        public:
            RenderTexture(const UVViewHelper& helper) :
            m_helper(helper),
            m_vertexArray(Renderer::VertexArray::move(getVertices())) {}
        private:
            std::vector<Vertex> getVertices() const {
                const auto normal = vm::vec3f(m_helper.face()->boundary().normal);

                const auto& camera = m_helper.camera();
                const auto& v = camera.zoomedViewport();
                const auto w2 = static_cast<float>(v.width) / 2.0f;
                const auto h2 = static_cast<float>(v.height) / 2.0f;

                const auto& p = camera.position();
                const auto& r = camera.right();
                const auto& u = camera.up();

                const auto pos1 = -w2 * r +h2 * u + p;
                const auto pos2 = +w2 * r +h2 * u + p;
                const auto pos3 = +w2 * r -h2 * u + p;
                const auto pos4 = -w2 * r -h2 * u + p;

                return {
                    Vertex(pos1, normal, m_helper.face()->textureCoords(vm::vec3(pos1))),
                    Vertex(pos2, normal, m_helper.face()->textureCoords(vm::vec3(pos2))),
                    Vertex(pos3, normal, m_helper.face()->textureCoords(vm::vec3(pos3))),
                    Vertex(pos4, normal, m_helper.face()->textureCoords(vm::vec3(pos4)))
                };
            }
        private:
            void doPrepareVertices(Renderer::VboManager& vboManager) override {
                m_vertexArray.prepare(vboManager);
            }

            void doRender(Renderer::RenderContext& renderContext) override {
                const auto& offset = m_helper.face()->attributes().offset();
                const auto& scale = m_helper.face()->attributes().scale();
                const auto toTex = m_helper.face()->toTexCoordSystemMatrix(offset, scale, true);

                const auto* texture = m_helper.face()->texture();
                ensure(texture != nullptr, "texture is null");

                texture->activate();

                Renderer::ActiveShader shader(renderContext.shaderManager(), Renderer::Shaders::UVViewShader);
                shader.set("ApplyTexture", true);
                shader.set("Color", texture->averageColor());
                shader.set("Brightness", pref(Preferences::Brightness));
                shader.set("RenderGrid", true);
                shader.set("GridSizes", vm::vec2f(texture->width(), texture->height()));
                shader.set("GridColor", vm::vec4f(Renderer::gridColorForTexture(texture), 0.6f)); // TODO: make this a preference
                shader.set("GridScales", scale);
                shader.set("GridMatrix", vm::mat4x4f(toTex));
                shader.set("GridDivider", vm::vec2f(m_helper.subDivisions()));
                shader.set("CameraZoom", m_helper.cameraZoom());
                shader.set("Texture", 0);

                m_vertexArray.render(Renderer::PrimType::Quads);

                texture->deactivate();
            }
        };

        void UVView::renderTexture(Renderer::RenderContext&, Renderer::RenderBatch& renderBatch) {
            const Assets::Texture* texture = m_helper.face()->texture();
            if (texture == nullptr)
                return;

            renderBatch.addOneShot(new RenderTexture(m_helper));
        }

        void UVView::renderFace(Renderer::RenderContext&, Renderer::RenderBatch& renderBatch) {
            assert(m_helper.valid());

            const auto faceVertices = m_helper.face()->vertices();

            using Vertex = Renderer::GLVertexTypes::P3::Vertex;
            std::vector<Vertex> edgeVertices;
            edgeVertices.reserve(faceVertices.size());

            for (const auto* vertex : faceVertices) {
                edgeVertices.push_back(Vertex(vm::vec3f(vertex->position())));
            }

            const Color edgeColor(1.0f, 1.0f, 1.0f, 1.0f); // TODO: make this a preference

            Renderer::DirectEdgeRenderer edgeRenderer(Renderer::VertexArray::move(std::move(edgeVertices)), Renderer::PrimType::LineLoop);
            edgeRenderer.renderOnTop(renderBatch, edgeColor, 2.5f);
        }

        void UVView::renderTextureAxes(Renderer::RenderContext&, Renderer::RenderBatch& renderBatch) {
            assert(m_helper.valid());

            const auto& normal = m_helper.face()->boundary().normal;

            const auto xAxis  = vm::vec3f(m_helper.face()->textureXAxis() - dot(m_helper.face()->textureXAxis(), normal) * normal);
            const auto yAxis  = vm::vec3f(m_helper.face()->textureYAxis() - dot(m_helper.face()->textureYAxis(), normal) * normal);
            const auto center = vm::vec3f(m_helper.face()->boundsCenter());

            const auto length = 32.0f / m_helper.cameraZoom();

            using Vertex = Renderer::GLVertexTypes::P3C4::Vertex;
            Renderer::DirectEdgeRenderer edgeRenderer(Renderer::VertexArray::move(std::vector<Vertex>({
                Vertex(center, pref(Preferences::XAxisColor)),
                Vertex(center + length * xAxis, pref(Preferences::XAxisColor)),
                Vertex(center, pref(Preferences::YAxisColor)),
                Vertex(center + length * yAxis, pref(Preferences::YAxisColor)),
            })), Renderer::PrimType::Lines);
            edgeRenderer.renderOnTop(renderBatch, 2.0f);
        }

        void UVView::renderToolBox(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            renderTools(renderContext, renderBatch);
        }

        void UVView::processEvent(const KeyEvent& event) {
            ToolBoxConnector::processEvent(event);
        }

        void UVView::processEvent(const MouseEvent& event) {
            ToolBoxConnector::processEvent(event);
        }

        void UVView::processEvent(const CancelEvent& event) {
            ToolBoxConnector::processEvent(event);
        }

        PickRequest UVView::doGetPickRequest(const float x, const float y) const {
            return PickRequest(vm::ray3(m_camera.pickRay(x, y)), m_camera);
        }

        Model::PickResult UVView::doPick(const vm::ray3& pickRay) const {
            Model::PickResult pickResult = Model::PickResult::byDistance();
            if (!m_helper.valid())
                return pickResult;

            const FloatType distance = m_helper.face()->intersectWithRay(pickRay);
            if (!vm::is_nan(distance)) {
                const vm::vec3 hitPoint = vm::point_at_distance(pickRay, distance);
                pickResult.addHit(Model::Hit(UVView::FaceHitType, distance, hitPoint, m_helper.face()));
            }
            return pickResult;
        }
    }
}
