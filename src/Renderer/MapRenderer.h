/*
 Copyright (C) 2010-2013 Kristian Duske
 
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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__MapRenderer__
#define __TrenchBroom__MapRenderer__

#include "Model/ModelTypes.h"
#include "Renderer/BrushRenderer.h"
#include "Renderer/EntityRenderer.h"
#include "Renderer/EntityLinkRenderer.h"
#include "Renderer/Vbo.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace IO {
        class Path;
    }
    
    namespace Model {
        class Filter;
        class Map;
        class SelectionResult;
    }
    
    namespace Renderer {
        class FontManager;
        class RenderContext;
        
        class MapRenderer {
        private:
            View::MapDocumentPtr m_document;
            FontManager& m_fontManager;
            BrushRenderer m_unselectedBrushRenderer;
            BrushRenderer m_selectedBrushRenderer;
            EntityRenderer m_unselectedEntityRenderer;
            EntityRenderer m_selectedEntityRenderer;
            EntityLinkRenderer m_entityLinkRenderer;
        public:
            MapRenderer(View::MapDocumentPtr document, FontManager& fontManager);
            ~MapRenderer();
            
            void render(RenderContext& context);
        private:
            void setupGL(RenderContext& context);
            void renderGeometry(RenderContext& context);
            void renderEntities(RenderContext& context);
            
            void bindObservers();
            void unbindObservers();
            
            void documentWasNewed();
            void documentWasLoaded();
            void objectWasAdded(Model::Object* object);
            void objectWillBeRemoved(Model::Object* object);
            void objectDidChange(Model::Object* object);
            void faceDidChange(Model::BrushFace* face);
            void selectionDidChange(const Model::SelectionResult& result);
            void modsDidChange();
            void entityDefinitionsDidChange();
            void preferenceDidChange(const IO::Path& path);
            
            void clearState();
            void loadMap(Model::Map& map);
        };
    }
}

#endif /* defined(__TrenchBroom__MapRenderer__) */
