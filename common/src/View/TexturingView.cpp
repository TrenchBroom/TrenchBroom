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

#include "TexturingView.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Hit.h"
#include "Assets/Texture.h"
#include "Model/BrushFace.h"
#include "Model/BrushVertex.h"
#include "Renderer/Camera.h"
#include "Renderer/EdgeRenderer.h"
#include "Renderer/RenderContext.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Vbo.h"
#include "Renderer/VertexArray.h"
#include "Renderer/VertexSpec.h"
#include "View/Grid.h"
#include "View/MapDocument.h"
#include "View/TexturingViewCameraTool.h"
#include "View/TexturingViewOffsetTool.h"
#include "View/TexturingViewRotateTool.h"
#include "View/TexturingViewScaleTool.h"
#include "View/TexturingViewOriginTool.h"

#include <cassert>
#include <iostream>

namespace TrenchBroom {
    namespace View {
        const Hit::HitType TexturingView::FaceHit = Hit::freeHitType();
        
        TexturingView::TexturingView(wxWindow* parent, GLContextHolder::Ptr sharedContext, MapDocumentWPtr document, ControllerWPtr controller) :
        RenderView(parent, sharedContext),
        m_document(document),
        m_controller(controller),
        m_helper(m_camera),
        m_toolBox(this, this),
        m_rotateTool(NULL),
        m_originTool(NULL),
        m_scaleTool(NULL),
        m_offsetTool(NULL),
        m_cameraTool(NULL) {
            m_toolBox.setClickToActivate(false);
            createTools();
            m_toolBox.disable();
            bindObservers();
        }
        
        TexturingView::~TexturingView() {
            unbindObservers();
            destroyTools();
        }

        void TexturingView::setSubDivisions(const Vec2i& subDivisions) {
            m_helper.setSubDivisions(subDivisions);
            Refresh();
        }

        void TexturingView::createTools() {
            m_rotateTool = new TexturingViewRotateTool(m_document, m_controller, m_helper);
            m_originTool = new TexturingViewOriginTool(m_document, m_controller, m_helper);
            m_scaleTool = new TexturingViewScaleTool(m_document, m_controller, m_helper);
            m_offsetTool = new TexturingViewOffsetTool(m_document, m_controller, m_helper);
            m_cameraTool = new TexturingViewCameraTool(m_document, m_controller, m_camera);

            m_toolBox.addTool(m_rotateTool);
            m_toolBox.addTool(m_originTool);
            m_toolBox.addTool(m_scaleTool);
            m_toolBox.addTool(m_offsetTool);
            m_toolBox.addTool(m_cameraTool);
        }
        
        void TexturingView::destroyTools() {
            delete m_cameraTool;
            m_cameraTool = NULL;
            delete m_offsetTool;
            m_offsetTool = NULL;
            delete m_originTool;
            m_originTool = NULL;
            delete m_scaleTool;
            m_scaleTool = NULL;
            delete m_rotateTool;
            m_rotateTool = NULL;
        }

        void TexturingView::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->objectDidChangeNotifier.addObserver(this, &TexturingView::objectDidChange);
            document->faceDidChangeNotifier.addObserver(this, &TexturingView::faceDidChange);
            document->selectionDidChangeNotifier.addObserver(this, &TexturingView::selectionDidChange);
            document->grid().gridDidChangeNotifier.addObserver(this, &TexturingView::gridDidChange);
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.addObserver(this, &TexturingView::preferenceDidChange);
            
            m_camera.cameraDidChangeNotifier.addObserver(this, &TexturingView::cameraDidChange);
        }
        
        void TexturingView::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->objectDidChangeNotifier.removeObserver(this, &TexturingView::objectDidChange);
                document->faceDidChangeNotifier.removeObserver(this, &TexturingView::faceDidChange);
                document->selectionDidChangeNotifier.removeObserver(this, &TexturingView::selectionDidChange);
                document->grid().gridDidChangeNotifier.removeObserver(this, &TexturingView::gridDidChange);
            }
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.removeObserver(this, &TexturingView::preferenceDidChange);

            m_camera.cameraDidChangeNotifier.removeObserver(this, &TexturingView::cameraDidChange);
        }
        
        void TexturingView::selectionDidChange(const Model::SelectionResult& result) {
            MapDocumentSPtr document = lock(m_document);
            const Model::BrushFaceList& faces = document->selectedFaces();
            if (faces.empty())
                m_helper.setFace(NULL);
            else
                m_helper.setFace(faces.back());
            
            if (m_helper.valid()) {
                m_toolBox.enable();
                m_helper.resetCamera();
            } else {
                m_toolBox.disable();
            }
            Refresh();
        }
        
        void TexturingView::objectDidChange(Model::Object* object) {
            Refresh();
        }

        void TexturingView::faceDidChange(Model::BrushFace* face) {
            Refresh();
        }

        void TexturingView::gridDidChange() {
            Refresh();
        }

        void TexturingView::preferenceDidChange(const IO::Path& path) {
            Refresh();
        }

        void TexturingView::cameraDidChange(const Renderer::Camera* camera) {
            Refresh();
        }

        void TexturingView::doUpdateViewport(int x, int y, int width, int height) {
            m_camera.setViewport(Renderer::Camera::Viewport(x, y, width, height));
        }
        
        void TexturingView::doRender() {
            if (m_helper.valid()) {
                MapDocumentSPtr document = lock(m_document);
                document->commitPendingRenderStateChanges();
                
                const View::Grid& grid = document->grid();
                Renderer::RenderContext renderContext(m_camera, contextHolder()->shaderManager(), grid.visible(), grid.actualSize());
                
                setupGL(renderContext);
                renderTexture(renderContext);
                renderFace(renderContext);
                renderToolBox(renderContext);
            }
        }
        
        void TexturingView::setupGL(Renderer::RenderContext& renderContext) {
            const Renderer::Camera::Viewport& viewport = renderContext.camera().viewport();
            glViewport(viewport.x, viewport.y, viewport.width, viewport.height);
            
            glEnable(GL_MULTISAMPLE);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glShadeModel(GL_SMOOTH);
            glDisable(GL_DEPTH_TEST);
        }

        void TexturingView::renderTexture(Renderer::RenderContext& renderContext) {
            const Model::BrushFace* face = m_helper.face();
            const Assets::Texture* texture = face->texture();
            if (texture == NULL)
                return;
            
            const Vec3f::List positions = getTextureQuad();
            const Vec3f normal(face->boundary().normal);
            
            typedef Renderer::VertexSpecs::P3NT2::Vertex Vertex;
            Vertex::List vertices(positions.size());
            
            for (size_t i = 0; i < positions.size(); ++i)
                vertices[i] = Vertex(positions[i],
                                     normal,
                                     face->textureCoords(positions[i]));
            
            Renderer::VertexArray vertexArray = Renderer::VertexArray::swap(GL_QUADS, vertices);
            
            const Vec2f& offset = face->offset();
            const Vec2f& scale = face->scale();
            const Mat4x4 toTex = face->toTexCoordSystemMatrix(offset, scale, true);
            
            Renderer::SetVboState setVboState(m_vbo);
            setVboState.mapped();
            vertexArray.prepare(m_vbo);
            setVboState.active();
            
            PreferenceManager& prefs = PreferenceManager::instance();
            
            Renderer::ActiveShader shader(renderContext.shaderManager(), Renderer::Shaders::TexturingViewShader);
            shader.set("Brightness", prefs.get(Preferences::Brightness));
            shader.set("RenderGrid", true);
            shader.set("GridSizes", Vec2f(texture->width(), texture->height()));
            shader.set("GridColor", Color(1.0f, 1.0f, 0.0f, 1.0f));
            shader.set("GridScales", scale);
            shader.set("GridMatrix", toTex);
            shader.set("GridDivider", Vec2f(m_helper.subDivisions()));
            shader.set("CameraZoom", m_camera.zoom().x());
            shader.set("Texture", 0);
            
            activateTexture(shader);
            vertexArray.render();
            deactivateTexture();
        }
        
        Vec3f::List TexturingView::getTextureQuad() const {
            Vec3f::List vertices(4);
            
            const Renderer::Camera::Viewport& v = m_camera.viewport();
            const Vec2f& z = m_camera.zoom();
            const float w2 = static_cast<float>(v.width)  / z.x() / 2.0f;
            const float h2 = static_cast<float>(v.height) / z.y() / 2.0f;
            
            const Vec3f& p = m_camera.position();
            const Vec3f& r = m_camera.right();
            const Vec3f& u = m_camera.up();
            
            vertices[0] = -w2 * r +h2 * u + p;
            vertices[1] = +w2 * r +h2 * u + p;
            vertices[2] = +w2 * r -h2 * u + p;
            vertices[3] = -w2 * r -h2 * u + p;
            
            return vertices;
        }
        
        void TexturingView::activateTexture(Renderer::ActiveShader& shader) {
            const Model::BrushFace* face = m_helper.face();
            Assets::Texture* texture = face->texture();
            if (texture != NULL) {
                shader.set("ApplyTexture", true);
                shader.set("Color", texture->averageColor());
                texture->activate();
            } else {
                PreferenceManager& prefs = PreferenceManager::instance();
                shader.set("ApplyTexture", false);
                shader.set("Color", prefs.get(Preferences::FaceColor));
            }
        }
        
        void TexturingView::deactivateTexture() {
            const Model::BrushFace* face = m_helper.face();
            Assets::Texture* texture = face->texture();
            if (texture != NULL)
                texture->deactivate();
        }
        
        void TexturingView::renderFace(Renderer::RenderContext& renderContext) {
            assert(m_helper.valid()); 
            
            const Model::BrushFace* face = m_helper.face();
            const Model::BrushVertexList& faceVertices = face->vertices();
            const size_t count = faceVertices.size();
            
            typedef Renderer::VertexSpecs::P3::Vertex Vertex;
            Vertex::List edgeVertices(count);
            
            for (size_t i = 0; i < count; ++i)
                edgeVertices[i] = Vertex(faceVertices[i]->position);
            
            Renderer::EdgeRenderer edgeRenderer(Renderer::VertexArray::swap(GL_LINE_LOOP, edgeVertices));
            edgeRenderer.setUseColor(true);
            edgeRenderer.setColor(Color(1.0f, 1.0f, 1.0f, 0.8f));
            
            glLineWidth(2.0f);
            edgeRenderer.render(renderContext);
            glLineWidth(1.0f);
        }

        void TexturingView::renderToolBox(Renderer::RenderContext& renderContext) {
            m_toolBox.renderTools(renderContext);
        }

        Ray3 TexturingView::doGetPickRay(const int x, const int y) const {
            return m_camera.pickRay(x, y);
        }
        
        Hits TexturingView::doPick(const Ray3& pickRay) const {
            Hits hits;
            if (!m_helper.valid())
                return hits;
            
            Model::BrushFace* face = m_helper.face();
            const FloatType distance = face->intersectWithRay(pickRay);
            if (!Math::isnan(distance)) {
                const Vec3 hitPoint = pickRay.pointAtDistance(distance);
                hits.addHit(Hit(TexturingView::FaceHit, distance, hitPoint, face));
            }
            return hits;
        }
    }
}
