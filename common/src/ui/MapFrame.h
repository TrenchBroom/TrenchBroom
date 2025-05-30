/*
 Copyright (C) 2010 Kristian Duske

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

#pragma once

#include <QDialog>
#include <QMainWindow>
#include <QPointer>

#include "NotifierConnection.h"
#include "Result.h"
#include "io/ExportOptions.h"
#include "mdl/MapFormat.h"
#include "ui/Selection.h"

#include <chrono>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

class QAction;
class QComboBox;
class QDialog;
class QDropEvent;
class QMenuBar;
class QLabel;
class QSplitter;
class QTimer;
class QToolBar;

namespace tb
{
class Logger;
}

namespace tb::mdl
{
class Game;
class GroupNode;
class LayerNode;
class Material;
} // namespace tb::mdl

namespace tb::ui
{
class Action;
class Autosaver;
class Console;
class FrameManager;
class GLContextManager;
class InfoPanel;
class Inspector;
enum class InspectorPage;
class MapDocument;
class MapViewBase;
class ObjExportDialog;
enum class PasteType;
class SignalDelayer;
class SwitchableMapViewContainer;
class Tool;

class MapFrame : public QMainWindow
{
  Q_OBJECT
private:
  FrameManager& m_frameManager;
  std::shared_ptr<MapDocument> m_document;

  std::chrono::time_point<std::chrono::system_clock> m_lastInputTime;
  std::unique_ptr<Autosaver> m_autosaver;
  QTimer* m_autosaveTimer = nullptr;
  QTimer* m_processResourcesTimer = nullptr;

  QToolBar* m_toolBar = nullptr;

  QSplitter* m_hSplitter = nullptr;
  QSplitter* m_vSplitter = nullptr;

  std::unique_ptr<GLContextManager> m_contextManager;
  SwitchableMapViewContainer* m_mapView = nullptr;
  /**
   * Last focused MapViewBase. It's a QPointer to handle changing from e.g. a 2-pane map
   * view to 1-pane.
   */
  QPointer<MapViewBase> m_currentMapView;
  InfoPanel* m_infoPanel = nullptr;
  Console* m_console = nullptr;
  Inspector* m_inspector = nullptr;

  QComboBox* m_gridChoice = nullptr;
  QLabel* m_statusBarLabel = nullptr;

  QPointer<QDialog> m_compilationDialog;
  QPointer<ObjExportDialog> m_objExportDialog;

  NotifierConnection m_notifierConnection;

private: // shortcuts
  using ActionMap = std::unordered_map<const Action*, QAction*>;
  ActionMap m_actionMap;

private: // special menu entries
  QMenu* m_recentDocumentsMenu;
  QAction* m_undoAction;
  QAction* m_redoAction;

private:
  SignalDelayer* m_updateTitleSignalDelayer = nullptr;
  SignalDelayer* m_updateActionStateSignalDelayer = nullptr;
  SignalDelayer* m_updateStatusBarSignalDelayer = nullptr;

public:
  MapFrame(FrameManager& frameManager, std::shared_ptr<MapDocument> document);
  ~MapFrame() override;

  void positionOnScreen(QWidget* reference);
  std::shared_ptr<MapDocument> document() const;

public: // getters and such
  Logger& logger() const;
  QAction* findAction(const std::filesystem::path& path);

private: // title bar contents
  void updateTitle();
  void updateTitleDelayed();

private: // menu bar
  void createMenus();
  void updateShortcuts();
  void updateActionState();
  void updateActionStateDelayed();
  void updateUndoRedoActions();

  void addRecentDocumentsMenu();
  void removeRecentDocumentsMenu();
  void updateRecentDocumentsMenu();

private: // tool bar
  void createToolBar();
  void updateToolBarWidgets();

private: // status bar
  void createStatusBar();
  void updateStatusBar();
  void updateStatusBarDelayed();

private: // gui creation
  void createGui();

private: // notification handlers
  void connectObservers();

  void documentWasCleared(ui::MapDocument* document);
  void documentDidChange(ui::MapDocument* document);
  void documentModificationStateDidChange();

  void transactionDone(const std::string&);
  void transactionUndone(const std::string&);

  void preferenceDidChange(const std::filesystem::path& path);
  void gridDidChange();
  void toolActivated(Tool& tool);
  void toolDeactivated(Tool& tool);
  void toolHandleSelectionChanged(Tool& tool);
  void selectionDidChange(const Selection& selection);
  void currentLayerDidChange(const tb::mdl::LayerNode* layer);
  void groupWasOpened(mdl::GroupNode* group);
  void groupWasClosed(mdl::GroupNode* group);
  void nodeVisibilityDidChange(const std::vector<mdl::Node*>& nodes);
  void editorContextDidChange();
  void pointFileDidChange();
  void portalFileDidChange();

private: // menu event handlers
  void bindEvents();

public:
  Result<bool> newDocument(std::shared_ptr<mdl::Game> game, mdl::MapFormat mapFormat);
  Result<bool> openDocument(
    std::shared_ptr<mdl::Game> game,
    mdl::MapFormat mapFormat,
    const std::filesystem::path& path);
  bool saveDocument();
  bool saveDocumentAs();
  void revertDocument();
  bool exportDocumentAsObj();
  bool exportDocumentAsMap();
  bool exportDocument(const io::ExportOptions& options);

private:
  bool confirmOrDiscardChanges();
  bool confirmRevertDocument();

public:
  void loadPointFile();
  void reloadPointFile();
  void unloadPointFile();
  bool canReloadPointFile() const;
  bool canUnloadPortalFile() const;

  void loadPortalFile();
  void reloadPortalFile();
  void unloadPortalFile();
  bool canReloadPortalFile() const;
  bool canUnloadPointFile() const;

  void reloadMaterialCollections();
  void reloadEntityDefinitions();
  void closeDocument();

  void undo();
  void redo();
  bool canUndo() const;
  bool canRedo() const;

  void repeatLastCommands();
  void clearRepeatableCommands();
  bool hasRepeatableCommands() const;

  void cutSelection();
  void copySelection();
  void copyToClipboard();
  bool canCutSelection() const;
  bool canCopySelection() const;

  void pasteAtCursorPosition();
  void pasteAtOriginalPosition();
  PasteType paste();
  bool canPaste() const;

  void duplicateSelection();
  bool canDuplicateSelectino() const;

  void deleteSelection();
  bool canDeleteSelection() const;

  void selectAll();
  void selectSiblings();
  void selectTouching();
  void selectInside();
  void selectTall();
  void selectByLineNumber();
  void selectInverse();
  void selectNone();

  bool canSelect() const;
  bool canSelectSiblings() const;
  bool canSelectByBrush() const;
  bool canSelectTall() const;
  bool canDeselect() const;
  bool canChangeSelection() const;
  bool canSelectInverse() const;

  void groupSelectedObjects();
  bool canGroupSelectedObjects() const;

  void ungroupSelectedObjects();
  bool canUngroupSelectedObjects() const;

  void renameSelectedGroups();
  bool canRenameSelectedGroups() const;

  void moveSelectedObjects();
  bool canMoveSelectedObjects() const;

  bool anyModalToolActive() const;

  void toggleAssembleBrushTool();
  bool canToggleAssembleBrushTool() const;
  bool assembleBrushToolActive() const;

  void toggleClipTool();
  bool canToggleClipTool() const;
  bool clipToolActive() const;

  void toggleRotateTool();
  bool canToggleRotateTool() const;
  bool rotateToolActive() const;

  void toggleScaleTool();
  bool canToggleScaleTool() const;
  bool scaleToolActive() const;

  void toggleShearTool();
  bool canToggleShearTool() const;
  bool shearToolActive() const;

  bool anyVertexToolActive() const;

  void toggleVertexTool();
  bool canToggleVertexTool() const;
  bool vertexToolActive() const;

  void toggleEdgeTool();
  bool canToggleEdgeTool() const;
  bool edgeToolActive() const;

  void toggleFaceTool();
  bool canToggleFaceTool() const;
  bool faceToolActive() const;

  void csgConvexMerge();
  bool canDoCsgConvexMerge() const;

  void csgSubtract();
  bool canDoCsgSubtract() const;

  void csgHollow();
  bool canDoCsgHollow() const;

  void csgIntersect();
  bool canDoCsgIntersect() const;

  void snapVerticesToInteger();
  void snapVerticesToGrid();
  bool canSnapVertices() const;

  void replaceMaterial();

  void toggleAlignmentLock();
  void toggleUVLock();

  void toggleShowGrid();
  void toggleSnapToGrid();

  void incGridSize();
  bool canIncGridSize() const;

  void decGridSize();
  bool canDecGridSize() const;

  void setGridSize(int size);

  void moveCameraToNextPoint();
  bool canMoveCameraToNextPoint() const;

  void moveCameraToPreviousPoint();
  bool canMoveCameraToPreviousPoint() const;

  void reset2dCameras();

  void focusCameraOnSelection();
  bool canFocusCamera() const;

  void moveCameraToPosition();

  void isolateSelection();
  bool canIsolateSelection() const;

  void hideSelection();
  bool canHideSelection() const;

  void showAll();

  void switchToInspectorPage(InspectorPage page);

  void toggleToolbar();
  bool toolbarVisible() const;

  void toggleInfoPanel();
  bool infoPanelVisible() const;

  void toggleInspector();
  bool inspectorVisible() const;

  void toggleMaximizeCurrentView();
  bool currentViewMaximized() const;

  void showCompileDialog();
  bool closeCompileDialog();

  void showLaunchEngineDialog();

  bool canRevealMaterial() const;
  void revealMaterial();

  void revealMaterial(const mdl::Material* material);

  void debugPrintVertices();
  void debugCreateBrush();
  void debugCreateCube();
  void debugClipBrush();
  void debugCrash();
  void debugThrowExceptionDuringCommand();
  void debugSetWindowSize();
  void debugShowPalette();

  void focusChange(QWidget* oldFocus, QWidget* newFocus);

  MapViewBase* currentMapViewBase();

private:
  bool canCompile() const;
  bool canLaunch() const;

public: // drag and drop
  void dragEnterEvent(QDragEnterEvent* event) override;
  void dropEvent(QDropEvent* event) override;

protected: // other event handlers
  void changeEvent(QEvent* event) override;
  void closeEvent(QCloseEvent* event) override;

public: // event filter (suppress autosave for user input events)
  bool eventFilter(QObject* target, QEvent* event) override;

private:
  void triggerAutosave();
  void triggerProcessResources();
};

class DebugPaletteWindow : public QDialog
{
  Q_OBJECT
public:
  explicit DebugPaletteWindow(QWidget* parent = nullptr);
  ~DebugPaletteWindow() override;
};

} // namespace tb::ui
