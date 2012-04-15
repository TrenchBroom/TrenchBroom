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
#include "GLH/glplat.h"
#include "Utilities/VecMath.h"

using namespace std;

namespace TrenchBroom {
    class Filter;

    namespace Model {
        class Map;
        class Entity;
        class Brush;
        class Face;
        class SelectionEventData;
        class Preferences;

        namespace Assets {
            class Texture;
        }
    }

    namespace Controller {
        class Editor;
        class Camera;
        class TransientOptions;
    }

    namespace Renderer {
        class Vbo;
        class VboBlock;
        class EntityRenderer;
        class EntityRendererManager;
        class StringRenderer;
        class TextRenderer;
        class GridRenderer;
        class FontManager;

        class RenderContext {
        public:
            Controller::Camera& camera;
            Filter& filter;
            Controller::TransientOptions& options;
            Model::Preferences& preferences;
            RenderContext(Controller::Camera& camera, Filter& filter, Controller::TransientOptions& options);
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
            typedef map<Model::Entity*, EntityRenderer*> EntityRenderers;

            Controller::Editor& m_editor;
            Vbo* m_faceVbo;

            // level geometry rendering
            FaceIndexBuffers m_faceIndexBuffers;
            FaceIndexBuffers m_selectedFaceIndexBuffers;
            IndexBuffer m_edgeIndexBuffer;
            IndexBuffer m_selectedEdgeIndexBuffer;

            // grid
            GridRenderer* m_gridRenderer;

            // entity bounds rendering
            Vbo* m_entityBoundsVbo;
            Vbo* m_selectedEntityBoundsVbo;
            int m_entityBoundsVertexCount;
            int m_selectedEntityBoundsVertexCount;

            // entity model rendering
            EntityRendererManager* m_entityRendererManager;
            EntityRenderers m_entityRenderers;
            EntityRenderers m_selectedEntityRenderers;
            bool m_entityRendererCacheValid;

            // classnames
            TextRenderer* m_classnameRenderer;
            TextRenderer* m_selectedClassnameRenderer;

            // selection guides
            BBox m_selectionBounds;
            StringRenderer* m_guideStrings[3];

            ChangeSet m_changeSet;
            Model::Assets::Texture* m_selectionDummyTexture;
            FontManager& m_fontManager;

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

            void writeFaceVertices(RenderContext& context, Model::Face& face, VboBlock& block);
            void writeFaceIndices(RenderContext& context, Model::Face& face, IndexBuffer& triangleBuffer, IndexBuffer& edgeBuffer);
            void writeEntityBounds(RenderContext& context, Model::Entity& entity, VboBlock& block);
            void updateSelectionBounds(RenderContext& context);

            void rebuildFaceIndexBuffers(RenderContext& context);
            void rebuildSelectedFaceIndexBuffers(RenderContext& context);

            void validateEntityRendererCache(RenderContext& context);
            void validateAddedEntities(RenderContext& context);
            void validateRemovedEntities(RenderContext& context);
            void validateChangedEntities(RenderContext& context);
            void validateAddedBrushes(RenderContext& context);
            void validateRemovedBrushes(RenderContext& context);
            void validateChangedBrushes(RenderContext& context);
            void validateChangedFaces(RenderContext& context);
            void validateSelection(RenderContext& context);
            void validateDeselection(RenderContext& context);
            void validate(RenderContext& context);

            void renderSelectionGuides(RenderContext& context, const Vec4f& color);
            void renderEntityBounds(RenderContext& context, const Vec4f* color, int vertexCount);
            void renderEntityModels(RenderContext& context, EntityRenderers& entities);
            void renderEdges(RenderContext& context, const Vec4f* color, const IndexBuffer& indexBuffer);
            void renderFaces(RenderContext& context, bool textured, bool selected, FaceIndexBuffers& indexBuffers);
        public:
            MapRenderer(Controller::Editor& editor, FontManager& fontManager);
            ~MapRenderer();
            void render(RenderContext& context);
        };
    }
}

#endif
