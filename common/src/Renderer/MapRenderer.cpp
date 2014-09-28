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

#include "MapRenderer.h"

#include "CollectionUtils.h"
#include "Model/Layer.h"
#include "Model/Node.h"
#include "Model/NodeVisitor.h"
#include "Model/World.h"
#include "Renderer/ObjectRenderer.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace Renderer {
        MapRenderer::MapRenderer(View::MapDocumentWPtr document) :
        m_document(document) {
            bindObservers();
        }

        MapRenderer::~MapRenderer() {
            unbindObservers();
            clear();
        }

        void MapRenderer::clear() {
            MapUtils::clearAndDelete(m_layerRenderers);
        }

        void MapRenderer::render(RenderContext& renderContext) {
            renderLayers(renderContext);
            renderSelection(renderContext);
            renderEntityLinks(renderContext);
        }
        
        void MapRenderer::renderLayers(RenderContext& renderContext) {
            RendererMap::iterator it, end;
            for (it = m_layerRenderers.begin(), end = m_layerRenderers.end(); it != end; ++it) {
                // const Model::Layer* layer = it->first;
                ObjectRenderer* renderer = it->second;
                renderer->render(renderContext);
            }
        }
        
        void MapRenderer::renderSelection(RenderContext& renderContext) {
        }
        
        void MapRenderer::renderEntityLinks(RenderContext& renderContext) {
        }

        void MapRenderer::bindObservers() {
            assert(!expired(m_document));
            View::MapDocumentSPtr document = lock(m_document);
            document->documentWasClearedNotifier.addObserver(this, &MapRenderer::documentWasCleared);
            document->documentWasNewedNotifier.addObserver(this, &MapRenderer::documentWasNewedOrLoaded);
            document->documentWasLoadedNotifier.addObserver(this, &MapRenderer::documentWasNewedOrLoaded);
        }
        
        void MapRenderer::unbindObservers() {
            if (!expired(m_document)) {
                View::MapDocumentSPtr document = lock(m_document);
                document->documentWasClearedNotifier.removeObserver(this, &MapRenderer::documentWasCleared);
                document->documentWasNewedNotifier.removeObserver(this, &MapRenderer::documentWasNewedOrLoaded);
                document->documentWasLoadedNotifier.removeObserver(this, &MapRenderer::documentWasNewedOrLoaded);
            }
        }

        void MapRenderer::documentWasCleared(View::MapDocument* document) {
            m_layerRenderers.clear();
        }
        
        class MapRenderer::AddLayer : public Model::NodeVisitor {
        private:
            RendererMap& m_layerRenderers;
        public:
            AddLayer(RendererMap& layerRenderers) : m_layerRenderers(layerRenderers) {}
        private:
            void doVisit(Model::World* world)   {}
            void doVisit(Model::Layer* layer)   {
                ObjectRenderer* renderer = new ObjectRenderer();
                MapUtils::insertOrFail(m_layerRenderers, layer, renderer);
                renderer->addObjects(layer->children());
                stopRecursion(); // don't visit my children
            }
            void doVisit(Model::Group* group)   { assert(false); }
            void doVisit(Model::Entity* entity) { assert(false); }
            void doVisit(Model::Brush* brush)   { assert(false); }
        };
        
        void MapRenderer::documentWasNewedOrLoaded(View::MapDocument* document) {
            Model::World* world = document->world();
            AddLayer visitor(m_layerRenderers);
            world->acceptAndRecurse(visitor);
        }
    }
}
