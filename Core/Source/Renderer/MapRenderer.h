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
#include "GL/GLee.h"
#include "Renderer/ChangeSet.h"
#include "Renderer/FontManager.h"
#include "Utilities/Event.h"
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
            class TextureManager;
        }
    }

    namespace Controller {
        class Camera;
        class Editor;
        class Grid;
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
        class ChangeSet;
        class RenderContext;
        class Figure;

        class MapRenderer {
        private:
            typedef vector<GLuint> IndexBuffer;
            typedef map<Model::Assets::Texture*, VboBlock* > FaceIndexBlocks;
            typedef map<Model::Entity*, EntityRenderer*> EntityRenderers;

            Controller::Editor& m_editor;
            Vbo* m_faceVbo;
            Vbo* m_faceIndexVbo;
            Vbo* m_edgeIndexVbo;

            // level geometry rendering
            FaceIndexBlocks m_faceIndexBlocks;
            FaceIndexBlocks m_selectedFaceIndexBlocks;
            VboBlock* m_edgeIndexBlock;
            VboBlock* m_selectedEdgeIndexBlock;

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

            // figures
            std::vector<Figure*> m_figures;
            
            ChangeSet m_changeSet;
            Model::Assets::Texture* m_dummyTexture;
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
            void textureManagerDidChange(Model::Assets::TextureManager& textureManager);
            void cameraDidChange(Controller::Camera& camera);
            void gridDidChange(Controller::Grid& grid);
            void preferencesDidChange(const std::string& name);

            void writeFaceVertices(RenderContext& context, Model::Face& face, VboBlock& block);
            unsigned int writeFaceIndices(RenderContext& context, Model::Face& face, VboBlock& block, unsigned int offset);
            unsigned int writeEdgeIndices(RenderContext& context, Model::Face& face, VboBlock& block, unsigned int offset);
            void writeEntityBounds(RenderContext& context, Model::Entity& entity, VboBlock& block);

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
            void renderEdges(RenderContext& context, const Vec4f* color, const VboBlock* indexBlock);
            void renderFaces(RenderContext& context, bool textured, bool selected, const FaceIndexBlocks& indexBlocks);
            void renderFigures(RenderContext& context);
        public:
            typedef Event<MapRenderer&> MapRendererEvent;
            
            MapRenderer(Controller::Editor& editor, FontManager& fontManager);
            ~MapRenderer();
            
            void addFigure(Figure& figure);
            void removeFigure(Figure& figure);
            
            void render(RenderContext& context);
            
            EntityRendererManager& entityRendererManager();
            
            MapRendererEvent rendererChanged;
        };
    }
}

#endif
