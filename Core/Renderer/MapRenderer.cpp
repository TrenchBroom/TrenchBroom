/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "MapRenderer.h"
#include <set>
#include "Map.h"
#include "Selection.h"
#include "Entity.h"
#include "EntityDefinition.h"
#include "Brush.h"
#include "BrushGeometry.h"
#include "Face.h"
#include "Vbo.h"
#include "Editor.h"
#include "RenderUtils.h"

namespace TrenchBroom {
    namespace Renderer {
        static const Vec4f FaceDefaultColor(0.2f, 0.2f, 0.2f, 1);
        static const Vec4f EdgeDefaultColor(0.6f, 0.6f, 0.6f, 0.6f);
        static const Vec4f  SelectionColor(1, 0, 0, 1);
        static const Vec4f  SelectionColor2(1, 0, 0, 0.35f);
        static const Vec4f  SelectionColor3(1, 0, 0, 0.6f);
        static const int VertexSize = 3 * sizeof(float);
        static const int ColorSize = 4;
        static const int TexCoordSize = 2 * sizeof(float);

        
        RenderContext::RenderContext() {
            backgroundColor.x = 0;
            backgroundColor.y = 0;
            backgroundColor.z = 0;
            backgroundColor.w = 1;
            renderOrigin = true;
            originAxisLength = 64;
        }

#pragma mark ChangeSet
        
        void ChangeSet::entitiesAdded(const vector<Model::Entity*>& entities) {
            m_addedEntities.insert(m_addedEntities.end(), entities.begin(), entities.end());
        }
        
        void ChangeSet::entitiesRemoved(const vector<Model::Entity*>& entities) {
            m_removedEntities.insert(m_removedEntities.end(), entities.begin(), entities.end());
        }
        
        void ChangeSet::entitiesChanged(const vector<Model::Entity*>& entities) {
            m_changedEntities.insert(m_changedEntities.end(), entities.begin(), entities.end());
        }
        
        void ChangeSet::entitiesSelected(const vector<Model::Entity*>& entities) {
            if (!m_deselectedEntities.empty()) {
                for (int i = 0; i < entities.size(); i++) {
                    Model::Entity* entity = entities[i];
                    vector<Model::Entity*>::iterator it = find(m_deselectedEntities.begin(), m_deselectedEntities.end(), entity);
                    if (it != m_deselectedEntities.end()) m_deselectedEntities.erase(it);
                    else m_selectedEntities.push_back(entity);
                }
            } else {
                m_selectedEntities.insert(m_selectedEntities.end(), entities.begin(), entities.end());
            }
        }
        
        void ChangeSet::entitiesDeselected(const vector<Model::Entity*>& entities) {
            m_deselectedEntities.insert(m_deselectedEntities.end(), entities.begin(), entities.end());
        }
        
        void ChangeSet::brushesAdded(const vector<Model::Brush*>& brushes) {
            m_addedBrushes.insert(m_addedBrushes.end(), brushes.begin(), brushes.end());
        }
        
        void ChangeSet::brushesRemoved(const vector<Model::Brush*>& brushes) {
            m_removedBrushes.insert(m_removedBrushes.end(), brushes.begin(), brushes.end());
        }
        
        void ChangeSet::brushesChanged(const vector<Model::Brush*>& brushes) {
            m_changedBrushes.insert(m_changedBrushes.end(), brushes.begin(), brushes.end());
        }
        
        void ChangeSet::brushesSelected(const vector<Model::Brush*>& brushes) {
            if (!m_deselectedBrushes.empty()) {
                for (int i = 0; i < brushes.size(); i++) {
                    Model::Brush* brush = brushes[i];
                    vector<Model::Brush*>::iterator it = find(m_deselectedBrushes.begin(), m_deselectedBrushes.end(), brush);
                    if (it != m_deselectedBrushes.end()) m_deselectedBrushes.erase(it);
                    else m_selectedBrushes.push_back(brush);
                }
            } else {
                m_selectedBrushes.insert(m_selectedBrushes.end(), brushes.begin(), brushes.end());
            }
        }
        
        void ChangeSet::brushesDeselected(const vector<Model::Brush*>& brushes) {
            m_deselectedBrushes.insert(m_deselectedBrushes.end(), brushes.begin(), brushes.end());
        }
        
        void ChangeSet::facesChanged(const vector<Model::Face*>& faces) {
            m_changedFaces.insert(m_changedFaces.end(), faces.begin(), faces.end());
        }
        
        void ChangeSet::facesSelected(const vector<Model::Face*>& faces) {
            if (!m_deselectedFaces.empty()) {
                for (int i = 0; i < faces.size(); i++) {
                    Model::Face* face = faces[i];
                    vector<Model::Face*>::iterator it = find(m_deselectedFaces.begin(), m_deselectedFaces.end(), face);
                    if (it != m_deselectedFaces.end()) m_deselectedFaces.erase(it);
                    else m_selectedFaces.push_back(face);
                }
            } else {
                m_selectedFaces.insert(m_selectedFaces.end(), faces.begin(), faces.end());
            }
        }
        
        void ChangeSet::facesDeselected(const vector<Model::Face*>& faces) {
            m_deselectedFaces.insert(m_deselectedFaces.end(), faces.begin(), faces.end());
        }
        
        void ChangeSet::setFilterChanged() {
            m_filterChanged = true;
        }
        
        void ChangeSet::setTextureManagerChanged() {
            m_textureManagerChanged = true;
        }
        
        void ChangeSet::clear() {
            m_addedEntities.clear();
            m_removedEntities.clear();
            m_changedEntities.clear();
            m_selectedEntities.clear();
            m_deselectedEntities.clear();
            m_addedBrushes.clear();
            m_removedBrushes.clear();
            m_changedBrushes.clear();
            m_selectedBrushes.clear();
            m_deselectedBrushes.clear();
            m_changedFaces.clear();
            m_selectedFaces.clear();
            m_deselectedFaces.clear();
            m_filterChanged = false;
            m_textureManagerChanged = false;
        }
        
        
        const vector<Model::Entity*> ChangeSet::addedEntities() const {
            return m_addedEntities;
        }
        
        const vector<Model::Entity*> ChangeSet::removedEntities() const {
            return m_removedEntities;
        }
        
        const vector<Model::Entity*> ChangeSet::changedEntities() const {
            return m_changedEntities;
        }
        
        const vector<Model::Entity*> ChangeSet::selectedEntities() const {
            return m_selectedEntities;
        }
        
        const vector<Model::Entity*> ChangeSet::deselectedEntities() const {
            return m_deselectedEntities;
        }
        
        const vector<Model::Brush*> ChangeSet::addedBrushes() const {
            return m_addedBrushes;
        }
        
        const vector<Model::Brush*> ChangeSet::removedBrushes() const {
            return m_removedBrushes;
        }
        
        const vector<Model::Brush*> ChangeSet::changedBrushes() const {
            return m_changedBrushes;
        }
        
        const vector<Model::Brush*> ChangeSet::selectedBrushes() const {
            return m_selectedBrushes;
        }
        
        const vector<Model::Brush*> ChangeSet::deselectedBrushes() const {
            return m_deselectedBrushes;
        }
        
        const vector<Model::Face*> ChangeSet::changedFaces() const {
            return m_changedFaces;
        }
        
        const vector<Model::Face*> ChangeSet::selectedFaces() const {
            return m_selectedFaces;
        }
        
        const vector<Model::Face*> ChangeSet::deselectedFaces() const {
            return m_deselectedFaces;
        }
        
        bool ChangeSet::filterChanged() const {
            return m_filterChanged;
        }
        
        bool ChangeSet::textureManagerChanged() const {
            return m_textureManagerChanged;
        }
        

#pragma mark MapRenderer
        
        void MapRenderer::addEntities(const vector<Model::Entity*>& entities) {
            m_changeSet.entitiesAdded(entities);
            
            for (int i = 0; i < entities.size(); i++)
                addBrushes(entities[i]->brushes());
        }
        
        void MapRenderer::removeEntities(const vector<Model::Entity*>& entities) {
            m_changeSet.entitiesRemoved(entities);
            
            for (int i = 0; i < entities.size(); i++)
                removeBrushes(entities[i]->brushes());
        }
        
        void MapRenderer::addBrushes(const vector<Model::Brush*>& brushes) {
            m_changeSet.brushesAdded(brushes);
        }
        
        void MapRenderer::removeBrushes(const vector<Model::Brush*>& brushes) {
            m_changeSet.brushesRemoved(brushes);
        }

        void MapRenderer::entitiesWereAdded(const vector<Model::Entity*>& entities) {
            addEntities(entities);
        }
        
        void MapRenderer::entitiesWillBeRemoved(const vector<Model::Entity*>& entities) {
            removeEntities(entities);
        }
        
        void MapRenderer::propertiesDidChange(const vector<Model::Entity*>& entities) {
            m_changeSet.entitiesChanged(entities);
            
            Model::Entity* worldspawn = m_editor.map().worldspawn(true);
            if (find(entities.begin(), entities.end(), worldspawn) != entities.end()) {
                // if mods changed, invalidate renderer cache here
            }
        }
        
        void MapRenderer::brushesWereAdded(const vector<Model::Brush*>& brushes) {
            addBrushes(brushes);
        }
        
        void MapRenderer::brushesWillBeRemoved(const vector<Model::Brush*>& brushes) {
            removeBrushes(brushes);
        }
        
        void MapRenderer::brushesDidChange(const vector<Model::Brush*>& brushes) {
            m_changeSet.brushesChanged(brushes);
            
            vector<Model::Entity*> entities;
            for (int i = 0; i < brushes.size(); i++) {
                Model::Entity* entity = brushes[i]->entity();
                if (!entity->worldspawn() && entity->entityDefinition()->type == Model::EDT_BRUSH) {
                    if (find(entities.begin(), entities.end(), entity) == entities.end())
                        entities.push_back(entity);
                }
            }
            
            m_changeSet.entitiesChanged(entities);
        }
        
        void MapRenderer::facesDidChange(const vector<Model::Face*>& faces) {
            m_changeSet.facesChanged(faces);
        }
        
        void MapRenderer::mapLoaded(Model::Map& map) {
            addEntities(map.entities());
        }
        
        void MapRenderer::mapCleared(Model::Map& map) {
        }

        void MapRenderer::selectionAdded(const Model::SelectionEventData& event) {
            if (!event.entities.empty())
                m_changeSet.entitiesSelected(event.entities);
            if (!event.brushes.empty())
                m_changeSet.brushesSelected(event.brushes);
            if (!event.faces.empty())
                m_changeSet.facesSelected(event.faces);
        }
        
        void MapRenderer::selectionRemoved(const Model::SelectionEventData& event) {
            if (!event.entities.empty())
                m_changeSet.entitiesDeselected(event.entities);
            if (!event.brushes.empty())
                m_changeSet.brushesDeselected(event.brushes);
            if (!event.faces.empty())
                m_changeSet.facesDeselected(event.faces);
        }

        void MapRenderer::writeFaceVertices(Model::Face& face, VboBlock& block) {
            Vec2f texCoords, gridCoords;
            
            Model::Assets::Texture* texture = face.texture();
            Vec4f color = texture != NULL && !texture->dummy ? texture->averageColor : FaceDefaultColor;
            int width = texture != NULL ? texture->width : 1;
            int height = texture != NULL ? texture->height : 1;
            
            int offset = 0;
            const vector<Model::Vertex*>& vertices = face.vertices();
            for (int i = 0; i < vertices.size(); i++) {
                const Model::Vertex* vertex = vertices[i];
                gridCoords = face.gridCoords(vertex->position);
                texCoords = face.textureCoords(vertex->position);
                texCoords.x /= width;
                texCoords.y /= height;
                
                offset = block.writeVec(gridCoords, offset);
                offset = block.writeVec(texCoords, offset);
                offset = block.writeColor(EdgeDefaultColor, offset);
                offset = block.writeColor(color, offset);
                offset = block.writeVec(vertex->position, offset);
            }
        }

        void MapRenderer::writeFaceIndices(Model::Face& face, IndexBuffer& triangleBuffer, IndexBuffer& edgeBuffer) {
            int baseIndex = face.vboBlock()->address / (TexCoordSize + TexCoordSize + ColorSize + ColorSize + VertexSize);
            int vertexCount = (int)face.vertices().size();
         
            edgeBuffer.push_back(baseIndex);
            edgeBuffer.push_back(baseIndex + 1);
            
            for (int i = 1; i < vertexCount - 1; i++) {
                triangleBuffer.push_back(baseIndex);
                triangleBuffer.push_back(baseIndex + i);
                triangleBuffer.push_back(baseIndex + i + 1);
                
                edgeBuffer.push_back(baseIndex + i);
                edgeBuffer.push_back(baseIndex + i + 1);
            }
            
            edgeBuffer.push_back(baseIndex + vertexCount - 1);
            edgeBuffer.push_back(baseIndex);
        }

        void MapRenderer::rebuildFaceIndexBuffers() {
            for (FaceIndexBuffers::iterator it = m_faceIndexBuffers.begin(); it != m_faceIndexBuffers.end(); ++it)
                delete it->second;
            m_faceIndexBuffers.clear();
            m_edgeIndexBuffer.clear();
            m_edgeIndexBuffer.reserve(0xFFFF);
            
            // possible improvement: collect all faces and sort them by their texture, then create the buffers sequentially
            
            const vector<Model::Entity*>& entities = m_editor.map().entities();
            for (int i = 0; i < entities.size(); i++) {
                const vector<Model::Brush*>& brushes = entities[i]->brushes();
                for (int j = 0; j < brushes.size(); j++) {
                    Model::Brush* brush = brushes[j];
                    if (!brush->selected()) {
                        const vector<Model::Face*>& faces = brush->faces();
                        for (int k = 0; k < faces.size(); k++) {
                            Model::Face* face = faces[k];
                            if (!face->selected()) {
                                Model::Assets::Texture* texture = face->texture();
                                IndexBuffer* indexBuffer = NULL;
                                FaceIndexBuffers::iterator it = m_faceIndexBuffers.find(texture);
                                if (it == m_faceIndexBuffers.end()) {
                                    indexBuffer = new vector<GLuint>();
                                    indexBuffer->reserve(0xFF);
                                    m_faceIndexBuffers[texture] = indexBuffer;
                                } else {
                                    indexBuffer = it->second;
                                }
                                writeFaceIndices(*face, *indexBuffer, m_edgeIndexBuffer);
                            }
                        }
                    }
                }
            }
        }
        
        void MapRenderer::rebuildSelectedFaceIndexBuffers() {
            for (FaceIndexBuffers::iterator it = m_selectedFaceIndexBuffers.begin(); it != m_selectedFaceIndexBuffers.end(); ++it)
                delete it->second;
            m_selectedFaceIndexBuffers.clear();
            m_selectedEdgeIndexBuffer.clear();
            m_selectedEdgeIndexBuffer.reserve(0xFFFF);
            
            Model::Selection& selection = m_editor.map().selection();
            
            // possible improvement: collect all faces and sort them by their texture, then create the buffers sequentially
            
            const vector<Model::Brush*> brushes = selection.brushes();
            for (int i = 0; i < brushes.size(); i++) {
                const vector<Model::Face*>& faces = brushes[i]->faces();
                for (int j = 0; j < faces.size(); j++) {
                    Model::Face* face = faces[j];
                    Model::Assets::Texture* texture = face->texture();
                    IndexBuffer* indexBuffer = NULL;
                    FaceIndexBuffers::iterator it = m_selectedFaceIndexBuffers.find(texture);
                    if (it == m_selectedFaceIndexBuffers.end()) {
                        indexBuffer = new vector<GLuint>();
                        indexBuffer->reserve(0xFF);
                        m_selectedFaceIndexBuffers[texture] = indexBuffer;
                    } else {
                        indexBuffer = it->second;
                    }
                    writeFaceIndices(*face, *indexBuffer, m_selectedEdgeIndexBuffer);
                }
            }

            const vector<Model::Face*>& faces = selection.faces();
            for (int i = 0; i < faces.size(); i++) {
                Model::Face* face = faces[i];
                Model::Assets::Texture* texture = face->texture();
                IndexBuffer* indexBuffer = NULL;
                FaceIndexBuffers::iterator it = m_selectedFaceIndexBuffers.find(texture);
                if (it == m_selectedFaceIndexBuffers.end()) {
                    indexBuffer = new vector<GLuint>();
                    indexBuffer->reserve(0xFF);
                    m_selectedFaceIndexBuffers[texture] = indexBuffer;
                } else {
                    indexBuffer = it->second;
                }
                writeFaceIndices(*face, *indexBuffer, m_selectedEdgeIndexBuffer);
            }
        }
        
        void MapRenderer::validateEntityRendererCache() {
        }
        
        void MapRenderer::validateAddedEntities() {
        }
        
        void MapRenderer::validateRemovedEntities() {
        }
        
        void MapRenderer::validateChangedEntities() {
        }
        
        void MapRenderer::validateAddedBrushes() {
            const vector<Model::Brush*>& addedBrushes = m_changeSet.addedBrushes();
            if (!addedBrushes.empty()) {
                m_faceVbo->activate();
                m_faceVbo->map();
                
                for (int i = 0; i < addedBrushes.size(); i++) {
                    vector<Model::Face*> addedFaces = addedBrushes[i]->faces();
                    for (int j = 0; j < addedFaces.size(); j++) {
                        Model::Face* face = addedFaces[j];
                        VboBlock& block = m_faceVbo->allocBlock((int)face->vertices().size() * (TexCoordSize + TexCoordSize + ColorSize + ColorSize + VertexSize));
                        writeFaceVertices(*face, block);
                        face->setVboBlock(&block);
                    }
                }
                
                m_faceVbo->unmap();
                m_faceVbo->deactivate();
            }
        }
        
        void MapRenderer::validateRemovedBrushes() {
        }
        
        void MapRenderer::validateChangedBrushes() {
        }
        
        void MapRenderer::validateChangedFaces() {
        }
        
        void MapRenderer::validateSelection() {
            
        }
        
        void MapRenderer::validateDeselection() {
        }

        void MapRenderer::validate() {
            validateEntityRendererCache();
            validateAddedEntities();
            validateAddedBrushes();
            validateSelection();
            validateChangedEntities();
            validateChangedBrushes();
            validateChangedFaces();
            validateDeselection();
            validateRemovedEntities();
            validateRemovedBrushes();
            
            if (!m_changeSet.addedBrushes().empty() ||
                !m_changeSet.removedBrushes().empty() ||
                !m_changeSet.selectedBrushes().empty() ||
                !m_changeSet.deselectedBrushes().empty() ||
                !m_changeSet.selectedFaces().empty() ||
                !m_changeSet.deselectedFaces().empty() ||
                m_changeSet.filterChanged() ||
                m_changeSet.textureManagerChanged()) {
                rebuildFaceIndexBuffers();
            }
            
            if (!m_changeSet.changedBrushes().empty() ||
                !m_changeSet.changedFaces().empty() ||
                !m_changeSet.selectedBrushes().empty() ||
                !m_changeSet.deselectedBrushes().empty() ||
                !m_changeSet.selectedFaces().empty() ||
                !m_changeSet.deselectedFaces().empty() ||
                m_changeSet.filterChanged() ||
                m_changeSet.textureManagerChanged()) {
                rebuildSelectedFaceIndexBuffers();
            }
            
            m_changeSet.clear();
        }

        void MapRenderer::renderEdges(const Vec4f* color, const IndexBuffer& indexBuffer) {
            glDisable(GL_TEXTURE_2D);
            glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
            
            if (color != NULL) {
                glColorV4f(*color);
                glVertexPointer(3, GL_FLOAT, TexCoordSize + TexCoordSize + ColorSize + ColorSize + VertexSize, (const GLvoid *)(long)(TexCoordSize + TexCoordSize + ColorSize + ColorSize));
            } else {
                glEnableClientState(GL_COLOR_ARRAY);
                glColorPointer(4, GL_UNSIGNED_BYTE, TexCoordSize + TexCoordSize + ColorSize + ColorSize + VertexSize, (const GLvoid *)(long)(TexCoordSize + TexCoordSize));
                glVertexPointer(3, GL_FLOAT, TexCoordSize + TexCoordSize + ColorSize + ColorSize + VertexSize, (const GLvoid *)(long)(TexCoordSize + TexCoordSize + ColorSize + ColorSize));
            }
            
            GLsizei count = (GLsizei)indexBuffer.size();
            GLvoid* offset = (GLvoid*)&indexBuffer[0];
            
            glDrawElements(GL_LINES, count, GL_UNSIGNED_INT, offset);
            glPopClientAttrib();        
        }
        
        void MapRenderer::renderFaces(bool textured, bool selected, FaceIndexBuffers& indexBuffers) {
            glPolygonMode(GL_FRONT, GL_FILL);
            glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
            
            /*
            Grid* grid = [[windowController options] grid];
            if ([grid draw]) {
                glActiveTexture(GL_TEXTURE2);
                glEnable(GL_TEXTURE_2D);
                [grid activateTexture];
                glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
                glClientActiveTexture(GL_TEXTURE2);
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                glTexCoordPointer(2, GL_FLOAT, TexCoordSize + TexCoordSize + ColorSize + ColorSize + VertexSize, (const GLvoid *)0);
            }
             */
            
            if (selected) {
                if (m_selectionDummyTexture == NULL) {
                    unsigned char image = 0;
                    m_selectionDummyTexture = new Model::Assets::Texture("selection dummy", &image, 1, 1);
                }
                
                glActiveTexture(GL_TEXTURE1);
                glEnable(GL_TEXTURE_2D);
                m_selectionDummyTexture->activate();
                glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
                float color[3] = {0.6f, 0.35f, 0.35f};
                glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
                glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
                glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, color);
                glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PREVIOUS);
                glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_PREVIOUS);
                glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_CONSTANT);
                glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, 2);
            }
            
            glActiveTexture(GL_TEXTURE0);
            if (textured) {
                glEnable(GL_TEXTURE_2D);
                glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
                
                /*
                PreferencesManager* preferences = [PreferencesManager sharedManager];
                float brightness = [preferences brightness];
                float color[3] = {brightness / 2, brightness / 2, brightness / 2};
                */
                 
                float color[4] = {0.5f, 0.5f, 0.5f, 1.0f};
                
                glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
                glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
                glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, color);
                
                glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
                glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
                glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_CONSTANT);
                
                glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, 2.0f);
                
                glClientActiveTexture(GL_TEXTURE0);
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                glTexCoordPointer(2, GL_FLOAT, TexCoordSize + TexCoordSize + ColorSize + ColorSize + VertexSize, (const GLvoid *)(long)TexCoordSize);
            } else {
                glDisable(GL_TEXTURE_2D);
            }
            
            glEnableClientState(GL_COLOR_ARRAY);
            glColorPointer(4, GL_UNSIGNED_BYTE, TexCoordSize + TexCoordSize + ColorSize + ColorSize + VertexSize, (const GLvoid *)(long)(TexCoordSize + TexCoordSize + ColorSize));
            glVertexPointer(3, GL_FLOAT, TexCoordSize + TexCoordSize + ColorSize + ColorSize + VertexSize, (const GLvoid *)(long)(TexCoordSize + TexCoordSize + ColorSize + ColorSize));
            
            FaceIndexBuffers::iterator it;
            for (it = indexBuffers.begin(); it != indexBuffers.end(); ++it) {
                Model::Assets::Texture* texture = it->first;
                if (textured) texture->activate();
                IndexBuffer* indices = it->second;
                glDrawElements(GL_TRIANGLES, (GLsizei)indices->size(), GL_UNSIGNED_INT, &(*indices)[0]);
                if (textured) texture->deactivate();
            }
            
            if (textured)
                glDisable(GL_TEXTURE_2D);
            
            if (selected) {
                glActiveTexture(GL_TEXTURE1);
                m_selectionDummyTexture->deactivate();
                glDisable(GL_TEXTURE_2D);
            }
            
            /*
            if ([grid draw]) {
                glActiveTexture(GL_TEXTURE2);
                [grid deactivateTexture];
                glDisable(GL_TEXTURE_2D);
                glActiveTexture(GL_TEXTURE0);
            }
             */
            
            glPopClientAttrib();
        }

        MapRenderer::MapRenderer(Controller::Editor& editor) : m_editor(editor) {
            m_faceVbo = new Vbo(GL_ARRAY_BUFFER, 0xFFFF);
            m_selectionDummyTexture = NULL;
            
            Model::Map& map = m_editor.map();
            Model::Selection& selection = map.selection();
            
            map.mapLoaded               += new Model::Map::MapEvent::T<MapRenderer>(this, &MapRenderer::mapLoaded);
            map.mapCleared              += new Model::Map::MapEvent::T<MapRenderer>(this, &MapRenderer::mapCleared);
            selection.selectionAdded    += new Model::Selection::SelectionEvent::T<MapRenderer>(this, &MapRenderer::selectionAdded);
            selection.selectionRemoved  += new Model::Selection::SelectionEvent::T<MapRenderer>(this, &MapRenderer::selectionRemoved);
            
            addEntities(map.entities());
        }
        
        MapRenderer::~MapRenderer() {
            Model::Map& map = m_editor.map();
            Model::Selection& selection = map.selection();
            
            map.mapLoaded               -= new Model::Map::MapEvent::T<MapRenderer>(this, &MapRenderer::mapLoaded);
            map.mapCleared              -= new Model::Map::MapEvent::T<MapRenderer>(this, &MapRenderer::mapCleared);
            selection.selectionAdded    -= new Model::Selection::SelectionEvent::T<MapRenderer>(this, &MapRenderer::selectionAdded);
            selection.selectionRemoved  -= new Model::Selection::SelectionEvent::T<MapRenderer>(this, &MapRenderer::selectionRemoved);
            delete m_faceVbo;
            
            if (m_selectionDummyTexture != NULL)
                delete m_selectionDummyTexture;
        }

        
        void MapRenderer::render(RenderContext& context) {
            validate();
            
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glFrontFace(GL_CW);
            glEnable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);
            glShadeModel(GL_FLAT);
            glResetEdgeOffset();
            
            if (context.renderOrigin) {
                glDisable(GL_TEXTURE_2D);
                glBegin(GL_LINES);
                glColor4f(1, 0, 0, 0.5f);
                glVertex3f(-context.originAxisLength, 0, 0);
                glVertex3f(context.originAxisLength, 0, 0);
                glColor4f(0, 1, 0, 0.5f);
                glVertex3f(0, -context.originAxisLength, 0);
                glVertex3f(0, context.originAxisLength, 0);
                glColor4f(0, 0, 1, 0.5f);
                glVertex3f(0, 0, -context.originAxisLength);
                glVertex3f(0, 0, context.originAxisLength);
                glEnd();
            }
            
            m_faceVbo->activate();
            glEnableClientState(GL_VERTEX_ARRAY);
            renderFaces(true, false, m_faceIndexBuffers);
            if (!m_editor.map().selection().empty())
                renderFaces(true, true, m_selectedFaceIndexBuffers);
            
            glSetEdgeOffset(0.1f);
            renderEdges(NULL, m_edgeIndexBuffer);
            glResetEdgeOffset();
            
            if (!m_editor.map().selection().empty()) {
                glDisable(GL_DEPTH_TEST);
                renderEdges(&SelectionColor2, m_selectedEdgeIndexBuffer);
                glEnable(GL_DEPTH_TEST);
                
                glSetEdgeOffset(0.2f);
                glDepthFunc(GL_LEQUAL);
                renderEdges(&SelectionColor, m_selectedEdgeIndexBuffer);
                glDepthFunc(GL_LESS);
                glResetEdgeOffset();
            }
            
            glDisableClientState(GL_VERTEX_ARRAY);
            m_faceVbo->deactivate();
        }
    }
}