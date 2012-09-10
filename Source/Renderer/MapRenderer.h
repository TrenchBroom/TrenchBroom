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
#include "Model/Face.h"
#include "Model/Texture.h"
#include "Renderer/VertexArray.h"
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
        class Shader;
        class ShaderProgram;
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
            
            class FaceRenderInfo {
            public:
                Model::Texture* texture;
                mutable VertexArrayPtr vertexArray;
                
                FaceRenderInfo(Model::Texture* texture, VertexArrayPtr vertexArray) :
                texture(texture),
                vertexArray(vertexArray) {}
                
                FaceRenderInfo(const FaceRenderInfo& other) : texture(other.texture), vertexArray(other.vertexArray) {}
                FaceRenderInfo() : texture(NULL) {}
            };
            
            typedef std::vector<FaceRenderInfo> FaceRenderInfoList;
            
            class CachedEntityRenderer {
            public:
                EntityRenderer* renderer;
                String classname;
                CachedEntityRenderer() : renderer(NULL), classname("") {}
                CachedEntityRenderer(EntityRenderer* renderer, const String& classname) : renderer(renderer), classname(classname) {}
            };
            
            class TextureFaceList {
            private:
                Model::FaceList m_faces;
                size_t m_vertexCount;
            public:
                TextureFaceList() : m_vertexCount(0) {}

                inline void add(Model::Face& face) {
                    m_faces.push_back(&face);
                    m_vertexCount += (face.vertices().size() - 2) * 3;
                }
                
                inline const Model::FaceList& faces() const {
                    return m_faces;
                }
                
                inline size_t vertexCount() const {
                    return m_vertexCount;
                }
            };
            
            typedef std::vector<GLuint> IndexBuffer;
            typedef std::map<Model::Texture*, TextureFaceList, CompareTexturesById> FacesByTexture;
            typedef std::map<Model::Entity*, CachedEntityRenderer> EntityRenderers;

            // level geometry rendering
            VboPtr m_faceVbo;
            FaceRenderInfoList m_faceRenderInfos;
            FaceRenderInfoList m_selectedFaceRenderInfos;
            FaceRenderInfoList m_lockedFaceRenderInfos;
            VboPtr m_edgeVbo;
            VertexArrayPtr m_edgeVertexArray;
            VertexArrayPtr m_selectedEdgeVertexArray;
            VertexArrayPtr m_lockedEdgeVertexArray;
            
            // entity bounds rendering
            VboPtr m_entityBoundsVbo;
            
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

            // shaders
            bool m_shadersCreated;
            ShaderPtr m_coloredEdgeVertexShader;
            ShaderPtr m_constantColoredEdgeVertexShader;
            ShaderPtr m_edgeFragmentShader;
            ShaderPtr m_faceVertexShader;
            ShaderPtr m_faceFragmentShader;
            ShaderProgramPtr m_coloredEdgeProgram;
            ShaderProgramPtr m_constantColoredEdgeProgram;
            ShaderProgramPtr m_faceProgram;
            
            /*
            // selection guides
            SizeGuideFigure* m_sizeGuideFigure;
            
            // figures
            Vbo* m_figureVbo;
            std::vector<Figure*> m_figures;
            */
             
            // state
            bool m_rendering;
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
            
            void writeFaceData(RenderContext& context, const FacesByTexture& facesByTexture, FaceRenderInfoList& renderInfos, ShaderProgram& program);
            void writeColoredEdgeData(RenderContext& context, const Model::BrushList& brushes, const Model::FaceList& faces, VertexArray& vertexArray);
            void writeEdgeData(RenderContext& context, const Model::BrushList& brushes, const Model::FaceList& faces, VertexArray& vertexArray);
            void rebuildGeometryData(RenderContext& context);
//            void writeEntityBounds(RenderContext& context, const Model::EntityList& entities, EdgeRenderInfo& renderInfo, VboBlock& block);
            void rebuildEntityData(RenderContext& context);
            bool reloadEntityModel(const Model::Entity& entity, CachedEntityRenderer& cachedRenderer);
            void reloadEntityModels(RenderContext& context, EntityRenderers& renderers);
            void reloadEntityModels(RenderContext& context);
            
            void createShaders();
            void validate(RenderContext& context);

//            void renderEntityBounds(RenderContext& context, const EdgeRenderInfo& renderInfo, const Color* color);
//            void renderEntityModels(RenderContext& context, EntityRenderers& entities);
            void renderEdges(RenderContext& context, VertexArray* renderInfo, const Color* color);
//            void renderFaces(RenderContext& context, bool textured, bool selected, bool locked, const FaceRenderInfos& renderInfos);
//            void renderFigures(RenderContext& context);
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
