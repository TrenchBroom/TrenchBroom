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
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include "VecMath.h"

using namespace std;

namespace TrenchBroom {
    namespace Model {
        class Map;
        class Entity;
        class Brush;
        class Face;
        class SelectionEventData;
        
        namespace Assets {
            class Texture;
        }
    }

    namespace Controller {
        class Editor;
    }
    
    namespace Renderer {
        class Vbo;
        class VboBlock;
        
        class RenderContext {
        public:
            Vec4f backgroundColor;
            bool renderOrigin;
            float originAxisLength;
            RenderContext();
        };
        
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
        
        class MapRenderer {
        private:
            typedef vector<GLuint> IndexBuffer;
            typedef map<Model::Assets::Texture*, IndexBuffer* > FaceIndexBuffers;

            Controller::Editor& m_editor;
            Vbo* m_faceVbo;
            FaceIndexBuffers m_faceIndexBuffers;
            FaceIndexBuffers m_selectedFaceIndexBuffers;
            IndexBuffer m_edgeIndexBuffer;
            IndexBuffer m_selectedEdgeIndexBuffer;
            ChangeSet m_changeSet;
            Model::Assets::Texture* m_selectionDummyTexture;
            
            void addEntities(const vector<Model::Entity*>& entities);
            void removeEntities(const vector<Model::Entity*>& entities);
            void addBrushes(const vector<Model::Brush*>& brushes);
            void removeBrushes(const vector<Model::Brush*>& brushes);
            void entitiesWereAdded(const vector<Model::Entity*>& entities);
            void entitiesWillBeRemoved(const vector<Model::Entity*>& entities);
            void propertiesDidChange(const vector<Model::Entity*>& entities);
            void brushesWereAdded(const vector<Model::Brush*>& brushes);
            void brushesWillBeRemoved(const vector<Model::Brush*>& brushes);
            void brushesDidChange(const vector<Model::Brush*>& brushes);
            void facesDidChange(const vector<Model::Face*>& faces);
            void mapLoaded(Model::Map& map);
            void mapCleared(Model::Map& map);
            void selectionAdded(const Model::SelectionEventData& event);
            void selectionRemoved(const Model::SelectionEventData& event);
            
            void writeFaceVertices(Model::Face& face, VboBlock& block);
            void writeFaceIndices(Model::Face& face, IndexBuffer& triangleBuffer, IndexBuffer& edgeBuffer);
            
            void rebuildFaceIndexBuffers();
            void rebuildSelectedFaceIndexBuffers();
            
            void validateEntityRendererCache();
            void validateAddedEntities();
            void validateRemovedEntities();
            void validateChangedEntities();
            void validateAddedBrushes();
            void validateRemovedBrushes();
            void validateChangedBrushes();
            void validateChangedFaces();
            void validateSelection();
            void validateDeselection();
            void validate();
            
            void renderEdges(const Vec4f* color, const IndexBuffer& indexBuffer);
            void renderFaces(bool textured, bool selected, FaceIndexBuffers& indexBuffers);
        public:
            MapRenderer(Controller::Editor& editor);
            ~MapRenderer();
            void render(RenderContext& context);
        };
    }
}

#endif
