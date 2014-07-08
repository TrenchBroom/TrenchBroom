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
#include "CastIterator.h"
#include "FilterIterator.h"
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
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/VertexSpec.h"
#include "Renderer/VertexArray.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace Renderer {
        struct UnselectedBrushRendererFilter : public BrushRenderer::Filter {
        private:
            const Model::ModelFilter& m_filter;
        public:
            UnselectedBrushRendererFilter(const Model::ModelFilter& filter) :
            m_filter(filter) {}
            
            bool operator()(const Model::Brush* brush) const {
                return !brush->selected() && m_filter.visible(brush);
            }
            
            bool operator()(const Model::BrushFace* face) const {
                return !face->selected() && m_filter.visible(face);
            }
            
            bool operator()(const Model::BrushEdge* edge) const {
                const Model::BrushFace* left = edge->left->face;
                const Model::BrushFace* right = edge->right->face;
                const Model::Brush* brush = left->parent();
                return (!brush->selected() ||
                        (!left->selected() && !right->selected()));
            }
        };
        
        struct SelectedBrushRendererFilter : public BrushRenderer::Filter {
            const Model::ModelFilter& m_filter;
        public:
            SelectedBrushRendererFilter(const Model::ModelFilter& filter) :
            m_filter(filter) {}
            
            bool operator()(const Model::Brush* brush) const {
                return (brush->selected() || brush->partiallySelected()) && m_filter.visible(brush);
            }
            
            bool operator()(const Model::BrushFace* face) const {
                return (face->parent()->selected() || face->selected()) && m_filter.visible(face);
            }
            
            bool operator()(const Model::BrushEdge* edge) const {
                const Model::BrushFace* left = edge->left->face;
                const Model::BrushFace* right = edge->right->face;
                const Model::Brush* brush = left->parent();
                return (brush->selected() ||
                        left->selected() || right->selected());
            }
        };
        
        MapRenderer::MapRenderer(View::MapDocumentWPtr document, FontManager& fontManager) :
        m_document(document),
        m_fontManager(fontManager),
        m_unselectedBrushRenderer(UnselectedBrushRendererFilter(lock(document)->filter())),
        m_selectedBrushRenderer(SelectedBrushRendererFilter(lock(document)->filter())),
        m_unselectedEntityRenderer(lock(document)->entityModelManager(), m_fontManager, lock(document)->filter()),
        m_selectedEntityRenderer(lock(document)->entityModelManager(), m_fontManager, lock(document)->filter()) {
            bindObservers();
        }
        
        MapRenderer::~MapRenderer() {
            unbindObservers();
        }

        void MapRenderer::render(RenderContext& context) {
            setupGL(context);
            
            renderGeometry(context);
            renderEntities(context);
            renderEntityLinks(context);
            renderPointFile(context);
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

        void MapRenderer::renderGeometry(RenderContext& context) {
            PreferenceManager& prefs = PreferenceManager::instance();

            m_unselectedBrushRenderer.setFaceColor(prefs.get(Preferences::FaceColor));
            m_unselectedBrushRenderer.setEdgeColor(prefs.get(Preferences::EdgeColor));
            m_unselectedBrushRenderer.render(context);
            
            if (!context.hideSelection()) {
                const bool applyTinting = context.tintSelection(); // && lock(m_document)->selectedFaces().empty();
                
                m_selectedBrushRenderer.setFaceColor(prefs.get(Preferences::FaceColor));
                m_selectedBrushRenderer.setEdgeColor(prefs.get(Preferences::SelectedEdgeColor));
                m_selectedBrushRenderer.setTintFaces(applyTinting);
                m_selectedBrushRenderer.setTintColor(prefs.get(Preferences::SelectedFaceColor));
                m_selectedBrushRenderer.setRenderOccludedEdges(true);
                m_selectedBrushRenderer.setOccludedEdgeColor(prefs.get(Preferences::OccludedSelectedEdgeColor));
                m_selectedBrushRenderer.render(context);
            }
        }

        void MapRenderer::bindObservers() {
            View::MapDocumentSPtr document = lock(m_document);
            document->documentWasClearedNotifier.addObserver(this, &MapRenderer::documentWasCleared);
            document->documentWasNewedNotifier.addObserver(this, &MapRenderer::documentWasNewedOrLoaded);
            document->documentWasLoadedNotifier.addObserver(this, &MapRenderer::documentWasNewedOrLoaded);
            document->pointFileWasLoadedNotifier.addObserver(this, &MapRenderer::pointFileWasLoadedOrUnloaded);
            document->pointFileWasUnloadedNotifier.addObserver(this, &MapRenderer::pointFileWasLoadedOrUnloaded);
            document->objectWasAddedNotifier.addObserver(this, &MapRenderer::objectWasAdded);
            document->objectWillBeRemovedNotifier.addObserver(this, &MapRenderer::objectWillBeRemoved);
            document->objectDidChangeNotifier.addObserver(this, &MapRenderer::objectDidChange);
            document->faceDidChangeNotifier.addObserver(this, &MapRenderer::faceDidChange);
            document->selectionDidChangeNotifier.addObserver(this, &MapRenderer::selectionDidChange);
            document->modsDidChangeNotifier.addObserver(this, &MapRenderer::modsDidChange);
            document->entityDefinitionsDidChangeNotifier.addObserver(this, &MapRenderer::entityDefinitionsDidChange);
            
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
                document->objectWasAddedNotifier.removeObserver(this, &MapRenderer::objectWasAdded);
                document->objectWillBeRemovedNotifier.removeObserver(this, &MapRenderer::objectWillBeRemoved);
                document->objectDidChangeNotifier.removeObserver(this, &MapRenderer::objectDidChange);
                document->faceDidChangeNotifier.removeObserver(this, &MapRenderer::faceDidChange);
                document->selectionDidChangeNotifier.removeObserver(this, &MapRenderer::selectionDidChange);
                document->modsDidChangeNotifier.removeObserver(this, &MapRenderer::modsDidChange);
                document->entityDefinitionsDidChangeNotifier.removeObserver(this, &MapRenderer::entityDefinitionsDidChange);
            }
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.removeObserver(this, &MapRenderer::preferenceDidChange);
        }

        void MapRenderer::documentWasCleared() {
            clearState();
        }

        void MapRenderer::documentWasNewedOrLoaded() {
            View::MapDocumentSPtr document = lock(m_document);
            loadMap(*document->map());
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

        void MapRenderer::objectWasAdded(Model::Object* object) {
            if (object->type() == Model::Object::Type_Entity) {
                Model::Entity* entity = static_cast<Model::Entity*>(object);
                m_unselectedEntityRenderer.addEntity(entity);
                m_entityLinkRenderer.invalidate();
                
                const Model::BrushList& brushes = entity->brushes();
                m_unselectedBrushRenderer.addBrushes(brushes.begin(), brushes.end());
            } else if (object->type() == Model::Object::Type_Brush) {
                m_unselectedBrushRenderer.addBrush(static_cast<Model::Brush*>(object));
            }
        }
        
        void MapRenderer::objectWillBeRemoved(Model::Object* object) {
            if (object->type() == Model::Object::Type_Entity) {
                Model::Entity* entity = static_cast<Model::Entity*>(object);
                m_unselectedEntityRenderer.removeEntity(entity);
                m_entityLinkRenderer.invalidate();
                
                const Model::BrushList& brushes = entity->brushes();
                m_unselectedBrushRenderer.removeBrushes(brushes.begin(), brushes.end());
            } else if (object->type() == Model::Object::Type_Brush) {
                m_unselectedBrushRenderer.removeBrush(static_cast<Model::Brush*>(object));
            }
        }
        
        void MapRenderer::objectDidChange(Model::Object* object) {
            if (object->type() == Model::Object::Type_Entity) {
                if (object->selected())
                    m_selectedEntityRenderer.updateEntity(static_cast<Model::Entity*>(object));
                else
                    m_unselectedEntityRenderer.updateEntity(static_cast<Model::Entity*>(object));
                m_entityLinkRenderer.invalidate();
            } else if (object->type() == Model::Object::Type_Brush) {
                if (object->selected())
                    m_selectedBrushRenderer.invalidate();
                else
                    m_unselectedBrushRenderer.invalidate();
            }
        }
        
        void MapRenderer::faceDidChange(Model::BrushFace* face) {
            m_selectedBrushRenderer.invalidate();
        }

        void MapRenderer::selectionDidChange(const Model::SelectionResult& result) {
            View::MapDocumentSPtr document = lock(m_document);

            const Model::BrushList unselectedBrushes = document->unselectedBrushes();
            const Model::BrushList& selectedBrushes = document->allSelectedBrushes();
            
            m_unselectedBrushRenderer.setBrushes(unselectedBrushes);
            m_selectedBrushRenderer.setBrushes(selectedBrushes);
            
            m_unselectedEntityRenderer.removeEntities(Model::entityIterator(result.selectedObjects().begin(), result.selectedObjects().end()),
                                                      Model::entityIterator(result.selectedObjects().end()));
            m_unselectedEntityRenderer.addEntities(Model::entityIterator(result.deselectedObjects().begin(), result.deselectedObjects().end()),
                                                   Model::entityIterator(result.deselectedObjects().end()));
            m_selectedEntityRenderer.removeEntities(Model::entityIterator(result.deselectedObjects().begin(), result.deselectedObjects().end()),
                                                    Model::entityIterator(result.deselectedObjects().end()));
            m_selectedEntityRenderer.addEntities(Model::entityIterator(result.selectedObjects().begin(), result.selectedObjects().end()),
                                                 Model::entityIterator(result.selectedObjects().end()));
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

        void MapRenderer::preferenceDidChange(const IO::Path& path) {
            View::MapDocumentSPtr document = lock(m_document);
            if (document->isGamePathPreference(path)) {
                m_unselectedEntityRenderer.reloadModels();
                m_selectedEntityRenderer.reloadModels();
            }
        }

        class EntityLinkFilter : public EntityLinkRenderer::Filter {
        private:
            Color m_selectedColor;
            Color m_linkColor;
            Color m_killColor;
        public:
            EntityLinkFilter() :
            m_selectedColor(1.0f, 0.0f, 0.0f, 1.0f),
            m_linkColor(0.0f, 1.0f, 0.0f, 1.0f),
            m_killColor(0.0f, 0.0f, 1.0f, 1.0f) {}
            
            bool doGetShowLink(const Model::Entity* source, const Model::Entity* target, bool isConnectedToSelected) const {
                return source->selected() || target->selected() || isConnectedToSelected;
            }
            
            const Color& doGetLinkColor(const Model::Entity* source, const Model::Entity* target, bool isConnectedToSelected) const {
                if (source->selected() || target->selected() || isConnectedToSelected)
                    return m_selectedColor;
                return m_linkColor;
            }
            
            const Color& doGetKillColor(const Model::Entity* source, const Model::Entity* target, bool isConnectedToSelected) const {
                if (source->selected() || target->selected() || isConnectedToSelected)
                    return m_selectedColor;
                return m_killColor;
            }
        };
        
        void MapRenderer::renderEntities(RenderContext& context) {
            PreferenceManager& prefs = PreferenceManager::instance();
            
            m_unselectedEntityRenderer.setOverlayTextColor(prefs.get(Preferences::InfoOverlayTextColor));
            m_unselectedEntityRenderer.setOverlayBackgroundColor(prefs.get(Preferences::InfoOverlayBackgroundColor));
            m_unselectedEntityRenderer.setBoundsColor(prefs.get(Preferences::UndefinedEntityColor));
            m_unselectedEntityRenderer.setApplyTinting(false);
            m_unselectedEntityRenderer.render(context);
            
            if (!context.hideSelection()) {
                const bool applyTinting = context.tintSelection();
                m_selectedEntityRenderer.setOverlayTextColor(prefs.get(Preferences::SelectedInfoOverlayTextColor));
                m_selectedEntityRenderer.setOverlayBackgroundColor(prefs.get(Preferences::SelectedInfoOverlayBackgroundColor));
                m_selectedEntityRenderer.setOverrideBoundsColor(true);
                m_selectedEntityRenderer.setBoundsColor(prefs.get(Preferences::SelectedEdgeColor));
                m_selectedEntityRenderer.setRenderOccludedBounds(true);
                m_selectedEntityRenderer.setOccludedBoundsColor(prefs.get(Preferences::OccludedSelectedEdgeColor));
                m_selectedEntityRenderer.setApplyTinting(applyTinting);
                m_selectedEntityRenderer.setTintColor(prefs.get(Preferences::SelectedFaceColor));
                m_selectedEntityRenderer.render(context);
            }
        }
        
        void MapRenderer::renderEntityLinks(RenderContext& context) {
            View::MapDocumentSPtr document = lock(m_document);
            
            if (!m_entityLinkRenderer.valid() && document->map() != NULL)
                m_entityLinkRenderer.validate(EntityLinkFilter(), document->map()->entities());
            
            if (m_entityLinkRenderer.valid())
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

        void MapRenderer::clearState() {
            m_unselectedBrushRenderer.clear();
            m_selectedBrushRenderer.clear();
            m_unselectedEntityRenderer.clear();
            m_selectedEntityRenderer.clear();
            m_entityLinkRenderer.invalidate();
            m_pointFileRenderer = EdgeRenderer();
        }

        void MapRenderer::loadMap(Model::Map& map) {
            m_unselectedBrushRenderer.setBrushes(map.brushes());
            
            m_unselectedEntityRenderer.addEntities(map.entities().begin(),
                                                   map.entities().end());
            m_entityLinkRenderer.invalidate();
        }
    }
}
