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

#include "MapView.h"

#include "CollectionUtils.h"
#include "Exceptions.h"
#include "Logger.h"
#include "Notifier.h"
#include "Preferences.h"
#include "Model/Brush.h"
#include "Model/BrushVertex.h"
#include "Model/Entity.h"
#include "Model/HitAdapter.h"
#include "Model/Map.h"
#include "Model/Object.h"
#include "Renderer/PerspectiveCamera.h"
#include "Renderer/RenderContext.h"
#include "Renderer/Transformation.h"
#include "Renderer/Vertex.h"
#include "Renderer/VertexArray.h"
#include "Renderer/VertexSpec.h"
#include "View/Animation.h"
#include "View/CameraAnimation.h"
#include "View/CameraTool.h"
#include "View/ClipTool.h"
#include "View/CommandIds.h"
#include "View/CreateBrushTool.h"
#include "View/CreateEntityTool.h"
#include "View/MapDocument.h"
#include "View/ToolBoxDropTarget.h"
#include "View/MoveObjectsTool.h"
#include "View/ResizeBrushesTool.h"
#include "View/RotateObjectsTool.h"
#include "View/SelectionTool.h"
#include "View/TextureTool.h"
#include "View/VertexTool.h"

#include <wx/menu.h>
#include <wx/timer.h>

namespace TrenchBroom {
    namespace View {
        MapView::MapView(wxWindow* parent, Logger* logger, View::MapDocumentWPtr document, ControllerWPtr controller, Renderer::Camera& camera) :
        RenderView(parent, attribs()),
        m_logger(logger),
        m_document(document),
        m_controller(controller),
        m_camera(camera),
        m_animationManager(new AnimationManager()),
        m_toolBox(this, this),
        m_cameraTool(NULL),
        m_clipTool(NULL),
        m_createBrushTool(NULL),
        m_createEntityTool(NULL),
        m_moveObjectsTool(NULL),
        m_vertexTool(NULL),
        m_resizeBrushesTool(NULL),
        m_rotateObjectsTool(NULL),
        m_selectionTool(NULL),
        m_textureTool(NULL),
        m_renderer(document, contextHolder()->fontManager()),
        m_compass(),
        m_selectionGuide(defaultFont(contextHolder()->fontManager())) {
            createTools();
            bindEvents();
            bindObservers();
            
            SetDropTarget(new ToolBoxDropTarget(m_toolBox));
        }
        
        MapView::~MapView() {
            unbindObservers();
            deleteTools();
            m_animationManager->Delete();
            m_animationManager = NULL;
            m_logger = NULL;
        }

        void MapView::centerCameraOnSelection() {
            MapDocumentSPtr document = lock(m_document);
            const Model::EntityList& entities = document->selectedEntities();
            const Model::BrushList& brushes = document->selectedBrushes();
            assert(!entities.empty() || !brushes.empty());
            
            const Vec3 newPosition = centerCameraOnObjectsPosition(entities, brushes);
            animateCamera(newPosition, m_camera.direction(), m_camera.up(), 150);
        }

        void MapView::animateCamera(const Vec3f& position, const Vec3f& direction, const Vec3f& up, const wxLongLong duration) {
            CameraAnimation* animation = new CameraAnimation(m_camera, position, direction, up, duration);
            m_animationManager->runAnimation(animation, true);
        }

        void MapView::toggleMovementRestriction() {
            m_movementRestriction.toggleHorizontalRestriction(m_camera);
            Refresh();
        }

        bool MapView::anyToolActive() const {
            return m_toolBox.anyToolActive();
        }

        void MapView::toggleClipTool() {
            m_toolBox.toggleTool(m_clipTool);
        }
        
        bool MapView::clipToolActive() const {
            return m_toolBox.toolActive(m_clipTool);
        }

        bool MapView::canToggleClipSide() const {
            assert(clipToolActive());
            return m_clipTool->canToggleClipSide();
        }

        void MapView::toggleClipSide() {
            assert(clipToolActive());
            m_clipTool->toggleClipSide();
            Refresh();
        }
        
        bool MapView::canPerformClip() const {
            assert(clipToolActive());
            return m_clipTool->canPerformClip();
        }

        void MapView::performClip() {
            assert(clipToolActive());
            m_clipTool->performClip();
            Refresh();
        }

        bool MapView::canDeleteLastClipPoint() const {
            assert(clipToolActive());
            return m_clipTool->canDeleteLastClipPoint();
        }
        
        void MapView::deleteLastClipPoint() {
            assert(clipToolActive());
            m_clipTool->deleteLastClipPoint();
            Refresh();
        }

        void MapView::toggleRotateObjectsTool() {
            m_toolBox.toggleTool(m_rotateObjectsTool);
        }
        
        bool MapView::rotateObjectsToolActive() const {
            return m_toolBox.toolActive(m_rotateObjectsTool);
        }

        void MapView::toggleVertexTool() {
            m_toolBox.toggleTool(m_vertexTool);
        }
        
        bool MapView::vertexToolActive() const {
            return m_toolBox.toolActive(m_vertexTool);
        }
        
        bool MapView::hasSelectedVertices() const {
            return vertexToolActive() && m_vertexTool->hasSelectedHandles();
        }

        bool MapView::canSnapVertices() const {
            return vertexToolActive() && m_vertexTool->canSnapVertices();
        }
        
        void MapView::snapVertices(const size_t snapTo) {
            assert(vertexToolActive());
            m_vertexTool->snapVertices(snapTo);
        }

        void MapView::toggleTextureTool() {
            m_toolBox.toggleTool(m_textureTool);
        }
        
        bool MapView::textureToolActive() const {
            return m_toolBox.toolActive(m_textureTool);
        }

        void MapView::moveObjects(const Math::Direction direction) {
            MapDocumentSPtr document = lock(m_document);
            const Model::ObjectList& objects = document->selectedObjects();
            if (objects.empty())
                return;
            
            ControllerSPtr controller = lock(m_controller);
            const Grid& grid = document->grid();
            const Vec3 delta = moveDirection(direction) * static_cast<FloatType>(grid.actualSize());
            
            controller->moveObjects(objects, delta, document->textureLock());
        }
        
        void MapView::rotateObjects(const RotationAxis axisSpec, const bool clockwise) {
            MapDocumentSPtr document = lock(m_document);
            const Model::ObjectList& objects = document->selectedObjects();
            if (objects.empty())
                return;
            
            Vec3 axis;
            switch (axisSpec) {
                case RotationAxis_Roll:
                    axis = moveDirection(Math::Direction_Forward);
                    break;
                case RotationAxis_Pitch:
                    axis = moveDirection(Math::Direction_Right);
                    break;
                case RotationAxis_Yaw:
                    axis = Vec3::PosZ;
                    break;
            }
            
            if (!clockwise)
                axis *= -1.0;
            
            ControllerSPtr controller = lock(m_controller);
            const Grid& grid = document->grid();
            const Vec3 center = grid.referencePoint(document->selectionBounds());
            controller->rotateObjects(objects, center, axis, Math::C::PiOverTwo, document->textureLock());
        }
        
        void MapView::flipObjects(const Math::Direction direction) {
            MapDocumentSPtr document = lock(m_document);
            const Model::ObjectList& objects = document->selectedObjects();
            if (objects.empty())
                return;
            
            const Grid& grid = document->grid();
            const Vec3 center = grid.referencePoint(document->selectionBounds());
            const Math::Axis::Type axis = moveDirection(direction).firstComponent();
            
            ControllerSPtr controller = lock(m_controller);
            controller->flipObjects(objects, center, axis, document->textureLock());
        }
        
        void MapView::moveTextures(const Math::Direction direction, const bool snapToGrid) {
            MapDocumentSPtr document = lock(m_document);
            const Model::BrushFaceList& faces = document->allSelectedFaces();
            if (faces.empty())
                return;
            
            const Grid& grid = document->grid();
            const float distance = snapToGrid ? static_cast<float>(grid.actualSize()) : 1.0f;
            
            ControllerSPtr controller = lock(m_controller);
            controller->moveTextures(faces, m_camera.up(), m_camera.right(), direction, distance);
        }
        
        void MapView::moveVertices(const Math::Direction direction) {
            assert(vertexToolActive());
            MapDocumentSPtr document = lock(m_document);
            const Grid& grid = document->grid();
            const Vec3 delta = moveDirection(direction) * static_cast<FloatType>(grid.actualSize());
            m_vertexTool->moveVerticesAndRebuildBrushGeometry(delta);
        }
        
        Vec3 MapView::pasteObjectsDelta(const BBox3& bounds) const {
            MapDocumentSPtr document = lock(m_document);
            const Grid& grid = document->grid();
            const wxMouseState mouseState = wxGetMouseState();
            const wxPoint clientCoords = ScreenToClient(mouseState.GetPosition());
            if (HitTest(clientCoords) == wxHT_WINDOW_INSIDE) {
                const Ray3f pickRay = m_camera.pickRay(clientCoords.x, clientCoords.y);
                const Hits& hits = document->pick(Ray3(pickRay));
                const Hit& hit = Model::findFirstHit(hits, Model::Brush::BrushHit, document->filter(), true);
                if (hit.isMatch()) {
                    const Model::BrushFace* face = Model::hitAsFace(hit);
                    const Vec3 snappedHitPoint = grid.snap(hit.hitPoint());
                    return grid.moveDeltaForBounds(face, bounds, document->worldBounds(), pickRay, snappedHitPoint);
                } else {
                    const Vec3 snappedCenter = grid.snap(bounds.center());
                    const Vec3 snappedDefaultPoint = grid.snap(m_camera.defaultPoint(pickRay));
                    return snappedDefaultPoint - snappedCenter;
                }
            } else {
                const Vec3 snappedCenter = grid.snap(bounds.center());
                const Vec3 snappedDefaultPoint = grid.snap(m_camera.defaultPoint());
                return snappedDefaultPoint - snappedCenter;
            }
        }

        void MapView::OnKey(wxKeyEvent& event) {
            m_movementRestriction.setVerticalRestriction(event.AltDown());
            event.Skip();
        }

        void MapView::OnActivateFrame(wxActivateEvent& event) {
            m_toolBox.updateLastActivation();
            event.Skip();
        }

        void MapView::OnPopupReparentBrushes(wxCommandEvent& event) {
            MapDocumentSPtr document = lock(m_document);
            const Model::BrushList& brushes = document->selectedBrushes();
            Model::Entity* newParent = findNewBrushParent(brushes);
            assert(newParent != NULL);
            
            reparentBrushes(brushes, newParent);
        }
        
        void MapView::OnPopupMoveBrushesToWorld(wxCommandEvent& event) {
            MapDocumentSPtr document = lock(m_document);
            const Model::BrushList& brushes = document->selectedBrushes();
            reparentBrushes(brushes, document->worldspawn());
        }
        
        void MapView::OnPopupCreatePointEntity(wxCommandEvent& event) {
            MapDocumentSPtr document = lock(m_document);
            Assets::EntityDefinitionManager& manager = document->entityDefinitionManager();
            const Assets::EntityDefinitionGroups groups = manager.groups(Assets::EntityDefinition::Type_PointEntity);
            const size_t index = static_cast<size_t>(event.GetId() - CommandIds::CreateEntityPopupMenu::LowestPointEntityItem);
            const Assets::EntityDefinition* definition = findEntityDefinition(groups, index);
            assert(definition != NULL);
            assert(definition->type() == Assets::EntityDefinition::Type_PointEntity);
            createPointEntity(*static_cast<const Assets::PointEntityDefinition*>(definition));
        }
        
        void MapView::OnPopupCreateBrushEntity(wxCommandEvent& event) {
            MapDocumentSPtr document = lock(m_document);
            Assets::EntityDefinitionManager& manager = document->entityDefinitionManager();
            const Assets::EntityDefinitionGroups groups = manager.groups(Assets::EntityDefinition::Type_BrushEntity);
            const size_t index = static_cast<size_t>(event.GetId() - CommandIds::CreateEntityPopupMenu::LowestBrushEntityItem);
            const Assets::EntityDefinition* definition = findEntityDefinition(groups, index);
            assert(definition != NULL);
            assert(definition->type() == Assets::EntityDefinition::Type_BrushEntity);
            createBrushEntity(*static_cast<const Assets::BrushEntityDefinition*>(definition));
        }

        void MapView::OnUpdatePopupMenuItem(wxUpdateUIEvent& event) {
            switch (event.GetId()) {
                case CommandIds::CreateEntityPopupMenu::ReparentBrushes:
                    updateReparentBrushesMenuItem(event);
                    break;
                case CommandIds::CreateEntityPopupMenu::MoveBrushesToWorld:
                    updateMoveBrushesToWorldMenuItem(event);
                    break;
                default:
                    event.Enable(true);
                    break;
            }
        }
        
        void MapView::updateReparentBrushesMenuItem(wxUpdateUIEvent& event) const {
            MapDocumentSPtr document = lock(m_document);
            const Model::BrushList& brushes = document->selectedBrushes();
            StringStream name;
            name << "Move " << (brushes.size() == 1 ? "Brush" : "Brushes") << " to ";
            
            if (!document->hasSelectedBrushes() || document->hasSelectedEntities() || document->hasSelectedFaces()) {
                event.Enable(false);
                name << "Entity";
            } else {
                const Model::Entity* newParent = findNewBrushParent(brushes);
                if (newParent != NULL) {
                    event.Enable(true);
                    name << newParent->classname("<missing classname>");
                } else {
                    event.Enable(false);
                    name << "Entity";
                }
            }
            event.SetText(name.str());
        }

        void MapView::updateMoveBrushesToWorldMenuItem(wxUpdateUIEvent& event) const {
            MapDocumentSPtr document = lock(m_document);
            const Model::BrushList& brushes = document->selectedBrushes();
            StringStream name;
            name << "Move " << (brushes.size() == 1 ? "Brush" : "Brushes") << " to World";
            event.Enable(canReparentBrushes(brushes, document->worldspawn()));
            event.SetText(name.str());
        }

        Model::Entity* MapView::findNewBrushParent(const Model::BrushList& brushes) const {
            Model::Entity* newParent = NULL;

            MapDocumentSPtr document = lock(m_document);
            const Hit& hit = Model::findFirstHit(m_toolBox.hits(), Model::Entity::EntityHit | Model::Brush::BrushHit, document->filter(), true);
            if (hit.isMatch()) {
                if (hit.type() == Model::Entity::EntityHit) {
                    newParent = Model::hitAsEntity(hit);
                } else if (hit.type() == Model::Brush::BrushHit) {
                    const Model::Brush* brush = Model::hitAsBrush(hit);
                    newParent = brush->parent();
                }
            }
            
            if (newParent == NULL)
                return NULL;
            if (canReparentBrushes(brushes, newParent))
                return newParent;
            return NULL;
        }

        bool MapView::canReparentBrushes(const Model::BrushList& brushes, const Model::Entity* newParent) const {
            Model::BrushList::const_iterator it, end;
            for (it = brushes.begin(), end = brushes.end(); it != end; ++it) {
                const Model::Brush* brush = *it;
                if (brush->parent() != newParent)
                    return true;
            }
            return false;
        }

        // note that we make a copy of the brush list on purpose here
        void MapView::reparentBrushes(const Model::BrushList brushes, Model::Entity* newParent) {
            assert(newParent != NULL);
            assert(canReparentBrushes(brushes, newParent));
            
            ControllerSPtr controller = lock(m_controller);

            StringStream name;
            name << "Move " << (brushes.size() == 1 ? "Brush" : "Brushes") << " to " << newParent->classname("<missing classname>");
            
            controller->beginUndoableGroup(name.str());
            controller->deselectAll();
            controller->reparentBrushes(brushes, newParent);
            controller->selectObjects(VectorUtils::cast<Model::Object*>(brushes));
            controller->closeGroup();
        }

        Assets::EntityDefinition* MapView::findEntityDefinition(const Assets::EntityDefinitionGroups& groups, const size_t index) const {
            Assets::EntityDefinitionGroups::const_iterator groupIt, groupEnd;
            Assets::EntityDefinitionList::const_iterator defIt, defEnd;
            
            size_t count = 0;
            for (groupIt = groups.begin(), groupEnd = groups.end(); groupIt != groupEnd; ++groupIt) {
                const Assets::EntityDefinitionList& definitions = groupIt->second;
                if (index < count + definitions.size())
                    return definitions[index - count];
                count += definitions.size();
            }
            return NULL;
        }

        void MapView::createPointEntity(const Assets::PointEntityDefinition& definition) {
            MapDocumentSPtr document = lock(m_document);
            ControllerSPtr controller = lock(m_controller);

            Model::Entity* entity = document->map()->createEntity();
            entity->addOrUpdateProperty(Model::PropertyKeys::Classname, definition.name());
            
            Vec3 delta;
            View::Grid& grid = document->grid();
            
            const Hit& hit = Model::findFirstHit(m_toolBox.hits(), Model::Brush::BrushHit, document->filter(), true);
            if (hit.isMatch()) {
                delta = grid.moveDeltaForBounds(Model::hitAsFace(hit), definition.bounds(), document->worldBounds(), m_toolBox.pickRay(), hit.hitPoint());
            } else {
                const Vec3 newPosition(m_camera.defaultPoint(m_toolBox.pickRay()));
                delta = grid.moveDeltaForPoint(definition.bounds().center(), document->worldBounds(), newPosition - definition.bounds().center());
            }
            
            StringStream name;
            name << "Create " << definition.name();
            
            controller->beginUndoableGroup(name.str());
            controller->deselectAll();
            controller->addEntity(entity);
            controller->selectObject(entity);
            controller->moveObjects(Model::ObjectList(1, entity), delta, false);
            controller->closeGroup();
        }
        
        Vec3 MapView::moveDirection(const Math::Direction direction) const {
            switch (direction) {
                case Math::Direction_Forward: {
                    Vec3 dir = m_camera.direction().firstAxis();
                    if (dir.z() < 0.0)
                        dir = m_camera.up().firstAxis();
                    else if (dir.z() > 0.0)
                        dir = -m_camera.up().firstAxis();
                    return dir;
                }
                case Math::Direction_Backward:
                    return -moveDirection(Math::Direction_Forward);
                case Math::Direction_Left:
                    return -moveDirection(Math::Direction_Right);
                case Math::Direction_Right: {
                    Vec3 dir = m_camera.right().firstAxis();
                    if (dir == moveDirection(Math::Direction_Forward))
                        dir = crossed(dir, Vec3::PosZ);
                    return dir;
                }
                case Math::Direction_Up:
                    return Vec3::PosZ;
                case Math::Direction_Down:
                    return Vec3::NegZ;
                default:
                    assert(false);
                    return Vec3::Null;
            }
        }
        
        Vec3f MapView::centerCameraOnObjectsPosition(const Model::EntityList& entities, const Model::BrushList& brushes) {
            Model::EntityList::const_iterator entityIt, entityEnd;
            Model::BrushList::const_iterator brushIt, brushEnd;
            
            float minDist = std::numeric_limits<float>::max();
            Vec3 center;
            size_t count = 0;
            
            for (entityIt = entities.begin(), entityEnd = entities.end(); entityIt != entityEnd; ++entityIt) {
                const Model::Entity* entity = *entityIt;
                if (entity->brushes().empty()) {
                    const Vec3::List vertices = bBoxVertices(entity->bounds());
                    for (size_t i = 0; i < vertices.size(); ++i) {
                        const Vec3f vertex(vertices[i]);
                        const Vec3f toPosition = vertex - m_camera.position();
                        minDist = std::min(minDist, toPosition.dot(m_camera.direction()));
                        center += vertices[i];
                        ++count;
                    }
                }
            }
            
            for (brushIt = brushes.begin(), brushEnd = brushes.end(); brushIt != brushEnd; ++brushIt) {
                const Model::Brush* brush = *brushIt;
                const Model::BrushVertexList& vertices = brush->vertices();
                for (size_t i = 0; i < vertices.size(); ++i) {
                    const Model::BrushVertex* vertex = vertices[i];
                    const Vec3f toPosition = Vec3f(vertex->position) - m_camera.position();
                    minDist = std::min(minDist, toPosition.dot(m_camera.direction()));
                    center += vertex->position;
                    ++count;
                }
            }

            center /= static_cast<FloatType>(count);
            
            // act as if the camera were there already:
            const Vec3f oldPosition = m_camera.position();
            m_camera.moveTo(Vec3f(center));
            
            float offset = std::numeric_limits<float>::max();
            
            Plane3f frustumPlanes[4];
            m_camera.frustumPlanes(frustumPlanes[0], frustumPlanes[1], frustumPlanes[2], frustumPlanes[3]);
            
            for (entityIt = entities.begin(), entityEnd = entities.end(); entityIt != entityEnd; ++entityIt) {
                const Model::Entity* entity = *entityIt;
                if (entity->brushes().empty()) {
                    const Vec3::List vertices = bBoxVertices(entity->bounds());
                    for (size_t i = 0; i < vertices.size(); ++i) {
                        const Vec3f vertex(vertices[i]);
                        
                        for (size_t j = 0; j < 4; ++j) {
                            const Plane3f& plane = frustumPlanes[j];
                            const float dist = (vertex - m_camera.position()).dot(plane.normal) - 8.0f; // adds a bit of a border
                            offset = std::min(offset, -dist / m_camera.direction().dot(plane.normal));
                        }
                    }
                }
            }
            
            for (brushIt = brushes.begin(), brushEnd = brushes.end(); brushIt != brushEnd; ++brushIt) {
                const Model::Brush* brush = *brushIt;
                const Model::BrushVertexList& vertices = brush->vertices();
                for (size_t i = 0; i < vertices.size(); ++i) {
                    const Model::BrushVertex* vertex = vertices[i];
                    
                    for (size_t j = 0; j < 4; ++j) {
                        const Plane3f& plane = frustumPlanes[j];
                        const float dist = (Vec3f(vertex->position) - m_camera.position()).dot(plane.normal) - 8.0f; // adds a bit of a border
                        offset = std::min(offset, -dist / m_camera.direction().dot(plane.normal));
                    }
                }
            }
            
            // jump back
            m_camera.moveTo(oldPosition);
            
            return center + m_camera.direction() * offset;
        }
        
        void MapView::createBrushEntity(const Assets::BrushEntityDefinition& definition) {
            MapDocumentSPtr document = lock(m_document);
            ControllerSPtr controller = lock(m_controller);

            const Model::BrushList brushes = document->selectedBrushes();
            assert(!brushes.empty());

            // if all brushes belong to the same entity, and that entity is not worldspawn, copy its properties
            Model::BrushList::const_iterator it = brushes.begin();
            Model::BrushList::const_iterator end = brushes.end();
            Model::Entity* entityTemplate = (*it++)->parent();
            while (it != end && entityTemplate != NULL)
                if ((*it++)->parent() != entityTemplate)
                    entityTemplate = NULL;
            
            Model::Entity* entity = document->map()->createEntity();
            if (entityTemplate != NULL && !entityTemplate->worldspawn())
                entity->setProperties(entityTemplate->properties());
            entity->addOrUpdateProperty(Model::PropertyKeys::Classname, definition.name());
            
            StringStream name;
            name << "Create " << definition.name();
            
            controller->beginUndoableGroup(name.str());
            controller->deselectAll();
            controller->addEntity(entity);
            controller->reparentBrushes(brushes, entity);
            controller->selectObjects(VectorUtils::cast<Model::Object*>(brushes));
            controller->closeGroup();
        }

        void MapView::resetCamera() {
            m_camera.setDirection(Vec3f(-1.0f, -1.0f, -0.65f).normalized(), Vec3f::PosZ);
            m_camera.moveTo(Vec3f(160.0f, 160.0f, 48.0f));
        }

        void MapView::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->documentWasNewedNotifier.addObserver(this, &MapView::documentWasNewedOrLoaded);
            document->documentWasLoadedNotifier.addObserver(this, &MapView::documentWasNewedOrLoaded);
            document->objectWasAddedNotifier.addObserver(this, &MapView::objectWasAddedOrRemoved);
            document->objectWasRemovedNotifier.addObserver(this, &MapView::objectWasAddedOrRemoved);
            document->objectDidChangeNotifier.addObserver(this, &MapView::objectDidChange);
            document->faceDidChangeNotifier.addObserver(this, &MapView::faceDidChange);
            document->selectionDidChangeNotifier.addObserver(this, &MapView::selectionDidChange);
            document->modsDidChangeNotifier.addObserver(this, &MapView::modsDidChange);
            document->selectionDidChangeNotifier.addObserver(this, &MapView::selectionDidChange);
            document->grid().gridDidChangeNotifier.addObserver(this, &MapView::gridDidChange);

            ControllerSPtr controller = lock(m_controller);
            controller->commandDoneNotifier.addObserver(this, &MapView::commandDoneOrUndone);
            controller->commandUndoneNotifier.addObserver(this, &MapView::commandDoneOrUndone);
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.addObserver(this, &MapView::preferenceDidChange);
            
            m_camera.cameraDidChangeNotifier.addObserver(this, &MapView::cameraDidChange);
        }
        
        void MapView::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->documentWasNewedNotifier.removeObserver(this, &MapView::documentWasNewedOrLoaded);
                document->documentWasLoadedNotifier.removeObserver(this, &MapView::documentWasNewedOrLoaded);
                document->objectWasAddedNotifier.removeObserver(this, &MapView::objectWasAddedOrRemoved);
                document->objectWasRemovedNotifier.removeObserver(this, &MapView::objectWasAddedOrRemoved);
                document->objectDidChangeNotifier.removeObserver(this, &MapView::objectDidChange);
                document->faceDidChangeNotifier.removeObserver(this, &MapView::faceDidChange);
                document->selectionDidChangeNotifier.removeObserver(this, &MapView::selectionDidChange);
                document->modsDidChangeNotifier.removeObserver(this, &MapView::modsDidChange);
                document->selectionDidChangeNotifier.removeObserver(this, &MapView::selectionDidChange);
                document->grid().gridDidChangeNotifier.removeObserver(this, &MapView::gridDidChange);
            }
            
            if (!expired(m_controller)) {
                ControllerSPtr controller = lock(m_controller);
                controller->commandDoneNotifier.removeObserver(this, &MapView::commandDoneOrUndone);
                controller->commandUndoneNotifier.removeObserver(this, &MapView::commandDoneOrUndone);
            }
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.removeObserver(this, &MapView::preferenceDidChange);
            
            // Unfortunately due to the order in which objects and their fields are destroyed by the runtime system,
            // the camera has already been destroyed at this point.
            // m_camera.cameraDidChangeNotifier.removeObserver(this, &MapView::cameraDidChange);
        }

        void MapView::documentWasNewedOrLoaded() {
            resetCamera();
        }
        
        void MapView::objectWasAddedOrRemoved(Model::Object* object) {
            Refresh();
        }

        void MapView::objectDidChange(Model::Object* object) {
            View::MapDocumentSPtr document = lock(m_document);
            if (document->hasSelectedObjects())
                m_selectionGuide.setBounds(lock(m_document)->selectionBounds());
        }
        
        void MapView::faceDidChange(Model::BrushFace* face) {
            Refresh();
        }
        
        void MapView::selectionDidChange(const Model::SelectionResult& result) {
            View::MapDocumentSPtr document = lock(m_document);
            if (document->hasSelectedObjects())
                m_selectionGuide.setBounds(lock(m_document)->selectionBounds());
            Refresh();
        }
        
        void MapView::gridDidChange() {
            Refresh();
        }

        void MapView::modsDidChange() {
            Refresh();
        }
        
        void MapView::commandDoneOrUndone(Controller::Command::Ptr command) {
            m_toolBox.updateHits();
            Refresh();
        }
        
        void MapView::preferenceDidChange(const IO::Path& path) {
            Refresh();
        }
        
        void MapView::cameraDidChange(const Renderer::Camera* camera) {
            Refresh();
        }

        void MapView::createTools() {
            Renderer::TextureFont& font = defaultFont(contextHolder()->fontManager());

            m_cameraTool = new CameraTool(m_document, m_controller, m_camera);
            m_clipTool = new ClipTool(m_document, m_controller, m_camera);
            m_createBrushTool = new CreateBrushTool(m_document, m_controller, m_camera, font);
            m_createEntityTool = new CreateEntityTool(m_document, m_controller, m_camera, contextHolder()->fontManager());
            m_moveObjectsTool = new MoveObjectsTool(m_document, m_controller, m_movementRestriction);
            m_resizeBrushesTool = new ResizeBrushesTool(m_document, m_controller, m_camera);
            m_rotateObjectsTool = new RotateObjectsTool(m_document, m_controller, m_camera, m_movementRestriction, font);
            m_selectionTool = new SelectionTool(m_document, m_controller);
            m_textureTool = new TextureTool(m_document, m_controller);
            m_vertexTool = new VertexTool(m_document, m_controller, m_movementRestriction, font);
            
            m_toolBox.addTool(m_cameraTool);
            m_toolBox.addTool(m_textureTool);
            m_toolBox.addTool(m_clipTool);
            m_toolBox.addTool(m_rotateObjectsTool);
            m_toolBox.addTool(m_vertexTool);
            m_toolBox.addTool(m_createEntityTool);
            m_toolBox.addTool(m_createBrushTool);
            m_toolBox.addTool(m_moveObjectsTool);
            m_toolBox.addTool(m_resizeBrushesTool);
            m_toolBox.addTool(m_selectionTool);
        }
        
        void MapView::deleteTools() {
            delete m_cameraTool;
            m_cameraTool = NULL;
            delete m_clipTool;
            m_clipTool = NULL;
            delete m_createBrushTool;
            m_createBrushTool = NULL;
            delete m_createEntityTool;
            m_createEntityTool = NULL;
            delete m_moveObjectsTool;
            m_moveObjectsTool = NULL;
            delete m_resizeBrushesTool;
            m_resizeBrushesTool = NULL;
            delete m_rotateObjectsTool;
            m_rotateObjectsTool = NULL;
            delete m_selectionTool;
            m_selectionTool = NULL;
            delete m_textureTool;
            m_textureTool = NULL;
            delete m_vertexTool;
            m_vertexTool = NULL;
        }

        void MapView::doUpdateViewport(int x, int y, int width, int height) {
            const Renderer::Camera::Viewport viewport(x, y, width, height);
            m_camera.setViewport(viewport);
        }
        
        void MapView::doInitializeGL() {
            const char* vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
            const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
            const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));

            assert(vendor != NULL);
            assert(renderer != NULL);
            assert(version != NULL);
            
            // provoke a crash on Ubuntu due UTF-8 character in string (glGetString may return UTF8 strings)
            // m_logger->info("Renderer info: Mesa DRI Mobile Intel® GM45 Express Chipset x86/MMX/SSE2 version 2.1 Mesa 9.2.1 from Intel Open Source Technology Center");

            m_logger->info("Renderer info: %s version %s from %s", renderer, version, vendor);
            m_logger->info("Depth buffer bits: %d", depthBits());
            
            if (multisample())
                m_logger->info("Multisampling enabled");
            else
                m_logger->info("Multisampling disabled");
            
            Renderer::SetVboState setVboState(m_vbo);
            setVboState.mapped();
            m_compass.prepare(m_vbo);
        }
        
        void MapView::doRender() {
            MapDocumentSPtr document = lock(m_document);
            document->commitPendingRenderStateChanges();

            const View::Grid& grid = document->grid();
            Renderer::RenderContext context(m_camera, contextHolder()->shaderManager(), grid.visible(), grid.actualSize());
            
            setupGL(context);
            setRenderOptions(context);
            renderMap(context);
            renderSelectionGuide(context);
            renderToolBox(context);
            renderCoordinateSystem(context);
            renderCompass(context);
        }
        
        void MapView::setupGL(Renderer::RenderContext& context) {
            const Renderer::Camera::Viewport& viewport = context.camera().viewport();
            glViewport(viewport.x, viewport.y, viewport.width, viewport.height);
            
            glEnable(GL_MULTISAMPLE);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glShadeModel(GL_SMOOTH);
        }
        
        void MapView::setRenderOptions(Renderer::RenderContext& context) {
            m_toolBox.setRenderOptions(context);
        }

        void MapView::renderCoordinateSystem(Renderer::RenderContext& context) {
            PreferenceManager& prefs = PreferenceManager::instance();
            const Color& xColor = prefs.get(Preferences::XAxisColor);
            const Color& yColor = prefs.get(Preferences::YAxisColor);
            const Color& zColor = prefs.get(Preferences::ZAxisColor);
            
            Renderer::ActiveShader shader(context.shaderManager(), Renderer::Shaders::VaryingPCShader);
            Renderer::SetVboState setVboState(m_vbo);
            setVboState.active();
            glDisable(GL_DEPTH_TEST);
            renderCoordinateSystem(Color(xColor, 0.3f), Color(yColor, 0.3f), Color(zColor, 0.3f));
            glEnable(GL_DEPTH_TEST);
            renderCoordinateSystem(xColor, yColor, zColor);
        }
        
        void MapView::renderCoordinateSystem(const Color& xColor, const Color& yColor, const Color& zColor) {
            MapDocumentSPtr document = lock(m_document);
            Renderer::VertexSpecs::P3C4::Vertex::List vertices = Renderer::coordinateSystem(document->worldBounds(), xColor, yColor, zColor);
            Renderer::VertexArray array = Renderer::VertexArray::swap(GL_LINES, vertices);
            
            Renderer::SetVboState setVboState(m_vbo);
            setVboState.mapped();
            array.prepare(m_vbo);
            setVboState.active();
            array.render();
        }
        
        void MapView::renderMap(Renderer::RenderContext& context) {
            m_renderer.render(context);
        }
        
        void MapView::renderSelectionGuide(Renderer::RenderContext& context) {
            if (context.showSelectionGuide() && !expired(m_document)) {
                View::MapDocumentSPtr document = lock(m_document);
                if (document->hasSelectedObjects()) {
                    PreferenceManager& prefs = PreferenceManager::instance();
                    const Color& color = prefs.get(Preferences::HandleColor);
                    m_selectionGuide.setColor(color);
                    m_selectionGuide.render(context, document);
                }
            }
        }
        
        void MapView::renderToolBox(Renderer::RenderContext& context) {
            m_toolBox.renderTools(context);
        }

        void MapView::renderCompass(Renderer::RenderContext& context) {
            Renderer::SetVboState setVboState(m_vbo);
            setVboState.active();
            m_compass.render(context, m_movementRestriction);
        }
        
        Ray3 MapView::doGetPickRay(const int x, const int y) const {
            return m_camera.pickRay(x, y);
        }
        
        Hits MapView::doPick(const Ray3& pickRay) const {
            MapDocumentSPtr document = lock(m_document);
            return document->pick(pickRay);
        }

        void MapView::doShowPopupMenu() {
            MapDocumentSPtr document = lock(m_document);
            Assets::EntityDefinitionManager& manager = document->entityDefinitionManager();
            const Assets::EntityDefinitionGroups pointGroups = manager.groups(Assets::EntityDefinition::Type_PointEntity);
            const Assets::EntityDefinitionGroups brushGroups = manager.groups(Assets::EntityDefinition::Type_BrushEntity);
            
            wxMenu menu;
            menu.SetEventHandler(this);
            menu.Append(CommandIds::CreateEntityPopupMenu::ReparentBrushes, "Move Brushes to...");
            menu.Append(CommandIds::CreateEntityPopupMenu::MoveBrushesToWorld, "Move Brushes to World");
            menu.AppendSeparator();
            menu.AppendSubMenu(makeEntityGroupsMenu(pointGroups, CommandIds::CreateEntityPopupMenu::LowestPointEntityItem), "Create Point Entity");
            menu.AppendSubMenu(makeEntityGroupsMenu(brushGroups, CommandIds::CreateEntityPopupMenu::LowestBrushEntityItem), "Create Brush Entity");
            
            menu.UpdateUI(this);
            PopupMenu(&menu);
        }
        
        wxMenu* MapView::makeEntityGroupsMenu(const Assets::EntityDefinitionGroups& groups, int id) {
            wxMenu* menu = new wxMenu();
            Assets::EntityDefinitionGroups::const_iterator gIt, gEnd;
            for (gIt = groups.begin(), gEnd = groups.end(); gIt != gEnd; ++gIt) {
                const String& groupName = gIt->first;
                const Assets::EntityDefinitionList& definitions = gIt->second;
                
                wxMenu* groupMenu = new wxMenu();
                groupMenu->SetEventHandler(this);
                
                Assets::EntityDefinitionList::const_iterator dIt, dEnd;
                for (dIt = definitions.begin(), dEnd = definitions.end(); dIt != dEnd; ++dIt) {
                    const Assets::EntityDefinition* definition = *dIt;
                    if (definition->name() != Model::PropertyValues::WorldspawnClassname)
                        groupMenu->Append(id++, definition->shortName());
                }
                
                menu->AppendSubMenu(groupMenu, groupName);
            }
            return menu;
        }

        void MapView::bindEvents() {
            Bind(wxEVT_KEY_DOWN, &MapView::OnKey, this);
            Bind(wxEVT_KEY_UP, &MapView::OnKey, this);

            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnPopupReparentBrushes, this, CommandIds::CreateEntityPopupMenu::ReparentBrushes);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnPopupMoveBrushesToWorld, this, CommandIds::CreateEntityPopupMenu::MoveBrushesToWorld);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnPopupCreatePointEntity, this, CommandIds::CreateEntityPopupMenu::LowestPointEntityItem, CommandIds::CreateEntityPopupMenu::HighestPointEntityItem);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &MapView::OnPopupCreateBrushEntity, this, CommandIds::CreateEntityPopupMenu::LowestBrushEntityItem, CommandIds::CreateEntityPopupMenu::HighestBrushEntityItem);
            
            Bind(wxEVT_UPDATE_UI, &MapView::OnUpdatePopupMenuItem, this, CommandIds::CreateEntityPopupMenu::ReparentBrushes);
            Bind(wxEVT_UPDATE_UI, &MapView::OnUpdatePopupMenuItem, this, CommandIds::CreateEntityPopupMenu::MoveBrushesToWorld);
            Bind(wxEVT_UPDATE_UI, &MapView::OnUpdatePopupMenuItem, this, CommandIds::CreateEntityPopupMenu::LowestPointEntityItem, CommandIds::CreateEntityPopupMenu::HighestPointEntityItem);
            Bind(wxEVT_UPDATE_UI, &MapView::OnUpdatePopupMenuItem, this, CommandIds::CreateEntityPopupMenu::LowestBrushEntityItem, CommandIds::CreateEntityPopupMenu::HighestBrushEntityItem);
        }
        
        const GLContextHolder::GLAttribs& MapView::attribs() {
            static bool initialized = false;
            static GLContextHolder::GLAttribs attribs;
            if (initialized)
                return attribs;

            int testAttribs[] =
            {
                // 32 bit depth buffer, 4 samples
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       32,
                WX_GL_SAMPLE_BUFFERS,   1,
                WX_GL_SAMPLES,          4,
                0,
                // 24 bit depth buffer, 4 samples
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       24,
                WX_GL_SAMPLE_BUFFERS,   1,
                WX_GL_SAMPLES,          4,
                0,
                // 32 bit depth buffer, 2 samples
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       32,
                WX_GL_SAMPLE_BUFFERS,   1,
                WX_GL_SAMPLES,          2,
                0,
                // 24 bit depth buffer, 2 samples
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       24,
                WX_GL_SAMPLE_BUFFERS,   1,
                WX_GL_SAMPLES,          2,
                0,
                // 16 bit depth buffer, 4 samples
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       16,
                WX_GL_SAMPLE_BUFFERS,   1,
                WX_GL_SAMPLES,          4,
                0,
                // 16 bit depth buffer, 2 samples
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       16,
                WX_GL_SAMPLE_BUFFERS,   1,
                WX_GL_SAMPLES,          2,
                0,
                // 32 bit depth buffer, no multisampling
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       32,
                0,
                // 24 bit depth buffer, no multisampling
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       24,
                0,
                // 16 bit depth buffer, no multisampling
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       16,
                0,
                0,
            };

            size_t index = 0;
            while (!initialized && testAttribs[index] != 0) {
                size_t count = 0;
                for (; testAttribs[index + count] != 0; ++count);
                if (wxGLCanvas::IsDisplaySupported(&testAttribs[index])) {
                    for (size_t i = 0; i < count; ++i)
                        attribs.push_back(testAttribs[index + i]);
                    attribs.push_back(0);
                    initialized = true;
                }
                index += count + 1;
            }

            assert(initialized);
            assert(!attribs.empty());
            return attribs;
        }

        int MapView::depthBits() {
            return attribs()[3];
        }
        
        bool MapView::multisample() {
            return attribs()[4] != 0;
        }

        Renderer::TextureFont& MapView::defaultFont(Renderer::FontManager& fontManager) {
            PreferenceManager& prefs = PreferenceManager::instance();
            const String& fontName = prefs.get(Preferences::RendererFontName);
            const size_t fontSize = static_cast<size_t>(prefs.get(Preferences::RendererFontSize));
            return fontManager.font(Renderer::FontDescriptor(fontName, fontSize));
        }
    }
}
