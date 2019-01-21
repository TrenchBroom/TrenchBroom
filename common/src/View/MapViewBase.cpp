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
#include "View/ActionList.h"
#include "View/Animation.h"
#include "View/CameraAnimation.h"
#include "View/FlashSelectionAnimation.h"
#include "View/FlyModeHelper.h"
#include "View/Grid.h"
#include "View/MapDocument.h"
#include "View/MapViewConfig.h"
#include "View/MapViewToolBox.h"
// FIXME:
//#include "View/ToolBoxDropTarget.h"
#include "View/ViewUtils.h"
#include "View/wxUtils.h"

#include <vecmath/polygon.h>
#include <vecmath/util.h>

#include <QShortcut>
#include <QMenu>
#include <QString>

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
        ToolBoxConnector(this),
        m_logger(logger),
        m_document(document),
        m_toolBox(toolBox),
        m_animationManager(new AnimationManager(this)),
        m_renderer(renderer),
        m_compass(nullptr),
        m_portalFileRenderer(nullptr) {
            setToolBox(toolBox);
            toolBox.addWindow(this->widgetContainer());
            createActions();
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
            updateBindings();
            update();
        }

        void MapViewBase::commandDone(Command::Ptr command) {
            updatePickResult();
            update();
        }

        void MapViewBase::commandUndone(UndoableCommand::Ptr command) {
            updatePickResult();
            update();
        }
        
        void MapViewBase::selectionDidChange(const Selection& selection) {
            updateBindings();
        }

        void MapViewBase::textureCollectionsDidChange() {
            update();
        }

        void MapViewBase::entityDefinitionsDidChange() {
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

            updateBindings();
            update();
        }

        void MapViewBase::documentDidChange(MapDocument* document) {
            updatePickResult();
            update();
        }

        void MapViewBase::bindEvents() {
#if 0
            wxFrame* frame = findFrame(this);
            frame->Bind(wxEVT_ACTIVATE, &MapViewBase::OnActivateFrame, this);
#endif
        }

        QShortcut* MapViewBase::createAndRegisterShortcut(const ActionInfo& info, Callback callback) {
            QShortcut* shortcut = new QShortcut(this->widgetContainer());
            shortcut->setContext(Qt::WidgetShortcut); // Only in this widget
            registerBinding(shortcut, info);
            connect(shortcut, &QShortcut::activated, this, callback);
            return shortcut;
        }

        void MapViewBase::createAndRegister2D3DShortcut(const ActionInfo& info, Callback callback2D, Callback callback3D) {
            QShortcut* shortcut2D = createAndRegisterShortcut(info, callback2D);
            m_2DOnlyShortcuts.push_back(shortcut2D);

            QShortcut* shortcut3D = createAndRegisterShortcut(info, callback3D);
            m_3DOnlyShortcuts.push_back(shortcut3D);
        }

        void MapViewBase::createActions() {
            // clip
            createAndRegisterShortcut(ActionList::instance().controlsMapViewToggleClipSideInfo, &MapViewBase::OnToggleClipSide);
            createAndRegisterShortcut(ActionList::instance().controlsMapViewPerformclipInfo, &MapViewBase::OnPerformClip);

            // vertex movement
            createAndRegister2D3DShortcut(ActionList::instance().controlsMapViewMoveVerticesUpForwardInfo, &MapViewBase::OnMoveVerticesUp, &MapViewBase::OnMoveVerticesForward);
            createAndRegister2D3DShortcut(ActionList::instance().controlsMapViewMoveVerticesDownBackwardInfo, &MapViewBase::OnMoveVerticesDown, &MapViewBase::OnMoveVerticesBackward);
            createAndRegisterShortcut(ActionList::instance().controlsMapViewMoveVerticesLeftInfo, &MapViewBase::OnMoveVerticesLeft);
            createAndRegisterShortcut(ActionList::instance().controlsMapViewMoveVerticesRightInfo, &MapViewBase::OnMoveVerticesRight);
            createAndRegister2D3DShortcut(ActionList::instance().controlsMapViewMoveVerticesBackwardUpInfo, &MapViewBase::OnMoveVerticesBackward, &MapViewBase::OnMoveVerticesUp);
            createAndRegister2D3DShortcut(ActionList::instance().controlsMapViewMoveVerticesForwardDownInfo, &MapViewBase::OnMoveVerticesForward, &MapViewBase::OnMoveVerticesDown);

            // object movement
            createAndRegister2D3DShortcut(ActionList::instance().controlsMapViewMoveObjectsUpForwardInfo, &MapViewBase::OnMoveObjectsUp, &MapViewBase::OnMoveObjectsForward);
            createAndRegister2D3DShortcut(ActionList::instance().controlsMapViewMoveObjectsDownBackwardInfo, &MapViewBase::OnMoveObjectsDown, &MapViewBase::OnMoveObjectsBackward);
            createAndRegisterShortcut(ActionList::instance().controlsMapViewMoveObjectsLeftInfo, &MapViewBase::OnMoveObjectsLeft);
            createAndRegisterShortcut(ActionList::instance().controlsMapViewMoveObjectsRightInfo, &MapViewBase::OnMoveObjectsRight);
            createAndRegister2D3DShortcut(ActionList::instance().controlsMapViewMoveObjectsBackwardUupInfo, &MapViewBase::OnMoveObjectsBackward, &MapViewBase::OnMoveObjectsUp);
            createAndRegister2D3DShortcut(ActionList::instance().controlsMapViewMoveObjectsForwardDownInfo, &MapViewBase::OnMoveObjectsForward, &MapViewBase::OnMoveObjectsDown);

            // object rotation
            createAndRegisterShortcut(ActionList::instance().controlsMapViewRollObjectsClockwiseInfo, &MapViewBase::OnRollObjectsCW);
            createAndRegisterShortcut(ActionList::instance().controlsMapViewRollObjectsCounterClockwiseInfo, &MapViewBase::OnRollObjectsCCW);
            createAndRegisterShortcut(ActionList::instance().controlsMapViewYawObjectsClockwiseInfo, &MapViewBase::OnYawObjectsCW);
            createAndRegisterShortcut(ActionList::instance().controlsMapViewYawObjectsCounterClockwiseInfo, &MapViewBase::OnYawObjectsCCW);
            createAndRegisterShortcut(ActionList::instance().controlsMapViewPitchobjectsClockwiseInfo, &MapViewBase::OnPitchObjectsCW);
            createAndRegisterShortcut(ActionList::instance().controlsMapViewPitchobjectsCounterClockwiseInfo, &MapViewBase::OnPitchObjectsCCW);

            // flips
            createAndRegisterShortcut(ActionList::instance().controlsMapViewFlipobjectsHorizontallyInfo, &MapViewBase::OnFlipObjectsH);
            createAndRegisterShortcut(ActionList::instance().controlsMapViewFlipobjectsBerticallyInfo, &MapViewBase::OnFlipObjectsV);

            // duplicate
            createAndRegister2D3DShortcut(ActionList::instance().controlsMapViewDuplicateAndMoveObjectsUpForwardInfo, &MapViewBase::OnDuplicateObjectsUp, &MapViewBase::OnDuplicateObjectsForward);
            createAndRegister2D3DShortcut(ActionList::instance().controlsMapViewDuplicateAndMoveObjectsDownBackwardInfo, &MapViewBase::OnDuplicateObjectsDown, &MapViewBase::OnDuplicateObjectsBackward);
            createAndRegisterShortcut(ActionList::instance().controlsMapViewDuplicateAndMoveObjectsLeftInfo, &MapViewBase::OnDuplicateObjectsLeft);
            createAndRegisterShortcut(ActionList::instance().controlsMapViewDuplicateAndMoveObjectsRightInfo, &MapViewBase::OnDuplicateObjectsRight);
            createAndRegister2D3DShortcut(ActionList::instance().controlsMapViewDuplicateAndMoveObjectsBackwardUpInfo, &MapViewBase::OnDuplicateObjectsBackward, &MapViewBase::OnDuplicateObjectsUp);
            createAndRegister2D3DShortcut(ActionList::instance().controlsMapViewDuplicateAndMoveObjectsForwardDownInfo, &MapViewBase::OnDuplicateObjectsForward, &MapViewBase::OnDuplicateObjectsDown);

            // move rotation center
            createAndRegister2D3DShortcut(ActionList::instance().controlsMapViewMoveRotationCenterUpForwardInfo, &MapViewBase::OnMoveRotationCenterUp, &MapViewBase::OnMoveRotationCenterForward);
            createAndRegister2D3DShortcut(ActionList::instance().controlsMapViewMoveRotationCenterDownBackwardInfo, &MapViewBase::OnMoveRotationCenterDown, &MapViewBase::OnMoveRotationCenterBackward);
            createAndRegisterShortcut(ActionList::instance().controlsMapViewMoveRotationCenterLeftInfo, &MapViewBase::OnMoveRotationCenterLeft); //,       this, CommandIds::Actions::MoveRotationCenterLeft);
            createAndRegisterShortcut(ActionList::instance().controlsMapViewMoveRotationCenterRightInfo, &MapViewBase::OnMoveRotationCenterRight); //,      this, CommandIds::Actions::MoveRotationCenterRight);
            createAndRegister2D3DShortcut(ActionList::instance().controlsMapViewMoveRotationCenterBackwardUpInfo, &MapViewBase::OnMoveRotationCenterBackward, &MapViewBase::OnMoveRotationCenterUp); //,         this, CommandIds::Actions::MoveRotationCenterUp);
            createAndRegister2D3DShortcut(ActionList::instance().controlsMapViewMoveRotationCenterForwardDownInfo, &MapViewBase::OnMoveRotationCenterForward, &MapViewBase::OnMoveRotationCenterDown);

            // current tool actions
            createAndRegisterShortcut(ActionList::instance().controlsMapViewCancelInfo, &MapViewBase::OnCancel);
            createAndRegisterShortcut(ActionList::instance().controlsMapViewDeactivatecurrenttoolInfo, &MapViewBase::OnDeactivateTool);
        }

        void MapViewBase::registerBinding(QShortcut* action, const ActionInfo& info) {
            m_actionInfoList.emplace_back(std::make_pair(action, &info));
        }

        void MapViewBase::updateBindings() {
            qDebug("updating key binds");

            // refresh key bindings, start with all shortcuts enabled
            for (auto [shortcut, menuInfo] : m_actionInfoList) {
                shortcut->setKey(menuInfo->defaultKey);
                shortcut->setEnabled(true);
            }

            // Disable shortcuts that are for the wrong action view (2D or 3D)
            if (doGetActionView() != ActionView_Map2D) {
                for (auto* shortcut : m_2DOnlyShortcuts) {
                    shortcut->setEnabled(false);
                }
            }
            if (doGetActionView() != ActionView_Map3D) {
                for (auto* shortcut : m_3DOnlyShortcuts) {
                    shortcut->setEnabled(false);
                }
            }

            // Disable shortcuts that are in the wrong action context
            const auto ourActionContext = actionContext();
            for (auto [shortcut, menuInfo] : m_actionInfoList) {
                const auto shortcutActionContext = menuInfo->actionContext;
                if (0 == (ourActionContext & shortcutActionContext)) {
                    shortcut->setEnabled(false);
                }

                qDebug("Updated shortcut (%s, pref path: %s) to: %s",
                       shortcut->key().toString(QKeySequence::PortableText).toStdString().c_str(),
                       menuInfo->preferencePath.asString().c_str(),
                       shortcut->isEnabled() ? "enabled" : "disabled");
            }
        }

        void MapViewBase::OnMoveObjectsForward() {
            moveObjects(vm::direction::forward);
        }

        void MapViewBase::OnMoveObjectsBackward() {
            moveObjects(vm::direction::backward);
        }

        void MapViewBase::OnMoveObjectsLeft() {
            moveObjects(vm::direction::left);
        }

        void MapViewBase::OnMoveObjectsRight() {
            moveObjects(vm::direction::right);
        }

        void MapViewBase::OnMoveObjectsUp() {
            moveObjects(vm::direction::up);
        }

        void MapViewBase::OnMoveObjectsDown() {
            moveObjects(vm::direction::down);
        }

        void MapViewBase::OnDuplicateObjectsForward() {
            duplicateAndMoveObjects(vm::direction::forward);
        }

        void MapViewBase::OnDuplicateObjectsBackward() {
            duplicateAndMoveObjects(vm::direction::backward);
        }

        void MapViewBase::OnDuplicateObjectsLeft() {
            duplicateAndMoveObjects(vm::direction::left);
        }

        void MapViewBase::OnDuplicateObjectsRight() {
            duplicateAndMoveObjects(vm::direction::right);
        }

        void MapViewBase::OnDuplicateObjectsUp() {
            duplicateAndMoveObjects(vm::direction::up);
        }

        void MapViewBase::OnDuplicateObjectsDown() {
            duplicateAndMoveObjects(vm::direction::down);
        }

        void MapViewBase::OnRollObjectsCW() {
            rotateObjects(vm::rotation_axis::roll, true);
        }

        void MapViewBase::OnRollObjectsCCW() {
            rotateObjects(vm::rotation_axis::roll, false);
        }

        void MapViewBase::OnPitchObjectsCW() {
            rotateObjects(vm::rotation_axis::pitch, true);
        }

        void MapViewBase::OnPitchObjectsCCW() {
            rotateObjects(vm::rotation_axis::pitch, false);
        }

        void MapViewBase::OnYawObjectsCW() {
            rotateObjects(vm::rotation_axis::yaw, true);
        }

        void MapViewBase::OnYawObjectsCCW() {
            rotateObjects(vm::rotation_axis::yaw, false);
        }

        void MapViewBase::OnFlipObjectsH() {
            flipObjects(vm::direction::left);
        }

        void MapViewBase::OnFlipObjectsV() {
            flipObjects(vm::direction::up);
        }

        void MapViewBase::duplicateAndMoveObjects(const vm::direction direction) {
            Transaction transaction(m_document);
            duplicateObjects();
            moveObjects(direction);
        }

        void MapViewBase::duplicateObjects() {
            MapDocumentSPtr document = lock(m_document);
            if (!document->hasSelectedNodes())
                return;

            document->duplicateObjects();
        }

        void MapViewBase::moveObjects(const vm::direction direction) {
            MapDocumentSPtr document = lock(m_document);
            if (!document->hasSelectedNodes())
                return;

            const Grid& grid = document->grid();
            const vm::vec3 delta = moveDirection(direction) * static_cast<FloatType>(grid.actualSize());
            document->translateObjects(delta);
        }

        vm::vec3 MapViewBase::moveDirection(const vm::direction direction) const {
            return doGetMoveDirection(direction);
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

        void MapViewBase::OnToggleRotateObjectsTool() {
            m_toolBox.toggleRotateObjectsTool();
        }

        void MapViewBase::OnToggleScaleObjectsTool() {
            m_toolBox.toggleScaleObjectsTool();
        }

        void MapViewBase::OnToggleShearObjectsTool() {
            m_toolBox.toggleShearObjectsTool();
        }

        void MapViewBase::OnMoveRotationCenterForward() {
            moveRotationCenter(vm::direction::forward);
        }

        void MapViewBase::OnMoveRotationCenterBackward() {
            moveRotationCenter(vm::direction::backward);
        }

        void MapViewBase::OnMoveRotationCenterLeft() {
            moveRotationCenter(vm::direction::left);
        }

        void MapViewBase::OnMoveRotationCenterRight() {
            moveRotationCenter(vm::direction::right);
        }

        void MapViewBase::OnMoveRotationCenterUp() {
            moveRotationCenter(vm::direction::up);
        }

        void MapViewBase::OnMoveRotationCenterDown() {
            moveRotationCenter(vm::direction::down);
        }

        void MapViewBase::moveRotationCenter(const vm::direction direction) {
            MapDocumentSPtr document = lock(m_document);
            const Grid& grid = document->grid();
            const vm::vec3 delta = moveDirection(direction) * static_cast<FloatType>(grid.actualSize());
            m_toolBox.moveRotationCenter(delta);
            update();
        }

        void MapViewBase::OnToggleClipSide() {
            m_toolBox.toggleClipSide();
        }

        void MapViewBase::OnPerformClip() {
            m_toolBox.performClip();
        }

        void MapViewBase::OnMoveVerticesForward() {
            moveVertices(vm::direction::forward);
        }

        void MapViewBase::OnMoveVerticesBackward() {
            moveVertices(vm::direction::backward);
        }

        void MapViewBase::OnMoveVerticesLeft() {
            moveVertices(vm::direction::left);
        }

        void MapViewBase::OnMoveVerticesRight() {
            moveVertices(vm::direction::right);
        }

        void MapViewBase::OnMoveVerticesUp() {
            moveVertices(vm::direction::up);
        }

        void MapViewBase::OnMoveVerticesDown() {
            moveVertices(vm::direction::down);
        }

        void MapViewBase::moveVertices(const vm::direction direction) {
            MapDocumentSPtr document = lock(m_document);
            const Grid& grid = document->grid();
            const vm::vec3 delta = moveDirection(direction) * static_cast<FloatType>(grid.actualSize());
            m_toolBox.moveVertices(delta);
        }

        void MapViewBase::OnCancel() {
            if (MapViewBase::cancel())
                return;
            if (ToolBoxConnector::cancel())
                return;
            
            MapDocumentSPtr document = lock(m_document);
            if (document->hasSelection()) {
                document->deselectAll();
            } else if (document->currentGroup() != nullptr) {
                document->closeGroup();
            }
        }

        bool MapViewBase::cancel() {
            return doCancel();
        }

        void MapViewBase::OnDeactivateTool() {
            m_toolBox.deactivateAllTools();
        }

        void MapViewBase::OnGroupSelectedObjects() {
            MapDocumentSPtr document = lock(m_document);
            if (document->hasSelectedNodes()) {
                const String name = queryGroupName(this->widgetContainer());
                if (!name.empty())
                    document->groupSelection(name);
            }
        }

        void MapViewBase::OnUngroupSelectedObjects() {
            MapDocumentSPtr document = lock(m_document);
            if (document->hasSelectedNodes() && document->selectedNodes().hasOnlyGroups())
                document->ungroupSelection();
        }

        void MapViewBase::OnRenameGroups() {
            MapDocumentSPtr document = lock(m_document);
            assert(document->selectedNodes().hasOnlyGroups());
            const String name = queryGroupName(this->widgetContainer());
            if (!name.empty())
                document->renameGroups(name);
        }

        void MapViewBase::OnCreatePointEntity() {
            auto* action = qobject_cast<const QAction*>(sender());
            MapDocumentSPtr document = lock(m_document);
            const size_t index = action->data().toUInt();
            const Assets::EntityDefinition* definition = findEntityDefinition(Assets::EntityDefinition::Type_PointEntity, index);
            ensure(definition != nullptr, "definition is null");
            assert(definition->type() == Assets::EntityDefinition::Type_PointEntity);
            createPointEntity(static_cast<const Assets::PointEntityDefinition*>(definition));
        }

        void MapViewBase::OnCreateBrushEntity() {
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

#if 0
        void MapViewBase::OnActivateFrame(wxActivateEvent& event) {
            if (event.GetActive())
                updateLastActivation();
            event.Skip();
        }
#endif

        ActionContext MapViewBase::actionContext() const {
            const ActionContext derivedContext = doGetActionContext();
            if (derivedContext != ActionContext_Default)
                return derivedContext;

            if (m_toolBox.createComplexBrushToolActive())
                return ActionContext_CreateComplexBrushTool;
            if (m_toolBox.clipToolActive())
                return ActionContext_ClipTool;
            if (m_toolBox.anyVertexToolActive())
                return ActionContext_AnyVertexTool;
            if (m_toolBox.rotateObjectsToolActive())
                return ActionContext_RotateTool;
            if (m_toolBox.scaleObjectsToolActive())
                return ActionContext_ScaleTool;
            if (m_toolBox.shearObjectsToolActive())
                return ActionContext_ShearTool;

            MapDocumentSPtr document = lock(m_document);
            if (document->hasSelectedNodes())
                return ActionContext_NodeSelection;
            if (document->hasSelectedBrushFaces())
                return ActionContext_FaceSelection;
            return ActionContext_Default;
        }

        void MapViewBase::doFlashSelection() {
            FlashSelectionAnimation* animation = new FlashSelectionAnimation(m_renderer, this->widgetContainer(), 180);
            m_animationManager->runAnimation(animation, true);
        }

        bool MapViewBase::doGetIsCurrent() const {
            return HasFocus();
        }
        
        void MapViewBase::doSetToolBoxDropTarget() {
            // FIXME: DND
            //SetDropTarget(new ToolBoxDropTarget(this));
        }
        
        void MapViewBase::doClearDropTarget() {
            // FIXME: DND
            //SetDropTarget(nullptr);
        }

        bool MapViewBase::doCanFlipObjects() const {
            MapDocumentSPtr document = lock(m_document);
            return !m_toolBox.anyToolActive() && document->hasSelectedNodes();
        }

        void MapViewBase::doFlipObjects(const vm::direction direction) {
            auto document = lock(m_document);
            if (!document->hasSelectedNodes())
                return;

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

        bool MapViewBase::doCancelMouseDrag() {
            return ToolBoxConnector::cancelDrag();
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
            if (!doBeforePopupMenu())
                return;

            MapDocumentSPtr document = lock(m_document);
            const Model::NodeList& nodes = document->selectedNodes().nodes();
            Model::Node* newBrushParent = findNewParentEntityForBrushes(nodes);
            Model::Node* currentGroup = document->editorContext().currentGroup();
            Model::Node* newGroup = findNewGroupForObjects(nodes);
            Model::Node* mergeGroup = findGroupToMergeGroupsInto(document->selectedNodes());
            
            QMenu menu;
            QAction* groupAction = menu.addAction(tr("Group"), this, &MapViewBase::OnGroupSelectedObjects);
            groupAction->setEnabled(canGroupObjects());

            QAction* ungroupAction = menu.addAction(tr("Ungroup"), this, &MapViewBase::OnUngroupSelectedObjects);
            ungroupAction->setEnabled(canUngroupObjects());

            QAction* mergeGroupAction = nullptr;
            if (mergeGroup != nullptr) {
                mergeGroupAction = menu.addAction(tr("Merge Groups into %1").arg(QString::fromStdString(mergeGroup->name())), this, &MapViewBase::OnMergeGroups);
            } else {
                mergeGroupAction = menu.addAction(tr("Merge Groups"), this, &MapViewBase::OnMergeGroups);
            }
            mergeGroupAction->setEnabled(canMergeGroups());

            QAction* renameAction = menu.addAction(tr("Rename"), this, &MapViewBase::OnRenameGroups);
            renameAction->setEnabled(canRenameGroups());

            if (newGroup != nullptr && newGroup != currentGroup) {
                menu.addAction(tr("Add Objects to Group %1").arg(QString::fromStdString(newGroup->name())), this, &MapViewBase::OnAddObjectsToGroup);
            }
            if (currentGroup != nullptr && !document->selectedNodes().empty()) {
                menu.addAction(tr("Remove Objects from Group %1").arg(QString::fromStdString(currentGroup->name())), this, &MapViewBase::OnRemoveObjectsFromGroup);
            }
            menu.addSeparator();
            
            menu.addMenu(makeEntityGroupsMenu(Assets::EntityDefinition::Type_PointEntity));
            menu.addMenu(makeEntityGroupsMenu(Assets::EntityDefinition::Type_BrushEntity));
            
            if (document->selectedNodes().hasOnlyBrushes()) {
                if (!isEntity(newBrushParent)) {
                    QAction* moveToWorldAction = menu.addAction(tr("Move Brushes to World"), this, &MapViewBase::OnMoveBrushesTo);
                    moveToWorldAction->setEnabled(canMoveBrushesToWorld());
                } else {
                    menu.addAction(tr("Move Brushes to Entity %1").arg(QString::fromStdString(newBrushParent->name())), this, &MapViewBase::OnMoveBrushesTo);
                }
            }

            menu.exec(QCursor::pos());

            doAfterPopupMenu();
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
                                action = groupMenu->addAction(label, this, &MapViewBase::OnCreatePointEntity);
                                break;
                            }
                            case Assets::EntityDefinition::Type_BrushEntity: {
                                action = groupMenu->addAction(label, this, &MapViewBase::OnCreateBrushEntity);
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

        void MapViewBase::OnAddObjectsToGroup() {
            MapDocumentSPtr document = lock(m_document);
            const Model::NodeList nodes = document->selectedNodes().nodes();
            Model::Node* newGroup = findNewGroupForObjects(nodes);
            ensure(newGroup != nullptr, "newGroup is null");
            
            Transaction transaction(document, "Add Objects to Group");
            reparentNodes(nodes, newGroup, true);
            document->deselectAll();
            document->select(newGroup);
        }
        
        void MapViewBase::OnRemoveObjectsFromGroup() {
            MapDocumentSPtr document = lock(m_document);
            const Model::NodeList nodes = document->selectedNodes().nodes();
            Model::Node* currentGroup = document->editorContext().currentGroup();
            ensure(currentGroup != nullptr, "currentGroup is null");
            
            Transaction transaction(document, "Remove Objects from Group");
            if (currentGroup->childCount() == nodes.size())
                document->closeGroup();
            reparentNodes(nodes, document->currentLayer(), true);
        }

        Model::Node* MapViewBase::findNewGroupForObjects(const Model::NodeList& nodes) const {
            Model::Node* newGroup = nullptr;
            
            MapDocumentSPtr document = lock(m_document);
            const Model::Hit& hit = pickResult().query().pickable().type(Model::Group::GroupHit).first();
            if (hit.isMatch())
                newGroup = Model::hitToNode(hit);
            
            if (newGroup != nullptr && canReparentNodes(nodes, newGroup))
                return newGroup;
            return nullptr;
        }
        
        void MapViewBase::OnMergeGroups() {
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
            const Model::Hit& hit = pickResult().query().pickable().type(Model::Group::GroupHit).first();
            if (hit.isMatch()) {
                mergeTarget = Model::hitToGroup(hit);
            } else {
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
        
        void MapViewBase::OnMoveBrushesTo() {
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
            
            if (newParent != nullptr && newParent != document->world() && canReparentNodes(nodes, newParent))
                return newParent;
            
            if (!nodes.empty()) {
                Model::Node* lastNode = nodes.back();
                
                Model::Group* group = Model::findGroup(lastNode);
                if (group != nullptr)
                    return group;
                
                Model::Layer* layer = Model::findLayer(lastNode);
                if (layer != nullptr)
                    return layer;
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
            typedef Model::CollectMatchingNodesVisitor<BrushesToEntities, Model::UniqueNodeCollectionStrategy, Model::StopRecursionIfMatched> BrushesToEntitiesVisitor;
            
            BrushesToEntitiesVisitor collect(world);
            Model::Node::acceptAndEscalate(std::begin(selectedNodes), std::end(selectedNodes), collect);
            return collect.nodes();
        }

        void MapViewBase::reparentNodes(const Model::NodeList& nodes, Model::Node* newParent, const bool preserveEntities) {
            ensure(newParent != nullptr, "newParent is null");

            MapDocumentSPtr document = lock(m_document);
            Model::PushSelection pushSelection(document);
            
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
        }

        Model::NodeList MapViewBase::collectReparentableNodes(const Model::NodeList& nodes, const Model::Node* newParent) const {
            Model::NodeList result;
            std::copy_if(std::begin(nodes), std::end(nodes), std::back_inserter(result), [=](const Model::Node* node) { return newParent != node && newParent != node->parent() && !newParent->isDescendantOf(node); });
            return result;
        }

        bool MapViewBase::canGroupObjects() const {
            MapDocumentSPtr document = lock(m_document);
            return !document->selectedNodes().empty();
        }
    
        bool MapViewBase::canUngroupObjects() const {
            MapDocumentSPtr document = lock(m_document);
            return document->selectedNodes().hasOnlyGroups();
        }
        
        bool MapViewBase::canMergeGroups() const {
            MapDocumentSPtr document = lock(m_document);
            Model::Node* mergeGroup = findGroupToMergeGroupsInto(document->selectedNodes());
            return mergeGroup != nullptr;
        }
        
        bool MapViewBase::canRenameGroups() const {
            MapDocumentSPtr document = lock(m_document);
            return document->selectedNodes().hasOnlyGroups();
        }

        bool MapViewBase::canMoveBrushesToWorld() const {
            MapDocumentSPtr document = lock(m_document);
            const Model::NodeList& nodes = document->selectedNodes().nodes();
            Model::Node* newBrushParent = findNewParentEntityForBrushes(nodes);
            return !isEntity(newBrushParent)
                && !collectReparentableNodes(nodes, newBrushParent).empty();
        }

        void MapViewBase::doPreRender() {}

        void MapViewBase::doRenderExtras(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {}

        bool MapViewBase::doBeforePopupMenu() { return true; }
        void MapViewBase::doAfterPopupMenu() {}
    }
}
