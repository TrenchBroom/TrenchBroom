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

#include "UVView.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Hit.h"
#include "Assets/Texture.h"
#include "Model/BrushFace.h"
#include "Model/BrushVertex.h"
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

#include <cassert>
#include <iostream>

namespace TrenchBroom {
    namespace View {
        const Hit::HitType UVView::FaceHit = Hit::freeHitType();
        
        UVView::UVView(wxWindow* parent, MapDocumentWPtr document, GLContextManager& contextManager) :
        RenderView(parent, contextManager, buildAttribs()),
        ToolBoxConnector(this),
        m_document(document),
        m_helper(m_camera),
        m_rotateTool(NULL),
        m_originTool(NULL),
        m_scaleTool(NULL),
        m_shearTool(NULL),
        m_offsetTool(NULL),
        m_cameraTool(NULL) {
            setToolBox(m_toolBox);
            m_toolBox.setClickToActivate(false);
            createTools();
            m_toolBox.disable();
            bindObservers();
        }
        
        UVView::~UVView() {
            unbindObservers();
            destroyTools();
        }

        void UVView::setSubDivisions(const Vec2i& subDivisions) {
            m_helper.setSubDivisions(subDivisions);
            Refresh();
        }

        void UVView::createTools() {
            m_rotateTool = new UVRotateTool(m_document, m_helper);
            m_originTool = new UVOriginTool(m_document, m_helper);
            m_scaleTool = new UVScaleTool(m_document, m_helper);
            m_shearTool = new UVShearTool(m_document, m_helper);
            m_offsetTool = new UVOffsetTool(m_document, m_helper);
            m_cameraTool = new UVCameraTool(m_document, m_camera);

            addTool(m_rotateTool);
            addTool(m_originTool);
            addTool(m_scaleTool);
            addTool(m_shearTool);
            addTool(m_offsetTool);
            addTool(m_cameraTool);
        }
        
        void UVView::destroyTools() {
            delete m_cameraTool;
            m_cameraTool = NULL;
            delete m_offsetTool;
            m_offsetTool = NULL;
            delete m_originTool;
            m_originTool = NULL;
            delete m_scaleTool;
            m_scaleTool = NULL;
            delete m_shearTool;
            m_shearTool = NULL;
            delete m_rotateTool;
            m_rotateTool = NULL;
        }

        void UVView::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
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
                m_helper.setFace(NULL);
            else
                m_helper.setFace(faces.back());

            if (m_helper.valid())
                m_toolBox.enable();
            else
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
            m_camera.setViewport(Renderer::Camera::Viewport(x, y, width, height));
            m_helper.cameraViewportChanged();
        }
        
        void UVView::doRender() {
            if (m_helper.valid()) {
                MapDocumentSPtr document = lock(m_document);
                document->commitPendingAssets();
                
                Renderer::RenderContext renderContext(Renderer::RenderContext::RenderMode_2D, m_camera, fontManager(), shaderManager());
                
                Renderer::RenderBatch renderBatch(sharedVbo());
                
                setupGL(renderContext);
                renderTexture(renderContext, renderBatch);
                renderFace(renderContext, renderBatch);
                renderTextureAxes(renderContext, renderBatch);
                renderToolBox(renderContext, renderBatch);
            }
        }
        
        bool UVView::doShouldRenderFocusIndicator() const {
            return false;
        }

        void UVView::setupGL(Renderer::RenderContext& renderContext) {
            const Renderer::Camera::Viewport& viewport = renderContext.camera().unzoomedViewport();
            glViewport(viewport.x, viewport.y, viewport.width, viewport.height);
            
            glEnable(GL_MULTISAMPLE);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glShadeModel(GL_SMOOTH);
            glDisable(GL_DEPTH_TEST);
        }

        class UVView::RenderTexture : public Renderer::Renderable {
        private:
            typedef Renderer::VertexSpecs::P3NT2::Vertex Vertex;
            
            const UVViewHelper& m_helper;
            Renderer::VertexArray m_vertexArray;
        public:
            RenderTexture(const UVViewHelper& helper) :
            m_helper(helper) {
                Vertex::List vertices = getVertices();
                m_vertexArray = Renderer::VertexArray::swap(GL_QUADS, vertices);
            }
        private:
            Vertex::List getVertices() {
                const Model::BrushFace* face = m_helper.face();
                const Vec3& normal = face->boundary().normal;
                
                Vertex::List vertices;
                vertices.reserve(4);
                
                const Renderer::Camera& camera = m_helper.camera();
                const Renderer::Camera::Viewport& v = camera.zoomedViewport();
                const float w2 = static_cast<float>(v.width) / 2.0f;
                const float h2 = static_cast<float>(v.height) / 2.0f;
                
                const Vec3f& p = camera.position();
                const Vec3f& r = camera.right();
                const Vec3f& u = camera.up();
                
                const Vec3f pos1 = -w2 * r +h2 * u + p;
                const Vec3f pos2 = +w2 * r +h2 * u + p;
                const Vec3f pos3 = +w2 * r -h2 * u + p;
                const Vec3f pos4 = -w2 * r -h2 * u + p;
                
                vertices.push_back(Vertex(pos1, normal, face->textureCoords(pos1)));
                vertices.push_back(Vertex(pos2, normal, face->textureCoords(pos2)));
                vertices.push_back(Vertex(pos3, normal, face->textureCoords(pos3)));
                vertices.push_back(Vertex(pos4, normal, face->textureCoords(pos4)));
                
                return vertices;
            }
        private:
            void doPrepare(Renderer::Vbo& vbo) {
                m_vertexArray.prepare(vbo);
            }
            
            void doRender(Renderer::RenderContext& renderContext) {
                const Model::BrushFace* face = m_helper.face();
                const Vec2f& offset = face->offset();
                const Vec2f& scale = face->scale();
                const Mat4x4 toTex = face->toTexCoordSystemMatrix(offset, scale, true);

                const Assets::Texture* texture = face->texture();
                assert(texture != NULL);

                texture->activate();
                
                Renderer::ActiveShader shader(renderContext.shaderManager(), Renderer::Shaders::UVViewShader);
                shader.set("ApplyTexture", true);
                shader.set("Color", texture->averageColor());
                shader.set("Brightness", pref(Preferences::Brightness));
                shader.set("RenderGrid", true);
                shader.set("GridSizes", Vec2f(texture->width(), texture->height()));
                shader.set("GridColor", Color(1.0f, 1.0f, 0.0f, 1.0f)); // TODO: make this a preference
                shader.set("GridScales", scale);
                shader.set("GridMatrix", toTex);
                shader.set("GridDivider", Vec2f(m_helper.subDivisions()));
                shader.set("CameraZoom", m_helper.cameraZoom());
                shader.set("Texture", 0);
                
                texture->deactivate();
            }
        };
        
        void UVView::renderTexture(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            const Model::BrushFace* face = m_helper.face();
            const Assets::Texture* texture = face->texture();
            if (texture == NULL)
                return;

            renderBatch.addOneShot(new RenderTexture(m_helper));
        }
        
        void UVView::renderFace(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            assert(m_helper.valid()); 
            
            const Model::BrushFace* face = m_helper.face();
            const Model::BrushVertexList& faceVertices = face->vertices();
            const size_t count = faceVertices.size();
            
            typedef Renderer::VertexSpecs::P3::Vertex Vertex;
            Vertex::List edgeVertices(count);
            
            for (size_t i = 0; i < count; ++i)
                edgeVertices[i] = Vertex(faceVertices[i]->position);
            
            const Color edgeColor(1.0f, 1.0f, 1.0f, 0.8f); // TODO: make this a preference
            
            Renderer::EdgeRenderer edgeRenderer(Renderer::VertexArray::swap(GL_LINE_LOOP, edgeVertices));
            Renderer::RenderEdges* renderEdges = new Renderer::RenderUnoccludedEdges(edgeRenderer, true, edgeColor);
            renderEdges->setWidth(2.0f);
            renderBatch.addOneShot(renderEdges);
        }

        void UVView::renderTextureAxes(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            assert(m_helper.valid());
            
            const Model::BrushFace* face = m_helper.face();
            const Vec3 xAxis = face->textureXAxis();
            const Vec3 yAxis = face->textureYAxis();
            const Vec3 center = face->boundsCenter();
            
            typedef Renderer::VertexSpecs::P3C4::Vertex Vertex;
            Vertex::List vertices;
            vertices.reserve(4);
            
            vertices.push_back(Vertex(center, pref(Preferences::XAxisColor)));
            vertices.push_back(Vertex(center + 32.0 * xAxis, pref(Preferences::XAxisColor)));
            vertices.push_back(Vertex(center, pref(Preferences::YAxisColor)));
            vertices.push_back(Vertex(center + 32.0 * yAxis, pref(Preferences::YAxisColor)));
            
            Renderer::EdgeRenderer edgeRenderer(Renderer::VertexArray::swap(GL_LINES, vertices));
            Renderer::RenderEdges* renderEdges = new Renderer::RenderOccludedEdges(edgeRenderer, false);
            renderEdges->setWidth(2.0f);
            renderBatch.addOneShot(renderEdges);
            
            edgeRenderer.setUseColor(false);
            
            glDisable(GL_DEPTH_TEST);
            glLineWidth(2.0f);
            edgeRenderer.render(renderContext);
            glLineWidth(1.0f);
            glEnable(GL_DEPTH_TEST);
            
        }

        void UVView::renderToolBox(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            renderTools(renderContext, renderBatch);
        }

        PickRequest UVView::doGetPickRequest(const int x, const int y) const {
            return PickRequest(Ray3(m_camera.pickRay(x, y)), m_camera);
        }
        
        Hits UVView::doPick(const Ray3& pickRay) const {
            Hits hits;
            if (!m_helper.valid())
                return hits;
            
            Model::BrushFace* face = m_helper.face();
            const FloatType distance = face->intersectWithRay(pickRay);
            if (!Math::isnan(distance)) {
                const Vec3 hitPoint = pickRay.pointAtDistance(distance);
                hits.addHit(Hit(UVView::FaceHit, distance, hitPoint, face));
            }
            return hits;
        }
    }
}
