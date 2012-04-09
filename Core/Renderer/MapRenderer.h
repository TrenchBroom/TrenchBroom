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

#ifndef TrenchBroom_MapRenderer_h
#define TrenchBroom_MapRenderer_h

#include <map>
#include <vector>
#include "VecMath.h"
#include "Vbo.h"
#include "Editor.h"

namespace TrenchBroom {
    namespace Renderer {
        class Entity;
        class Brush;
        class Face;
        
        class RenderContext {
        public:
            Vec4f backgroundColor;
            bool renderOrigin;
            float originAxisLength;
            RenderContext();
        };
        
        class ChangeSet {
        private:
            vector<Entity*> m_addedEntities;
            vector<Entity*> m_removedEntities;
            vector<Entity*> m_changedEntities;
            vector<Entity*> m_selectedEntities;
            vector<Entity*> m_deselectedEntities;
            vector<Brush*> m_addedBrushes;
            vector<Brush*> m_removedBrushes;
            vector<Brush*> m_changedBrushes;
            vector<Brush*> m_selectedBrushes;
            vector<Brush*> m_deselectedBrushes;
            vector<Face*> m_changedFaces;
            vector<Face*> m_selectedFaces;
            vector<Face*> m_deselectedFaces;
            bool m_filterChanged;
            bool m_textureManagerChanged;
        public:
            void entitiesAdded(const vector<Entity*>& entities);
            void entitiesRemoved(const vector<Entity*>& entities);
            void entitiesChanged(const vector<Entity*>& entities);
            void entitiesSelected(const vector<Entity*>& entities);
            void entitiesDeselected(const vector<Entity*>& entities);
            void brushesAdded(const vector<Brush*>& brushes);
            void brushesRemoved(const vector<Brush*>& brushes);
            void brushesChanged(const vector<Brush*>& brushes);
            void brushesSelected(const vector<Brush*>& brushes);
            void brushesDeselected(const vector<Brush*>& brushes);
            void facesChanged(const vector<Face*>& faces);
            void facesSelected(const vector<Face*>& faces);
            void facesDeselected(const vector<Face*>& faces);
            void filterChanged();
            void textureManagerChanged();
            void clear();
            
            const vector<Entity*> addedEntities() const;
            const vector<Entity*> removedEntities() const;
            const vector<Entity*> changedEntities() const;
            const vector<Entity*> selectedEntities() const;
            const vector<Entity*> deselectedEntities() const;
            const vector<Brush*> addedBrushes() const;
            const vector<Brush*> removedBrushes() const;
            const vector<Brush*> changedBrushes() const;
            const vector<Brush*> selectedBrushes() const;
            const vector<Brush*> deselectedBrushes() const;
            const vector<Face*> changedFaces() const;
            const vector<Face*> selectedFaces() const;
            const vector<Face*> deselectedFaces() const;
            bool filterChanged() const;
            bool textureManagerChanged() const;
        };
        
        class MapRenderer {
        private:
            Controller::Editor& m_editor;
            Vbo* m_faceVbo;
            map<int, vector<unsigned char> > m_faceIndexBuffers;
            vector<unsigned char> m_edgeIndexBuffer;
            ChangeSet m_changeSet;
            
            void addEntities(const vector<Entity*>& entities);
            void removeEntities(const vector<Entity*>& entities);
            void addBrushes(const vector<Brush*>& brushes);
            void removeBrushes(const vector<Brush*>& brushes);
        public:
            MapRenderer(Controller::Editor& editor);
            ~MapRenderer();
            void render(RenderContext& context);
        };
    }
}

#endif
