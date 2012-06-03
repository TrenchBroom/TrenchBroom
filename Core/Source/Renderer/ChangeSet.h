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

#ifndef TrenchBroom_ChangeSet_h
#define TrenchBroom_ChangeSet_h

#include <vector>

namespace TrenchBroom {
    namespace Model {
        class Entity;
        class Brush;
        class Face;
    }
    
    namespace Renderer {
        class ChangeSet {
        private:
            std::vector<Model::Entity*> m_addedEntities;
            std::vector<Model::Entity*> m_removedEntities;
            std::vector<Model::Entity*> m_changedEntities;
            std::vector<Model::Entity*> m_selectedEntities;
            std::vector<Model::Entity*> m_deselectedEntities;
            std::vector<Model::Brush*> m_addedBrushes;
            std::vector<Model::Brush*> m_removedBrushes;
            std::vector<Model::Brush*> m_changedBrushes;
            std::vector<Model::Brush*> m_selectedBrushes;
            std::vector<Model::Brush*> m_deselectedBrushes;
            std::vector<Model::Face*> m_changedFaces;
            std::vector<Model::Face*> m_selectedFaces;
            std::vector<Model::Face*> m_deselectedFaces;
            bool m_filterChanged;
            bool m_textureManagerChanged;
        public:
			ChangeSet();
            
            void entitiesAdded(const std::vector<Model::Entity*>& entities);
            void entitiesRemoved(const std::vector<Model::Entity*>& entities);
            void entitiesChanged(const std::vector<Model::Entity*>& entities);
            void entitiesSelected(const std::vector<Model::Entity*>& entities);
            void entitiesDeselected(const std::vector<Model::Entity*>& entities);
            void brushesAdded(const std::vector<Model::Brush*>& brushes);
            void brushesRemoved(const std::vector<Model::Brush*>& brushes);
            void brushesChanged(const std::vector<Model::Brush*>& brushes);
            void brushesSelected(const std::vector<Model::Brush*>& brushes);
            void brushesDeselected(const std::vector<Model::Brush*>& brushes);
            void facesChanged(const std::vector<Model::Face*>& faces);
            void facesSelected(const std::vector<Model::Face*>& faces);
            void facesDeselected(const std::vector<Model::Face*>& faces);
            void setFilterChanged();
            void setTextureManagerChanged();
            void clear();
            
            const std::vector<Model::Entity*> addedEntities() const;
            const std::vector<Model::Entity*> removedEntities() const;
            const std::vector<Model::Entity*> changedEntities() const;
            const std::vector<Model::Entity*> selectedEntities() const;
            const std::vector<Model::Entity*> deselectedEntities() const;
            const std::vector<Model::Brush*> addedBrushes() const;
            const std::vector<Model::Brush*> removedBrushes() const;
            const std::vector<Model::Brush*> changedBrushes() const;
            const std::vector<Model::Brush*> selectedBrushes() const;
            const std::vector<Model::Brush*> deselectedBrushes() const;
            const std::vector<Model::Face*> changedFaces() const;
            const std::vector<Model::Face*> selectedFaces() const;
            const std::vector<Model::Face*> deselectedFaces() const;
            bool filterChanged() const;
            bool textureManagerChanged() const;
        };
    }
}

#endif
