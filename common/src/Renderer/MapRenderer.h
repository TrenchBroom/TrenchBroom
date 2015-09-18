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

#ifndef TrenchBroom_MapRenderer
#define TrenchBroom_MapRenderer

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
            class SelectedBrushRendererFilter;
            class LockedBrushRendererFilter;
            class UnselectedBrushRendererFilter;
            
            typedef std::map<Model::Layer*, ObjectRenderer*> RendererMap;
            
            View::MapDocumentWPtr m_document;

            ObjectRenderer* m_defaultRenderer;
            ObjectRenderer* m_selectionRenderer;
            ObjectRenderer* m_lockedRenderer;
            EntityLinkRenderer* m_entityLinkRenderer;
        public:
            MapRenderer(View::MapDocumentWPtr document);
            ~MapRenderer();
        private:
            static ObjectRenderer* createDefaultRenderer(View::MapDocumentWPtr document);
            static ObjectRenderer* createSelectionRenderer(View::MapDocumentWPtr document);
            static ObjectRenderer* createLockRenderer(View::MapDocumentWPtr document);
            void clear();
        public: // color config
            void overrideSelectionColors(const Color& color, float mix);
            void restoreSelectionColors();
        public: // rendering
            void render(RenderContext& renderContext, RenderBatch& renderBatch);
        private:
            void commitPendingChanges();
            void setupGL(RenderBatch& renderBatch);
            void renderDefault(RenderContext& renderContext, RenderBatch& renderBatch);
            void renderSelection(RenderContext& renderContext, RenderBatch& renderBatch);
            void renderLocked(RenderContext& renderContext, RenderBatch& renderBatch);
            void renderEntityLinks(RenderContext& renderContext, RenderBatch& renderBatch);
            
            void setupRenderers();
            void setupDefaultRenderer(ObjectRenderer* renderer);
            void setupSelectionRenderer(ObjectRenderer* renderer);
            void setupLockedRenderer(ObjectRenderer* renderer);
            void setupEntityLinkRenderer();

            typedef enum {
                Renderer_Default            = 1,
                Renderer_Selection          = 2,
                Renderer_Locked             = 4,
                Renderer_Default_Selection  = Renderer_Default | Renderer_Selection,
                Renderer_Default_Locked     = Renderer_Default | Renderer_Locked,
                Renderer_All                = Renderer_Default | Renderer_Selection | Renderer_Locked
            } Renderer;
            
            class CollectRenderableNodes;
            
            void updateRenderers(Renderer renderers);
            void invalidateRenderers(Renderer renderers);
            void invalidateEntityLinkRenderer();
            void reloadEntityModels();
        private: // notification
            void bindObservers();
            void unbindObservers();
            
            void documentWasCleared(View::MapDocument* document);
            void documentWasNewedOrLoaded(View::MapDocument* document);
            
            void nodesWereAdded(const Model::NodeList& nodes);
            void nodesWereRemoved(const Model::NodeList& nodes);
            void nodesDidChange(const Model::NodeList& nodes);
            
            void nodeVisibilityDidChange(const Model::NodeList& nodes);
            void nodeLockingDidChange(const Model::NodeList& nodes);
            
            void groupWasOpened(Model::Group* group);
            void groupWasClosed(Model::Group* group);
            
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

#endif /* defined(TrenchBroom_MapRenderer) */
