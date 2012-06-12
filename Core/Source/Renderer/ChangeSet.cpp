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

#include "Renderer/ChangeSet.h"
#include <algorithm>

#include "Model/Map/Entity.h"
#include "Model/Map/Brush.h"
#include "Model/Map/Face.h"

namespace TrenchBroom {
    namespace Renderer {
		ChangeSet::ChangeSet() : m_filterChanged(false), m_textureManagerChanged(false) {}
        
        void ChangeSet::entitiesAdded(const std::vector<Model::Entity*>& entities) {
            m_addedEntities.insert(entities.begin(), entities.end());
        }
        
        void ChangeSet::entitiesRemoved(const std::vector<Model::Entity*>& entities) {
            for (unsigned int i = 0; i < entities.size(); i++) {
                std::set<Model::Entity*>::iterator it;
                if ((it = find(m_addedEntities.begin(), m_addedEntities.end(), entities[i])) != m_addedEntities.end())
                    m_addedEntities.erase(it);
                if ((it = find(m_changedEntities.begin(), m_changedEntities.end(), entities[i])) != m_changedEntities.end())
                    m_changedEntities.erase(it);
                if ((it = find(m_selectedEntities.begin(), m_selectedEntities.end(), entities[i])) != m_selectedEntities.end())
                    m_selectedEntities.erase(it);
                if ((it = find(m_deselectedEntities.begin(), m_deselectedEntities.end(), entities[i])) != m_deselectedEntities.end())
                    m_deselectedEntities.erase(it);
            }
            m_removedEntities.insert(entities.begin(), entities.end());
        }
        
        void ChangeSet::entitiesChanged(const std::vector<Model::Entity*>& entities) {
            m_changedEntities.insert(entities.begin(), entities.end());
        }
        
        void ChangeSet::entitiesSelected(const std::vector<Model::Entity*>& entities) {
            if (!m_deselectedEntities.empty()) {
                for (unsigned int i = 0; i < entities.size(); i++) {
                    Model::Entity* entity = entities[i];
                    std::set<Model::Entity*>::iterator it = find(m_deselectedEntities.begin(), m_deselectedEntities.end(), entity);
                    if (it != m_deselectedEntities.end()) m_deselectedEntities.erase(it);
                    else m_selectedEntities.insert(entity);
                }
            } else {
                m_selectedEntities.insert(entities.begin(), entities.end());
            }
        }
        
        void ChangeSet::entitiesDeselected(const std::vector<Model::Entity*>& entities) {
            m_deselectedEntities.insert(entities.begin(), entities.end());
        }
        
        void ChangeSet::brushesAdded(const std::vector<Model::Brush*>& brushes) {
            m_addedBrushes.insert(brushes.begin(), brushes.end());
        }
        
        void ChangeSet::brushesRemoved(const std::vector<Model::Brush*>& brushes) {
            for (unsigned int i = 0; i < brushes.size(); i++) {
                std::set<Model::Brush*>::iterator it;
                if ((it = find(m_addedBrushes.begin(), m_addedBrushes.end(), brushes[i])) != m_addedBrushes.end())
                    m_addedBrushes.erase(it);
                if ((it = find(m_changedBrushes.begin(), m_changedBrushes.end(), brushes[i])) != m_changedBrushes.end())
                    m_changedBrushes.erase(it);
                if ((it = find(m_selectedBrushes.begin(), m_selectedBrushes.end(), brushes[i])) != m_selectedBrushes.end())
                    m_selectedBrushes.erase(it);
                if ((it = find(m_deselectedBrushes.begin(), m_deselectedBrushes.end(), brushes[i])) != m_deselectedBrushes.end())
                    m_deselectedBrushes.erase(it);
            }
            m_removedBrushes.insert(brushes.begin(), brushes.end());
        }
        
        void ChangeSet::brushesChanged(const std::vector<Model::Brush*>& brushes) {
            m_changedBrushes.insert(brushes.begin(), brushes.end());
        }
        
        void ChangeSet::brushesSelected(const std::vector<Model::Brush*>& brushes) {
            if (!m_deselectedBrushes.empty()) {
                for (unsigned int i = 0; i < brushes.size(); i++) {
                    Model::Brush* brush = brushes[i];
                    std::set<Model::Brush*>::iterator it = find(m_deselectedBrushes.begin(), m_deselectedBrushes.end(), brush);
                    if (it != m_deselectedBrushes.end()) m_deselectedBrushes.erase(it);
                    else m_selectedBrushes.insert(brush);
                }
            } else {
                m_selectedBrushes.insert(brushes.begin(), brushes.end());
            }
        }
        
        void ChangeSet::brushesDeselected(const std::vector<Model::Brush*>& brushes) {
            m_deselectedBrushes.insert(brushes.begin(), brushes.end());
        }
        
        void ChangeSet::facesChanged(const std::vector<Model::Face*>& faces) {
            m_changedFaces.insert(faces.begin(), faces.end());
        }
        
        void ChangeSet::facesSelected(const std::vector<Model::Face*>& faces) {
            if (!m_deselectedFaces.empty()) {
                for (unsigned int i = 0; i < faces.size(); i++) {
                    Model::Face* face = faces[i];
                    std::set<Model::Face*>::iterator it = find(m_deselectedFaces.begin(), m_deselectedFaces.end(), face);
                    if (it != m_deselectedFaces.end()) m_deselectedFaces.erase(it);
                    else m_selectedFaces.insert(face);
                }
            } else {
                m_selectedFaces.insert(faces.begin(), faces.end());
            }
        }
        
        void ChangeSet::facesDeselected(const std::vector<Model::Face*>& faces) {
            m_deselectedFaces.insert(faces.begin(), faces.end());
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
        
        
        const std::vector<Model::Entity*> ChangeSet::addedEntities() const {
            return std::vector<Model::Entity*>(m_addedEntities.begin(), m_addedEntities.end());
        }
        
        const std::vector<Model::Entity*> ChangeSet::removedEntities() const {
            return std::vector<Model::Entity*>(m_removedEntities.begin(), m_removedEntities.end());
        }
        
        const std::vector<Model::Entity*> ChangeSet::changedEntities() const {
            return std::vector<Model::Entity*>(m_changedEntities.begin(), m_changedEntities.end());
        }
        
        const std::vector<Model::Entity*> ChangeSet::selectedEntities() const {
            return std::vector<Model::Entity*>(m_selectedEntities.begin(), m_selectedEntities.end());
        }
        
        const std::vector<Model::Entity*> ChangeSet::deselectedEntities() const {
            return std::vector<Model::Entity*>(m_deselectedEntities.begin(), m_deselectedEntities.end());
        }
        
        const std::vector<Model::Brush*> ChangeSet::addedBrushes() const {
            return std::vector<Model::Brush*>(m_addedBrushes.begin(), m_addedBrushes.end());
        }
        
        const std::vector<Model::Brush*> ChangeSet::removedBrushes() const {
            return std::vector<Model::Brush*>(m_removedBrushes.begin(), m_removedBrushes.end());
        }
        
        const std::vector<Model::Brush*> ChangeSet::changedBrushes() const {
            return std::vector<Model::Brush*>(m_changedBrushes.begin(), m_changedBrushes.end());
        }
        
        const std::vector<Model::Brush*> ChangeSet::selectedBrushes() const {
            return std::vector<Model::Brush*>(m_selectedBrushes.begin(), m_selectedBrushes.end());
        }
        
        const std::vector<Model::Brush*> ChangeSet::deselectedBrushes() const {
            return std::vector<Model::Brush*>(m_deselectedBrushes.begin(), m_deselectedBrushes.end());
        }
        
        const std::vector<Model::Face*> ChangeSet::changedFaces() const {
            return std::vector<Model::Face*>(m_changedFaces.begin(), m_changedFaces.end());
        }
        
        const std::vector<Model::Face*> ChangeSet::selectedFaces() const {
            return std::vector<Model::Face*>(m_selectedFaces.begin(), m_selectedFaces.end());
        }
        
        const std::vector<Model::Face*> ChangeSet::deselectedFaces() const {
            return std::vector<Model::Face*>(m_deselectedFaces.begin(), m_deselectedFaces.end());
        }
        
        bool ChangeSet::filterChanged() const {
            return m_filterChanged;
        }
        
        bool ChangeSet::textureManagerChanged() const {
            return m_textureManagerChanged;
        }
    }
}