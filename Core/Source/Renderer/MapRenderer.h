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

#include "GL/GLee.h"
#include "Model/Assets/Texture.h"
#include "Model/Map/BrushTypes.h"
#include "Model/Map/EntityTypes.h"
#include "Model/Map/Face.h"
#include "Renderer/FontManager.h"
#include "Renderer/TextRenderer.h"
#include "Utilities/Event.h"
#include "Utilities/VecMath.h"

#include <map>
#include <vector>

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
        class FontManager;
        class ChangeSet;
        class RenderContext;
        class Figure;
        class SizeGuideFigure;

        class MapRenderer {
        private:
            class CompareTexturesById {
            public:
                bool operator() (const Model::Assets::Texture* left, const Model::Assets::Texture* right) const {
                    return left->uniqueId < right->uniqueId;
                }
            };

            class EdgeRenderInfo {
            public:
                GLuint offset;
                GLuint vertexCount;
                EdgeRenderInfo() : offset(0), vertexCount(0) {};
                EdgeRenderInfo(GLuint offset, GLuint vertexCount);
            };
            
            class TexturedTriangleRenderInfo {
            public:
                Model::Assets::Texture* texture;
                GLuint offset;
                GLuint vertexCount;
                TexturedTriangleRenderInfo(Model::Assets::Texture* texture, GLuint offset, GLuint vertexCount);
            };
            
            class CachedEntityRenderer {
            public:
                EntityRenderer* renderer;
                std::string classname;
                CachedEntityRenderer() : renderer(NULL), classname("") {}
                CachedEntityRenderer(EntityRenderer* renderer, const std::string& classname) : renderer(renderer), classname(classname) {}
            };
            
            typedef std::vector<GLuint> IndexBuffer;
            typedef std::map<Model::Assets::Texture*, Model::FaceList, CompareTexturesById> FacesByTexture;
            typedef std::vector<TexturedTriangleRenderInfo> FaceRenderInfos;
            typedef std::map<Model::Entity*, CachedEntityRenderer> EntityRenderers;

            Controller::Editor& m_editor;

            // level geometry rendering
            Vbo* m_faceVbo;
            VboBlock* m_faceBlock;
            VboBlock* m_selectedFaceBlock;
            Vbo* m_edgeVbo;
            VboBlock* m_edgeBlock;
            VboBlock* m_selectedEdgeBlock;
            FaceRenderInfos m_faceRenderInfos;
            FaceRenderInfos m_selectedFaceRenderInfos;
            EdgeRenderInfo m_edgeRenderInfo;
            EdgeRenderInfo m_selectedEdgeRenderInfo;

            // entity bounds rendering
            Vbo* m_entityBoundsVbo;
            VboBlock* m_entityBoundsBlock;
            VboBlock* m_selectedEntityBoundsBlock;
            EdgeRenderInfo m_entityBoundsRenderInfo;
            EdgeRenderInfo m_selectedEntityBoundsRenderInfo;

            // entity model rendering
            EntityRendererManager* m_entityRendererManager;
            EntityRenderers m_entityRenderers;
            EntityRenderers m_selectedEntityRenderers;
            bool m_entityRendererCacheValid;

            // classnames
            TextRenderer<Model::Entity*>* m_classnameRenderer;
            TextRenderer<Model::Entity*>* m_selectedClassnameRenderer;

            // selection guides
            SizeGuideFigure* m_sizeGuideFigure;

            // figures
            Vbo* m_figureVbo;
            std::vector<Figure*> m_figures;
            
            // state
            bool m_entityDataValid;
            bool m_selectedEntityDataValid;
            bool m_geometryDataValid;
            bool m_selectedGeometryDataValid;

            GridRenderer* m_gridRenderer;
            Model::Assets::Texture* m_dummyTexture;
            FontManager& m_fontManager;

            void writeFaceData(RenderContext& context, FacesByTexture& facesByTexture, FaceRenderInfos& renderInfos, VboBlock& block);
            void writeEdgeData(RenderContext& context, Model::BrushList& brushes, Model::FaceList& faces, EdgeRenderInfo& renderInfo, VboBlock& block);
            void rebuildGeometryData(RenderContext& context);
            void writeEntityBounds(RenderContext& context, const Model::EntityList& entities, EdgeRenderInfo& renderInfo, VboBlock& block);
            void rebuildEntityData(RenderContext& context);
            bool reloadEntityModel(const Model::Entity& entity, CachedEntityRenderer& cachedRenderer);
            void reloadEntityModels(RenderContext& context, EntityRenderers& renderers);
            void reloadEntityModels(RenderContext& context);
            
            void entitiesWereAdded(const Model::EntityList& entities);
            void entitiesWillBeRemoved(const Model::EntityList& entities);
            void propertiesDidChange(const Model::EntityList& entities);
            void brushesWereAdded(const Model::BrushList& brushes);
            void brushesWillBeRemoved(const Model::BrushList& brushes);
            void brushesWillChange(const Model::BrushList& brushes);
            void brushesDidChange(const Model::BrushList& brushes);
            void facesDidChange(const Model::FaceList& faces);
            void mapLoaded(Model::Map& map);
            void mapCleared(Model::Map& map);
            void selectionAdded(const Model::SelectionEventData& event);
            void selectionRemoved(const Model::SelectionEventData& event);
            void textureManagerDidChange(Model::Assets::TextureManager& textureManager);
            void cameraDidChange(Controller::Camera& camera);
            void gridDidChange(Controller::Grid& grid);
            void preferencesDidChange(const std::string& name);
            void optionsDidChange(const Controller::TransientOptions& options);

            void validate(RenderContext& context);

            void renderEntityBounds(RenderContext& context, const EdgeRenderInfo& renderInfo, const Vec4f* color);
            void renderEntityModels(RenderContext& context, EntityRenderers& entities);
            void renderEdges(RenderContext& context, const EdgeRenderInfo& renderInfo, const Vec4f* color);
            void renderFaces(RenderContext& context, bool textured, bool selected, const FaceRenderInfos& renderInfos);
            void renderFigures(RenderContext& context);
        public:
            typedef Event<MapRenderer&> MapRendererEvent;
            
            MapRenderer(Controller::Editor& editor, FontManager& fontManager);
            ~MapRenderer();
            
            void addFigure(Figure& figure);
            void removeFigure(Figure& figure);
            
            void render();
            
            EntityRendererManager& entityRendererManager();
            FontManager& fontManager();
            
            MapRendererEvent rendererChanged;
        };
    }
}

#endif
