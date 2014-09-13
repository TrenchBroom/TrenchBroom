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

#include "Color.h"
#include "Preferences.h"
#include "Renderer/GL.h"
#include "Model/Brush.h"
#include "Model/BrushEdge.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceGeometry.h"
#include "Model/ModelFilter.h"
#include "Model/Map.h"
#include "Model/ModelUtils.h"
#include "Model/Object.h"
#include "Model/PointFile.h"
#include "Model/SelectionResult.h"
#include "Renderer/Camera.h"
#include "Renderer/Mesh.h"
#include "Renderer/RenderConfig.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/VertexSpec.h"
#include "Renderer/VertexArray.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace Renderer {
        MapRenderer::AddObject::AddObject(MapRenderer& renderer) :
        m_renderer(renderer) {}
        
        void MapRenderer::AddObject::doVisit(Model::Entity* entity) {
            Model::Layer* layer = entity->layer();
            if (layer->locked())
                m_renderer.m_lockedEntityRenderer.addEntity(entity);
            else if (entity->selected() || entity->partiallySelected())
                m_renderer.m_selectedEntityRenderer.addEntity(entity);
            else
                m_renderer.m_unselectedEntityRenderer.addEntity(entity);
            m_renderer.m_entityLinkRenderer.invalidate();
        }
        
        void MapRenderer::AddObject::doVisit(Model::Brush* brush) {
            Model::Layer* layer = brush->layer();
            if (layer->locked())
                m_renderer.m_lockedBrushRenderer.addBrush(brush);
            else if (brush->selected() || brush->partiallySelected())
                m_renderer.m_selectedBrushRenderer.addBrush(brush);
            else
                m_renderer.m_unselectedBrushRenderer.addBrush(brush);
            m_renderer.m_entityLinkRenderer.invalidate();
        }

        MapRenderer::UpdateObject::UpdateObject(MapRenderer& renderer) :
        m_renderer(renderer) {}
        
        void MapRenderer::UpdateObject::doVisit(Model::Entity* entity) {
            Model::Layer* layer = entity->layer();
            if (layer->locked())
                m_renderer.m_lockedEntityRenderer.updateEntity(entity);
            else if (entity->selected() || entity->partiallySelected())
                m_renderer.m_selectedEntityRenderer.updateEntity(entity);
            else
                m_renderer.m_unselectedEntityRenderer.updateEntity(entity);
            m_renderer.m_entityLinkRenderer.invalidate();
        }
        
        void MapRenderer::UpdateObject::doVisit(Model::Brush* brush) {
            Model::Layer* layer = brush->layer();
            if (layer->locked())
                m_renderer.m_lockedBrushRenderer.invalidate();
            else if (brush->selected() || brush->partiallySelected())
                m_renderer.m_selectedBrushRenderer.invalidate();
            else
                m_renderer.m_unselectedBrushRenderer.invalidate();
            m_renderer.m_entityLinkRenderer.invalidate();
        }

        MapRenderer::RemoveObject::RemoveObject(MapRenderer& renderer) :
        m_renderer(renderer) {}
        
        void MapRenderer::RemoveObject::doVisit(Model::Entity* entity) {
            Model::Layer* layer = entity->layer();
            if (layer->locked())
                m_renderer.m_lockedEntityRenderer.removeEntity(entity);
            else if (entity->selected() || entity->partiallySelected())
                m_renderer.m_selectedEntityRenderer.removeEntity(entity);
            else
                m_renderer.m_unselectedEntityRenderer.removeEntity(entity);
            m_renderer.m_entityLinkRenderer.invalidate();
        }
        
        void MapRenderer::RemoveObject::doVisit(Model::Brush* brush) {
            Model::Layer* layer = brush->layer();
            if (layer->locked())
                m_renderer.m_lockedBrushRenderer.removeBrush(brush);
            else if (brush->selected() || brush->partiallySelected())
                m_renderer.m_selectedBrushRenderer.removeBrush(brush);
            else
                m_renderer.m_unselectedBrushRenderer.removeBrush(brush);
            m_renderer.m_entityLinkRenderer.invalidate();
        }

        struct BrushRendererFilter : public BrushRenderer::Filter {
        private:
            const Model::ModelFilter& m_filter;
        public:
            BrushRendererFilter(const Model::ModelFilter& filter) :
            m_filter(filter) {}
            
            virtual ~BrushRendererFilter() {}
        protected:
            bool visible(const Model::Brush* brush) const {
                return m_filter.visible(brush);
            }
            
            bool visible(const Model::BrushFace* face) const {
                return m_filter.visible(face);
            }
            
            bool selected(const Model::Brush* brush) const {
                return brush->selected();
            }
            
            bool partiallySelected(const Model::Brush* brush) const {
                return brush->partiallySelected();
            }
            
            bool selected(const Model::BrushFace* face) const {
                return face->selected() || selected(face->parent());
            }
            
            bool selected(const Model::BrushEdge* edge) const {
                const Model::BrushFace* left = edge->left->face;
                const Model::BrushFace* right = edge->right->face;
                const Model::Brush* brush = left->parent();
                return selected(brush) || selected(left) || selected(right);
            }
            
            bool locked(const Model::Brush* brush) const {
                return m_filter.locked(brush);
            }
            
            bool locked(const Model::BrushFace* face) const {
                return locked(face->parent());
            }
        };
        
        struct UnselectedBrushRendererFilter : public BrushRendererFilter {
            UnselectedBrushRendererFilter(const Model::ModelFilter& filter) :
            BrushRendererFilter(filter) {}
            
            bool operator()(const Model::Brush* brush) const {
                return !locked(brush) && !selected(brush) && visible(brush);
            }
            
            bool operator()(const Model::BrushFace* face) const {
                return !locked(face) && !selected(face) && visible(face);
            }
            
            bool operator()(const Model::BrushEdge* edge) const {
                return !selected(edge);
            }
        };
        
        struct SelectedBrushRendererFilter : public BrushRendererFilter {
        public:
            SelectedBrushRendererFilter(const Model::ModelFilter& filter) :
            BrushRendererFilter(filter) {}
            
            bool operator()(const Model::Brush* brush) const {
                return !locked(brush) && (selected(brush) || partiallySelected(brush)) && visible(brush);
            }
            
            bool operator()(const Model::BrushFace* face) const {
                return !locked(face) && selected(face) && visible(face);
            }
            
            bool operator()(const Model::BrushEdge* edge) const {
                return selected(edge);
            }
        };
        
        struct LockedBrushRendererFilter : public BrushRendererFilter {
        public:
            LockedBrushRendererFilter(const Model::ModelFilter& filter) :
            BrushRendererFilter(filter) {}
            
            bool operator()(const Model::Brush* brush) const {
                return locked(brush) && visible(brush);
            }
            
            bool operator()(const Model::BrushFace* face) const {
                return locked(face) && visible(face);
            }
            
            bool operator()(const Model::BrushEdge* edge) const {
                return true;
            }
        };
        
        MapRenderer::MapRenderer(View::MapDocumentWPtr document, FontManager& fontManager) :
        m_document(document),
        m_layerObserver(m_document),
        m_fontManager(fontManager),
        m_unselectedBrushRenderer(UnselectedBrushRendererFilter(lock(document)->filter())),
        m_selectedBrushRenderer(SelectedBrushRendererFilter(lock(document)->filter())),
        m_lockedBrushRenderer(LockedBrushRendererFilter(lock(document)->filter())),
        m_unselectedEntityRenderer(lock(document)->entityModelManager(), m_fontManager, lock(document)->filter()),
        m_selectedEntityRenderer(lock(document)->entityModelManager(), m_fontManager, lock(document)->filter()),
        m_lockedEntityRenderer(lock(document)->entityModelManager(), m_fontManager, lock(document)->filter()),
        m_entityLinkRenderer(m_document) {
            bindObservers();
            setupRendererColors();
            m_selectedEntityRenderer.setShowHiddenEntities(true);
            m_selectedBrushRenderer.setShowHiddenBrushes(true);
        }
        
        MapRenderer::~MapRenderer() {
            unbindObservers();
        }
        
        void MapRenderer::overrideSelectionColors(const Color& color, const float mix) {
            PreferenceManager& prefs = PreferenceManager::instance();
            
            m_selectedEntityRenderer.setBoundsColor(prefs.get(Preferences::SelectedEdgeColor).mixed(color, mix));
            m_selectedEntityRenderer.setOccludedBoundsColor(prefs.get(Preferences::OccludedSelectedEdgeColor).mixed(color, mix));
            m_selectedEntityRenderer.setTintColor(prefs.get(Preferences::SelectedFaceColor).mixed(color, mix));
            
            m_selectedBrushRenderer.setEdgeColor(prefs.get(Preferences::SelectedEdgeColor).mixed(color, mix));
            m_selectedBrushRenderer.setTintColor(prefs.get(Preferences::SelectedFaceColor).mixed(color, mix));
            m_selectedBrushRenderer.setOccludedEdgeColor(prefs.get(Preferences::OccludedSelectedEdgeColor).mixed(color, mix));
        }
        
        void MapRenderer::restoreSelectionColors() {
            setupRendererColors();
        }
        
        void MapRenderer::render(RenderContext& context) {
            setupGL(context);
            
            renderUnselectedGeometry(context);
            renderUnselectedEntities(context);
            
            renderLockedGeometry(context);
            renderLockedEntities(context);
            
            renderSelectedGeometry(context);
            renderSelectedEntities(context);

            renderEntityLinks(context);
            renderPointFile(context);
        }
        
        void MapRenderer::setupRendererColors() {
            PreferenceManager& prefs = PreferenceManager::instance();
            
            m_unselectedEntityRenderer.setOverlayTextColor(prefs.get(Preferences::InfoOverlayTextColor));
            m_unselectedEntityRenderer.setOverlayBackgroundColor(prefs.get(Preferences::InfoOverlayBackgroundColor));
            m_unselectedEntityRenderer.setBoundsColor(prefs.get(Preferences::UndefinedEntityColor));
            m_unselectedEntityRenderer.setApplyTinting(false);
            
            m_selectedEntityRenderer.setOverlayTextColor(prefs.get(Preferences::SelectedInfoOverlayTextColor));
            m_selectedEntityRenderer.setOverlayBackgroundColor(prefs.get(Preferences::SelectedInfoOverlayBackgroundColor));
            m_selectedEntityRenderer.setOverrideBoundsColor(true);
            m_selectedEntityRenderer.setRenderOccludedBounds(true);
            m_selectedEntityRenderer.setBoundsColor(prefs.get(Preferences::SelectedEdgeColor));
            m_selectedEntityRenderer.setOccludedBoundsColor(prefs.get(Preferences::OccludedSelectedEdgeColor));
            m_selectedEntityRenderer.setTintColor(prefs.get(Preferences::SelectedFaceColor));
            m_selectedEntityRenderer.setRenderAngles(true);
            m_selectedEntityRenderer.setAngleColor(prefs.get(Preferences::AngleIndicatorColor));
            m_selectedEntityRenderer.setShowClassnamesOnTop(true);
            
            m_lockedEntityRenderer.setOverlayTextColor(prefs.get(Preferences::InfoOverlayTextColor));
            m_lockedEntityRenderer.setOverlayBackgroundColor(prefs.get(Preferences::InfoOverlayBackgroundColor));
            m_lockedEntityRenderer.setBoundsColor(prefs.get(Preferences::UndefinedEntityColor));
            m_lockedEntityRenderer.setApplyTinting(true);
            m_lockedEntityRenderer.setTintColor(prefs.get(Preferences::LockedFaceColor));
            
            m_unselectedBrushRenderer.setFaceColor(prefs.get(Preferences::FaceColor));
            m_unselectedBrushRenderer.setEdgeColor(prefs.get(Preferences::EdgeColor));
            m_unselectedBrushRenderer.setTransparencyAlpha(prefs.get(Preferences::TransparentFaceAlpha));
            
            m_selectedBrushRenderer.setFaceColor(prefs.get(Preferences::FaceColor));
            m_selectedBrushRenderer.setEdgeColor(prefs.get(Preferences::SelectedEdgeColor));
            m_selectedBrushRenderer.setTintColor(prefs.get(Preferences::SelectedFaceColor));
            m_selectedBrushRenderer.setOccludedEdgeColor(prefs.get(Preferences::OccludedSelectedEdgeColor));
            m_selectedBrushRenderer.setRenderOccludedEdges(true);
            m_selectedBrushRenderer.setTransparencyAlpha(prefs.get(Preferences::TransparentFaceAlpha));

            m_lockedBrushRenderer.setFaceColor(prefs.get(Preferences::FaceColor));
            m_lockedBrushRenderer.setEdgeColor(prefs.get(Preferences::LockedEdgeColor));
            m_lockedBrushRenderer.setTransparencyAlpha(prefs.get(Preferences::TransparentFaceAlpha));
            m_lockedBrushRenderer.setTintFaces(true);
            m_lockedBrushRenderer.setTintColor(prefs.get(Preferences::LockedFaceColor));
        }
        
        void MapRenderer::setupGL(RenderContext& context) {
            glDisableClientState(GL_VERTEX_ARRAY);
            glDisableClientState(GL_COLOR_ARRAY);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            glBindTexture(GL_TEXTURE_2D, 0);
            glDisable(GL_TEXTURE_2D);
            glFrontFace(GL_CW);
            glEnable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
            glResetEdgeOffset();
        }
        
        void MapRenderer::renderUnselectedGeometry(RenderContext& context) {
            if (context.renderConfig().showBrushes())
                m_unselectedBrushRenderer.render(context);
        }
        
        void MapRenderer::renderSelectedGeometry(RenderContext& context) {
            if (!context.hideSelection()) {
                const bool applyTinting = context.tintSelection(); // && lock(m_document)->selectedFaces().empty();
                m_selectedBrushRenderer.setTintFaces(applyTinting);
                m_selectedBrushRenderer.render(context);
            }
        }
        
        void MapRenderer::renderLockedGeometry(RenderContext& context) {
            m_lockedBrushRenderer.render(context);
        }

        void MapRenderer::renderUnselectedEntities(RenderContext& context) {
            m_unselectedEntityRenderer.render(context);
        }
        
        void MapRenderer::renderSelectedEntities(RenderContext& context) {
            if (!context.hideSelection()) {
                const bool applyTinting = context.tintSelection();
                m_selectedEntityRenderer.setApplyTinting(applyTinting);
                m_selectedEntityRenderer.render(context);
            }
        }

        void MapRenderer::renderLockedEntities(RenderContext& context) {
            m_lockedEntityRenderer.render(context);
        }

        void MapRenderer::renderEntityLinks(RenderContext& context) {
            m_entityLinkRenderer.render(context);
        }
        
        void MapRenderer::renderPointFile(RenderContext& context) {
            PreferenceManager& prefs = PreferenceManager::instance();
            m_pointFileRenderer.setUseColor(true);
            
            glDisable(GL_DEPTH_TEST);
            m_pointFileRenderer.setColor(Color(prefs.get(Preferences::PointFileColor), 0.35f));
            m_pointFileRenderer.render(context);
            glEnable(GL_DEPTH_TEST);
            m_pointFileRenderer.setColor(prefs.get(Preferences::PointFileColor));
            m_pointFileRenderer.render(context);
        }
        
        void MapRenderer::bindObservers() {
            View::MapDocumentSPtr document = lock(m_document);
            document->documentWasClearedNotifier.addObserver(this, &MapRenderer::documentWasCleared);
            document->documentWasNewedNotifier.addObserver(this, &MapRenderer::documentWasNewedOrLoaded);
            document->documentWasLoadedNotifier.addObserver(this, &MapRenderer::documentWasNewedOrLoaded);
            document->pointFileWasLoadedNotifier.addObserver(this, &MapRenderer::pointFileWasLoadedOrUnloaded);
            document->pointFileWasUnloadedNotifier.addObserver(this, &MapRenderer::pointFileWasLoadedOrUnloaded);
            document->objectsDidChangeNotifier.addObserver(this, &MapRenderer::objectsDidChange);
            document->facesDidChangeNotifier.addObserver(this, &MapRenderer::facesDidChange);
            document->selectionDidChangeNotifier.addObserver(this, &MapRenderer::selectionDidChange);
            document->modsDidChangeNotifier.addObserver(this, &MapRenderer::modsDidChange);
            document->textureCollectionsDidChangeNotifier.addObserver(this, &MapRenderer::textureCollectionsDidChange);
            document->entityDefinitionsDidChangeNotifier.addObserver(this, &MapRenderer::entityDefinitionsDidChange);
            document->modelFilterDidChangeNotifier.addObserver(this, &MapRenderer::modelFilterDidChange);
            document->renderConfigDidChangeNotifier.addObserver(this, &MapRenderer::renderConfigDidChange);
            
            m_layerObserver.layerWillChangeNotifier.addObserver(this, &MapRenderer::layerWillChange);
            m_layerObserver.layerDidChangeNotifier.addObserver(this, &MapRenderer::layerDidChange);
            m_layerObserver.objectWasAddedNotifier.addObserver(this, &MapRenderer::objectWasAddedToLayer);
            m_layerObserver.objectWasRemovedNotifier.addObserver(this, &MapRenderer::objectWasRemovedFromLayer);

            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.addObserver(this, &MapRenderer::preferenceDidChange);
        }
        
        void MapRenderer::unbindObservers() {
            if (!expired(m_document)) {
                View::MapDocumentSPtr document = lock(m_document);
                document->documentWasClearedNotifier.removeObserver(this, &MapRenderer::documentWasCleared);
                document->documentWasNewedNotifier.removeObserver(this, &MapRenderer::documentWasNewedOrLoaded);
                document->documentWasLoadedNotifier.removeObserver(this, &MapRenderer::documentWasNewedOrLoaded);
                document->pointFileWasLoadedNotifier.removeObserver(this, &MapRenderer::pointFileWasLoadedOrUnloaded);
                document->pointFileWasUnloadedNotifier.removeObserver(this, &MapRenderer::pointFileWasLoadedOrUnloaded);
                document->objectsDidChangeNotifier.removeObserver(this, &MapRenderer::objectsDidChange);
                document->facesDidChangeNotifier.removeObserver(this, &MapRenderer::facesDidChange);
                document->selectionDidChangeNotifier.removeObserver(this, &MapRenderer::selectionDidChange);
                document->modsDidChangeNotifier.removeObserver(this, &MapRenderer::modsDidChange);
                document->textureCollectionsDidChangeNotifier.removeObserver(this, &MapRenderer::textureCollectionsDidChange);
                document->entityDefinitionsDidChangeNotifier.removeObserver(this, &MapRenderer::entityDefinitionsDidChange);
                document->modelFilterDidChangeNotifier.removeObserver(this, &MapRenderer::modelFilterDidChange);
                document->renderConfigDidChangeNotifier.removeObserver(this, &MapRenderer::renderConfigDidChange);
            }
            
            m_layerObserver.layerWillChangeNotifier.removeObserver(this, &MapRenderer::layerWillChange);
            m_layerObserver.layerDidChangeNotifier.addObserver(this, &MapRenderer::layerDidChange);
            m_layerObserver.objectWasAddedNotifier.removeObserver(this, &MapRenderer::objectWasAddedToLayer);
            m_layerObserver.objectWasRemovedNotifier.removeObserver(this, &MapRenderer::objectWasRemovedFromLayer);

            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.removeObserver(this, &MapRenderer::preferenceDidChange);
        }
        
        void MapRenderer::documentWasCleared() {
            clearState();
        }
        
        void MapRenderer::documentWasNewedOrLoaded() {
            View::MapDocumentSPtr document = lock(m_document);
            Model::Map* map = lock(m_document)->map();
            loadMap(*map);
        }
        
        void MapRenderer::pointFileWasLoadedOrUnloaded() {
            const Vec3f::List& points = lock(m_document)->pointFile().points();
            if (points.empty()) {
                m_pointFileRenderer = EdgeRenderer();
            } else {
                typedef VertexSpecs::P3::Vertex Vertex;
                Vertex::List vertices = Vertex::fromLists(points, points.size());
                m_pointFileRenderer = EdgeRenderer(VertexArray::swap(GL_LINE_STRIP, vertices));
            }
        }
        
        void MapRenderer::modelFilterDidChange() {
            m_unselectedBrushRenderer.invalidate();
            m_unselectedEntityRenderer.invalidate();
            m_entityLinkRenderer.invalidate();
        }

        void MapRenderer::renderConfigDidChange() {
            m_unselectedBrushRenderer.invalidate();
            m_unselectedEntityRenderer.invalidate();
        }
        
        void MapRenderer::objectsDidChange(const Model::ObjectList& objects) {
            updateObjects(objects);
        }
        
        void MapRenderer::facesDidChange(const Model::BrushFaceList& faces) {
            m_selectedBrushRenderer.invalidate();
        }
        
        void MapRenderer::layerWillChange(Model::Layer* layer, const Model::Layer::Attr_Type attr) {
            if (attr == Model::Layer::Attr_Locked)
                removeObjects(layer->objects());
        }

        void MapRenderer::layerDidChange(Model::Layer* layer, const Model::Layer::Attr_Type attr) {
            if ((attr & Model::Layer::Attr_Editing) != 0) {
                m_unselectedBrushRenderer.invalidate();
                m_unselectedEntityRenderer.invalidate();
            }
            if (attr == Model::Layer::Attr_Locked)
                addObjects(layer->objects());
        }

        void MapRenderer::objectWasAddedToLayer(Model::Layer* layer, Model::Object* object) {
            AddObject visitor(*this);
            object->accept(visitor);
        }
        
        void MapRenderer::objectWasRemovedFromLayer(Model::Layer* layer, Model::Object* object) {
            RemoveObject visitor(*this);
            object->accept(visitor);
        }

        void MapRenderer::selectionDidChange(const Model::SelectionResult& result) {
            View::MapDocumentSPtr document = lock(m_document);
            
            Model::EntityList selectedEntities, deselectedEntities;
            Model::BrushList selectedBrushes, deselectedBrushes, partiallySelectedBrushes, partiallyDeselectedBrushes;

            Model::filterObjects(result.selectedObjects().begin(),
                                 result.selectedObjects().end(),
                                 selectedEntities, selectedBrushes);
            Model::filterObjects(result.partiallySelectedObjects().begin(),
                                 result.partiallySelectedObjects().end(),
                                 selectedEntities, partiallySelectedBrushes);
            
            Model::filterObjects(result.deselectedObjects().begin(),
                                 result.deselectedObjects().end(),
                                 deselectedEntities, deselectedBrushes);
            Model::filterObjects(result.partiallyDeselectedObjects().begin(),
                                 result.partiallyDeselectedObjects().end(),
                                 deselectedEntities, partiallyDeselectedBrushes);

            m_unselectedBrushRenderer.removeBrushes(selectedBrushes.begin(), selectedBrushes.end());
            m_unselectedBrushRenderer.addBrushes(deselectedBrushes.begin(), deselectedBrushes.end());
            m_selectedBrushRenderer.removeBrushes(deselectedBrushes.begin(), deselectedBrushes.end());
            m_selectedBrushRenderer.addBrushes(selectedBrushes.begin(), selectedBrushes.end());
            m_selectedBrushRenderer.removeBrushes(partiallyDeselectedBrushes.begin(), partiallyDeselectedBrushes.end());
            m_selectedBrushRenderer.addBrushes(partiallySelectedBrushes.begin(), partiallySelectedBrushes.end());

            // If only faces are additionally selected with already selected siblings, no brushes get added or removed
            // from the renderers, so we force them to be invalidated!
            m_unselectedBrushRenderer.invalidate();
            m_selectedBrushRenderer.invalidate();
            
            m_unselectedEntityRenderer.removeEntities(selectedEntities.begin(), selectedEntities.end());
            m_unselectedEntityRenderer.addEntities(deselectedEntities.begin(), deselectedEntities.end());
            m_selectedEntityRenderer.removeEntities(deselectedEntities.begin(), deselectedEntities.end());
            m_selectedEntityRenderer.addEntities(selectedEntities.begin(), selectedEntities.end());

            m_entityLinkRenderer.invalidate();
        }
        
        void MapRenderer::modsDidChange() {
            m_unselectedEntityRenderer.reloadModels();
            m_selectedEntityRenderer.reloadModels();
        }
        
        void MapRenderer::entityDefinitionsDidChange() {
            m_unselectedEntityRenderer.reloadModels();
            m_selectedEntityRenderer.reloadModels();
        }
        
        void MapRenderer::textureCollectionsDidChange() {
            m_unselectedBrushRenderer.invalidate();
            m_selectedBrushRenderer.invalidate();
        }
        
        void MapRenderer::preferenceDidChange(const IO::Path& path) {
            View::MapDocumentSPtr document = lock(m_document);
            if (document->isGamePathPreference(path)) {
                m_unselectedEntityRenderer.reloadModels();
				m_unselectedEntityRenderer.invalidate();
                m_selectedEntityRenderer.reloadModels();
				m_selectedEntityRenderer.invalidate();
            }
            setupRendererColors();
        }
        
        void MapRenderer::loadMap(Model::Map& map) {
            m_unselectedBrushRenderer.setBrushes(map.brushes());
            
            m_unselectedEntityRenderer.addEntities(map.entities().begin(),
                                                   map.entities().end());
            m_entityLinkRenderer.invalidate();
        }

        void MapRenderer::clearState() {
            m_unselectedBrushRenderer.clear();
            m_selectedBrushRenderer.clear();
            m_unselectedEntityRenderer.clear();
            m_selectedEntityRenderer.clear();
            m_entityLinkRenderer.invalidate();
            m_pointFileRenderer = EdgeRenderer();
        }

        
        void MapRenderer::addObjects(const Model::ObjectList& objects) {
            AddObject visitor(*this);
            Model::Object::acceptRecursively(objects.begin(), objects.end(), visitor);
        }
        
        void MapRenderer::updateObjects(const Model::ObjectList& objects) {
            UpdateObject visitor(*this);
            Model::Object::acceptRecursively(objects.begin(), objects.end(), visitor);
        }
        
        void MapRenderer::removeObjects(const Model::ObjectList& objects) {
            RemoveObject visitor(*this);
            Model::Object::acceptRecursively(objects.begin(), objects.end(), visitor);
        }
    }
}
