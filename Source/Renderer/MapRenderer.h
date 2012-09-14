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
#include "Model/TextureTypes.h"
#include "Renderer/RenderTypes.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/TexturedPolygonSorter.h"
#include "Renderer/TextureVertexArray.h"
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
            typedef Text::TextRenderer<Model::Entity*> EntityClassnameRenderer;
            typedef std::auto_ptr<EntityClassnameRenderer> EntityClassnameRendererPtr;
            typedef TexturedPolygonSorter<Model::Face*> FaceSorter;
            typedef FaceSorter::PolygonCollection FaceCollection;
            typedef FaceSorter::PolygonCollectionMap FaceCollectionMap;
        private:
            
            class CachedEntityRenderer {
            public:
                EntityRenderer* renderer;
                String classname;
                CachedEntityRenderer() : renderer(NULL), classname("") {}
                CachedEntityRenderer(EntityRenderer* renderer, const String& classname) : renderer(renderer), classname(classname) {}
            };
            
            typedef std::map<Model::Entity*, CachedEntityRenderer> EntityRenderers;

            // resources
            Model::TexturePtr m_dummyTexture;
            StringManagerPtr m_stringManager;

            // level geometry rendering
            VboPtr m_faceVbo;
            TextureVertexArrayList m_faceVertexArrays;
            TextureVertexArrayList m_selectedFaceVertexArrays;
            TextureVertexArrayList m_lockedFaceVertexArrays;
            VboPtr m_edgeVbo;
            VertexArrayPtr m_edgeVertexArray;
            VertexArrayPtr m_selectedEdgeVertexArray;
            VertexArrayPtr m_lockedEdgeVertexArray;
            
            // entity bounds rendering
            VboPtr m_entityBoundsVbo;
            VertexArrayPtr m_entityBoundsVertexArray;
            VertexArrayPtr m_selectedEntityBoundsVertexArray;
            VertexArrayPtr m_lockedEntityBoundsVertexArray;
            
            // entity model rendering
            EntityRendererManagerPtr m_entityRendererManager;
            EntityRenderers m_entityRenderers;
            EntityRenderers m_selectedEntityRenderers;
            EntityRenderers m_lockedEntityRenderers;
            bool m_entityRendererCacheValid;

            // classnames
            EntityClassnameRendererPtr m_classnameRenderer;
            EntityClassnameRendererPtr m_selectedClassnameRenderer;
            EntityClassnameRendererPtr m_lockedClassnameRenderer;

            // shaders
            bool m_shadersCreated;
            ShaderPtr m_coloredEdgeVertexShader;
            ShaderPtr m_edgeVertexShader;
            ShaderPtr m_edgeFragmentShader;
            ShaderPtr m_faceVertexShader;
            ShaderPtr m_faceFragmentShader;
            ShaderPtr m_entityModelVertexShader;
            ShaderPtr m_entityModelFragmentShader;
            ShaderPtr m_textVertexShader;
            ShaderPtr m_textFragmentShader;
            ShaderPtr m_textBackgroundVertexShader;
            ShaderPtr m_textBackgroundFragmentShader;
            ShaderProgramPtr m_coloredEdgeProgram;
            ShaderProgramPtr m_edgeProgram;
            ShaderProgramPtr m_faceProgram;
            ShaderProgramPtr m_entityModelProgram;
            ShaderProgramPtr m_textProgram;
            ShaderProgramPtr m_textBackgroundProgram;
            
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
            
            Model::MapDocument& m_document;
            
            void writeFaceData(RenderContext& context, const FaceCollectionMap& faceCollectionMap, TextureVertexArrayList& vertexArrays, ShaderProgram& program);
            void writeColoredEdgeData(RenderContext& context, const Model::BrushList& brushes, const Model::FaceList& faces, VertexArray& vertexArray);
            void writeEdgeData(RenderContext& context, const Model::BrushList& brushes, const Model::FaceList& faces, VertexArray& vertexArray);
            void rebuildGeometryData(RenderContext& context);
            void writeColoredEntityBounds(RenderContext& context, const Model::EntityList& entities, VertexArray& vertexArray);
            void writeEntityBounds(RenderContext& context, const Model::EntityList& entities, VertexArray& vertexArray);
            void rebuildEntityData(RenderContext& context);
            bool reloadEntityModel(const Model::Entity& entity, CachedEntityRenderer& cachedRenderer);
            void reloadEntityModels(RenderContext& context, EntityRenderers& renderers);
            void reloadEntityModels(RenderContext& context);
            void moveEntityRenderer(Model::Entity* entity, EntityRenderers& from, EntityRenderers& to);
            
            void createShaders();
            void validate(RenderContext& context);
            
            void renderFaces(RenderContext& context);
            void renderEdges(RenderContext& context);
            void renderEntityBounds(RenderContext& context);
            void renderEntityModels(RenderContext& context);
            void renderEntityClassnames(RenderContext& context);
        public:
            MapRenderer(Model::MapDocument& document);
            ~MapRenderer();
            
            void addEntities(const Model::EntityList& entities);
            void removeEntities(const Model::EntityList& entities);
            void changeEditState(const Model::EditStateChangeSet& changeSet);
            void loadMap();
            void clearMap();
            void invalidateEntities();
            void invalidateBrushes();
            void invalidateAll();
            void invalidateEntityRendererCache();
            
            void render(RenderContext& context);
        };
    }
}

#endif /* defined(__TrenchBroom__MapRenderer__) */
