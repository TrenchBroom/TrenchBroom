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

using namespace std;

namespace TrenchBroom {
    namespace Model {
        class Entity;
        class Brush;
        class Face;
    }
    
    namespace Renderer {
        class ChangeSet {
        private:
            vector<Model::Entity*> m_addedEntities;
            vector<Model::Entity*> m_removedEntities;
            vector<Model::Entity*> m_changedEntities;
            vector<Model::Entity*> m_selectedEntities;
            vector<Model::Entity*> m_deselectedEntities;
            vector<Model::Brush*> m_addedBrushes;
            vector<Model::Brush*> m_removedBrushes;
            vector<Model::Brush*> m_changedBrushes;
            vector<Model::Brush*> m_selectedBrushes;
            vector<Model::Brush*> m_deselectedBrushes;
            vector<Model::Face*> m_changedFaces;
            vector<Model::Face*> m_selectedFaces;
            vector<Model::Face*> m_deselectedFaces;
            bool m_filterChanged;
            bool m_textureManagerChanged;
        public:
			ChangeSet();
            
            void entitiesAdded(const vector<Model::Entity*>& entities);
            void entitiesRemoved(const vector<Model::Entity*>& entities);
            void entitiesChanged(const vector<Model::Entity*>& entities);
            void entitiesSelected(const vector<Model::Entity*>& entities);
            void entitiesDeselected(const vector<Model::Entity*>& entities);
            void brushesAdded(const vector<Model::Brush*>& brushes);
            void brushesRemoved(const vector<Model::Brush*>& brushes);
            void brushesChanged(const vector<Model::Brush*>& brushes);
            void brushesSelected(const vector<Model::Brush*>& brushes);
            void brushesDeselected(const vector<Model::Brush*>& brushes);
            void facesChanged(const vector<Model::Face*>& faces);
            void facesSelected(const vector<Model::Face*>& faces);
            void facesDeselected(const vector<Model::Face*>& faces);
            void setFilterChanged();
            void setTextureManagerChanged();
            void clear();
            
            const vector<Model::Entity*> addedEntities() const;
            const vector<Model::Entity*> removedEntities() const;
            const vector<Model::Entity*> changedEntities() const;
            const vector<Model::Entity*> selectedEntities() const;
            const vector<Model::Entity*> deselectedEntities() const;
            const vector<Model::Brush*> addedBrushes() const;
            const vector<Model::Brush*> removedBrushes() const;
            const vector<Model::Brush*> changedBrushes() const;
            const vector<Model::Brush*> selectedBrushes() const;
            const vector<Model::Brush*> deselectedBrushes() const;
            const vector<Model::Face*> changedFaces() const;
            const vector<Model::Face*> selectedFaces() const;
            const vector<Model::Face*> deselectedFaces() const;
            bool filterChanged() const;
            bool textureManagerChanged() const;
        };
    }
}

#endif
