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
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>

namespace TrenchBroom {
    namespace Renderer {
        RenderContext::RenderContext() {
            backgroundColor.x = 0;
            backgroundColor.y = 0;
            backgroundColor.z = 0;
            backgroundColor.w = 1;
            renderOrigin = true;
            originAxisLength = 64;
        }

#pragma mark ChangeSet
        
        void ChangeSet::entitiesAdded(const vector<Entity*>& entities) {
            m_addedEntities.insert(m_addedEntities.end(), entities.begin(), entities.end());
        }
        
        void ChangeSet::entitiesRemoved(const vector<Entity*>& entities) {
            m_removedEntities.insert(m_removedEntities.end(), entities.begin(), entities.end());
        }
        
        void ChangeSet::entitiesChanged(const vector<Entity*>& entities) {
            m_changedEntities.insert(m_changedEntities.end(), entities.begin(), entities.end());
        }
        
        void ChangeSet::entitiesSelected(const vector<Entity*>& entities) {
            if (!m_deselectedEntities.empty()) {
                for (int i = 0; i < entities.size(); i++) {
                    Entity* entity = entities[i];
                    vector<Entity*>::iterator it = find(m_deselectedEntities.begin(), m_deselectedEntities.end(), entity);
                    if (it == m_deselectedEntities.end()) m_deselectedEntities.erase(it);
                    else m_selectedEntities.push_back(entity);
                }
            } else {
                m_selectedEntities.insert(m_selectedEntities.end(), entities.begin(), entities.end());
            }
        }
        
        void ChangeSet::entitiesDeselected(const vector<Entity*>& entities) {
            m_deselectedEntities.insert(m_deselectedEntities.end(), entities.begin(), entities.end());
        }
        
        void ChangeSet::brushesAdded(const vector<Brush*>& brushes) {
            m_addedBrushes.insert(m_addedBrushes.end(), brushes.begin(), brushes.end());
        }
        
        void ChangeSet::brushesRemoved(const vector<Brush*>& brushes) {
            m_removedBrushes.insert(m_removedBrushes.end(), brushes.begin(), brushes.end());
        }
        
        void ChangeSet::brushesChanged(const vector<Brush*>& brushes) {
            m_changedBrushes.insert(m_changedBrushes.end(), brushes.begin(), brushes.end());
        }
        
        void ChangeSet::brushesSelected(const vector<Brush*>& brushes) {
            if (!m_deselectedBrushes.empty()) {
                for (int i = 0; i < brushes.size(); i++) {
                    Brush* brush = brushes[i];
                    vector<Brush*>::iterator it = find(m_deselectedBrushes.begin(), m_deselectedBrushes.end(), brush);
                    if (it == m_deselectedBrushes.end()) m_deselectedBrushes.erase(it);
                    else m_selectedBrushes.push_back(brush);
                }
            } else {
                m_selectedBrushes.insert(m_selectedBrushes.end(), brushes.begin(), brushes.end());
            }
        }
        
        void ChangeSet::brushesDeselected(const vector<Brush*>& brushes) {
            m_deselectedBrushes.insert(m_deselectedBrushes.end(), brushes.begin(), brushes.end());
        }
        
        void ChangeSet::facesChanged(const vector<Face*>& faces) {
            m_changedFaces.insert(m_changedFaces.end(), faces.begin(), faces.end());
        }
        
        void ChangeSet::facesSelected(const vector<Face*>& faces) {
            if (!m_deselectedFaces.empty()) {
                for (int i = 0; i < faces.size(); i++) {
                    Face* face = faces[i];
                    vector<Face*>::iterator it = find(m_deselectedFaces.begin(), m_deselectedFaces.end(), face);
                    if (it == m_deselectedFaces.end()) m_deselectedFaces.erase(it);
                    else m_selectedFaces.push_back(face);
                }
            } else {
                m_selectedFaces.insert(m_selectedFaces.end(), faces.begin(), faces.end());
            }
        }
        
        void ChangeSet::facesDeselected(const vector<Face*>& faces) {
            m_deselectedFaces.insert(m_deselectedFaces.end(), faces.begin(), faces.end());
        }
        
        void ChangeSet::filterChanged() {
            m_filterChanged = true;
        }
        
        void ChangeSet::textureManagerChanged() {
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
        
        
        const vector<Entity*> ChangeSet::addedEntities() const {
            return m_addedEntities;
        }
        
        const vector<Entity*> ChangeSet::removedEntities() const {
            return m_removedEntities;
        }
        
        const vector<Entity*> ChangeSet::changedEntities() const {
            return m_changedEntities;
        }
        
        const vector<Entity*> ChangeSet::selectedEntities() const {
            return m_selectedEntities;
        }
        
        const vector<Entity*> ChangeSet::deselectedEntities() const {
            return m_deselectedEntities;
        }
        
        const vector<Brush*> ChangeSet::addedBrushes() const {
            return m_addedBrushes;
        }
        
        const vector<Brush*> ChangeSet::removedBrushes() const {
            return m_removedBrushes;
        }
        
        const vector<Brush*> ChangeSet::changedBrushes() const {
            return m_changedBrushes;
        }
        
        const vector<Brush*> ChangeSet::selectedBrushes() const {
            return m_selectedBrushes;
        }
        
        const vector<Brush*> ChangeSet::deselectedBrushes() const {
            return m_deselectedBrushes;
        }
        
        const vector<Face*> ChangeSet::changedFaces() const {
            return m_changedFaces;
        }
        
        const vector<Face*> ChangeSet::selectedFaces() const {
            return m_selectedFaces;
        }
        
        const vector<Face*> ChangeSet::deselectedFaces() const {
            return m_deselectedFaces;
        }
        
        bool ChangeSet::filterChanged() const {
            return m_filterChanged;
        }
        
        bool ChangeSet::textureManagerChanged() const {
            return m_textureManagerChanged;
        }
        

#pragma mark MapRenderer
        
        void MapRenderer::addEntities(const vector<Entity*>& entities) {
            m_changeSet.entitiesAdded(entities);
        }
        
        void MapRenderer::removeEntities(const vector<Entity*>& entities) {
            m_changeSet.entitiesRemoved(entities);
        }
        
        void MapRenderer::addBrushes(const vector<Brush*>& brushes) {
            m_changeSet.brushesAdded(brushes);
        }
        
        void MapRenderer::removeBrushes(const vector<Brush*>& brushes) {
            m_changeSet.brushesRemoved(brushes);
        }

        MapRenderer::MapRenderer(Controller::Editor& editor) : m_editor(editor) {
            m_faceVbo = new Vbo(GL_ARRAY_BUFFER, 0xFFFF);
        }
        
        MapRenderer::~MapRenderer() {
            delete m_faceVbo;
        }

        
        void MapRenderer::render(RenderContext& context) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glFrontFace(GL_CW);
            glEnable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);
            glShadeModel(GL_FLAT);
            
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
        }
    }
}