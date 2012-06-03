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
            m_addedEntities.insert(m_addedEntities.end(), entities.begin(), entities.end());
        }
        
        void ChangeSet::entitiesRemoved(const std::vector<Model::Entity*>& entities) {
            m_removedEntities.insert(m_removedEntities.end(), entities.begin(), entities.end());
        }
        
        void ChangeSet::entitiesChanged(const std::vector<Model::Entity*>& entities) {
            m_changedEntities.insert(m_changedEntities.end(), entities.begin(), entities.end());
        }
        
        void ChangeSet::entitiesSelected(const std::vector<Model::Entity*>& entities) {
            if (!m_deselectedEntities.empty()) {
                for (unsigned int i = 0; i < entities.size(); i++) {
                    Model::Entity* entity = entities[i];
                    std::vector<Model::Entity*>::iterator it = find(m_deselectedEntities.begin(), m_deselectedEntities.end(), entity);
                    if (it != m_deselectedEntities.end()) m_deselectedEntities.erase(it);
                    else m_selectedEntities.push_back(entity);
                }
            } else {
                m_selectedEntities.insert(m_selectedEntities.end(), entities.begin(), entities.end());
            }
        }
        
        void ChangeSet::entitiesDeselected(const std::vector<Model::Entity*>& entities) {
            m_deselectedEntities.insert(m_deselectedEntities.end(), entities.begin(), entities.end());
        }
        
        void ChangeSet::brushesAdded(const std::vector<Model::Brush*>& brushes) {
            m_addedBrushes.insert(m_addedBrushes.end(), brushes.begin(), brushes.end());
        }
        
        void ChangeSet::brushesRemoved(const std::vector<Model::Brush*>& brushes) {
            m_removedBrushes.insert(m_removedBrushes.end(), brushes.begin(), brushes.end());
        }
        
        void ChangeSet::brushesChanged(const std::vector<Model::Brush*>& brushes) {
            m_changedBrushes.insert(m_changedBrushes.end(), brushes.begin(), brushes.end());
        }
        
        void ChangeSet::brushesSelected(const std::vector<Model::Brush*>& brushes) {
            if (!m_deselectedBrushes.empty()) {
                for (unsigned int i = 0; i < brushes.size(); i++) {
                    Model::Brush* brush = brushes[i];
                    std::vector<Model::Brush*>::iterator it = find(m_deselectedBrushes.begin(), m_deselectedBrushes.end(), brush);
                    if (it != m_deselectedBrushes.end()) m_deselectedBrushes.erase(it);
                    else m_selectedBrushes.push_back(brush);
                }
            } else {
                m_selectedBrushes.insert(m_selectedBrushes.end(), brushes.begin(), brushes.end());
            }
        }
        
        void ChangeSet::brushesDeselected(const std::vector<Model::Brush*>& brushes) {
            m_deselectedBrushes.insert(m_deselectedBrushes.end(), brushes.begin(), brushes.end());
        }
        
        void ChangeSet::facesChanged(const std::vector<Model::Face*>& faces) {
            m_changedFaces.insert(m_changedFaces.end(), faces.begin(), faces.end());
        }
        
        void ChangeSet::facesSelected(const std::vector<Model::Face*>& faces) {
            if (!m_deselectedFaces.empty()) {
                for (unsigned int i = 0; i < faces.size(); i++) {
                    Model::Face* face = faces[i];
                    std::vector<Model::Face*>::iterator it = find(m_deselectedFaces.begin(), m_deselectedFaces.end(), face);
                    if (it != m_deselectedFaces.end()) m_deselectedFaces.erase(it);
                    else m_selectedFaces.push_back(face);
                }
            } else {
                m_selectedFaces.insert(m_selectedFaces.end(), faces.begin(), faces.end());
            }
        }
        
        void ChangeSet::facesDeselected(const std::vector<Model::Face*>& faces) {
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
        
        
        const std::vector<Model::Entity*> ChangeSet::addedEntities() const {
            return m_addedEntities;
        }
        
        const std::vector<Model::Entity*> ChangeSet::removedEntities() const {
            return m_removedEntities;
        }
        
        const std::vector<Model::Entity*> ChangeSet::changedEntities() const {
            return m_changedEntities;
        }
        
        const std::vector<Model::Entity*> ChangeSet::selectedEntities() const {
            return m_selectedEntities;
        }
        
        const std::vector<Model::Entity*> ChangeSet::deselectedEntities() const {
            return m_deselectedEntities;
        }
        
        const std::vector<Model::Brush*> ChangeSet::addedBrushes() const {
            return m_addedBrushes;
        }
        
        const std::vector<Model::Brush*> ChangeSet::removedBrushes() const {
            return m_removedBrushes;
        }
        
        const std::vector<Model::Brush*> ChangeSet::changedBrushes() const {
            return m_changedBrushes;
        }
        
        const std::vector<Model::Brush*> ChangeSet::selectedBrushes() const {
            return m_selectedBrushes;
        }
        
        const std::vector<Model::Brush*> ChangeSet::deselectedBrushes() const {
            return m_deselectedBrushes;
        }
        
        const std::vector<Model::Face*> ChangeSet::changedFaces() const {
            return m_changedFaces;
        }
        
        const std::vector<Model::Face*> ChangeSet::selectedFaces() const {
            return m_selectedFaces;
        }
        
        const std::vector<Model::Face*> ChangeSet::deselectedFaces() const {
            return m_deselectedFaces;
        }
        
        bool ChangeSet::filterChanged() const {
            return m_filterChanged;
        }
        
        bool ChangeSet::textureManagerChanged() const {
            return m_textureManagerChanged;
        }
    }
}