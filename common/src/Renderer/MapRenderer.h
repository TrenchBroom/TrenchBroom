/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include "Color.h"
#include "Model/ModelTypes.h"
#include "View/ViewTypes.h"

#include <map>

namespace TrenchBroom {
    namespace IO {
        class Path;
    }
    
    namespace View {
        class Selection;
    }
    
    namespace Renderer {
        class EntityLinkRenderer;
        class FontManager;
        class ObjectRenderer;
        class RenderBatch;
        class RenderContext;
        
        class MapRenderer {
        private:
            typedef std::map<Model::Layer*, ObjectRenderer*> RendererMap;
            
            View::MapDocumentWPtr m_document;
            
            RendererMap m_layerRenderers;
            ObjectRenderer* m_selectionRenderer;
            EntityLinkRenderer* m_entityLinkRenderer;
            
            class HandleSelectedNode;
            class UpdateSelectedNode;
            class UpdateNode;
            class AddNode;
            class RemoveNode;
        public:
            MapRenderer(View::MapDocumentWPtr document);
            ~MapRenderer();
        private:
            static ObjectRenderer* createSelectionRenderer(View::MapDocumentWPtr document);
            void clear();
        public: // color config
            void overrideSelectionColors(const Color& color, float mix);
            void restoreSelectionColors();
        public: // rendering
            void render(RenderContext& renderContext, RenderBatch& renderBatch);
        private:
            void commitPendingChanges();
            void setupGL(RenderBatch& renderBatch);
            void renderLayers(RenderContext& renderContext, RenderBatch& renderBatch);
            void renderSelection(RenderContext& renderContext, RenderBatch& renderBatch);
            void renderEntityLinks(RenderContext& renderContext, RenderBatch& renderBatch);
            
            void setupRenderers();
            void setupLayerRenderers();
            void setupLayerRenderer(ObjectRenderer* renderer);
            void setupSelectionRenderer(ObjectRenderer* renderer);
            void setupEntityLinkRenderer();

            void invalidateLayerRenderers();
            void invalidateSelectionRenderer();
            void invalidateEntityLinkRenderer();
        private: // notification
            void bindObservers();
            void unbindObservers();
            
            void documentWasCleared(View::MapDocument* document);
            void documentWasNewedOrLoaded(View::MapDocument* document);
            
            void nodesWereAdded(const Model::NodeList& nodes);
            void nodesWillBeRemoved(const Model::NodeList& nodes);
            void nodesDidChange(const Model::NodeList& nodes);
            
            void brushFacesDidChange(const Model::BrushFaceList& faces);
            
            void selectionDidChange(const View::Selection& selection);
            Model::BrushSet collectBrushes(const Model::BrushFaceList& faces);
            
            void textureCollectionsDidChange();
            void entityDefinitionsDidChange();
            void modsDidChange();
            
            void editorContextDidChange();
            void mapViewConfigDidChange();
            
            void preferenceDidChange(const IO::Path& path);
        };
    }
}

#endif /* defined(__TrenchBroom__MapRenderer__) */
