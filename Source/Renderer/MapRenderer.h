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
        class EntityRenderer;
        class Figure;
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
            typedef TexturedPolygonSorter<Model::Texture, Model::Face*> FaceSorter;
            typedef FaceSorter::PolygonCollection FaceCollection;
            typedef FaceSorter::PolygonCollectionMap FaceCollectionMap;
            typedef std::vector<Figure*> FigureList;
        private:
            
            // level geometry rendering
            VboPtr m_faceVbo;
            VboPtr m_edgeVbo;
            TextureVertexArrayList m_faceVertexArrays;
            TextureVertexArrayList m_selectedFaceVertexArrays;
            TextureVertexArrayList m_lockedFaceVertexArrays;
            VertexArrayPtr m_edgeVertexArray;
            VertexArrayPtr m_selectedEdgeVertexArray;
            VertexArrayPtr m_lockedEdgeVertexArray;

            VboPtr m_entityVbo;
            EntityRendererPtr m_entityRenderer;
            EntityRendererPtr m_selectedEntityRenderer;
            EntityRendererPtr m_lockedEntityRenderer;
            
            VboPtr m_figureVbo;
            FigureList m_figures;
            FigureList m_deletedFigures;
            
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
            
            Model::MapDocument& m_document;
            
            void writeFaceData(RenderContext& context, const FaceCollectionMap& faceCollectionMap, TextureVertexArrayList& vertexArrays, ShaderProgram& program);
            void writeColoredEdgeData(RenderContext& context, const Model::BrushList& brushes, const Model::FaceList& faces, VertexArray& vertexArray);
            void writeEdgeData(RenderContext& context, const Model::BrushList& brushes, const Model::FaceList& faces, VertexArray& vertexArray);
            void rebuildGeometryData(RenderContext& context);
            void deleteFigures(FigureList& figures);
            
            void validate(RenderContext& context);
            
            void renderFaces(RenderContext& context);
            void renderEdges(RenderContext& context);
            void renderFigures(RenderContext& context);
        public:
            MapRenderer(Model::MapDocument& document);
            ~MapRenderer();
            
            void addEntity(Model::Entity& entity);
            void addEntities(const Model::EntityList& entities);
            void removeEntity(Model::Entity& entity);
            void removeEntities(const Model::EntityList& entities);
            void changeEditState(const Model::EditStateChangeSet& changeSet);
            void loadMap();
            void clearMap();
            void invalidateEntities();
            void invalidateSelectedEntities();
            void invalidateBrushes();
            void invalidateSelectedBrushes();
            void invalidateAll();
            void invalidateEntityModelRendererCache();
            
            void addFigure(Figure* figure);
            void removeFigure(Figure* figure);
            void deleteFigure(Figure* figure);
            
            void render(RenderContext& context);
        };
    }
}

#endif /* defined(__TrenchBroom__MapRenderer__) */
