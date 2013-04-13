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
#include "Renderer/EntityDecorator.h"
#include "Renderer/Figure.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/TexturedPolygonSorter.h"
#include "Renderer/TextureVertexArray.h"
#include "Renderer/VertexArray.h"
#include "Renderer/Text/TextRenderer.h"
#include "Utility/Color.h"

#include <map>
#include <vector>

namespace TrenchBroom {
    namespace Controller {
        class Command;
    }
    
    namespace Model {
        class EditStateChangeSet;
        class MapDocument;
    }
    
    namespace Renderer {
        class EdgeRenderer;
        class EntityRenderer;
        class FaceRenderer;
        class Figure;
        class PointTraceRenderer;
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
        private:
            Model::MapDocument& m_document;
            
            // level geometry rendering
            Vbo* m_faceVbo;
            FaceRenderer* m_faceRenderer;
            FaceRenderer* m_selectedFaceRenderer;
            FaceRenderer* m_lockedFaceRenderer;
            
            Vbo* m_edgeVbo;
            EdgeRenderer* m_edgeRenderer;
            EdgeRenderer* m_selectedEdgeRenderer;
            EdgeRenderer* m_lockedEdgeRenderer;
            
            Vbo* m_entityVbo;
            EntityRenderer* m_entityRenderer;
            EntityRenderer* m_selectedEntityRenderer;
            EntityRenderer* m_lockedEntityRenderer;
            
            Vbo* m_utilityVbo;
            EntityDecorator::List m_entityDecorators;
            PointTraceRenderer* m_pointTraceRenderer;
            
            bool m_overrideSelectionColors;
            Color m_selectedFaceColor;
            Color m_selectedEdgeColor;
            Color m_occludedSelectedEdgeColor;
            
            // state
            bool m_rendering;
            bool m_geometryDataValid;
            bool m_selectedGeometryDataValid;
            bool m_lockedGeometryDataValid;
            
            void rebuildGeometryData(RenderContext& context);
            
            void validate(RenderContext& context);
            
            void renderFaces(RenderContext& context);
            void renderEdges(RenderContext& context);
            void renderDecorators(RenderContext& context);

            void changeEditState(const Model::EditStateChangeSet& changeSet);
            void invalidateEntities();
            void invalidateSelectedEntities();
            void invalidateBrushes();
            void invalidateSelectedBrushes();
            void invalidateAll();
            void invalidateEntityModelRendererCache();
            void invalidateSelectedEntityModelRendererCache();
            void invalidateDecorators();
            void clear();
        public:
            MapRenderer(Model::MapDocument& document);
            ~MapRenderer();
            
            inline void setOverrideSelectionColors(bool override, const Color& faceColor = Color(), const Color& edgeColor = Color(), const Color& occludedEdgeColor = Color()) {
                m_overrideSelectionColors = override;
                m_selectedFaceColor = faceColor;
                m_selectedEdgeColor = edgeColor;
                m_occludedSelectedEdgeColor = occludedEdgeColor;
            }
            
            void update(const Controller::Command& command);

            void setPointTrace(const Vec3f::List& points);
            void removePointTrace();
            
            void render(RenderContext& context);
        };
    }
}

#endif /* defined(__TrenchBroom__MapRenderer__) */
