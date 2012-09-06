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

#ifndef __TrenchBroom__MapRenderer__
#define __TrenchBroom__MapRenderer__

#include "Model/BrushTypes.h"
#include "Model/EntityTypes.h"
#include "Model/FaceTypes.h"
#include "Model/Texture.h"
#include "Renderer/Text/TextRenderer.h"
#include "Utility/Color.h"
#include "Utility/GLee.h"

#include <map>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class EditStateChangeSet;
        class MapDocument;
    }
    
    namespace Renderer {
        class EntityRendererManager;
        class EntityRenderer;
        class RenderContext;
        class Vbo;
        class VboBlock;
        
        namespace Text {
            class StringManager;
        }
        
        class MapRenderer {
        private:
            class CompareTexturesById {
            public:
                inline bool operator() (const Model::Texture* left, const Model::Texture* right) const {
                    return left->uniqueId() < right->uniqueId();
                }
            };
            
            class EdgeRenderInfo {
            public:
                GLuint offset;
                GLuint vertexCount;
                EdgeRenderInfo() : offset(0), vertexCount(0) {};
                EdgeRenderInfo(GLuint offset, GLuint vertexCount) : offset(offset), vertexCount(vertexCount) {}
            };
            
            class TexturedTriangleRenderInfo {
            public:
                Model::Texture* texture;
                GLuint offset;
                GLuint vertexCount;
                TexturedTriangleRenderInfo(Model::Texture* texture, GLuint offset, GLuint vertexCount) : texture(texture), offset(offset), vertexCount(vertexCount) {}
            };
            
            class CachedEntityRenderer {
            public:
                EntityRenderer* renderer;
                String classname;
                CachedEntityRenderer() : renderer(NULL), classname("") {}
                CachedEntityRenderer(EntityRenderer* renderer, const String& classname) : renderer(renderer), classname(classname) {}
            };
            
            typedef std::vector<GLuint> IndexBuffer;
            typedef std::map<Model::Texture*, Model::FaceList, CompareTexturesById> FacesByTexture;
            typedef std::vector<TexturedTriangleRenderInfo> FaceRenderInfos;
            typedef std::map<Model::Entity*, CachedEntityRenderer> EntityRenderers;

            // level geometry rendering
            Vbo* m_faceVbo;
            VboBlock* m_faceBlock;
            VboBlock* m_selectedFaceBlock;
            VboBlock* m_lockedFaceBlock;
            Vbo* m_edgeVbo;
            VboBlock* m_edgeBlock;
            VboBlock* m_selectedEdgeBlock;
            VboBlock* m_lockedEdgeBlock;
            FaceRenderInfos m_faceRenderInfos;
            FaceRenderInfos m_selectedFaceRenderInfos;
            FaceRenderInfos m_lockedFaceRenderInfos;
            EdgeRenderInfo m_edgeRenderInfo;
            EdgeRenderInfo m_selectedEdgeRenderInfo;
            EdgeRenderInfo m_lockedEdgeRenderInfo;
            
            // entity bounds rendering
            Vbo* m_entityBoundsVbo;
            VboBlock* m_entityBoundsBlock;
            VboBlock* m_selectedEntityBoundsBlock;
            VboBlock* m_lockedEntityBoundsBlock;
            EdgeRenderInfo m_entityBoundsRenderInfo;
            EdgeRenderInfo m_selectedEntityBoundsRenderInfo;
            EdgeRenderInfo m_lockedEntityBoundsRenderInfo;
            
            // entity model rendering
            EntityRendererManager* m_entityRendererManager;
            EntityRenderers m_entityRenderers;
            EntityRenderers m_selectedEntityRenderers;
            EntityRenderers m_lockedEntityRenderers;
            bool m_entityRendererCacheValid;

            // classnames
            Text::TextRenderer<Model::Entity*>* m_classnameRenderer;
            Text::TextRenderer<Model::Entity*>* m_selectedClassnameRenderer;
            Text::TextRenderer<Model::Entity*>* m_lockedClassnameRenderer;

            /*
            // selection guides
            SizeGuideFigure* m_sizeGuideFigure;
            
            // figures
            Vbo* m_figureVbo;
            std::vector<Figure*> m_figures;
            */
             
            // state
            bool m_geometryDataValid;
            bool m_selectedGeometryDataValid;
            bool m_lockedGeometryDataValid;
            bool m_entityDataValid;
            bool m_selectedEntityDataValid;
            bool m_lockedEntityDataValid;
            
            /*
            GridRenderer* m_gridRenderer;
             */
            
            Text::StringManager* m_stringManager;

            Model::Texture* m_dummyTexture;

            Model::MapDocument& m_document;
            
            void writeFaceData(RenderContext& context, const FacesByTexture& facesByTexture, FaceRenderInfos& renderInfos, VboBlock& block);
            void writeEdgeData(RenderContext& context, const Model::BrushList& brushes, const Model::FaceList& faces, EdgeRenderInfo& renderInfo, VboBlock& block);
            void rebuildGeometryData(RenderContext& context);
            void writeEntityBounds(RenderContext& context, const Model::EntityList& entities, EdgeRenderInfo& renderInfo, VboBlock& block);
            void rebuildEntityData(RenderContext& context);
            bool reloadEntityModel(const Model::Entity& entity, CachedEntityRenderer& cachedRenderer);
            void reloadEntityModels(RenderContext& context, EntityRenderers& renderers);
            void reloadEntityModels(RenderContext& context);
            
            void validate(RenderContext& context);

            void renderEntityBounds(RenderContext& context, const EdgeRenderInfo& renderInfo, const Color* color);
            void renderEntityModels(RenderContext& context, EntityRenderers& entities);
            void renderEdges(RenderContext& context, const EdgeRenderInfo& renderInfo, const Color* color);
            void renderFaces(RenderContext& context, bool textured, bool selected, bool locked, const FaceRenderInfos& renderInfos);
            void renderFigures(RenderContext& context);
        public:
            MapRenderer(Model::MapDocument& document);
            ~MapRenderer();
            
            void addEntities(const Model::EntityList& entities);
            void removeEntities(const Model::EntityList& entities);
            void changeEditState(const Model::EditStateChangeSet& changeSet);
            void loadMap();
            void clearMap();

            void render(RenderContext& context);
        };
    }
}

#endif /* defined(__TrenchBroom__MapRenderer__) */
