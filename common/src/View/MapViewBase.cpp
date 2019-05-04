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

#include "TrenchBroom.h"
#include "Logger.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Assets/EntityDefinitionManager.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/CollectMatchingNodesVisitor.h"
#include "Model/EditorContext.h"
#include "Model/Entity.h"
#include "Model/EntityAttributes.h"
#include "Model/FindGroupVisitor.h"
#include "Model/FindLayerVisitor.h"
#include "Model/Group.h"
#include "Model/Hit.h"
#include "Model/HitAdapter.h"
#include "Model/HitQuery.h"
#include "Model/Layer.h"
#include "Model/PointFile.h"
#include "Model/PortalFile.h"
#include "Model/PushSelection.h"
#include "Model/World.h"
#include "Renderer/Camera.h"
#include "Renderer/Compass.h"
#include "Renderer/FontDescriptor.h"
#include "Renderer/FontManager.h"
#include "Renderer/MapRenderer.h"
#include "Renderer/RenderBatch.h"
#include "Renderer/RenderService.h"
#include "View/Actions.h"
#include "View/Animation.h"
#include "View/CameraAnimation.h"
#include "View/EnableDisableTagCallback.h"
#include "View/FlashSelectionAnimation.h"
#include "View/FlyModeHelper.h"
#include "View/Grid.h"
#include "View/MapDocument.h"
#include "View/MapFrame.h"
#include "View/MapViewConfig.h"
#include "View/MapViewToolBox.h"
// FIXME:
//#include "View/ToolBoxDropTarget.h"
#include "View/SelectionTool.h"
#include "View/ViewUtils.h"
#include "View/wxUtils.h"

#include <vecmath/polygon.h>
#include <vecmath/util.h>

#include <QShortcut>
#include <QMenu>
#include <QString>
#include <QMimeData>

#include <algorithm>
#include <iterator>

namespace TrenchBroom {
    namespace View {
        static QString GLVendor, GLRenderer, GLVersion;

        const QString &MapViewBase::glRendererString() {
            return GLRenderer;
        }

        const QString &MapViewBase::glVendorString() {
            return GLVendor;
        }

        const QString &MapViewBase::glVersionString() {
            return GLVersion;
        }

        const int MapViewBase::DefaultCameraAnimationDuration = 250;

        MapViewBase::MapViewBase(QWidget* parent, Logger* logger, MapDocumentWPtr document, MapViewToolBox& toolBox, Renderer::MapRenderer& renderer, GLContextManager& contextManager) :
        RenderView(contextManager),
        m_logger(logger),
        m_document(document),
        m_toolBox(toolBox),
        m_animationManager(new AnimationManager(this)),
        m_renderer(renderer),
        m_compass(nullptr),
        m_portalFileRenderer(nullptr) {
            setToolBox(toolBox);
            toolBox.addWindow(this->widgetContainer());
            bindEvents();
            bindObservers();

            //updateBindings();
            // call updateBindings() the next time the event loop runs. We can't call it now because it needs to call
            // a virtual function (depends on whether we are a 3D or 2D map view) which you can't do from a constructor
            QMetaObject::invokeMethod(this, "updateBindings", Qt::QueuedConnection);
        }

        void MapViewBase::setCompass(Renderer::Compass* compass) {
            m_compass = compass;
        }

        MapViewBase::~MapViewBase() {
            // Deleting m_compass will access the VBO so we need to be current
            // see: http://doc.qt.io/qt-5/qopenglwidget.html#resource-initialization-and-cleanup
            makeCurrent();

            m_toolBox.removeWindow(this->widgetContainer());
            unbindObservers();
            delete m_compass;
        }

        void MapViewBase::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->nodesWereAddedNotifier.addObserver(this, &MapViewBase::nodesDidChange);
            document->nodesWereRemovedNotifier.addObserver(this, &MapViewBase::nodesDidChange);
            document->nodesDidChangeNotifier.addObserver(this, &MapViewBase::nodesDidChange);
            document->nodeVisibilityDidChangeNotifier.addObserver(this, &MapViewBase::nodesDidChange);
            document->nodeLockingDidChangeNotifier.addObserver(this, &MapViewBase::nodesDidChange);
            document->commandDoneNotifier.addObserver(this, &MapViewBase::commandDone);
            document->commandUndoneNotifier.addObserver(this, &MapViewBase::commandUndone);
            document->selectionDidChangeNotifier.addObserver(this, &MapViewBase::selectionDidChange);
            document->textureCollectionsDidChangeNotifier.addObserver(this, &MapViewBase::textureCollectionsDidChange);
            document->entityDefinitionsDidChangeNotifier.addObserver(this, &MapViewBase::entityDefinitionsDidChange);
            document->modsDidChangeNotifier.addObserver(this, &MapViewBase::modsDidChange);
            document->editorContextDidChangeNotifier.addObserver(this, &MapViewBase::editorContextDidChange);
            document->mapViewConfigDidChangeNotifier.addObserver(this, &MapViewBase::mapViewConfigDidChange);
            document->documentWasNewedNotifier.addObserver(this, &MapViewBase::documentDidChange);
            document->documentWasClearedNotifier.addObserver(this, &MapViewBase::documentDidChange);
            document->documentWasLoadedNotifier.addObserver(this, &MapViewBase::documentDidChange);
            document->pointFileWasLoadedNotifier.addObserver(this, &MapViewBase::pointFileDidChange);
            document->pointFileWasUnloadedNotifier.addObserver(this, &MapViewBase::pointFileDidChange);
            document->portalFileWasLoadedNotifier.addObserver(this, &MapViewBase::portalFileDidChange);
            document->portalFileWasUnloadedNotifier.addObserver(this, &MapViewBase::portalFileDidChange);

            Grid& grid = document->grid();
            grid.gridDidChangeNotifier.addObserver(this, &MapViewBase::gridDidChange);

            m_toolBox.toolActivatedNotifier.addObserver(this, &MapViewBase::toolChanged);
            m_toolBox.toolDeactivatedNotifier.addObserver(this, &MapViewBase::toolChanged);

            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.addObserver(this, &MapViewBase::preferenceDidChange);
        }

        void MapViewBase::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->nodesWereAddedNotifier.removeObserver(this, &MapViewBase::nodesDidChange);
                document->nodesWereRemovedNotifier.removeObserver(this, &MapViewBase::nodesDidChange);
                document->nodesDidChangeNotifier.removeObserver(this, &MapViewBase::nodesDidChange);
                document->nodeVisibilityDidChangeNotifier.removeObserver(this, &MapViewBase::nodesDidChange);
                document->nodeLockingDidChangeNotifier.removeObserver(this, &MapViewBase::nodesDidChange);
                document->commandDoneNotifier.removeObserver(this, &MapViewBase::commandDone);
                document->commandUndoneNotifier.removeObserver(this, &MapViewBase::commandUndone);
                document->selectionDidChangeNotifier.removeObserver(this, &MapViewBase::selectionDidChange);
                document->textureCollectionsDidChangeNotifier.removeObserver(this, &MapViewBase::textureCollectionsDidChange);
                document->entityDefinitionsDidChangeNotifier.removeObserver(this, &MapViewBase::entityDefinitionsDidChange);
                document->modsDidChangeNotifier.removeObserver(this, &MapViewBase::modsDidChange);
                document->editorContextDidChangeNotifier.removeObserver(this, &MapViewBase::editorContextDidChange);
                document->mapViewConfigDidChangeNotifier.removeObserver(this, &MapViewBase::mapViewConfigDidChange);
                document->documentWasNewedNotifier.removeObserver(this, &MapViewBase::documentDidChange);
                document->documentWasClearedNotifier.removeObserver(this, &MapViewBase::documentDidChange);
                document->documentWasLoadedNotifier.removeObserver(this, &MapViewBase::documentDidChange);
                document->pointFileWasLoadedNotifier.removeObserver(this, &MapViewBase::pointFileDidChange);
                document->pointFileWasUnloadedNotifier.removeObserver(this, &MapViewBase::pointFileDidChange);
                document->portalFileWasLoadedNotifier.removeObserver(this, &MapViewBase::portalFileDidChange);
                document->portalFileWasUnloadedNotifier.removeObserver(this, &MapViewBase::portalFileDidChange);

                Grid& grid = document->grid();
                grid.gridDidChangeNotifier.removeObserver(this, &MapViewBase::gridDidChange);
            }

            m_toolBox.toolActivatedNotifier.removeObserver(this, &MapViewBase::toolChanged);
            m_toolBox.toolDeactivatedNotifier.removeObserver(this, &MapViewBase::toolChanged);

            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.removeObserver(this, &MapViewBase::preferenceDidChange);
        }

        void MapViewBase::nodesDidChange(const Model::NodeList& nodes) {
            updatePickResult();
            update();
        }

        void MapViewBase::toolChanged(Tool* tool) {
            updatePickResult();
            updateActionStates();
            update();
        }

        void MapViewBase::commandDone(Command::Ptr command) {
            updateActionStates();
            updatePickResult();
            update();
        }

        void MapViewBase::commandUndone(UndoableCommand::Ptr command) {
            updateActionStates();
            updatePickResult();
            update();
        }

        void MapViewBase::selectionDidChange(const Selection& selection) {
            updateActionStates();
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

        void MapViewBase::mapViewConfigDidChange() {
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

        void MapViewBase::documentDidChange(MapDocument* document) {
            createActions();
            updateActionStates();
            updatePickResult();
            update();
        }

        void MapViewBase::bindEvents() {
            connect(this, &QWindow::activeChanged, this, &MapViewBase::onActiveChanged);
        }

        void MapViewBase::createActions() {
            m_shortcuts.clear();

            auto visitor = [this](const Action& action) {
                const auto keySequence = action.keySequence();
                auto shortcut = std::make_unique<QShortcut>(widgetContainer());
                shortcut->setKey(keySequence);
                connect(shortcut.get(), &QShortcut::activated, this, [this, &action]() { triggerAction(action); });
                connect(shortcut.get(), &QShortcut::activatedAmbiguously, this, [this, &action]() { triggerAmbiguousAction(action.name()); });
                m_shortcuts.emplace_back(std::move(shortcut), &action);
            };

            auto& actionManager = ActionManager::instance();
            actionManager.visitMapViewActions(visitor);

            auto document = lock(m_document);
            document->visitTagActions(visitor);
            document->visitEntityDefinitionActions(visitor);
        }

        void MapViewBase::updateActionBindings() {
            for (auto& [shortcut, action] : m_shortcuts) {
                shortcut->setKey(action->keySequence());
            }
        }


        void MapViewBase::updateActionStates() {
            ActionExecutionContext context(findMapFrame(widgetContainer()), this);
            for (auto& [shortcut, action] : m_shortcuts) {
                shortcut->setEnabled(hasFocus() && action->enabled(context));
            }
        }

        void MapViewBase::triggerAction(const Action& action) {
            qDebug() << "Action triggered: " << QString::fromStdString(action.name());
            auto* mapFrame = findMapFrame(widgetContainer());
            ActionExecutionContext context(mapFrame, this);
            action.execute(context);
        }

        void MapViewBase::triggerAmbiguousAction(const String& name) {
            qDebug() << "Ambiguous action triggered: " << QString::fromStdString(name);
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
            MapDocumentSPtr document = lock(m_document);
            const Grid& grid = document->grid();
            const vm::vec3 delta = moveDirection(direction) * static_cast<FloatType>(grid.actualSize());
            m_toolBox.moveRotationCenter(delta);
            update();
        }

        void MapViewBase::moveVertices(const vm::direction direction) {
            MapDocumentSPtr document = lock(m_document);
            const Grid& grid = document->grid();
            const vm::vec3 delta = moveDirection(direction) * static_cast<FloatType>(grid.actualSize());
            m_toolBox.moveVertices(delta);
        }

        void MapViewBase::moveObjects(const vm::direction direction) {
            MapDocumentSPtr document = lock(m_document);
            const Grid& grid = document->grid();
            const vm::vec3 delta = moveDirection(direction) * static_cast<FloatType>(grid.actualSize());
            document->translateObjects(delta);
        }

        vm::vec3 MapViewBase::moveDirection(const vm::direction direction) const {
            return doGetMoveDirection(direction);
        }

        void MapViewBase::duplicateObjects() {
            auto document = lock(m_document);
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
            MapDocumentSPtr document = lock(m_document);
            if (!document->hasSelectedNodes())
                return;

            const vm::vec3 axis = rotationAxis(axisSpec, clockwise);
            const double angle = m_toolBox.rotateObjectsToolActive() ? std::abs(m_toolBox.rotateToolAngle()) : vm::C::piOverTwo();

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
                auto document = lock(m_document);

                // If we snap the selection bounds' center to the grid size, then
                // selections that are an odd number of grid units wide get translated.
                // Instead, snap to 1/2 the grid size.
                // (see: https://github.com/kduske/TrenchBroom/issues/1495 )
                Grid halfGrid(document->grid().size());
                halfGrid.decSize();

                const auto center = halfGrid.referencePoint(document->selectionBounds());
                const auto axis = firstComponent(moveDirection(direction));

                document->flipObjects(center, axis);
            }
        }

        bool MapViewBase::canFlipObjects() const {
            MapDocumentSPtr document = lock(m_document);
            return !m_toolBox.anyToolActive() && document->hasSelectedNodes();
        }

        void MapViewBase::moveTextures(const vm::direction direction) {
            auto document = lock(m_document);
            if (document->hasSelectedBrushFaces()) {
                const auto offset = moveTextureOffset(direction);
                document->moveTextures(doGetCamera().up(), doGetCamera().right(), offset);
            }
        }

        vm::vec2f MapViewBase::moveTextureOffset(vm::direction direction) const {
            switch (direction) {
                case vm::direction::up:
                    return vm::vec2f(0.0f, moveTextureDistance());
                case vm::direction::down:
                    return vm::vec2f(0.0f, -moveTextureDistance());
                case vm::direction::left:
                    return vm::vec2f(-moveTextureDistance(), 0.0f);
                case vm::direction::right:
                    return vm::vec2f(moveTextureDistance(), 0.0f);
                default:
                    return vm::vec2f();
            }
        }

        float MapViewBase::moveTextureDistance() const {
            const auto& grid = lock(m_document)->grid();
            const auto gridSize = static_cast<float>(grid.actualSize());

            const Qt::KeyboardModifiers modifiers = QApplication::keyboardModifiers();
            switch (modifiers) {
                case Qt::ControlModifier:
                    return 1.0f;
                case Qt::ShiftModifier:
                    return 2.0f * gridSize;
                default:
                    return gridSize;
            }
        }

        void MapViewBase::rotateTextures(const bool clockwise) {
            auto document = lock(m_document);
            if (document->hasSelectedBrushFaces()) {
                const auto angle = rotateTextureAngle(clockwise);
                document->rotateTextures(angle);
            }
        }

        float MapViewBase::rotateTextureAngle(const bool clockwise) const {
            const auto& grid = lock(m_document)->grid();
            const auto gridAngle = static_cast<float>(vm::toDegrees(grid.angle()));
            float angle;

            const Qt::KeyboardModifiers modifiers = QApplication::keyboardModifiers();
            switch (modifiers) {
                case Qt::ControlModifier:
                    angle = 1.0f;
                    break;
                case Qt::ShiftModifier:
                    angle = 90.0f;
                    break;
                default:
                    angle = gridAngle;
                    break;
            }
            return clockwise ? angle : -angle;
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

            MapDocumentSPtr document = lock(m_document);
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
            MapDocumentSPtr document = lock(m_document);
            const size_t index = action->data().toUInt();
            const Assets::EntityDefinition* definition = findEntityDefinition(Assets::EntityDefinition::Type_PointEntity, index);
            ensure(definition != nullptr, "definition is null");
            assert(definition->type() == Assets::EntityDefinition::Type_PointEntity);
            createPointEntity(static_cast<const Assets::PointEntityDefinition*>(definition));
        }

        void MapViewBase::createBrushEntity() {
            auto* action = qobject_cast<const QAction*>(sender());
            MapDocumentSPtr document = lock(m_document);
            const size_t index = action->data().toUInt();
            const Assets::EntityDefinition* definition = findEntityDefinition(Assets::EntityDefinition::Type_BrushEntity, index);
            ensure(definition != nullptr, "definition is null");
            assert(definition->type() == Assets::EntityDefinition::Type_BrushEntity);
            createBrushEntity(static_cast<const Assets::BrushEntityDefinition*>(definition));
        }

        Assets::EntityDefinition* MapViewBase::findEntityDefinition(const Assets::EntityDefinition::Type type, const size_t index) const {
            size_t count = 0;
            for (const Assets::EntityDefinitionGroup& group : lock(m_document)->entityDefinitionManager().groups()) {
                const Assets::EntityDefinitionList definitions = group.definitions(type, Assets::EntityDefinition::Name);
                if (index < count + definitions.size())
                    return definitions[index - count];
                count += definitions.size();
            }
            return nullptr;
        }

        void MapViewBase::createPointEntity(const Assets::PointEntityDefinition* definition) {
            ensure(definition != nullptr, "definition is null");

            auto document = lock(m_document);
            const auto delta = doComputePointEntityPosition(definition->bounds());
            document->createPointEntity(definition, delta);
        }

        void MapViewBase::createBrushEntity(const Assets::BrushEntityDefinition* definition) {
            ensure(definition != nullptr, "definition is null");

            auto document = lock(m_document);
            document->createBrushEntity(definition);
        }

        bool MapViewBase::canCreateBrushEntity() {
            auto document = lock(m_document);
            return document->selectedNodes().hasOnlyBrushes();
        }

        void MapViewBase::toggleTagVisible(const Model::SmartTag& tag) {
            const auto tagIndex = tag.index();

            auto document = lock(m_document);
            auto& editorContext = document->editorContext();
            auto hiddenTags = editorContext.hiddenTags();
            hiddenTags ^= 1UL << tagIndex;
            editorContext.setHiddenTags(hiddenTags);
        }


        void MapViewBase::enableTag(const Model::SmartTag& tag) {
            assert(tag.canEnable());
            auto document = lock(m_document);

            Transaction transaction(document, "Turn Selection into " + tag.name());
            EnableDisableTagCallback callback;
            tag.enable(callback, *document);
        }

        void MapViewBase::disableTag(const Model::SmartTag& tag) {
            assert(tag.canDisable());
            auto document = lock(m_document);
            Transaction transaction(document, "Turn Selection into non-" + tag.name());
            EnableDisableTagCallback callback;
            tag.disable(callback, *document);
        }

        void MapViewBase::makeStructural() {
            auto document = lock(m_document);
            if (!document->selectedNodes().hasBrushes()) {
                return;
            }

            Transaction transaction(document, "Make Structural");
            Model::NodeList toReparent;
            for (auto* brush : document->selectedNodes().brushes()) {
                if (brush->entity() != document->world()) {
                    toReparent.push_back(brush);
                }
            }

            if (!toReparent.empty()) {
                reparentNodes(toReparent, document->currentParent(), false);
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
            auto document = lock(m_document);

            Model::EditorContext& editorContext = document->editorContext();
            editorContext.setEntityDefinitionHidden(definition, !editorContext.entityDefinitionHidden(definition));
        }

        void MapViewBase::createEntity(const Assets::EntityDefinition* definition) {
            auto document = lock(m_document);
            if (definition->type() == Assets::EntityDefinition::Type_PointEntity) {
                createPointEntity(static_cast<const Assets::PointEntityDefinition*>(definition));
            } else if (canCreateBrushEntity()) {
                createBrushEntity(static_cast<const Assets::BrushEntityDefinition*>(definition));
            }
        }

        void MapViewBase::toggleShowEntityClassnames() {
            MapDocumentSPtr document = lock(m_document);
            MapViewConfig& config = document->mapViewConfig();
            config.setShowEntityClassnames(!config.showEntityClassnames());
        }

        void MapViewBase::toggleShowGroupBounds() {
            MapDocumentSPtr document = lock(m_document);
            MapViewConfig& config = document->mapViewConfig();
            config.setShowGroupBounds(!config.showGroupBounds());
        }

        void MapViewBase::toggleShowBrushEntityBounds() {
            MapDocumentSPtr document = lock(m_document);
            MapViewConfig& config = document->mapViewConfig();
            config.setShowBrushEntityBounds(!config.showBrushEntityBounds());
        }

        void MapViewBase::toggleShowPointEntityBounds() {
            MapDocumentSPtr document = lock(m_document);
            MapViewConfig& config = document->mapViewConfig();
            config.setShowPointEntityBounds(!config.showPointEntityBounds());
        }

        void MapViewBase::toggleShowPointEntities() {
            MapDocumentSPtr document = lock(m_document);
            Model::EditorContext& editorContext = document->editorContext();
            editorContext.setShowPointEntities(!editorContext.showPointEntities());
        }

        void MapViewBase::toggleShowPointEntityModels() {
            MapDocumentSPtr document = lock(m_document);
            MapViewConfig& config = document->mapViewConfig();
            config.setShowPointEntityModels(!config.showPointEntityModels());
        }

        void MapViewBase::toggleShowBrushes() {
            MapDocumentSPtr document = lock(m_document);
            Model::EditorContext& editorContext = document->editorContext();
            editorContext.setShowBrushes(!editorContext.showBrushes());
        }

        void MapViewBase::showTextures() {
            MapDocumentSPtr document = lock(m_document);
            MapViewConfig& config = document->mapViewConfig();
            config.setFaceRenderMode(MapViewConfig::FaceRenderMode_Textured);
        }

        void MapViewBase::hideTextures() {
            MapDocumentSPtr document = lock(m_document);
            MapViewConfig& config = document->mapViewConfig();
            config.setFaceRenderMode(MapViewConfig::FaceRenderMode_Flat);
        }

        void MapViewBase::hideFaces() {
            MapDocumentSPtr document = lock(m_document);
            MapViewConfig& config = document->mapViewConfig();
            config.setFaceRenderMode(MapViewConfig::FaceRenderMode_Skip);
        }

        void MapViewBase::toggleShadeFaces() {
            MapDocumentSPtr document = lock(m_document);
            MapViewConfig& config = document->mapViewConfig();
            config.setShadeFaces(!config.shadeFaces());
        }

        void MapViewBase::toggleShowFog() {
            MapDocumentSPtr document = lock(m_document);
            MapViewConfig& config = document->mapViewConfig();
            config.setShowFog(!config.showFog());
        }

        void MapViewBase::toggleShowEdges() {
            MapDocumentSPtr document = lock(m_document);
            MapViewConfig& config = document->mapViewConfig();
            config.setShowEdges(!config.showEdges());
        }

        void MapViewBase::showAllEntityLinks() {
            MapDocumentSPtr document = lock(m_document);
            Model::EditorContext& editorContext = document->editorContext();
            editorContext.setEntityLinkMode(Model::EditorContext::EntityLinkMode_All);
        }

        void MapViewBase::showTransitivelySelectedEntityLinks() {
            MapDocumentSPtr document = lock(m_document);
            Model::EditorContext& editorContext = document->editorContext();
            editorContext.setEntityLinkMode(Model::EditorContext::EntityLinkMode_Transitive);
        }

        void MapViewBase::showDirectlySelectedEntityLinks() {
            MapDocumentSPtr document = lock(m_document);
            Model::EditorContext& editorContext = document->editorContext();
            editorContext.setEntityLinkMode(Model::EditorContext::EntityLinkMode_Direct);
        }

        void MapViewBase::hideAllEntityLinks() {
            MapDocumentSPtr document = lock(m_document);
            Model::EditorContext& editorContext = document->editorContext();
            editorContext.setEntityLinkMode(Model::EditorContext::EntityLinkMode_None);
        }

        void MapViewBase::onActiveChanged() {
            qDebug("MapViewBase::onActiveChanged: is active: %d, is focus window: %d has focus %d",
                (int)isActive(), (int)(QGuiApplication::focusWindow() == this), (int) hasFocus());
            requestUpdate(); // show/hide focus rectangle
            updateActionStates();  // enable/disable QShortcut's to reflect whether we have focus (needed because of QOpenGLWindow; see comment in createAndRegisterShortcut)

            // FIXME: wx called these on focus in/out
            //updateModifierKeys();
            //clearModifierKeys();
            // FIXME: wx called this when the actual window (ie MapFrame) became active:
            // updateLastActivation();
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
                auto document = lock(m_document);
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
            FlashSelectionAnimation* animation = new FlashSelectionAnimation(m_renderer, this->widgetContainer(), 180);
            m_animationManager->runAnimation(animation, true);
        }

        bool MapViewBase::doGetIsCurrent() const {
            return hasFocus();
        }

        void MapViewBase::doSetToolBoxDropTarget() {
            // FIXME: DND
            //SetDropTarget(new ToolBoxDropTarget(this, this));
        }

        void MapViewBase::doClearDropTarget() {
            // FIXME: DND
            //SetDropTarget(nullptr);
        }

        bool MapViewBase::doCancelMouseDrag() {
            return ToolBoxConnector::cancelDrag();
        }

        void MapViewBase::doRefreshViews() {
            requestUpdate();
        }

        void MapViewBase::initializeGL() {
            RenderView::initializeGL();
            GLVendor   = QString::fromUtf8(reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
            GLRenderer = QString::fromUtf8(reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
            GLVersion  = QString::fromUtf8(reinterpret_cast<const char*>(glGetString(GL_VERSION)));

            m_logger->info(tr("Renderer info: %1 version %2 from %3").arg(GLRenderer).arg(GLVersion).arg(GLVendor));
        // FIXME: use Qt
#if 0
            m_logger->info("Depth buffer bits: %d", depthBits());

            if (multisample())
                m_logger->info("Multisampling enabled");
            else
                m_logger->info("Multisampling disabled");
#endif

        }

        bool MapViewBase::doShouldRenderFocusIndicator() const {
            return true;
        }

        void MapViewBase::doRender() {
            doPreRender();

            const IO::Path& fontPath = pref(Preferences::RendererFontPath());
            const size_t fontSize = static_cast<size_t>(pref(Preferences::RendererFontSize));
            const Renderer::FontDescriptor fontDescriptor(fontPath, fontSize);

            MapDocumentSPtr document = lock(m_document);
            const MapViewConfig& mapViewConfig = document->mapViewConfig();
            const Grid& grid = document->grid();

            Renderer::RenderContext renderContext(doGetRenderMode(), doGetCamera(), fontManager(), shaderManager());
            renderContext.setShowTextures(mapViewConfig.showTextures());
            renderContext.setShowFaces(mapViewConfig.showFaces());
            renderContext.setShowEdges(mapViewConfig.showEdges());
            renderContext.setShadeFaces(mapViewConfig.shadeFaces());
            renderContext.setShowPointEntities(mapViewConfig.showPointEntities());
            renderContext.setShowPointEntityModels(mapViewConfig.showPointEntityModels());
            renderContext.setShowEntityClassnames(mapViewConfig.showEntityClassnames());
            renderContext.setShowGroupBounds(mapViewConfig.showGroupBounds());
            renderContext.setShowBrushEntityBounds(mapViewConfig.showBrushEntityBounds());
            renderContext.setShowPointEntityBounds(mapViewConfig.showPointEntityBounds());
            renderContext.setShowFog(mapViewConfig.showFog());
            renderContext.setShowGrid(grid.visible());
            renderContext.setGridSize(grid.actualSize());

            setupGL(renderContext);
            setRenderOptions(renderContext);

            Renderer::RenderBatch renderBatch(vertexVbo(), indexVbo());

            doRenderGrid(renderContext, renderBatch);
            doRenderMap(m_renderer, renderContext, renderBatch);
            doRenderTools(m_toolBox, renderContext, renderBatch);
            doRenderExtras(renderContext, renderBatch);
            renderCoordinateSystem(renderContext, renderBatch);
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
            glAssert(glViewport(x, y, width, height));

            glAssert(glEnable(GL_MULTISAMPLE));
            glAssert(glEnable(GL_BLEND));
            glAssert(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
            glAssert(glShadeModel(GL_SMOOTH));
        }

        void MapViewBase::renderCoordinateSystem(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            if (pref(Preferences::ShowAxes)) {
                MapDocumentSPtr document = lock(m_document);
                const vm::bbox3& worldBounds = document->worldBounds();

                Renderer::RenderService renderService(renderContext, renderBatch);
                renderService.renderCoordinateSystem(vm::bbox3f(worldBounds));
            }
        }

        void MapViewBase::renderPointFile(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            MapDocumentSPtr document = lock(m_document);
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

        void MapViewBase::validatePortalFileRenderer(Renderer::RenderContext& renderContext) {
            assert(m_portalFileRenderer == nullptr);
            m_portalFileRenderer = std::make_unique<Renderer::PrimitiveRenderer>();

            MapDocumentSPtr document = lock(m_document);
            Model::PortalFile* portalFile = document->portalFile();
            if (portalFile != nullptr) {
                for (const auto& poly : portalFile->portals()) {
                    m_portalFileRenderer->renderFilledPolygon(pref(Preferences::PortalFileFillColor),
                                                              Renderer::PrimitiveRenderer::OP_Hide,
                                                              Renderer::PrimitiveRenderer::CP_ShowBackfaces,
                                                              poly.vertices());

                    const auto lineWidth = 4.0f;
                    m_portalFileRenderer->renderPolygon(pref(Preferences::PortalFileBorderColor),
                                                        lineWidth,
                                                        Renderer::PrimitiveRenderer::OP_Hide,
                                                        poly.vertices());
                }
            }
        }

        void MapViewBase::renderCompass(Renderer::RenderBatch& renderBatch) {
            if (m_compass != nullptr)
                m_compass->render(renderBatch);
        }

        void MapViewBase::renderFPS(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            Renderer::RenderService renderService(renderContext, renderBatch);

            renderService.renderHeadsUp(m_currentFPS);
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

        static bool isEntity(const Model::Node* node) {
            class IsEntity : public Model::ConstNodeVisitor, public Model::NodeQuery<bool> {
            private:
                void doVisit(const Model::World* world) override   { setResult(false); }
                void doVisit(const Model::Layer* layer) override   { setResult(false); }
                void doVisit(const Model::Group* group) override   { setResult(false); }
                void doVisit(const Model::Entity* entity) override { setResult(true); }
                void doVisit(const Model::Brush* brush) override   { setResult(false); }
            };

            IsEntity visitor;
            node->accept(visitor);
            return visitor.result();
        }

        void MapViewBase::doShowPopupMenu() {
            // We process input events during paint event processing, but we cannot show a popup menu
            // during paint processing, so we enqueue an event for later.
            QMetaObject::invokeMethod(this, "OnShowPopupMenu", Qt::QueuedConnection);
        }

        void MapViewBase::OnShowPopupMenu() {
            if (!doBeforePopupMenu()) {
                return;
            }

            MapDocumentSPtr document = lock(m_document);
            const Model::NodeList& nodes = document->selectedNodes().nodes();
            Model::Node* newBrushParent = findNewParentEntityForBrushes(nodes);
            Model::Node* currentGroup = document->editorContext().currentGroup();
            Model::Node* newGroup = findNewGroupForObjects(nodes);
            Model::Node* mergeGroup = findGroupToMergeGroupsInto(document->selectedNodes());

            auto* mapFrame = findMapFrame(this->widgetContainer());

            QMenu menu;
            QAction* groupAction = menu.addAction(tr("Group"), mapFrame, &MapFrame::groupSelectedObjects);
            groupAction->setEnabled(mapFrame->canGroupSelectedObjects());

            QAction* ungroupAction = menu.addAction(tr("Ungroup"), mapFrame, &MapFrame::ungroupSelectedObjects);
            ungroupAction->setEnabled(mapFrame->canUngroupSelectedObjects());

            QAction* mergeGroupAction = nullptr;
            if (mergeGroup != nullptr) {
                mergeGroupAction = menu.addAction(tr("Merge Groups into %1").arg(QString::fromStdString(mergeGroup->name())), this,
                    &MapViewBase::mergeSelectedGroups);
            } else {
                mergeGroupAction = menu.addAction(tr("Merge Groups"), this, &MapViewBase::mergeSelectedGroups);
            }
            mergeGroupAction->setEnabled(canMergeGroups());

            QAction* renameAction = menu.addAction(tr("Rename"), mapFrame, &MapFrame::renameSelectedGroups);
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

            if (document->selectedNodes().hasOnlyBrushes()) {
                QAction* moveToWorldAction = menu.addAction(tr("Make Structural"), this, &MapViewBase::makeStructural);
                moveToWorldAction->setEnabled(canMakeStructural());

                if (isEntity(newBrushParent)) {
                    menu.addAction(tr("Move Brushes to Entity %1").arg(QString::fromStdString(newBrushParent->name())), this,
                        &MapViewBase::moveSelectedBrushesToEntity);
                }
            }

            menu.addSeparator();

            menu.addMenu(makeEntityGroupsMenu(Assets::EntityDefinition::Type_PointEntity));
            menu.addMenu(makeEntityGroupsMenu(Assets::EntityDefinition::Type_BrushEntity));

            menu.exec(QCursor::pos());

            // Generate a synthetic mouse move event to update the mouse position after the popup menu closes.
            // FIXME: needed with Qt?
#if 0
            wxMouseEvent mouseEvent(wxEVT_MOTION);
            mouseEvent.SetPosition(ScreenToClient(wxGetMousePosition()));
            OnMouse(mouseEvent);
#endif

            doAfterPopupMenu();
        }

        /**
         * Forward drag and drop events from QOpenGLWindow to ToolBoxConnector
         */
        bool MapViewBase::event(QEvent *ev) {
            switch (ev->type()) {
                case QEvent::DragEnter: {
                    auto* dragEnterEvent = static_cast<QDragEnterEvent *>(ev);
                    dragEnter(dragEnterEvent->pos().x(), dragEnterEvent->pos().y(), dragEnterEvent->mimeData()->text().toStdString());
                    break;
                }
                case QEvent::DragLeave: {
                    auto* dragLeaveEvent = static_cast<QDragLeaveEvent *>(ev);
                    dragLeave();
                    break;
                }
                case QEvent::DragMove: {
                    auto* dragMoveEvent = static_cast<QDragMoveEvent *>(ev);
                    dragMove(dragMoveEvent->pos().x(), dragMoveEvent->pos().y(), dragMoveEvent->mimeData()->text().toStdString());
                    break;
                }
                case QEvent::Drop: {
                    auto* dropEvent = static_cast<QDropEvent *>(ev);
                    dragDrop(dropEvent->pos().x(), dropEvent->pos().y(), dropEvent->mimeData()->text().toStdString());
                    break;
                }
                default:
                    break;
            }
            return RenderView::event(ev);
        }

        QMenu* MapViewBase::makeEntityGroupsMenu(const Assets::EntityDefinition::Type type) {
            auto* menu = new QMenu();

            switch (type) {
                case Assets::EntityDefinition::Type_PointEntity:
                    menu->setTitle(tr("Create Point Entity"));
                    break;
                case Assets::EntityDefinition::Type_BrushEntity:
                    menu->setTitle(tr("Create Brush Entity"));
                    break;
            }

            const bool enableMakeBrushEntity = canCreateBrushEntity();
            size_t id = 0;

            MapDocumentSPtr document = lock(m_document);
            for (const Assets::EntityDefinitionGroup& group : document->entityDefinitionManager().groups()) {
                const Assets::EntityDefinitionList definitions = group.definitions(type, Assets::EntityDefinition::Name);

                Assets::EntityDefinitionList filteredDefinitions;
                std::copy_if(std::begin(definitions), std::end(definitions), std::back_inserter(filteredDefinitions),
                             [](const Assets::EntityDefinition* definition) { return !StringUtils::caseSensitiveEqual(definition->name(), Model::AttributeValues::WorldspawnClassname); }
                );

                if (!filteredDefinitions.empty()) {
                    const auto groupName = QString::fromStdString(group.displayName());
                    auto* groupMenu = new QMenu(groupName);

                    for (Assets::EntityDefinition* definition : filteredDefinitions) {
                        const auto label = QString::fromStdString(definition->shortName());
                        QAction *action = nullptr;

                        switch (type) {
                            case Assets::EntityDefinition::Type_PointEntity: {
                                action = groupMenu->addAction(label, this, qOverload<>(&MapViewBase::createPointEntity));
                                break;
                            }
                            case Assets::EntityDefinition::Type_BrushEntity: {
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
            MapDocumentSPtr document = lock(m_document);
            const Model::NodeList nodes = document->selectedNodes().nodes();
            Model::Node* newGroup = findNewGroupForObjects(nodes);
            ensure(newGroup != nullptr, "newGroup is null");

            Transaction transaction(document, "Add Objects to Group");
            reparentNodes(nodes, newGroup, true);
            document->deselectAll();
            document->select(newGroup);
        }

        void MapViewBase::removeSelectedObjectsFromGroup() {
            MapDocumentSPtr document = lock(m_document);
            const Model::NodeList nodes = document->selectedNodes().nodes();
            Model::Node* currentGroup = document->editorContext().currentGroup();
            ensure(currentGroup != nullptr, "currentGroup is null");

            Transaction transaction(document, "Remove Objects from Group");
            reparentNodes(nodes, document->currentLayer(), true);

            while (document->currentGroup() != nullptr) {
                document->closeGroup();
            }
            document->select(nodes);
        }

        Model::Node* MapViewBase::findNewGroupForObjects(const Model::NodeList& nodes) const {
            Model::Node* newGroup = nullptr;

            MapDocumentSPtr document = lock(m_document);
            const Model::Hit& hit = pickResult().query().pickable().first();
            if (hit.isMatch())
                newGroup = findOutermostClosedGroup(Model::hitToNode(hit));

            if (newGroup != nullptr && canReparentNodes(nodes, newGroup))
                return newGroup;
            return nullptr;
        }

        void MapViewBase::mergeSelectedGroups() {
            auto document = lock(m_document);
            auto* newGroup = findGroupToMergeGroupsInto(document->selectedNodes());
            ensure(newGroup != nullptr, "newGroup is null");

            Transaction transaction(document, "Merge Groups");
            document->mergeSelectedGroupsWithGroup(newGroup);
        }

        Model::Group* MapViewBase::findGroupToMergeGroupsInto(const Model::NodeCollection& selectedNodes) const {
            if (!(selectedNodes.hasOnlyGroups() && selectedNodes.groupCount() >= 2)) {
                return nullptr;
            }

            Model::Group* mergeTarget = nullptr;

            auto document = lock(m_document);
            const Model::Hit& hit = pickResult().query().pickable().first();
            if (hit.isMatch()) {
                mergeTarget = findOutermostClosedGroup(Model::hitToNode(hit));
            }
            if (mergeTarget == nullptr) {
                return nullptr;
            }

            const auto& nodes = selectedNodes.nodes();
            const bool canReparentAll = std::all_of(nodes.begin(), nodes.end(), [&](const auto* node){
                return node == mergeTarget || this->canReparentNode(node, mergeTarget);
            });

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
            MapDocumentSPtr document = lock(m_document);
            const Model::NodeList nodes = document->selectedNodes().nodes();
            Model::Node* newParent = findNewParentEntityForBrushes(nodes);
            ensure(newParent != nullptr, "newParent is null");

            const Transaction transaction(document, "Move " + StringUtils::safePlural(nodes.size(), "Brush", "Brushes"));
            reparentNodes(nodes, newParent, false);

            document->deselectAll();
            document->select(nodes);
        }

        Model::Node* MapViewBase::findNewParentEntityForBrushes(const Model::NodeList& nodes) const {
            Model::Node* newParent = nullptr;

            MapDocumentSPtr document = lock(m_document);
            const Model::Hit& hit = pickResult().query().pickable().type(Model::Brush::BrushHit).occluded().first();
            if (hit.isMatch()) {
                const Model::Brush* brush = Model::hitToBrush(hit);
                newParent = brush->entity();
            }

            if (newParent != nullptr && newParent != document->world() && canReparentNodes(nodes, newParent)) {
                return newParent;
            }

            if (!nodes.empty()) {
                Model::Node* lastNode = nodes.back();

                Model::Group* group = Model::findGroup(lastNode);
                if (group != nullptr) {
                    return group;
                }

                Model::Layer* layer = Model::findLayer(lastNode);
                if (layer != nullptr) {
                    return layer;
            }
            }

            return document->currentLayer();
        }

        bool MapViewBase::canReparentNodes(const Model::NodeList& nodes, const Model::Node* newParent) const {
            for (const Model::Node* node : nodes) {
                if (canReparentNode(node, newParent)) {
                    return true;
                }
            }
            return false;
        }

        class BrushesToEntities {
        private:
            const Model::World* m_world;
        public:
            BrushesToEntities(const Model::World* world) : m_world(world) {}
        public:
            bool operator()(const Model::World* world) const   { return false; }
            bool operator()(const Model::Layer* layer) const   { return false; }
            bool operator()(const Model::Group* group) const   { return true;  }
            bool operator()(const Model::Entity* entity) const { return true; }
            bool operator()(const Model::Brush* brush) const   { return brush->entity() == m_world; }
        };

        static Model::NodeList collectEntitiesForBrushes(const Model::NodeList& selectedNodes, const Model::World *world) {
            using BrushesToEntitiesVisitor = Model::CollectMatchingNodesVisitor<BrushesToEntities, Model::UniqueNodeCollectionStrategy, Model::StopRecursionIfMatched>;

            BrushesToEntitiesVisitor collect(world);
            Model::Node::acceptAndEscalate(std::begin(selectedNodes), std::end(selectedNodes), collect);
            return collect.nodes();
        }

        void MapViewBase::reparentNodes(const Model::NodeList& nodes, Model::Node* newParent, const bool preserveEntities) {
            ensure(newParent != nullptr, "newParent is null");

            MapDocumentSPtr document = lock(m_document);
            Model::NodeList inputNodes;
            if (preserveEntities) {
                inputNodes = collectEntitiesForBrushes(nodes, document->world());
            } else {
                inputNodes = nodes;
            }

            const Model::NodeList reparentableNodes = collectReparentableNodes(inputNodes, newParent);
            assert(!reparentableNodes.empty());

            StringStream name;
            name << "Move " << (reparentableNodes.size() == 1 ? "Object" : "Objects") << " to " << newParent->name();

            const Transaction transaction(document, name.str());
            document->deselectAll();
            document->reparentNodes(newParent, reparentableNodes);
            document->select(reparentableNodes);
        }

        Model::NodeList MapViewBase::collectReparentableNodes(const Model::NodeList& nodes, const Model::Node* newParent) const {
            Model::NodeList result;
            std::copy_if(std::begin(nodes), std::end(nodes), std::back_inserter(result), [=](const Model::Node* node) { return newParent != node && newParent != node->parent() && !newParent->isDescendantOf(node); });
            return result;
        }

        bool MapViewBase::canMergeGroups() const {
            MapDocumentSPtr document = lock(m_document);
            Model::Node* mergeGroup = findGroupToMergeGroupsInto(document->selectedNodes());
            return mergeGroup != nullptr;
        }

        bool MapViewBase::canMakeStructural() const {
            MapDocumentSPtr document = lock(m_document);
            if (document->selectedNodes().hasOnlyBrushes()) {
                const Model::BrushList& brushes = document->selectedNodes().brushes();
                for (const auto* brush : brushes) {
                    if (brush->hasAnyTag() || brush->entity() != document->world() || brush->anyFaceHasAnyTag()) {
                        return true;
                    }
                }
            }
            return false;
        }

        void MapViewBase::doPreRender() {}

        void MapViewBase::doRenderExtras(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {}

        bool MapViewBase::doBeforePopupMenu() { return true; }
        void MapViewBase::doAfterPopupMenu() {}
    }
}
