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

#include <cassert>
#include <iostream>

namespace TrenchBroom {
    namespace View {
        const Model::Hit::HitType UVView::FaceHit = Model::Hit::freeHitType();
        
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
            m_originTool = new UVOriginTool(m_helper);
            m_scaleTool = new UVScaleTool(m_document, m_helper);
            m_shearTool = new UVShearTool(m_document, m_helper);
            m_offsetTool = new UVOffsetTool(m_document, m_helper);
            m_cameraTool = new UVCameraTool(m_camera);

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
            void doPrepareVertices(Renderer::Vbo& vertexVbo) {
                m_vertexArray.prepare(vertexVbo);
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
                shader.set("GridColor", Color(0.6f, 0.6f, 0.6f, 1.0f)); // TODO: make this a preference
                shader.set("GridScales", scale);
                shader.set("GridMatrix", toTex);
                shader.set("GridDivider", Vec2f(m_helper.subDivisions()));
                shader.set("CameraZoom", m_helper.cameraZoom());
                shader.set("Texture", 0);
                
                m_vertexArray.render(GL_QUADS);
                
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
            const Model::BrushFace::VertexList faceVertices = face->vertices();
            
            typedef Renderer::VertexSpecs::P3::Vertex Vertex;
            Vertex::List edgeVertices;
            edgeVertices.reserve(faceVertices.size());
            
            Model::BrushFace::VertexList::const_iterator it, end;
            for (it = faceVertices.begin(), end = faceVertices.end(); it != end; ++it)
                edgeVertices.push_back(Vertex((*it)->position()));
            
            const Color edgeColor(1.0f, 1.0f, 1.0f, 1.0f); // TODO: make this a preference
            
            Renderer::DirectEdgeRenderer edgeRenderer(Renderer::VertexArray::swap(edgeVertices), GL_LINE_LOOP);
            edgeRenderer.renderOnTop(renderBatch, edgeColor, 2.5f);
        }

        void UVView::renderTextureAxes(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            assert(m_helper.valid());
            
            const Model::BrushFace* face = m_helper.face();
            const Vec3& normal = face->boundary().normal;
            
            const Vec3 xAxis = face->textureXAxis() - face->textureXAxis().dot(normal) * normal;
            const Vec3 yAxis = face->textureYAxis() - face->textureYAxis().dot(normal) * normal;
            const Vec3 center = face->boundsCenter();
            
            typedef Renderer::VertexSpecs::P3C4::Vertex Vertex;
            Vertex::List vertices;
            vertices.reserve(4);
            
            const FloatType length = 32.0 / FloatType(m_helper.cameraZoom());
            
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
            return PickRequest(Ray3(m_camera.pickRay(x, y)), m_camera);
        }
        
        Model::PickResult UVView::doPick(const Ray3& pickRay) const {
            Model::PickResult pickResult = Model::PickResult::byDistance(lock(m_document)->editorContext());
            if (!m_helper.valid())
                return pickResult;
            
            Model::BrushFace* face = m_helper.face();
            const FloatType distance = face->intersectWithRay(pickRay);
            if (!Math::isnan(distance)) {
                const Vec3 hitPoint = pickRay.pointAtDistance(distance);
                pickResult.addHit(Model::Hit(UVView::FaceHit, distance, hitPoint, face));
            }
            return pickResult;
        }
    }
}
