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

#include <GL/glew.h>
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

#include <map>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class EditStateChangeSet;
        class MapDocument;
    }
    
    namespace Renderer {
        class EntityModelRendererManager;
        class EntityModelRenderer;
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
            typedef TexturedPolygonSorter<Model::Texture, Model::Face*> FaceSorter;
            typedef FaceSorter::PolygonCollection FaceCollection;
            typedef FaceSorter::PolygonCollectionMap FaceCollectionMap;
        private:
            
            class CachedEntityModelRenderer {
            public:
                EntityModelRenderer* renderer;
                String classname;
                CachedEntityModelRenderer() : renderer(NULL), classname("") {}
                CachedEntityModelRenderer(EntityModelRenderer* renderer, const String& classname) : renderer(renderer), classname(classname) {}
            };
            
            typedef std::map<Model::Entity*, CachedEntityModelRenderer> EntityModelRenderers;

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
            EntityModelRenderers m_modelRenderers;
            EntityModelRenderers m_selectedEntityModelRenderers;
            EntityModelRenderers m_lockedEntityModelRenderers;
            bool m_modelRendererCacheValid;

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
            bool reloadEntityModel(const Model::Entity& entity, CachedEntityModelRenderer& cachedRenderer);
            void reloadEntityModels(RenderContext& context, EntityModelRenderers& renderers);
            void reloadEntityModels(RenderContext& context);
            void moveEntityModelRenderer(Model::Entity* entity, EntityModelRenderers& from, EntityModelRenderers& to);
            
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
            void invalidateSelectedBrushes();
            void invalidateAll();
            void invalidateEntityModelRendererCache();
            
            void render(RenderContext& context);
        };
    }
}

#endif /* defined(__TrenchBroom__MapRenderer__) */
