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

#include "TrenchBroom.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Assets/Texture.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Renderer/Camera.h"
#include "Renderer/EdgeRenderer.h"
#include "Renderer/Renderable.h"
#include "Renderer/RenderBatch.h"
#include "Renderer/RenderContext.h"
#include "Renderer/Shaders.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Vbo.h"
#include "Renderer/VertexArray.h"
#include "Renderer/VertexSpec.h"
#include "View/GLAttribs.h"
#include "View/Grid.h"
#include "View/MapDocument.h"
#include "View/UVCameraTool.h"
#include "View/UVOffsetTool.h"
#include "View/UVRotateTool.h"
#include "View/UVScaleTool.h"
#include "View/UVShearTool.h"
#include "View/UVOriginTool.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>

namespace TrenchBroom {
    namespace View {
        const Model::Hit::HitType UVView::FaceHit = Model::Hit::freeHitType();
        
        UVView::UVView(wxWindow* parent, MapDocumentWPtr document, GLContextManager& contextManager) :
        RenderView(parent, contextManager, GLAttribs::attribs()),
        ToolBoxConnector(this),
        m_document(document),
        m_helper(m_camera) {
            setToolBox(m_toolBox);
            m_toolBox.setClickToActivate(false);
            createTools();
            m_toolBox.disable();
            bindObservers();
        }
        
        UVView::~UVView() {
            unbindObservers();
        }

        void UVView::setSubDivisions(const vm::vec2i& subDivisions) {
            m_helper.setSubDivisions(subDivisions);
            Refresh();
        }

        void UVView::createTools() {
            addTool(new UVRotateTool(m_document, m_helper));
            addTool(new UVOriginTool(m_helper));
            addTool(new UVScaleTool(m_document, m_helper));
            addTool(new UVShearTool(m_document, m_helper));
            addTool(new UVOffsetTool(m_document, m_helper));
            addTool(new UVCameraTool(m_camera));
        }

        void UVView::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->documentWasClearedNotifier.addObserver(this, &UVView::documentWasCleared);
            document->nodesDidChangeNotifier.addObserver(this, &UVView::nodesDidChange);
            document->brushFacesDidChangeNotifier.addObserver(this, &UVView::brushFacesDidChange);
            document->selectionDidChangeNotifier.addObserver(this, &UVView::selectionDidChange);
            document->grid().gridDidChangeNotifier.addObserver(this, &UVView::gridDidChange);
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.addObserver(this, &UVView::preferenceDidChange);
            
            m_camera.cameraDidChangeNotifier.addObserver(this, &UVView::cameraDidChange);
        }
        
        void UVView::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->documentWasClearedNotifier.removeObserver(this, &UVView::documentWasCleared);
                document->nodesDidChangeNotifier.removeObserver(this, &UVView::nodesDidChange);
                document->brushFacesDidChangeNotifier.removeObserver(this, &UVView::brushFacesDidChange);
                document->selectionDidChangeNotifier.removeObserver(this, &UVView::selectionDidChange);
                document->grid().gridDidChangeNotifier.removeObserver(this, &UVView::gridDidChange);
            }
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.removeObserver(this, &UVView::preferenceDidChange);

            m_camera.cameraDidChangeNotifier.removeObserver(this, &UVView::cameraDidChange);
        }
        
        void UVView::selectionDidChange(const Selection& selection) {
            MapDocumentSPtr document = lock(m_document);
            const Model::BrushFaceList& faces = document->selectedBrushFaces();
            if (faces.size() != 1)
                m_helper.setFace(nullptr);
            else
                m_helper.setFace(faces.back());

            if (m_helper.valid())
                m_toolBox.enable();
            else
                m_toolBox.disable();
            Refresh();
        }
        
        void UVView::documentWasCleared(MapDocument* document) {
            m_helper.setFace(nullptr);
            m_toolBox.disable();
            Refresh();
        }
        
        void UVView::nodesDidChange(const Model::NodeList& nodes) {
            Refresh();
        }

        void UVView::brushFacesDidChange(const Model::BrushFaceList& faces) {
            Refresh();
        }

        void UVView::gridDidChange() {
            Refresh();
        }

        void UVView::preferenceDidChange(const IO::Path& path) {
            Refresh();
        }

        void UVView::cameraDidChange(const Renderer::Camera* camera) {
            Refresh();
        }

        void UVView::doUpdateViewport(int x, int y, int width, int height) {
            if (m_camera.setViewport(Renderer::Camera::Viewport(x, y, width, height))) {
                m_helper.cameraViewportChanged();
            }
        }
        
        void UVView::doRender() {
            if (m_helper.valid()) {
                MapDocumentSPtr document = lock(m_document);
                document->commitPendingAssets();
                
                Renderer::RenderContext renderContext(Renderer::RenderContext::RenderMode_2D, m_camera, fontManager(), shaderManager());
                Renderer::RenderBatch renderBatch(vertexVbo(), indexVbo());
                
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

        void UVView::setupGL(Renderer::RenderContext& renderContext) {
            const Renderer::Camera::Viewport& viewport = renderContext.camera().unzoomedViewport();
            glAssert(glViewport(viewport.x, viewport.y, viewport.width, viewport.height));
            
            glAssert(glEnable(GL_MULTISAMPLE));
            glAssert(glEnable(GL_BLEND));
            glAssert(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
            glAssert(glShadeModel(GL_SMOOTH));
            glAssert(glDisable(GL_DEPTH_TEST));
        }

        class UVView::RenderTexture : public Renderer::DirectRenderable {
        private:
            typedef Renderer::VertexSpecs::P3NT2::Vertex Vertex;
            
            const UVViewHelper& m_helper;
            Renderer::VertexArray m_vertexArray;
        public:
            RenderTexture(const UVViewHelper& helper) :
            m_helper(helper) {
                Vertex::List vertices = getVertices();
                m_vertexArray = Renderer::VertexArray::swap(vertices);
            }
        private:
            Vertex::List getVertices() {
                const auto* face = m_helper.face();
                const auto normal = vm::vec3f(face->boundary().normal);
                
                Vertex::List vertices;
                vertices.reserve(4);
                
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
                
                vertices.push_back(Vertex(pos1, normal, face->textureCoords(vm::vec3(pos1))));
                vertices.push_back(Vertex(pos2, normal, face->textureCoords(vm::vec3(pos2))));
                vertices.push_back(Vertex(pos3, normal, face->textureCoords(vm::vec3(pos3))));
                vertices.push_back(Vertex(pos4, normal, face->textureCoords(vm::vec3(pos4))));
                
                return vertices;
            }
        private:
            void doPrepareVertices(Renderer::Vbo& vertexVbo) override {
                m_vertexArray.prepare(vertexVbo);
            }
            
            void doRender(Renderer::RenderContext& renderContext) override {
                const auto* face = m_helper.face();
                const auto& offset = face->offset();
                const auto& scale = face->scale();
                const auto toTex = face->toTexCoordSystemMatrix(offset, scale, true);

                const auto* texture = face->texture();
                ensure(texture != nullptr, "texture is null");

                texture->activate();
                
                Renderer::ActiveShader shader(renderContext.shaderManager(), Renderer::Shaders::UVViewShader);
                shader.set("ApplyTexture", true);
                shader.set("Color", texture->averageColor());
                shader.set("Brightness", pref(Preferences::Brightness));
                shader.set("RenderGrid", true);
                shader.set("GridSizes", vm::vec2f(texture->width(), texture->height()));
                shader.set("GridColor", Color(0.6f, 0.6f, 0.6f, 1.0f)); // TODO: make this a preference
                shader.set("GridScales", scale);
                shader.set("GridMatrix", vm::mat4x4f(toTex));
                shader.set("GridDivider", vm::vec2f(m_helper.subDivisions()));
                shader.set("CameraZoom", m_helper.cameraZoom());
                shader.set("Texture", 0);
                
                m_vertexArray.render(GL_QUADS);
                
                texture->deactivate();
            }
        };
        
        void UVView::renderTexture(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            const Model::BrushFace* face = m_helper.face();
            const Assets::Texture* texture = face->texture();
            if (texture == nullptr)
                return;

            renderBatch.addOneShot(new RenderTexture(m_helper));
        }
        
        void UVView::renderFace(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            assert(m_helper.valid()); 
            
            const auto* face = m_helper.face();
            const auto faceVertices = face->vertices();
            
            using Vertex = Renderer::VertexSpecs::P3::Vertex;
            Vertex::List edgeVertices;
            edgeVertices.reserve(faceVertices.size());
            
            std::transform(std::begin(faceVertices), std::end(faceVertices), std::back_inserter(edgeVertices),
                           [](const Model::BrushVertex* vertex) { return Vertex(vm::vec3f(vertex->position())); });
            
            const Color edgeColor(1.0f, 1.0f, 1.0f, 1.0f); // TODO: make this a preference
            
            Renderer::DirectEdgeRenderer edgeRenderer(Renderer::VertexArray::swap(edgeVertices), GL_LINE_LOOP);
            edgeRenderer.renderOnTop(renderBatch, edgeColor, 2.5f);
        }

        void UVView::renderTextureAxes(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            assert(m_helper.valid());
            
            const auto* face = m_helper.face();
            const auto& normal = face->boundary().normal;
            
            const auto xAxis  = vm::vec3f(face->textureXAxis() - dot(face->textureXAxis(), normal) * normal);
            const auto yAxis  = vm::vec3f(face->textureYAxis() - dot(face->textureYAxis(), normal) * normal);
            const auto center = vm::vec3f(face->boundsCenter());
            
            typedef Renderer::VertexSpecs::P3C4::Vertex Vertex;
            Vertex::List vertices;
            vertices.reserve(4);
            
            const auto length = 32.0f / m_helper.cameraZoom();
            
            vertices.push_back(Vertex(center, pref(Preferences::XAxisColor)));
            vertices.push_back(Vertex(center + length * xAxis, pref(Preferences::XAxisColor)));
            vertices.push_back(Vertex(center, pref(Preferences::YAxisColor)));
            vertices.push_back(Vertex(center + length * yAxis, pref(Preferences::YAxisColor)));
            
            Renderer::DirectEdgeRenderer edgeRenderer(Renderer::VertexArray::swap(vertices), GL_LINES);
            edgeRenderer.renderOnTop(renderBatch, 2.0f);
        }

        void UVView::renderToolBox(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            renderTools(renderContext, renderBatch);
        }

        PickRequest UVView::doGetPickRequest(const int x, const int y) const {
            return PickRequest(vm::ray3(m_camera.pickRay(x, y)), m_camera);
        }
        
        Model::PickResult UVView::doPick(const vm::ray3& pickRay) const {
            Model::PickResult pickResult = Model::PickResult::byDistance(lock(m_document)->editorContext());
            if (!m_helper.valid())
                return pickResult;
            
            Model::BrushFace* face = m_helper.face();
            const FloatType distance = face->intersectWithRay(pickRay);
            if (!vm::isNan(distance)) {
                const vm::vec3 hitPoint = pickRay.pointAtDistance(distance);
                pickResult.addHit(Model::Hit(UVView::FaceHit, distance, hitPoint, face));
            }
            return pickResult;
        }
    }
}
