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
#include "Renderer/RenderResources.h"
#include "Renderer/Vbo.h"
#include "Renderer/VertexArray.h"
#include "Renderer/VertexSpec.h"
#include "View/Grid.h"
#include "View/MapDocument.h"
#include "View/TexturingViewCameraTool.h"
#include "View/TexturingViewOffsetTool.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        TexturingViewState::TexturingViewState() :
        m_face(NULL) {}
        
        bool TexturingViewState::valid() const {
            return m_face != NULL;
        }
        
        Model::BrushFace* TexturingViewState::face() const {
            return m_face;
        }
        
        const Vec3& TexturingViewState::origin() const {
            assert(valid());
            return m_origin;
        }
        
        const Vec3& TexturingViewState::xAxis() const {
            assert(valid());
            return m_xAxis;
        }
        
        const Vec3& TexturingViewState::yAxis() const {
            assert(valid());
            return m_yAxis;
        }
        
        const Vec3& TexturingViewState::zAxis() const {
            assert(valid());
            return m_zAxis;
        }
        
        BBox3 TexturingViewState::computeBounds() const {
            assert(valid());
            
            BBox3 result;
            const Model::BrushVertexList& vertices = m_face->vertices();
            result.min = result.max = transformToFace(vertices[0]->position);
            for (size_t i = 1; i < vertices.size(); ++i)
                result.mergeWith(transformToFace(vertices[i]->position));
            return result;
        }
        
        Vec3 TexturingViewState::transformToFace(const Vec3& point) const {
            assert(valid());
            return m_toFaceTransform * point;
        }
        
        Vec3 TexturingViewState::transformFromFace(const Vec3& point) const {
            assert(valid());
            return m_fromFaceTransform * point;
        }

        Vec2f TexturingViewState::textureCoords(const Vec3f& point) const {
            assert(valid());
            return m_face->textureCoords(Vec3(point));
        }

        Vec3::List TexturingViewState::textureSeamVertices(const Renderer::OrthographicCamera& camera) const {
            assert(valid());

            const Assets::Texture* texture = m_face->texture();
            if (texture == NULL)
                return Vec3::EmptyList;
            
            const Vec2f offset(m_face->xOffset(), m_face->yOffset());
            const Vec2f scale(m_face->xScale(), m_face->yScale());
            const Mat4x4 worldToTex = Mat4x4::ZerZ * m_face->toTexCoordSystemMatrix(offset, scale);
            const Mat4x4 texToWorld = m_face->fromTexCoordSystemMatrix(offset, scale);

            const Vec3 texZAxis = m_face->fromTexCoordSystemMatrix() * Vec3::PosZ;
            const Plane3& boundary = m_face->boundary();
            const Mat4x4 planeToWorld = planeProjectionMatrix(boundary.distance, boundary.normal, texZAxis);
            const Mat4x4 worldToPlane = Mat4x4::ZerZ * invertedMatrix(planeToWorld);
            const Mat4x4 texToWorldWithProjection = planeToWorld * worldToPlane * texToWorld;
            
            const Vec3::List viewportVertices = worldToTex * camera.viewportVertices();
            
            const BBox3 viewportBounds(viewportVertices);
            const Vec3& min = viewportBounds.min;
            const Vec3& max = viewportBounds.max;

            Vec3::List seamVertices;
            
            const FloatType dx = min.x() / texture->width();
            FloatType x = (dx < 0.0 ? Math::ceil(dx) : Math::floor(dx)) * texture->width();
            while (Math::lte(x, max.x())) {
                seamVertices.push_back(Vec3(x, min.y(), 0.0));
                seamVertices.push_back(Vec3(x, max.y(), 0.0));
                x += texture->width();
            }
            
            const FloatType dy = min.y() / texture->height();
            FloatType y = (dy < 0.0 ? Math::ceil(dy) : Math::floor(dy)) * texture->height();
            while (Math::lte(y, max.y())) {
                seamVertices.push_back(Vec3(min.x(), y, 0.0));
                seamVertices.push_back(Vec3(max.x(), y, 0.0));
                y += texture->height();
            }

            return texToWorldWithProjection * seamVertices;
        }

        void TexturingViewState::activateTexture(Renderer::ActiveShader& shader) {
            assert(valid());
            Assets::Texture* texture = m_face->texture();
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
        
        void TexturingViewState::deactivateTexture() {
            assert(valid());
            Assets::Texture* texture = m_face->texture();
            if (texture != NULL)
                texture->deactivate();
        }

        Hits TexturingViewState::pick(const Ray3& pickRay) const {
            assert(valid());
            
            Hits hits;
            const FloatType distance = m_face->intersectWithRay(pickRay);
            if (!Math::isnan(distance)) {
                const Vec3 hitPoint = pickRay.pointAtDistance(distance);
                hits.addHit(Hit(TexturingView::FaceHit, distance, hitPoint, m_face));
            }
            return hits;
        }

        void TexturingViewState::setFace(Model::BrushFace* face) {
            m_face = face;
            validate();
        }
        
        void TexturingViewState::validate() {
            if (m_face != NULL) {
                m_origin = m_face->center();
                m_zAxis = m_face->boundary().normal;
                
                if (Math::lt(Math::abs(Vec3::PosZ.dot(m_zAxis)), 1.0))
                    m_xAxis = crossed(Vec3::PosZ, m_zAxis).normalized();
                else
                    m_xAxis = Vec3::PosX;
                m_yAxis = crossed(m_zAxis, m_xAxis).normalized();
                
                m_fromFaceTransform = coordinateSystemMatrix(m_xAxis, m_yAxis, m_zAxis, m_origin);
                bool invertible = true;
                m_toFaceTransform = invertedMatrix(m_fromFaceTransform, invertible);
                assert(invertible);
            }
        }
        
        const Hit::HitType TexturingView::FaceHit = Hit::freeHitType();
        
        TexturingView::TexturingView(wxWindow* parent, MapDocumentWPtr document, ControllerWPtr controller, Renderer::RenderResources& renderResources) :
        RenderView(parent, renderResources.glAttribs(), renderResources.sharedContext()),
        m_document(document),
        m_controller(controller),
        m_renderResources(renderResources),
        m_toolBox(this, this),
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

        void TexturingView::createTools() {
            m_offsetTool = new TexturingViewOffsetTool(m_document, m_controller);
            m_cameraTool = new TexturingViewCameraTool(m_document, m_controller, m_camera);
            m_toolBox.addTool(m_offsetTool);
            m_toolBox.addTool(m_cameraTool);
        }
        
        void TexturingView::destroyTools() {
            delete m_cameraTool;
            m_cameraTool = NULL;
            delete m_offsetTool;
            m_offsetTool = NULL;
        }

        void TexturingView::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->objectDidChangeNotifier.addObserver(this, &TexturingView::objectDidChange);
            document->faceDidChangeNotifier.addObserver(this, &TexturingView::faceDidChange);
            document->selectionDidChangeNotifier.addObserver(this, &TexturingView::selectionDidChange);
            document->grid().gridDidChangeNotifier.addObserver(this, &TexturingView::gridDidChange);
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.addObserver(this, &TexturingView::preferenceDidChange);
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
        }
        
        void TexturingView::objectDidChange(Model::Object* object) {
            Refresh();
        }

        void TexturingView::faceDidChange(Model::BrushFace* face) {
            Refresh();
        }
        
        void TexturingView::selectionDidChange(const Model::SelectionResult& result) {
            MapDocumentSPtr document = lock(m_document);
            const Model::BrushFaceList& faces = document->selectedFaces();
            if (faces.empty())
                m_state.setFace(NULL);
            else
                m_state.setFace(faces.back());

            if (m_state.valid()) {
                m_toolBox.enable();
                m_camera.setZoom(computeZoomFactor());
                m_camera.moveTo(Vec3f(m_state.origin()));
            } else {
                m_toolBox.disable();
            }
            Refresh();
        }

        void TexturingView::gridDidChange() {
            Refresh();
        }

        void TexturingView::preferenceDidChange(const IO::Path& path) {
            Refresh();
        }

        void TexturingView::doUpdateViewport(int x, int y, int width, int height) {
            m_camera.setViewport(Renderer::Camera::Viewport(x, y, width, height));
        }
        
        void TexturingView::doRender() {
            if (m_state.valid()) {
                setupCamera();

                MapDocumentSPtr document = lock(m_document);
                document->commitPendingRenderStateChanges();

                const View::Grid& grid = document->grid();
                Renderer::RenderContext renderContext(m_camera, m_renderResources.shaderManager(), grid.visible(), grid.actualSize());
                
                setupGL(renderContext);
                renderTexture(renderContext);
                renderFace(renderContext);
                renderTextureSeams(renderContext);
            }
        }
        
        void TexturingView::setupCamera() {
            assert(m_state.valid());
            
            m_camera.setNearPlane(-1.0);
            m_camera.setFarPlane(1.0);
            m_camera.setDirection(-m_state.zAxis(), m_state.yAxis());
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
            const Vec3f::List positions = getTextureQuad();
            const Vec3f normal(m_state.face()->boundary().normal);
            
            typedef Renderer::VertexSpecs::P3NT2::Vertex Vertex;
            Vertex::List vertices(positions.size());
            
            for (size_t i = 0; i < positions.size(); ++i)
                vertices[i] = Vertex(positions[i],
                                     normal,
                                     m_state.textureCoords(positions[i]));
            
            Renderer::VertexArray vertexArray = Renderer::VertexArray::swap(GL_QUADS, vertices);
            
            Renderer::SetVboState setVboState(m_vbo);
            setVboState.mapped();
            vertexArray.prepare(m_vbo);
            setVboState.active();
            
            PreferenceManager& prefs = PreferenceManager::instance();
            
            Renderer::ActiveShader shader(renderContext.shaderManager(), Renderer::Shaders::TexturingViewShader);
            shader.set("Brightness", prefs.get(Preferences::Brightness));
            shader.set("RenderGrid", renderContext.gridVisible());
            shader.set("GridSize", static_cast<float>(renderContext.gridSize()));
            shader.set("GridAlpha", prefs.get(Preferences::GridAlpha));
            shader.set("Texture", 0);

            m_state.activateTexture(shader);
            vertexArray.render();
            m_state.deactivateTexture();
        }
        
        void TexturingView::renderFace(Renderer::RenderContext& renderContext) {
            assert(m_state.valid());
            
            const Model::BrushFace* face = m_state.face();
            const Model::BrushVertexList& faceVertices = face->vertices();
            const size_t count = faceVertices.size();
            
            typedef Renderer::VertexSpecs::P3::Vertex Vertex;
            Vertex::List edgeVertices(count);
            
            for (size_t i = 0; i < count; ++i)
                edgeVertices[i] = Vertex(faceVertices[i]->position);
            
            Renderer::EdgeRenderer edgeRenderer(Renderer::VertexArray::swap(GL_LINE_LOOP, edgeVertices));
            edgeRenderer.setUseColor(true);
            edgeRenderer.setColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
            edgeRenderer.render(renderContext);
        }

        void TexturingView::renderTextureSeams(Renderer::RenderContext& renderContext) {
            assert(m_state.valid());
            
            const Vec3::List positions = m_state.textureSeamVertices(m_camera);
            const size_t count = positions.size();
            
            typedef Renderer::VertexSpecs::P3::Vertex Vertex;
            Vertex::List vertices(count);
            
            for (size_t i = 0; i < count; ++i)
                vertices[i] = Vertex(positions[i]);
            
            Renderer::EdgeRenderer edgeRenderer(Renderer::VertexArray::swap(GL_LINES, vertices));
            edgeRenderer.setUseColor(true);
            edgeRenderer.setColor(Color(1.0f, 1.0f, 0.0f, 1.0f));
            edgeRenderer.render(renderContext);
        }

        float TexturingView::computeZoomFactor() const {
            m_state.transformToFace(Vec3::Null);
            const BBox3 bounds = m_state.computeBounds();
            const Vec3f size(bounds.size());
            const float w = static_cast<float>(m_camera.viewport().width - 20);
            const float h = static_cast<float>(m_camera.viewport().height - 20);
            
            float zoom = 1.0f;
            if (size.x() > w)
                zoom = Math::min(zoom, w / size.x());
            if (size.y() > h)
                zoom = Math::min(zoom, h / size.y());
            return zoom;
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

        Ray3 TexturingView::doGetPickRay(const int x, const int y) const {
            return m_camera.pickRay(x, y);
        }
        
        Hits TexturingView::doPick(const Ray3& pickRay) const {
            if (!m_state.valid())
                return Hits();
            return m_state.pick(pickRay);
        }
    }
}
