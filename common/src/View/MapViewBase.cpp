/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "MapViewBase.h"

#include "FloatType.h"
#include "Logger.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Assets/EntityDefinition.h"
#include "Assets/EntityDefinitionGroup.h"
#include "Assets/EntityDefinitionManager.h"
#include "Model/BezierPatch.h"
#include "Model/BrushNode.h"
#include "Model/BrushFace.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "Model/EditorContext.h"
#include "Model/EntityNode.h"
#include "Model/EntityProperties.h"
#include "Model/GroupNode.h"
#include "Model/Hit.h"
#include "Model/HitAdapter.h"
#include "Model/HitFilter.h"
#include "Model/LayerNode.h"
#include "Model/ModelUtils.h"
#include "Model/PatchNode.h"
#include "Model/PointFile.h"
#include "Model/PortalFile.h"
#include "Model/WorldNode.h"
#include "Renderer/Camera.h"
#include "Renderer/Compass.h"
#include "Renderer/FontDescriptor.h"
#include "Renderer/FontManager.h"
#include "Renderer/MapRenderer.h"
#include "Renderer/PrimitiveRenderer.h"
#include "Renderer/RenderBatch.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderService.h"
#include "View/Actions.h"
#include "View/Animation.h"
#include "View/EnableDisableTagCallback.h"
#include "View/FlashSelectionAnimation.h"
#include "View/GLContextManager.h"
#include "View/Grid.h"
#include "View/MapDocument.h"
#include "View/MapFrame.h"
#include "View/MapViewActivationTracker.h"
#include "View/MapViewToolBox.h"
#include "View/SelectionTool.h"
#include "View/SignalDelayer.h"
#include "View/QtUtils.h"
#include "kdl/vector_utils.h"

#include <kdl/memory_utils.h>
#include <kdl/string_compare.h>
#include <kdl/string_format.h>

#include <vecmath/polygon.h>
#include <vecmath/util.h>

#include <sstream>
#include <vector>

#include <QtGlobal>
#include <QDebug>
#include <QMenu>
#include <QMimeData>
#include <QShortcut>
#include <QString>

namespace TrenchBroom {
    namespace View {
        const int MapViewBase::DefaultCameraAnimationDuration = 250;

        MapViewBase::MapViewBase(Logger* logger, std::weak_ptr<MapDocument> document, MapViewToolBox& toolBox, Renderer::MapRenderer& renderer, GLContextManager& contextManager) :
        RenderView(contextManager),
        m_logger(logger),
        m_document(std::move(document)),
        m_toolBox(toolBox),
        m_animationManager(std::make_unique<AnimationManager>(this)),
        m_renderer(renderer),
        m_compass(nullptr),
        m_portalFileRenderer(nullptr),
        m_isCurrent(false),
        m_updateActionStatesSignalDelayer{new SignalDelayer{this}} {
            setToolBox(toolBox);
            connectObservers();

            setAcceptDrops(true);
        }

        void MapViewBase::setCompass(std::unique_ptr<Renderer::Compass> compass) {
            m_compass = std::move(compass);
        }

        void MapViewBase::mapViewBaseVirtualInit() {
            createActionsAndUpdatePicking();
        }

        MapViewBase::~MapViewBase() {
            // Deleting m_compass will access the VBO so we need to be current
            // see: http://doc.qt.io/qt-5/qopenglwidget.html#resource-initialization-and-cleanup
            makeCurrent();
        }

        void MapViewBase::setIsCurrent(const bool isCurrent) {
            m_isCurrent = isCurrent;
        }

        void MapViewBase::connectObservers() {
            auto document = kdl::mem_lock(m_document);
            m_notifierConnection += document->nodesWereAddedNotifier.connect(this, &MapViewBase::nodesDidChange);
            m_notifierConnection += document->nodesWereRemovedNotifier.connect(this, &MapViewBase::nodesDidChange);
            m_notifierConnection += document->nodesDidChangeNotifier.connect(this, &MapViewBase::nodesDidChange);
            m_notifierConnection += document->nodeVisibilityDidChangeNotifier.connect(this, &MapViewBase::nodesDidChange);
            m_notifierConnection += document->nodeLockingDidChangeNotifier.connect(this, &MapViewBase::nodesDidChange);
            m_notifierConnection += document->commandDoneNotifier.connect(this, &MapViewBase::commandDone);
            m_notifierConnection += document->commandUndoneNotifier.connect(this, &MapViewBase::commandUndone);
            m_notifierConnection += document->selectionDidChangeNotifier.connect(this, &MapViewBase::selectionDidChange);
            m_notifierConnection += document->textureCollectionsDidChangeNotifier.connect(this, &MapViewBase::textureCollectionsDidChange);
            m_notifierConnection += document->entityDefinitionsDidChangeNotifier.connect(this, &MapViewBase::entityDefinitionsDidChange);
            m_notifierConnection += document->modsDidChangeNotifier.connect(this, &MapViewBase::modsDidChange);
            m_notifierConnection += document->editorContextDidChangeNotifier.connect(this, &MapViewBase::editorContextDidChange);
            m_notifierConnection += document->documentWasNewedNotifier.connect(this, &MapViewBase::documentDidChange);
            m_notifierConnection += document->documentWasClearedNotifier.connect(this, &MapViewBase::documentDidChange);
            m_notifierConnection += document->documentWasLoadedNotifier.connect(this, &MapViewBase::documentDidChange);
            m_notifierConnection += document->pointFileWasLoadedNotifier.connect(this, &MapViewBase::pointFileDidChange);
            m_notifierConnection += document->pointFileWasUnloadedNotifier.connect(this, &MapViewBase::pointFileDidChange);
            m_notifierConnection += document->portalFileWasLoadedNotifier.connect(this, &MapViewBase::portalFileDidChange);
            m_notifierConnection += document->portalFileWasUnloadedNotifier.connect(this, &MapViewBase::portalFileDidChange);

            Grid& grid = document->grid();
            m_notifierConnection += grid.gridDidChangeNotifier.connect(this, &MapViewBase::gridDidChange);

            m_notifierConnection += m_toolBox.toolActivatedNotifier.connect(this, &MapViewBase::toolChanged);
            m_notifierConnection += m_toolBox.toolDeactivatedNotifier.connect(this, &MapViewBase::toolChanged);

            PreferenceManager& prefs = PreferenceManager::instance();
            m_notifierConnection += prefs.preferenceDidChangeNotifier.connect(this, &MapViewBase::preferenceDidChange);
        }

        /**
         * Full re-initialization of QActions and picking state.
         */
        void MapViewBase::createActionsAndUpdatePicking() {
            createActions();
            updateActionStates();
            updatePickResult();
        }

        void MapViewBase::nodesDidChange(const std::vector<Model::Node*>&) {
            updatePickResult();
            update();
        }

        void MapViewBase::toolChanged(Tool&) {
            updatePickResult();
            updateActionStates();
            update();
        }

        void MapViewBase::commandDone(Command*) {
            updateActionStatesDelayed();
            updatePickResult();
            update();
        }

        void MapViewBase::commandUndone(UndoableCommand*) {
            updateActionStatesDelayed();
            updatePickResult();
            update();
        }

        void MapViewBase::selectionDidChange(const Selection&) {
            updateActionStatesDelayed();
        }

        void MapViewBase::textureCollectionsDidChange() {
            update();
        }

        void MapViewBase::entityDefinitionsDidChange() {
            createActions();
            updateActionStates();
            update();
        }

        void MapViewBase::modsDidChange() {
            update();
        }

        void MapViewBase::editorContextDidChange() {
            update();
        }

        void MapViewBase::gridDidChange() {
            update();
        }

        void MapViewBase::pointFileDidChange() {
            update();
        }

        void MapViewBase::portalFileDidChange() {
            invalidatePortalFileRenderer();
            update();
        }

        void MapViewBase::preferenceDidChange(const IO::Path& path) {
            if(path == Preferences::RendererFontSize.path()) {
                fontManager().clearCache();
            }

            updateActionBindings();
            update();
        }

        void MapViewBase::documentDidChange(MapDocument*) {
            createActionsAndUpdatePicking();
            update();
        }

        void MapViewBase::createActions() {
            // Destroy existing QShortcuts via the weak references in m_shortcuts
            for (auto& [shortcut, action] : m_shortcuts) {
                unused(action);
                delete shortcut;
            }
            m_shortcuts.clear();

            auto visitor = [this](const Action& action) {
                const QKeySequence keySequence = action.keySequence();

                QShortcut* shortcut = new QShortcut(this);
                shortcut->setContext(Qt::WidgetWithChildrenShortcut);
                shortcut->setKey(keySequence);
                connect(shortcut, &QShortcut::activated, this, [this, &action]() { triggerAction(action); });
                connect(shortcut, &QShortcut::activatedAmbiguously, this, [this, &action]() { triggerAmbiguousAction(action.label()); });
                m_shortcuts.emplace_back(shortcut, &action);
            };

            auto& actionManager = ActionManager::instance();
            // We don't create a QShortcut for actions whose key binding is handled
            // by the menu or toolbar since they would conflict.
            actionManager.visitMapViewActions(visitor);

            auto document = kdl::mem_lock(m_document);
            document->visitTagActions(visitor);
            document->visitEntityDefinitionActions(visitor);
        }

        void MapViewBase::updateActionBindings() {
            for (auto& [shortcut, action] : m_shortcuts) {
                shortcut->setKey(action->keySequence());
            }
        }

        void MapViewBase::updateActionStates() {
            ActionExecutionContext context(findMapFrame(this), this);
            for (auto& [shortcut, action] : m_shortcuts) {
                shortcut->setEnabled(hasFocus() && action->enabled(context));
            }
        }

        void MapViewBase::updateActionStatesDelayed() {
            m_updateActionStatesSignalDelayer->queueSignal();
        }

        void MapViewBase::triggerAction(const Action& action) {
            auto* mapFrame = findMapFrame(this);
            ActionExecutionContext context(mapFrame, this);
            action.execute(context);
        }

        void MapViewBase::triggerAmbiguousAction(const QString& label) {
            qDebug() << "Ambiguous action triggered: " << label;
        }

        void MapViewBase::move(const vm::direction direction) {
            if ((actionContext() & ActionContext::RotateTool) != 0) {
                moveRotationCenter(direction);
            } else if ((actionContext() & ActionContext::AnyVertexTool) != 0) {
                moveVertices(direction);
            } else if ((actionContext() & ActionContext::NodeSelection) != 0) {
                moveObjects(direction);
            }
        }

        void MapViewBase::moveRotationCenter(const vm::direction direction) {
            auto document = kdl::mem_lock(m_document);
            const Grid& grid = document->grid();
            const vm::vec3 delta = moveDirection(direction) * static_cast<FloatType>(grid.actualSize());
            m_toolBox.moveRotationCenter(delta);
            update();
        }

        void MapViewBase::moveVertices(const vm::direction direction) {
            auto document = kdl::mem_lock(m_document);
            const Grid& grid = document->grid();
            const vm::vec3 delta = moveDirection(direction) * static_cast<FloatType>(grid.actualSize());
            m_toolBox.moveVertices(delta);
        }

        void MapViewBase::moveObjects(const vm::direction direction) {
            auto document = kdl::mem_lock(m_document);
            const Grid& grid = document->grid();
            const vm::vec3 delta = moveDirection(direction) * static_cast<FloatType>(grid.actualSize());
            document->translateObjects(delta);
        }

        vm::vec3 MapViewBase::moveDirection(const vm::direction direction) const {
            return doGetMoveDirection(direction);
        }

        void MapViewBase::duplicateObjects() {
            auto document = kdl::mem_lock(m_document);
            if (document->hasSelectedNodes()) {
                document->duplicateObjects();
            }
        }

        void MapViewBase::duplicateAndMoveObjects(const vm::direction direction) {
            Transaction transaction(m_document);
            duplicateObjects();
            moveObjects(direction);
        }

        void MapViewBase::rotateObjects(const vm::rotation_axis axisSpec, const bool clockwise) {
            auto document = kdl::mem_lock(m_document);
            if (!document->hasSelectedNodes())
                return;

            const vm::vec3 axis = rotationAxis(axisSpec, clockwise);
            const double angle = m_toolBox.rotateObjectsToolActive() ? vm::abs(m_toolBox.rotateToolAngle()) : vm::C::half_pi();

            const Grid& grid = document->grid();
            const vm::vec3 center = m_toolBox.rotateObjectsToolActive() ? m_toolBox.rotateToolCenter() : grid.referencePoint(document->selectionBounds());

            document->rotateObjects(center, axis, angle);
        }

        vm::vec3 MapViewBase::rotationAxis(const vm::rotation_axis axisSpec, const bool clockwise) const {
            vm::vec3 axis;
            switch (axisSpec) {
                case vm::rotation_axis::roll:
                    axis = -moveDirection(vm::direction::forward);
                    break;
                case vm::rotation_axis::pitch:
                    axis = moveDirection(vm::direction::right);
                    break;
                case vm::rotation_axis::yaw:
                    axis = moveDirection(vm::direction::up);
                    break;
                    switchDefault()
            }

            if (clockwise)
                axis = -axis;
            return axis;
        }

        void MapViewBase::flipObjects(const vm::direction direction) {
            if (canFlipObjects()) {
                auto document = kdl::mem_lock(m_document);

                // If we snap the selection bounds' center to the grid size, then
                // selections that are an odd number of grid units wide get translated.
                // Instead, snap to 1/2 the grid size.
                // (see: https://github.com/TrenchBroom/TrenchBroom/issues/1495 )
                Grid halfGrid(document->grid().size());
                halfGrid.decSize();

                const auto center = halfGrid.referencePoint(document->selectionBounds());
                const size_t axis = doGetFlipAxis(direction);

                document->flipObjects(center, axis);
            }
        }

        bool MapViewBase::canFlipObjects() const {
            auto document = kdl::mem_lock(m_document);
            return !m_toolBox.anyToolActive() && document->hasSelectedNodes();
        }

        void MapViewBase::moveTextures(const vm::direction direction, const TextureActionMode mode) {
            auto document = kdl::mem_lock(m_document);
            if (document->hasSelectedBrushFaces()) {
                const auto offset = moveTextureOffset(direction, mode);
                document->moveTextures(doGetCamera().up(), doGetCamera().right(), offset);
            }
        }

        vm::vec2f MapViewBase::moveTextureOffset(const vm::direction direction, const TextureActionMode mode) const {
            switch (direction) {
                case vm::direction::up:
                    return vm::vec2f(0.0f, moveTextureDistance(mode));
                case vm::direction::down:
                    return vm::vec2f(0.0f, -moveTextureDistance(mode));
                case vm::direction::left:
                    return vm::vec2f(-moveTextureDistance(mode), 0.0f);
                case vm::direction::right:
                    return vm::vec2f(moveTextureDistance(mode), 0.0f);
                case vm::direction::forward:
                case vm::direction::backward:
                    return vm::vec2f();
                switchDefault()
            }
        }

        float MapViewBase::moveTextureDistance(const TextureActionMode mode) const {
            const auto& grid = kdl::mem_lock(m_document)->grid();
            const auto gridSize = static_cast<float>(grid.actualSize());

            switch (mode) {
                case TextureActionMode::Fine:
                    return 1.0f;
                case TextureActionMode::Coarse:
                    return 2.0f * gridSize;
                case TextureActionMode::Normal:
                    return gridSize;
                switchDefault();
            }
        }

        void MapViewBase::rotateTextures(const bool clockwise, const TextureActionMode mode) {
            auto document = kdl::mem_lock(m_document);
            if (document->hasSelectedBrushFaces()) {
                const auto angle = rotateTextureAngle(clockwise, mode);
                document->rotateTextures(angle);
            }
        }

        float MapViewBase::rotateTextureAngle(const bool clockwise, const TextureActionMode mode) const {
            const auto& grid = kdl::mem_lock(m_document)->grid();
            const auto gridAngle = static_cast<float>(vm::to_degrees(grid.angle()));
            float angle = 0.0f;

            switch (mode) {
                case TextureActionMode::Fine:
                    angle = 1.0f;
                    break;
                case TextureActionMode::Coarse:
                    angle = 90.0f;
                    break;
                case TextureActionMode::Normal:
                    angle = gridAngle;
                    break;
            }
            return clockwise ? angle : -angle;
        }

        void MapViewBase::flipTextures(const vm::direction direction) {
            auto document = kdl::mem_lock(m_document);
            if (document->hasSelectedBrushFaces()) {
                document->flipTextures(doGetCamera().up(), doGetCamera().right(), direction);
            }
        }

        void MapViewBase::resetTextures() {
            Model::ChangeBrushFaceAttributesRequest request;

            auto document = kdl::mem_lock(m_document);
            request.resetAll(document->game()->defaultFaceAttribs());
            document->setFaceAttributes(request);
        }

        void MapViewBase::resetTexturesToWorld() {
            Model::ChangeBrushFaceAttributesRequest request;

            auto document = kdl::mem_lock(m_document);
            request.resetAllToParaxial(document->game()->defaultFaceAttribs());
            document->setFaceAttributes(request);
        }

        void MapViewBase::createComplexBrush() {
            if (m_toolBox.createComplexBrushToolActive()) {
                m_toolBox.performCreateComplexBrush();
            }
        }

        void MapViewBase::toggleClipSide() {
            m_toolBox.toggleClipSide();
        }

        void MapViewBase::performClip() {
            m_toolBox.performClip();
        }

        void MapViewBase::resetCameraZoom() {
            doGetCamera().setZoom(1.0f);
        }

        void MapViewBase::cancel() {
            if (doCancel()) {
                return;
            }
            if (ToolBoxConnector::cancel()) {
                return;
            }

            auto document = kdl::mem_lock(m_document);
            if (document->hasSelection()) {
                document->deselectAll();
            } else if (document->currentGroup() != nullptr) {
                document->closeGroup();
            }
        }

        void MapViewBase::deactivateTool() {
            m_toolBox.deactivateAllTools();
        }

        void MapViewBase::createPointEntity() {
            auto* action = qobject_cast<const QAction*>(sender());
            auto document = kdl::mem_lock(m_document);
            const size_t index = action->data().toUInt();
            const Assets::EntityDefinition* definition = findEntityDefinition(Assets::EntityDefinitionType::PointEntity, index);
            ensure(definition != nullptr, "definition is null");
            assert(definition->type() == Assets::EntityDefinitionType::PointEntity);
            createPointEntity(static_cast<const Assets::PointEntityDefinition*>(definition));
        }

        void MapViewBase::createBrushEntity() {
            auto* action = qobject_cast<const QAction*>(sender());
            auto document = kdl::mem_lock(m_document);
            const size_t index = action->data().toUInt();
            const Assets::EntityDefinition* definition = findEntityDefinition(Assets::EntityDefinitionType::BrushEntity, index);
            ensure(definition != nullptr, "definition is null");
            assert(definition->type() == Assets::EntityDefinitionType::BrushEntity);
            createBrushEntity(static_cast<const Assets::BrushEntityDefinition*>(definition));
        }

        Assets::EntityDefinition* MapViewBase::findEntityDefinition(const Assets::EntityDefinitionType type, const size_t index) const {
            size_t count = 0;
            for (const Assets::EntityDefinitionGroup& group : kdl::mem_lock(m_document)->entityDefinitionManager().groups()) {
                const std::vector<Assets::EntityDefinition*> definitions = group.definitions(type, Assets::EntityDefinitionSortOrder::Name);
                if (index < count + definitions.size())
                    return definitions[index - count];
                count += definitions.size();
            }
            return nullptr;
        }

        void MapViewBase::createPointEntity(const Assets::PointEntityDefinition* definition) {
            ensure(definition != nullptr, "definition is null");

            auto document = kdl::mem_lock(m_document);
            const auto delta = doComputePointEntityPosition(definition->bounds());
            document->createPointEntity(definition, delta);
        }

        void MapViewBase::createBrushEntity(const Assets::BrushEntityDefinition* definition) {
            ensure(definition != nullptr, "definition is null");

            auto document = kdl::mem_lock(m_document);
            document->createBrushEntity(definition);
        }

        bool MapViewBase::canCreateBrushEntity() {
            auto document = kdl::mem_lock(m_document);
            return document->selectedNodes().hasOnlyBrushes();
        }

        void MapViewBase::toggleTagVisible(const Model::SmartTag& tag) {
            const auto tagIndex = tag.index();

            auto document = kdl::mem_lock(m_document);
            auto& editorContext = document->editorContext();
            auto hiddenTags = editorContext.hiddenTags();
            hiddenTags ^= Model::TagType::Type(1) << tagIndex;
            editorContext.setHiddenTags(hiddenTags);
        }


        void MapViewBase::enableTag(const Model::SmartTag& tag) {
            assert(tag.canEnable());
            auto document = kdl::mem_lock(m_document);

            Transaction transaction(document, "Turn Selection into " + tag.name());
            EnableDisableTagCallback callback;
            tag.enable(callback, *document);
        }

        void MapViewBase::disableTag(const Model::SmartTag& tag) {
            assert(tag.canDisable());
            auto document = kdl::mem_lock(m_document);
            Transaction transaction(document, "Turn Selection into non-" + tag.name());
            EnableDisableTagCallback callback;
            tag.disable(callback, *document);
        }

        void MapViewBase::makeStructural() {
            auto document = kdl::mem_lock(m_document);
            if (!document->selectedNodes().hasBrushes()) {
                return;
            }

            Transaction transaction(document, "Make Structural");
            std::vector<Model::Node*> toReparent;
            for (auto* brush : document->selectedNodes().brushes()) {
                if (brush->entity() != document->world()) {
                    toReparent.push_back(brush);
                }
            }

            if (!toReparent.empty()) {
                reparentNodes(toReparent, document->parentForNodes(toReparent), false);
            }

            bool anyTagDisabled = false;
            EnableDisableTagCallback callback;
            for (auto* brush : document->selectedNodes().brushes()) {
                for (const auto& tag : document->smartTags()) {
                    if (brush->hasTag(tag) || brush->anyFacesHaveAnyTagInMask(tag.type())) {
                        anyTagDisabled = true;
                        tag.disable(callback, *document);
                    }
                }
            }

            if (!anyTagDisabled && toReparent.empty()) {
                transaction.cancel();
            }
        }

        void MapViewBase::toggleEntityDefinitionVisible(const Assets::EntityDefinition* definition) {
            auto document = kdl::mem_lock(m_document);

            Model::EditorContext& editorContext = document->editorContext();
            editorContext.setEntityDefinitionHidden(definition, !editorContext.entityDefinitionHidden(definition));
        }

        void MapViewBase::createEntity(const Assets::EntityDefinition* definition) {
            auto document = kdl::mem_lock(m_document);
            if (definition->type() == Assets::EntityDefinitionType::PointEntity) {
                createPointEntity(static_cast<const Assets::PointEntityDefinition*>(definition));
            } else if (canCreateBrushEntity()) {
                createBrushEntity(static_cast<const Assets::BrushEntityDefinition*>(definition));
            }
        }

        void MapViewBase::toggleShowEntityClassnames() {
            togglePref(Preferences::ShowEntityClassnames);
        }

        void MapViewBase::toggleShowGroupBounds() {
            togglePref(Preferences::ShowGroupBounds);
        }

        void MapViewBase::toggleShowBrushEntityBounds() {
            togglePref(Preferences::ShowBrushEntityBounds);
        }

        void MapViewBase::toggleShowPointEntityBounds() {
            togglePref(Preferences::ShowPointEntityBounds);
        }

        void MapViewBase::toggleShowPointEntities() {
            togglePref(Preferences::ShowPointEntities);
        }

        void MapViewBase::toggleShowPointEntityModels() {
            togglePref(Preferences::ShowPointEntityModels);
        }

        void MapViewBase::toggleShowBrushes() {
            togglePref(Preferences::ShowBrushes);
        }

        void MapViewBase::showTextures() {
            setPref(Preferences::FaceRenderMode, Preferences::faceRenderModeTextured());
        }

        void MapViewBase::hideTextures() {
            setPref(Preferences::FaceRenderMode, Preferences::faceRenderModeFlat());
        }

        void MapViewBase::hideFaces() {
            setPref(Preferences::FaceRenderMode, Preferences::faceRenderModeSkip());
        }

        void MapViewBase::toggleShadeFaces() {
            togglePref(Preferences::ShadeFaces);
        }

        void MapViewBase::toggleShowFog() {
            togglePref(Preferences::ShowFog);
        }

        void MapViewBase::toggleShowEdges() {
            togglePref(Preferences::ShowEdges);
        }

        void MapViewBase::showAllEntityLinks() {
            setPref(Preferences::FaceRenderMode, Preferences::entityLinkModeAll());
        }

        void MapViewBase::showTransitivelySelectedEntityLinks() {
            setPref(Preferences::FaceRenderMode, Preferences::entityLinkModeTransitive());
        }

        void MapViewBase::showDirectlySelectedEntityLinks() {
            setPref(Preferences::FaceRenderMode, Preferences::entityLinkModeDirect());
        }

        void MapViewBase::hideAllEntityLinks() {
            setPref(Preferences::FaceRenderMode, Preferences::entityLinkModeNone());
        }

        void MapViewBase::focusInEvent(QFocusEvent* event) {
            updateActionStates();  // enable/disable QShortcut's to reflect whether we have focus (needed because of QOpenGLWindow; see comment in createAndRegisterShortcut)
            updateModifierKeys();
            update();
            RenderView::focusInEvent(event);
        }

        void MapViewBase::focusOutEvent(QFocusEvent* event) {
            clearModifierKeys();
            update();
            RenderView::focusOutEvent(event);
        }

        ActionContext::Type MapViewBase::actionContext() const {
            const auto derivedContext = doGetActionContext();
            if (m_toolBox.createComplexBrushToolActive()) {
                return derivedContext | ActionContext::CreateComplexBrushTool;
            } else if (m_toolBox.clipToolActive()) {
                return derivedContext | ActionContext::ClipTool;
            } else if (m_toolBox.anyVertexToolActive()) {
                return derivedContext | ActionContext::AnyVertexTool;
            } else if (m_toolBox.rotateObjectsToolActive()) {
                return derivedContext | ActionContext::RotateTool;
            } else if (m_toolBox.scaleObjectsToolActive()) {
                return derivedContext | ActionContext::ScaleTool;
            } else if (m_toolBox.shearObjectsToolActive()) {
                return derivedContext | ActionContext::ShearTool;
            } else {
                auto document = kdl::mem_lock(m_document);
                if (document->hasSelectedNodes()) {
                    return derivedContext | ActionContext::NodeSelection;
                } else if (document->hasSelectedBrushFaces()) {
                    return derivedContext | ActionContext::FaceSelection;
                } else {
                    return derivedContext;
                }
            }
        }

        void MapViewBase::doFlashSelection() {
            auto animation = std::make_unique<FlashSelectionAnimation>(m_renderer, this, 180);
            m_animationManager->runAnimation(std::move(animation), true);
        }

        void MapViewBase::doInstallActivationTracker(MapViewActivationTracker& activationTracker) {
            activationTracker.addWindow(this);
        }

        bool MapViewBase::doGetIsCurrent() const {
            return m_isCurrent;
        }

        MapViewBase* MapViewBase::doGetFirstMapViewBase() {
            return this;
        }

        bool MapViewBase::doCancelMouseDrag() {
            return ToolBoxConnector::cancelDrag();
        }

        void MapViewBase::doRefreshViews() {
            update();
        }

        void MapViewBase::initializeGL() {
            if (doInitializeGL()) {
                m_logger->info() << "Renderer info: " << GLContextManager::GLRenderer << " version " << GLContextManager::GLVersion << " from " << GLContextManager::GLVendor;
                m_logger->info() << "Depth buffer bits: " << depthBits();
                m_logger->info() << "Multisampling " << kdl::str_select(multisample(), "enabled", "disabled");
            }
        }

        bool MapViewBase::doShouldRenderFocusIndicator() const {
            return true;
        }

        void MapViewBase::doRender() {
            doPreRender();

            const IO::Path& fontPath = pref(Preferences::RendererFontPath());
            const size_t fontSize = static_cast<size_t>(pref(Preferences::RendererFontSize));
            const Renderer::FontDescriptor fontDescriptor(fontPath, fontSize);

            auto document = kdl::mem_lock(m_document);
            const Grid& grid = document->grid();

            Renderer::RenderContext renderContext(doGetRenderMode(), doGetCamera(), fontManager(), shaderManager());
            renderContext.setShowTextures(pref(Preferences::FaceRenderMode) == Preferences::faceRenderModeTextured());
            renderContext.setShowFaces(pref(Preferences::FaceRenderMode) != Preferences::faceRenderModeSkip());
            renderContext.setShowEdges(pref(Preferences::ShowEdges));
            renderContext.setShadeFaces(pref(Preferences::ShadeFaces));
            renderContext.setShowPointEntities(pref(Preferences::ShowPointEntities));
            renderContext.setShowPointEntityModels(pref(Preferences::ShowPointEntityModels));
            renderContext.setShowEntityClassnames(pref(Preferences::ShowEntityClassnames));
            renderContext.setShowGroupBounds(pref(Preferences::ShowGroupBounds));
            renderContext.setShowBrushEntityBounds(pref(Preferences::ShowBrushEntityBounds));
            renderContext.setShowPointEntityBounds(pref(Preferences::ShowPointEntityBounds));
            renderContext.setShowFog(pref(Preferences::ShowFog));
            renderContext.setShowGrid(grid.visible());
            renderContext.setGridSize(grid.actualSize());
            renderContext.setSoftMapBounds(pref(Preferences::ShowSoftMapBounds)
                ? vm::bbox3f(document->softMapBounds().bounds.value_or(vm::bbox3()))
                : vm::bbox3f());

            setupGL(renderContext);
            setRenderOptions(renderContext);

            Renderer::RenderBatch renderBatch(vboManager());

            doRenderGrid(renderContext, renderBatch);
            doRenderMap(m_renderer, renderContext, renderBatch);
            doRenderTools(m_toolBox, renderContext, renderBatch);
            doRenderExtras(renderContext, renderBatch);

            renderCoordinateSystem(renderContext, renderBatch);
            renderSoftMapBounds(renderContext, renderBatch);
            renderPointFile(renderContext, renderBatch);
            renderPortalFile(renderContext, renderBatch);
            renderCompass(renderBatch);
            renderFPS(renderContext, renderBatch);

            renderBatch.render(renderContext);
        }

        void MapViewBase::setupGL(Renderer::RenderContext& context) {
            const Renderer::Camera::Viewport& viewport = context.camera().viewport();
            const qreal r = devicePixelRatioF();
            const int x = static_cast<int>(viewport.x * r);
            const int y = static_cast<int>(viewport.y * r);
            const int width = static_cast<int>(viewport.width * r);
            const int height = static_cast<int>(viewport.height * r);
            glAssert(glViewport(x, y, width, height))

            if (pref(Preferences::EnableMSAA)) {
                glAssert(glEnable(GL_MULTISAMPLE))
            } else {
                glAssert(glDisable(GL_MULTISAMPLE))
            }
            glAssert(glEnable(GL_BLEND))
            glAssert(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA))
            glAssert(glShadeModel(GL_SMOOTH))
        }

        void MapViewBase::renderCoordinateSystem(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            if (pref(Preferences::ShowAxes)) {
                auto document = kdl::mem_lock(m_document);
                const vm::bbox3& worldBounds = document->worldBounds();

                Renderer::RenderService renderService(renderContext, renderBatch);
                renderService.renderCoordinateSystem(vm::bbox3f(worldBounds));
            }
        }

        void MapViewBase::renderSoftMapBounds(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            doRenderSoftWorldBounds(renderContext, renderBatch);
        }

        void MapViewBase::renderPointFile(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            auto document = kdl::mem_lock(m_document);
            Model::PointFile* pointFile = document->pointFile();
            if (pointFile != nullptr) {
                Renderer::RenderService renderService(renderContext, renderBatch);
                renderService.setForegroundColor(pref(Preferences::PointFileColor));
                renderService.renderLineStrip(pointFile->points());
            }
        }

        void MapViewBase::renderPortalFile(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            if (m_portalFileRenderer == nullptr) {
                validatePortalFileRenderer(renderContext);
                assert(m_portalFileRenderer != nullptr);
            }
            renderBatch.add(m_portalFileRenderer.get());
        }

        void MapViewBase::invalidatePortalFileRenderer() {
            m_portalFileRenderer = nullptr;
        }

        void MapViewBase::validatePortalFileRenderer(Renderer::RenderContext&) {
            assert(m_portalFileRenderer == nullptr);
            m_portalFileRenderer = std::make_unique<Renderer::PrimitiveRenderer>();

            auto document = kdl::mem_lock(m_document);
            Model::PortalFile* portalFile = document->portalFile();
            if (portalFile != nullptr) {
                for (const auto& poly : portalFile->portals()) {
                    m_portalFileRenderer->renderFilledPolygon(pref(Preferences::PortalFileFillColor),
                                                              Renderer::PrimitiveRendererOcclusionPolicy::Hide,
                                                              Renderer::PrimitiveRendererCullingPolicy::ShowBackfaces,
                                                              poly.vertices());

                    const auto lineWidth = 4.0f;
                    m_portalFileRenderer->renderPolygon(pref(Preferences::PortalFileBorderColor),
                                                        lineWidth,
                                                        Renderer::PrimitiveRendererOcclusionPolicy::Hide,
                                                        poly.vertices());
                }
            }
        }

        void MapViewBase::renderCompass(Renderer::RenderBatch& renderBatch) {
            if (m_compass != nullptr) {
                m_compass->render(renderBatch);
            }
        }

        void MapViewBase::renderFPS(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            if (pref(Preferences::ShowFPS)) {
                Renderer::RenderService renderService(renderContext, renderBatch);

                renderService.renderHeadsUp(m_currentFPS);
            }
        }

        void MapViewBase::processEvent(const KeyEvent& event) {
            ToolBoxConnector::processEvent(event);
        }

        void MapViewBase::processEvent(const MouseEvent& event) {
            ToolBoxConnector::processEvent(event);
        }

        void MapViewBase::processEvent(const CancelEvent& event) {
            ToolBoxConnector::processEvent(event);
        }

        void MapViewBase::doShowPopupMenu() {
            // We process input events during paint event processing, but we cannot show a popup menu
            // during paint processing, so we enqueue an event for later.
            QMetaObject::invokeMethod(this, "showPopupMenuLater", Qt::QueuedConnection);
        }

        void MapViewBase::showPopupMenuLater() {
            if (!doBeforePopupMenu()) {
                return;
            }

            auto document = kdl::mem_lock(m_document);
            const std::vector<Model::Node*>& nodes = document->selectedNodes().nodes();
            Model::Node* newBrushParent = findNewParentEntityForBrushes(nodes);
            Model::Node* currentGroup = document->editorContext().currentGroup();
            Model::Node* newGroup = findNewGroupForObjects(nodes);
            Model::Node* mergeGroup = findGroupToMergeGroupsInto(document->selectedNodes());

            auto* mapFrame = findMapFrame(this);

            QMenu menu;
            const auto addMainMenuAction = [&](const auto& path) -> QAction* {
                auto* groupAction = mapFrame->findAction(path);
                assert(groupAction != nullptr);
                menu.addAction(groupAction);
                return groupAction;
            };

            addMainMenuAction(IO::Path("Menu/Edit/Group"));
            addMainMenuAction(IO::Path("Menu/Edit/Ungroup"));

            QAction* mergeGroupAction = nullptr;
            if (mergeGroup != nullptr) {
                mergeGroupAction = menu.addAction(tr("Merge Groups into %1").arg(QString::fromStdString(mergeGroup->name())), this,
                    &MapViewBase::mergeSelectedGroups);
            } else {
                mergeGroupAction = menu.addAction(tr("Merge Groups"), this, &MapViewBase::mergeSelectedGroups);
            }
            mergeGroupAction->setEnabled(canMergeGroups());

            QAction* renameAction = menu.addAction(tr("Rename Groups"), mapFrame, &MapFrame::renameSelectedGroups);
            renameAction->setEnabled(mapFrame->canRenameSelectedGroups());

            if (newGroup != nullptr && newGroup != currentGroup) {
                menu.addAction(tr("Add Objects to Group %1").arg(QString::fromStdString(newGroup->name())), this,
                    &MapViewBase::addSelectedObjectsToGroup);
            }
            if (currentGroup != nullptr && !document->selectedNodes().empty()) {
                menu.addAction(tr("Remove Objects from Group %1").arg(QString::fromStdString(currentGroup->name())), this,
                    &MapViewBase::removeSelectedObjectsFromGroup);
            }
            menu.addSeparator();

            // Linked group operations

            addMainMenuAction(IO::Path("Menu/Edit/Create Linked Duplicate"));
            addMainMenuAction(IO::Path("Menu/Edit/Select Linked Groups"));
            addMainMenuAction(IO::Path("Menu/Edit/Separate Linked Groups"));
            menu.addSeparator();

            // Layer operations

            const std::vector<Model::LayerNode*> selectedObjectLayers = Model::findContainingLayersUserSorted(nodes);

            QMenu* moveSelectionTo = menu.addMenu(tr("Move to Layer"));
            for (Model::LayerNode* layer : document->world()->allLayersUserSorted()) {
                QAction* action = moveSelectionTo->addAction(QString::fromStdString(layer->name()), this, [=](){
                    document->moveSelectionToLayer(layer);
                });
                action->setEnabled(document->canMoveSelectionToLayer(layer));
            }

            const auto moveSelectionToItems = moveSelectionTo->actions();
            moveSelectionTo->setEnabled(std::any_of(std::begin(moveSelectionToItems), std::end(moveSelectionToItems), [](QAction *action) { return action->isEnabled(); }));

            if (selectedObjectLayers.size() == 1u) {
                Model::LayerNode* layer = selectedObjectLayers[0];
                QAction* action = menu.addAction(tr("Make Layer %1 Active").arg(QString::fromStdString(layer->name())), this, [=](){
                    document->setCurrentLayer(layer);
                });
                action->setEnabled(document->canSetCurrentLayer(layer));
            } else {
                QMenu* makeLayerActive = menu.addMenu(tr("Make Layer Active"));
                for (Model::LayerNode* layer : selectedObjectLayers) {
                    QAction* action = makeLayerActive->addAction(QString::fromStdString(layer->name()), this, [=](){
                        document->setCurrentLayer(layer);
                    });
                    action->setEnabled(document->canSetCurrentLayer(layer));
                }
                if (makeLayerActive->isEmpty()) {
                    makeLayerActive->setDisabled(true);
                }
            }

            QAction* hideLayersAction = menu.addAction(tr("Hide Layers"), this, [=](){
                document->hideLayers(selectedObjectLayers);
            });
            hideLayersAction->setEnabled(document->canHideLayers(selectedObjectLayers));
            QAction* isolateLayersAction = menu.addAction(tr("Isolate Layers"), this, [=](){
                document->isolateLayers(selectedObjectLayers);
            });
            isolateLayersAction->setEnabled(document->canIsolateLayers(selectedObjectLayers));
            QAction* selectAllInLayersAction = menu.addAction(tr("Select All in Layers"), this, [=](){
                document->selectAllInLayers(selectedObjectLayers);
            });
            selectAllInLayersAction->setEnabled(document->canSelectAllInLayers(selectedObjectLayers));

            menu.addSeparator();

            if (document->selectedNodes().hasOnlyBrushes()) {
                QAction* moveToWorldAction = menu.addAction(tr("Make Structural"), this, &MapViewBase::makeStructural);
                moveToWorldAction->setEnabled(canMakeStructural());

                const auto isEntity = newBrushParent->accept(kdl::overload(
                    [](const Model::WorldNode*)  { return false; },
                    [](const Model::LayerNode*)  { return false; },
                    [](const Model::GroupNode*)  { return false; },
                    [](const Model::EntityNode*) { return true; },
                    [](const Model::BrushNode*)  { return false; },
                    [](const Model::PatchNode*)  { return false; }
                ));

                if (isEntity) {
                    menu.addAction(tr("Move Brushes to Entity %1").arg(QString::fromStdString(newBrushParent->name())), this,
                        &MapViewBase::moveSelectedBrushesToEntity);
                }
            }

            menu.addSeparator();

            using namespace Model::HitFilters;
            const Model::Hit& hit = pickResult().first(type(Model::BrushNode::BrushHitType));
            const auto faceHandle = Model::hitToFaceHandle(hit);
            if (faceHandle) {
                const Assets::Texture* texture = faceHandle->face().texture();
                menu.addAction(tr("Reveal %1 in Texture Browser").arg(QString::fromStdString(faceHandle->face().attributes().textureName())), mapFrame, [=](){
                    mapFrame->revealTexture(texture);
                });

                menu.addSeparator();
            }

            menu.addMenu(makeEntityGroupsMenu(Assets::EntityDefinitionType::PointEntity));
            menu.addMenu(makeEntityGroupsMenu(Assets::EntityDefinitionType::BrushEntity));

            menu.exec(QCursor::pos());

            // Generate a synthetic mouse move event to update the mouse position after the popup menu closes.
            const auto screenPos = QCursor::pos();
            const auto windowPos = window()->mapFromGlobal(screenPos);
            const auto localPos  = mapFromGlobal(screenPos);
            auto mouseEvent = QMouseEvent(QEvent::MouseMove, localPos, windowPos, screenPos, Qt::NoButton, Qt::NoButton, Qt::NoModifier, Qt::MouseEventSynthesizedByApplication);
            mouseMoveEvent(&mouseEvent);

            doAfterPopupMenu();
        }

        /**
         * Forward drag and drop events from QWidget to ToolBoxConnector
         */
        void MapViewBase::dragEnterEvent(QDragEnterEvent* dragEnterEvent) {
            dragEnter(static_cast<float>(dragEnterEvent->posF().x()), static_cast<float>(dragEnterEvent->posF().y()), dragEnterEvent->mimeData()->text().toStdString());
            dragEnterEvent->acceptProposedAction();
        }

        void MapViewBase::dragLeaveEvent(QDragLeaveEvent*) {
            dragLeave();
        }

        void MapViewBase::dragMoveEvent(QDragMoveEvent* dragMoveEvent) {
            dragMove(static_cast<float>(dragMoveEvent->posF().x()), static_cast<float>(dragMoveEvent->posF().y()), dragMoveEvent->mimeData()->text().toStdString());
            dragMoveEvent->acceptProposedAction();
        }

        void MapViewBase::dropEvent(QDropEvent* dropEvent) {
            dragDrop(static_cast<float>(dropEvent->posF().x()), static_cast<float>(dropEvent->posF().y()), dropEvent->mimeData()->text().toStdString());
            dropEvent->acceptProposedAction();
        }

        QMenu* MapViewBase::makeEntityGroupsMenu(const Assets::EntityDefinitionType type) {
            auto* menu = new QMenu();

            switch (type) {
                case Assets::EntityDefinitionType::PointEntity:
                    menu->setTitle(tr("Create Point Entity"));
                    break;
                case Assets::EntityDefinitionType::BrushEntity:
                    menu->setTitle(tr("Create Brush Entity"));
                    break;
            }

            const bool enableMakeBrushEntity = canCreateBrushEntity();
            size_t id = 0;

            auto document = kdl::mem_lock(m_document);
            for (const Assets::EntityDefinitionGroup& group : document->entityDefinitionManager().groups()) {
                const std::vector<Assets::EntityDefinition*> definitions = group.definitions(type, Assets::EntityDefinitionSortOrder::Name);

                std::vector<Assets::EntityDefinition*> filteredDefinitions;
                for (auto* definition : definitions) {
                    if (!kdl::cs::str_is_equal(definition->name(), Model::PropertyValues::WorldspawnClassname)) {
                        filteredDefinitions.push_back(definition);
                    }
                }

                if (!filteredDefinitions.empty()) {
                    const auto groupName = QString::fromStdString(group.displayName());
                    auto* groupMenu = new QMenu(groupName);

                    for (Assets::EntityDefinition* definition : filteredDefinitions) {
                        const auto label = QString::fromStdString(definition->shortName());
                        QAction *action = nullptr;

                        switch (type) {
                            case Assets::EntityDefinitionType::PointEntity: {
                                action = groupMenu->addAction(label, this, qOverload<>(&MapViewBase::createPointEntity));
                                break;
                            }
                            case Assets::EntityDefinitionType::BrushEntity: {
                                action = groupMenu->addAction(label, this, qOverload<>(&MapViewBase::createBrushEntity));
                                action->setEnabled(enableMakeBrushEntity);
                                break;
                            }
                        }

                        // TODO: Would be cleaner to pass this as the string entity name
                        action->setData(QVariant::fromValue<size_t>(id++));
                    }

                    menu->addMenu(groupMenu);
                }
            }

            return menu;
        }

        void MapViewBase::addSelectedObjectsToGroup() {
            auto document = kdl::mem_lock(m_document);
            const std::vector<Model::Node*> nodes = document->selectedNodes().nodes();
            Model::Node* newGroup = findNewGroupForObjects(nodes);
            ensure(newGroup != nullptr, "newGroup is null");

            Transaction transaction(document, "Add Objects to Group");
            reparentNodes(nodes, newGroup, true);
            document->deselectAll();
            document->select(newGroup);
        }

        void MapViewBase::removeSelectedObjectsFromGroup() {
            auto document = kdl::mem_lock(m_document);
            const std::vector<Model::Node*> nodes = document->selectedNodes().nodes();
            Model::Node* currentGroup = document->editorContext().currentGroup();
            ensure(currentGroup != nullptr, "currentGroup is null");

            Transaction transaction(document, "Remove Objects from Group");
            reparentNodes(nodes, document->currentLayer(), true);

            while (document->currentGroup() != nullptr) {
                document->closeGroup();
            }
            document->select(nodes);
        }

        Model::Node* MapViewBase::findNewGroupForObjects(const std::vector<Model::Node*>& nodes) const {
            using namespace Model::HitFilters;

            Model::Node* newGroup = nullptr;

            auto document = kdl::mem_lock(m_document);
            const auto hits = pickResult().all(type(Model::nodeHitType()));
            if (!hits.empty()) {
                newGroup = Model::findOutermostClosedGroup(Model::hitToNode(hits.front()));
            }
            if (newGroup != nullptr && canReparentNodes(nodes, newGroup))
                return newGroup;
            return nullptr;
        }

        void MapViewBase::mergeSelectedGroups() {
            auto document = kdl::mem_lock(m_document);
            auto* newGroup = findGroupToMergeGroupsInto(document->selectedNodes());
            ensure(newGroup != nullptr, "newGroup is null");

            Transaction transaction(document, "Merge Groups");
            document->mergeSelectedGroupsWithGroup(newGroup);
        }

        Model::GroupNode* MapViewBase::findGroupToMergeGroupsInto(const Model::NodeCollection& selectedNodes) const {
            using namespace Model::HitFilters;

            if (!(selectedNodes.hasOnlyGroups() && selectedNodes.groupCount() >= 2)) {
                return nullptr;
            }

            Model::GroupNode* mergeTarget = nullptr;

            auto document = kdl::mem_lock(m_document);
            const auto hits = pickResult().all(type(Model::nodeHitType()));
            if (!hits.empty()) {
                mergeTarget = findOutermostClosedGroup(Model::hitToNode(hits.front()));
            }
            if (mergeTarget == nullptr) {
                return nullptr;
            }

            bool canReparentAll = true;
            for (const auto* node : selectedNodes.nodes()) {
                canReparentAll &= node == mergeTarget || canReparentNode(node, mergeTarget);
                if (!canReparentAll) {
                    break;
                }
            }

            if (canReparentAll) {
                return mergeTarget;
            } else {
                return nullptr;
            }
        }

        bool MapViewBase::canReparentNode(const Model::Node* node, const Model::Node* newParent) const {
            return newParent != node && newParent != node->parent() && !newParent->isDescendantOf(node);
        }

        void MapViewBase::moveSelectedBrushesToEntity() {
            auto document = kdl::mem_lock(m_document);
            const std::vector<Model::Node*> nodes = document->selectedNodes().nodes();
            Model::Node* newParent = findNewParentEntityForBrushes(nodes);
            ensure(newParent != nullptr, "newParent is null");

            const Transaction transaction(document, "Move " + kdl::str_plural(nodes.size(), "Brush", "Brushes"));
            reparentNodes(nodes, newParent, false);

            document->deselectAll();
            document->select(nodes);
        }

        Model::Node* MapViewBase::findNewParentEntityForBrushes(const std::vector<Model::Node*>& nodes) const {
            using namespace Model::HitFilters;

            Model::Node* newParent = nullptr;

            auto document = kdl::mem_lock(m_document);
            const Model::Hit& hit = pickResult().first(type(Model::BrushNode::BrushHitType));
            if (const auto faceHandle = Model::hitToFaceHandle(hit)) {
                Model::BrushNode* brush = faceHandle->node();
                newParent = brush->entity();
            }

            if (newParent != nullptr && newParent != document->world() && canReparentNodes(nodes, newParent)) {
                return newParent;
            }

            if (!nodes.empty()) {
                Model::Node* lastNode = nodes.back();

                Model::GroupNode* group = Model::findContainingGroup(lastNode);
                if (group != nullptr) {
                    return group;
                }

                Model::LayerNode* layer = Model::findContainingLayer(lastNode);
                if (layer != nullptr) {
                    return layer;
            }
            }

            return document->currentLayer();
        }

        bool MapViewBase::canReparentNodes(const std::vector<Model::Node*>& nodes, const Model::Node* newParent) const {
            for (const Model::Node* node : nodes) {
                if (canReparentNode(node, newParent)) {
                    return true;
                }
            }
            return false;
        }

        /**
         * Return the given nodes, but replace all entity brushes with the parent entity (with duplicates removed).
         */
        static std::vector<Model::Node*> collectEntitiesForNodes(const std::vector<Model::Node*>& selectedNodes, const Model::WorldNode* world) {
            std::vector<Model::Node*> result;
            const auto addNode = [&](auto&& thisLambda, auto* node) {
                if (node->entity() == world) {
                    result.push_back(node);
                } else {
                    node->visitParent(thisLambda);
                }
            };
            Model::Node::visitAll(selectedNodes, kdl::overload(
                [] (Model::WorldNode*)         {},
                [] (Model::LayerNode*)         {},
                [&](Model::GroupNode* group)   { result.push_back(group); },
                [&](Model::EntityNode* entity) { result.push_back(entity); },
                [&](auto&& thisLambda, Model::BrushNode* brush) {
                    addNode(thisLambda, brush);
                },
                [&](auto&& thisLambda, Model::PatchNode* patch) {
                    addNode(thisLambda, patch);
                }
            ));
            return kdl::vec_sort_and_remove_duplicates(std::move(result));
        }

        void MapViewBase::reparentNodes(const std::vector<Model::Node*>& nodes, Model::Node* newParent, const bool preserveEntities) {
            ensure(newParent != nullptr, "newParent is null");

            auto document = kdl::mem_lock(m_document);
            std::vector<Model::Node*> inputNodes;
            if (preserveEntities) {
                inputNodes = collectEntitiesForNodes(nodes, document->world());
            } else {
                inputNodes = nodes;
            }

            const std::vector<Model::Node*> reparentableNodes = collectReparentableNodes(inputNodes, newParent);
            assert(!reparentableNodes.empty());

            std::stringstream name;
            name << "Move " << (reparentableNodes.size() == 1 ? "Object" : "Objects") << " to " << newParent->name();

            const Transaction transaction(document, name.str());
            document->deselectAll();
            document->reparentNodes({{newParent, reparentableNodes}});
            document->select(reparentableNodes);
        }

        std::vector<Model::Node*> MapViewBase::collectReparentableNodes(const std::vector<Model::Node*>& nodes, const Model::Node* newParent) const {
            std::vector<Model::Node*> result;
            for (auto* node : nodes) {
                if (newParent != node && newParent != node->parent() && !newParent->isDescendantOf(node)) {
                    result.push_back(node);
                }
            }
            return result;
        }

        bool MapViewBase::canMergeGroups() const {
            auto document = kdl::mem_lock(m_document);
            Model::Node* mergeGroup = findGroupToMergeGroupsInto(document->selectedNodes());
            return mergeGroup != nullptr;
        }

        bool MapViewBase::canMakeStructural() const {
            auto document = kdl::mem_lock(m_document);
            if (document->selectedNodes().hasOnlyBrushes()) {
                const std::vector<Model::BrushNode*>& brushes = document->selectedNodes().brushes();
                for (const auto* brush : brushes) {
                    if (brush->hasAnyTag() || brush->entity() != document->world() || brush->anyFaceHasAnyTag()) {
                        return true;
                    }
                }
            }
            return false;
        }

        void MapViewBase::doPreRender() {}

        void MapViewBase::doRenderExtras(Renderer::RenderContext&, Renderer::RenderBatch&) {}

        bool MapViewBase::doBeforePopupMenu() { return true; }
        void MapViewBase::doAfterPopupMenu() {}
    }
}
