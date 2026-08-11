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

#include "ui/ActionManager.h"

#include <QtSystemDetection>

#include "base/PreferenceManager.h"
#include "mdl/EntityDefinition.h"
#include "mdl/EntityProperties.h"
#include "mdl/Grid.h" // IWYU pragma: keep
#include "mdl/Map.h"
#include "mdl/Map_Entities.h"
#include "mdl/Map_Groups.h"    // IWYU pragma: keep
#include "mdl/Map_Selection.h" // IWYU pragma: keep
#include "mdl/Tag.h"
#include "prefs/Preferences.h"
#include "ui/ActionExecutionContext.h"
#include "ui/AppController.h" // IWYU pragma: keep
#include "ui/Inspector.h"
#include "ui/MapView.h"
#include "ui/MapViewBase.h"
#include "ui/MapViewToolBox.h" // IWYU pragma: keep
#include "ui/MapWindow.h"
#include "ui/StandardShortcut.h"

#include "kd/contracts.h"

#include "vm/util.h"

#include <fmt/format.h>

#include <string>

namespace tb::ui
{

ActionManager::ActionManager()
{
  initialize();
}

std::vector<Action> ActionManager::createTagActions(
  const std::vector<mdl::SmartTag>& tags) const
{
  std::vector<Action> result;

  for (const auto& tag : tags)
  {
    result.emplace_back(
      std::filesystem::path{"Filters/Tags/" + tag.name() + "/Toggle Visible"},
      fmt::format("Toggle {} visible", tag.name()),
      ActionContext::Any,
      [&tag](auto& context) { context.mapView().toggleTagVisible(tag); },
      [](const auto& context) { return context.hasDocument(); });
    if (tag.canEnable())
    {
      result.emplace_back(
        std::filesystem::path{"Tags/" + tag.name() + "/Enable"},
        fmt::format("Turn Selection into {}", tag.name()),
        ActionContext::AnyView | ActionContext::AnySelection | ActionContext::AnyOrNoTool,
        [&tag](auto& context) { context.mapView().enableTag(tag); },
        [](const auto& context) { return context.hasDocument(); });
    }
    if (tag.canDisable())
    {
      result.emplace_back(
        std::filesystem::path{"Tags/" + tag.name() + "/Disable"},
        fmt::format("Turn Selection into non-{}", tag.name()),
        ActionContext::AnyView | ActionContext::AnySelection | ActionContext::AnyOrNoTool,
        [&tag](auto& context) { context.mapView().disableTag(tag); },
        [](const auto& context) { return context.hasDocument(); });
    }
  }

  return result;
}

std::vector<Action> ActionManager::createEntityDefinitionActions(
  const std::vector<mdl::EntityDefinition>& entityDefinitions) const
{
  std::vector<Action> result;

  for (const auto& definition : entityDefinitions)
  {
    result.emplace_back(
      std::filesystem::path{"Entities/" + definition.name + "/Toggle"},
      fmt::format("Toggle {} visible", definition.name),
      ActionContext::Any,
      [&](auto& context) { context.mapView().toggleEntityDefinitionVisible(definition); },
      [](const auto& context) { return context.hasDocument(); });
    if (definition.name != mdl::EntityPropertyValues::WorldspawnClassname)
    {
      result.emplace_back(
        std::filesystem::path{"Entities/" + definition.name + "/Create"},
        fmt::format("Create {}", definition.name),
        ActionContext::Any,
        [&](auto& context) { context.mapView().createEntity(definition); },
        [](const auto& context) { return context.hasDocument(); });
    }
  }

  return result;
}

const std::unordered_map<std::filesystem::path, Action, kdl::path_hash>& ActionManager::
  actionsMap() const
{
  return m_actions;
}

void ActionManager::resetAllKeySequences()
{
  auto& prefs = PreferenceManager::instance();

  const auto resetVisitor = kdl::overload(
    [](MenuSeparator&) {},
    [&](MenuAction& actionItem) { prefs.resetToDefault(actionItem.action.preference()); },
    [](auto& thisLambda, Menu& menu) { menu.visitEntries(thisLambda); });

  visitMainMenu(resetVisitor);
  visitToolBar(resetVisitor);

  visitMapViewActions([&](auto& action) { prefs.resetToDefault(action.preference()); });
}

void ActionManager::initialize()
{
  createViewActions();
  createMenu();
  createToolbar();
}

void ActionManager::createViewActions()
{
  /* ========== Editing Actions ========== */
  /* ========== Tool Specific Actions ========== */
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Create brush"},
    "Create Brush",
    ActionContext::View3D | ActionContext::AnyOrNoSelection
      | ActionContext::AssembleBrushTool,
    KeySequence{"Return"},
    [](auto& context) { context.mapView().assembleBrush(); },
    [](const auto& context) {
      return context.hasDocument()
             && context.mapWindow().toolBox().assembleBrushToolActive();
    },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Toggle clip side"},
    "Toggle Clip Side",
    ActionContext::AnyView | ActionContext::AnyOrNoSelection | ActionContext::ClipTool,
    KeySequence{"Ctrl+Return"},
    [](auto& context) { context.mapView().toggleClipSide(); },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().toolBox().clipToolActive();
    },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Perform clip"},
    "Perform Clip",
    ActionContext::AnyView | ActionContext::AnyOrNoSelection | ActionContext::ClipTool,
    KeySequence{"Return"},
    [](auto& context) { context.mapView().performClip(); },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().toolBox().clipToolActive();
    },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Perform sweep"},
    "Perform Sweep",
    ActionContext::AnyView | ActionContext::AnyOrNoSelection | ActionContext::SweepTool,
    KeySequence{"Return"},
    [](auto& context) { context.mapView().performSweep(); },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().toolBox().sweepToolActive();
    },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Decrease sweep scale"},
    "Decrease Sweep Scale",
    ActionContext::AnyView | ActionContext::SelectionOwnedByTool
      | ActionContext::SweepTool,
    KeySequence{"["},
    [](auto& context) { context.mapView().decreaseSweepScale(); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Increase sweep scale"},
    "Increase Sweep Scale",
    ActionContext::AnyView | ActionContext::SelectionOwnedByTool
      | ActionContext::SweepTool,
    KeySequence{"]"},
    [](auto& context) { context.mapView().increaseSweepScale(); },
    [](const auto& context) { return context.hasDocument(); },
  });

  /* ========== Translation ========== */
  // applies to objects, vertices, handles (e.g. rotation center, sweep handle)
  // these preference paths are structured like "action in 2D view; action in 3D view"
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Move objects up; Move objects forward"},
    "Move Forward",
    ActionContext::AnyView | ActionContext::NodeSelection
      | ActionContext::SelectionOwnedByTool | ActionContext::AnyNodeHandleTool
      | ActionContext::RotateTool | ActionContext::SweepTool | ActionContext::NoTool,
    KeySequence{"Up"},
    [](auto& context) { context.mapView().move(vm::direction::forward); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Move objects down; Move objects backward"},
    "Move Backward",
    ActionContext::AnyView | ActionContext::NodeSelection
      | ActionContext::SelectionOwnedByTool | ActionContext::AnyNodeHandleTool
      | ActionContext::RotateTool | ActionContext::SweepTool | ActionContext::NoTool,
    KeySequence{"Down"},
    [](auto& context) { context.mapView().move(vm::direction::backward); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Move objects left"},
    "Move Left",
    ActionContext::AnyView | ActionContext::NodeSelection
      | ActionContext::SelectionOwnedByTool | ActionContext::AnyNodeHandleTool
      | ActionContext::RotateTool | ActionContext::SweepTool | ActionContext::NoTool,
    KeySequence{"Left"},
    [](auto& context) { context.mapView().move(vm::direction::left); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Move objects right"},
    "Move Right",
    ActionContext::AnyView | ActionContext::NodeSelection
      | ActionContext::SelectionOwnedByTool | ActionContext::AnyNodeHandleTool
      | ActionContext::RotateTool | ActionContext::SweepTool | ActionContext::NoTool,
    KeySequence{"Right"},
    [](auto& context) { context.mapView().move(vm::direction::right); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Move objects backward; Move objects up"},
    "Move Up",
    ActionContext::AnyView | ActionContext::NodeSelection
      | ActionContext::SelectionOwnedByTool | ActionContext::AnyNodeHandleTool
      | ActionContext::RotateTool | ActionContext::SweepTool | ActionContext::NoTool,
    KeySequence{"PgUp"},
    [](auto& context) { context.mapView().move(vm::direction::up); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Move objects forward; Move objects down"},
    "Move Down",
    ActionContext::AnyView | ActionContext::NodeSelection
      | ActionContext::SelectionOwnedByTool | ActionContext::AnyNodeHandleTool
      | ActionContext::RotateTool | ActionContext::SweepTool | ActionContext::NoTool,
    KeySequence{"PgDown"},
    [](auto& context) { context.mapView().move(vm::direction::down); },
    [](const auto& context) { return context.hasDocument(); },
  });

  /* ========== Duplication ========== */
  // these preference paths are structured like "action in 2D view; action in 3D view"
  addAction(Action{
    std::filesystem::path(
      "Controls/Map view/Duplicate and move objects up; Duplicate and move "
      "objects forward"),
    "Duplicate and Move Forward",
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyOrNoTool,
    KeySequence{"Ctrl+Up"},
    [](auto& context) {
      context.mapView().duplicateAndMoveObjects(vm::direction::forward);
    },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path(
      "Controls/Map view/Duplicate and move objects down; Duplicate and move "
      "objects backward"),
    "Duplicate and Move Backward",
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyOrNoTool,
    KeySequence{"Ctrl+Down"},
    [](auto& context) {
      context.mapView().duplicateAndMoveObjects(vm::direction::backward);
    },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Duplicate and move objects left"},
    "Duplicate and Move Left",
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyOrNoTool,
    KeySequence{"Ctrl+Left"},
    [](auto& context) { context.mapView().duplicateAndMoveObjects(vm::direction::left); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Duplicate and move objects right"},
    "Duplicate and Move Right",
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyOrNoTool,
    KeySequence{"Ctrl+Right"},
    [](auto& context) {
      context.mapView().duplicateAndMoveObjects(vm::direction::right);
    },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path(
      "Controls/Map view/Duplicate and move objects backward; Duplicate and move "
      "objects up"),
    "Duplicate and Move Up",
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyOrNoTool,
    KeySequence{"Ctrl+PgUp"},
    [](auto& context) { context.mapView().duplicateAndMoveObjects(vm::direction::up); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path(
      "Controls/Map view/Duplicate and move objects forward; Duplicate and move "
      "objects down"),
    "Duplicate and Move Down",
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyOrNoTool,
    KeySequence{"Ctrl+PgDown"},
    [](auto& context) { context.mapView().duplicateAndMoveObjects(vm::direction::down); },
    [](const auto& context) { return context.hasDocument(); },
  });

  /* ========== Rotation ========== */
  // applies to objects, vertices, handles (e.g. rotation center), the sweep cap
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Roll objects clockwise"},
    "Roll Clockwise",
    ActionContext::AnyView | ActionContext::NodeSelection
      | ActionContext::SelectionOwnedByTool | ActionContext::RotateTool
      | ActionContext::SweepTool | ActionContext::NoTool,
    KeySequence{"Alt+Up"},
    [](auto& context) { context.mapView().rotate(vm::rotation_axis::roll, true); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Roll objects counter-clockwise"},
    "Roll Counter-clockwise",
    ActionContext::AnyView | ActionContext::NodeSelection
      | ActionContext::SelectionOwnedByTool | ActionContext::RotateTool
      | ActionContext::SweepTool | ActionContext::NoTool,
    KeySequence{"Alt+Down"},
    [](auto& context) { context.mapView().rotate(vm::rotation_axis::roll, false); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Yaw objects clockwise"},
    "Yaw Clockwise",
    ActionContext::AnyView | ActionContext::NodeSelection
      | ActionContext::SelectionOwnedByTool | ActionContext::RotateTool
      | ActionContext::SweepTool | ActionContext::NoTool,
    KeySequence{"Alt+Left"},
    [](auto& context) { context.mapView().rotate(vm::rotation_axis::yaw, true); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Yaw objects counter-clockwise"},
    "Yaw Counter-clockwise",
    ActionContext::AnyView | ActionContext::NodeSelection
      | ActionContext::SelectionOwnedByTool | ActionContext::RotateTool
      | ActionContext::SweepTool | ActionContext::NoTool,
    KeySequence{"Alt+Right"},
    [](auto& context) { context.mapView().rotate(vm::rotation_axis::yaw, false); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Pitch objects clockwise"},
    "Pitch Clockwise",
    ActionContext::AnyView | ActionContext::NodeSelection
      | ActionContext::SelectionOwnedByTool | ActionContext::RotateTool
      | ActionContext::SweepTool | ActionContext::NoTool,
    KeySequence{"Alt+PgUp"},
    [](auto& context) { context.mapView().rotate(vm::rotation_axis::pitch, true); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Pitch objects counter-clockwise"},
    "Pitch Counter-clockwise",
    ActionContext::AnyView | ActionContext::NodeSelection
      | ActionContext::SelectionOwnedByTool | ActionContext::RotateTool
      | ActionContext::SweepTool | ActionContext::NoTool,
    KeySequence{"Alt+PgDown"},
    [](auto& context) { context.mapView().rotate(vm::rotation_axis::pitch, false); },
    [](const auto& context) { return context.hasDocument(); },
  });

  /* ========== Texturing ========== */
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Move textures up"},
    "Move Textures Up",
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    KeySequence{"Up"},
    [](auto& context) {
      context.mapView().moveUv(vm::direction::up, MapViewBase::UvActionMode::Normal);
    },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Move textures up (coarse)"},
    "Move Textures Up (Coarse)",
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    KeySequence{"Shift+Up"},
    [](auto& context) {
      context.mapView().moveUv(vm::direction::up, MapViewBase::UvActionMode::Coarse);
    },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Move textures up (fine)"},
    "Move Textures Up (Fine)",
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    KeySequence{"Ctrl+Up"},
    [](auto& context) {
      context.mapView().moveUv(vm::direction::up, MapViewBase::UvActionMode::Fine);
    },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Move textures down"},
    "Move Textures Down",
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    KeySequence{"Down"},
    [](auto& context) {
      context.mapView().moveUv(vm::direction::down, MapViewBase::UvActionMode::Normal);
    },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Move textures down (coarse)"},
    "Move Textures Down (Coarse)",
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    KeySequence{"Shift+Down"},
    [](auto& context) {
      context.mapView().moveUv(vm::direction::down, MapViewBase::UvActionMode::Coarse);
    },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Move textures down (fine)"},
    "Move Textures Down (Fine)",
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    KeySequence{"Ctrl+Down"},
    [](auto& context) {
      context.mapView().moveUv(vm::direction::down, MapViewBase::UvActionMode::Fine);
    },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Move textures left"},
    "Move Textures Left",
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    KeySequence{"Left"},
    [](auto& context) {
      context.mapView().moveUv(vm::direction::left, MapViewBase::UvActionMode::Normal);
    },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Move textures left (coarse)"},
    "Move Textures Left (Coarse)",
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    KeySequence{"Shift+Left"},
    [](auto& context) {
      context.mapView().moveUv(vm::direction::left, MapViewBase::UvActionMode::Coarse);
    },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Move textures left (fine)"},
    "Move Textures Left (Fine)",
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    KeySequence{"Ctrl+Left"},
    [](auto& context) {
      context.mapView().moveUv(vm::direction::left, MapViewBase::UvActionMode::Fine);
    },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Move textures right"},
    "Move Textures Right",
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    KeySequence{"Right"},
    [](auto& context) {
      context.mapView().moveUv(vm::direction::right, MapViewBase::UvActionMode::Normal);
    },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Move textures right (coarse)"},
    "Move Textures Right (Coarse)",
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    KeySequence{"Shift+Right"},
    [](auto& context) {
      context.mapView().moveUv(vm::direction::right, MapViewBase::UvActionMode::Coarse);
    },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Move textures right (fine)"},
    "Move Textures Right (Fine)",
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    KeySequence{"Ctrl+Right"},
    [](auto& context) {
      context.mapView().moveUv(vm::direction::right, MapViewBase::UvActionMode::Fine);
    },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Rotate textures clockwise"},
    "Rotate Textures Clockwise",
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    KeySequence{"PgUp"},
    [](auto& context) {
      context.mapView().rotateUv(true, MapViewBase::UvActionMode::Normal);
    },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Rotate textures clockwise (coarse)"},
    "Rotate Textures Clockwise (Coarse)",
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    KeySequence{"Shift+PgUp"},
    [](auto& context) {
      context.mapView().rotateUv(true, MapViewBase::UvActionMode::Coarse);
    },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Rotate textures clockwise (fine)"},
    "Rotate Textures Clockwise (Fine)",
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    KeySequence{"Ctrl+PgUp"},
    [](auto& context) {
      context.mapView().rotateUv(true, MapViewBase::UvActionMode::Fine);
    },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Rotate textures counter-clockwise"},
    "Rotate Textures Counter-clockwise",
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    KeySequence{"PgDown"},
    [](auto& context) {
      context.mapView().rotateUv(false, MapViewBase::UvActionMode::Normal);
    },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Rotate textures counter-clockwise (coarse)"},
    "Rotate Textures Counter-clockwise (Coarse)",
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    KeySequence{"Shift+PgDown"},
    [](auto& context) {
      context.mapView().rotateUv(false, MapViewBase::UvActionMode::Coarse);
    },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Rotate textures counter-clockwise (fine)"},
    "Rotate Textures Counter-clockwise (Fine)",
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    KeySequence{"Ctrl+PgDown"},
    [](auto& context) {
      context.mapView().rotateUv(false, MapViewBase::UvActionMode::Fine);
    },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Reveal in texture browser"},
    "Reveal in texture browser",
    ActionContext::View3D | ActionContext::AnySelection | ActionContext::AnyOrNoTool,
    KeySequence{},
    [](auto& context) { context.mapWindow().revealMaterial(); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Flip textures horizontally"},
    "Flip textures horizontally",
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    KeySequence{"Ctrl+F"},
    [](auto& context) { context.mapView().flipUv(vm::direction::right); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Flip textures vertically"},
    "Flip textures vertically",
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    KeySequence{"Ctrl+Alt+F"},
    [](auto& context) { context.mapView().flipUv(vm::direction::up); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Reset texture alignment"},
    "Reset texture alignment",
    ActionContext::AnyView | ActionContext::AnySelection | ActionContext::AnyOrNoTool,
    KeySequence{"Shift+R"},
    [](auto& context) { context.mapView().resetUv(); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Reset texture alignment to world aligned"},
    "Reset texture alignment to world aligned",
    ActionContext::AnyView | ActionContext::AnySelection | ActionContext::AnyOrNoTool,
    KeySequence{"Alt+Shift+R"},
    [](auto& context) { context.mapView().resetUvToWorld(); },
    [](const auto& context) { return context.hasDocument(); },
  });

  /* ========== Tag Actions ========== */
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Make structural"},
    "Make Structural",
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyOrNoTool,
    KeySequence{"Alt+S"},
    [](auto& context) { context.mapView().makeSelectionStructural(); },
    [](const auto& context) { return context.hasDocument(); },
  });

  /* ========== View / Filter Actions ========== */
  addAction(Action{
    std::filesystem::path{
      "Controls/Map view/View Filter > Toggle show entity classnames"},
    "Toggle Show Entity Classnames",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { context.mapView().toggleShowEntityClassnames(); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/View Filter > Toggle show group bounds"},
    "Toggle Show Group Bounds",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { context.mapView().toggleShowGroupBounds(); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{
      "Controls/Map view/View Filter > Toggle show brush entity bounds"},
    "Toggle Show Brush Entity Bounds",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { context.mapView().toggleShowBrushEntityBounds(); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{
      "Controls/Map view/View Filter > Toggle show point entity bounds"},
    "Toggle Show Point Entity Bounds",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { context.mapView().toggleShowPointEntityBounds(); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/View Filter > Toggle show point entities"},
    "Toggle Show Point Entities",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { context.mapView().toggleShowPointEntities(); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{
      "Controls/Map view/View Filter > Toggle show point entity models"},
    "Toggle Show Point Entity Models",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { context.mapView().toggleShowPointEntityModels(); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/View Filter > Toggle show brushes"},
    "Toggle Show Brushes",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { context.mapView().toggleShowBrushes(); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/View Filter > Show textures"},
    "Show Textures",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { context.mapView().showMaterials(); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/View Filter > Hide textures"},
    "Hide Textures",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { context.mapView().hideMaterials(); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/View Filter > Hide faces"},
    "Hide Faces",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { context.mapView().hideFaces(); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/View Filter > Shade faces"},
    "Toggle Shade Faces",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { context.mapView().toggleShadeFaces(); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/View Filter > Use fog"},
    "Toggle Show Fog",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { context.mapView().toggleShowFog(); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/View Filter > Show edges"},
    "Toggle Show Edges",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { context.mapView().toggleShowEdges(); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/View Filter > Show all entity links"},
    "Show All Entity Links",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { context.mapView().showAllEntityLinks(); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{
      "Controls/Map view/View Filter > Show transitively selected entity links"},
    "Show Transitively Selected Entity Links",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { context.mapView().showTransitivelySelectedEntityLinks(); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{
      "Controls/Map view/View Filter > Show directly selected entity links"},
    "Show Directly Selected Entity Links",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { context.mapView().showDirectlySelectedEntityLinks(); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/View Filter > Hide entity links"},
    "Hide All Entity Links",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { context.mapView().hideAllEntityLinks(); },
    [](const auto& context) { return context.hasDocument(); },
  });

  /* ========== Misc Actions ========== */
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Cycle map view"},
    "Cycle View",
    ActionContext::Any,
    KeySequence{"Space"},
    [](auto& context) { context.mapView().cycleMapView(); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Reset camera zoom"},
    "Reset Camera Zoom",
    ActionContext::View3D | ActionContext::AnyOrNoTool | ActionContext::AnyOrNoSelection,
    KeySequence{"Ctrl+Alt+Z"},
    [](auto& context) { context.mapView().resetCameraZoom(); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Cancel"},
    "Cancel",
    ActionContext::Any,
    KeySequence{"Esc"},
    [](auto& context) { context.mapView().cancel(); },
    [](const auto& context) { return context.hasDocument(); },
  });
}

void ActionManager::createMenu()
{
  createFileMenu();
  createEditMenu();
  createSelectionMenu();
  createGroupsMenu();
  createToolsMenu();
  createViewMenu();
  createRunMenu();
  createDebugMenu();
  createHelpMenu();
}

void ActionManager::createFileMenu()
{
  auto& fileMenu = createMainMenu("File");
  fileMenu.addItem(addAction(Action{
    "Menu/File/New",
    "New Document",
    ActionContext::Any,
    standardShortcut(StandardShortcut::New),
    [](auto& context) { context.appController().newDocument(); },
    [](const auto&) { return true; },
  }));
  fileMenu.addSeparator();
  fileMenu.addItem(addAction(Action{
    "Menu/File/Open...",
    "Open Document...",
    ActionContext::Any,
    standardShortcut(StandardShortcut::Open),
    [](auto& context) { context.appController().openDocument(); },
    [](const auto&) { return true; },
  }));
  fileMenu.addMenu("Open Recent", MenuEntryType::RecentDocuments);
  fileMenu.addSeparator();
  fileMenu.addItem(addAction(Action{
    "Menu/File/Save",
    "Save Document",
    ActionContext::Any,
    standardShortcut(StandardShortcut::Save),
    [](auto& context) { context.mapWindow().saveDocument(); },
    [](const auto& context) { return context.hasDocument(); },
  }));
  fileMenu.addItem(addAction(Action{
    "Menu/File/Save as...",
    "Save Document as...",
    ActionContext::Any,
    standardShortcut(StandardShortcut::SaveAs),
    [](auto& context) { context.mapWindow().saveDocumentAs(); },
    [](const auto& context) { return context.hasDocument(); },
  }));

  auto& exportMenu = fileMenu.addMenu("Export");
  exportMenu.addItem(addAction(Action{
    "Menu/File/Export/Wavefront OBJ...",
    "Wavefront OBJ...",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { context.mapWindow().exportDocumentAsObj(); },
    [](const auto& context) { return context.hasDocument(); },
  }));
  exportMenu.addItem(addAction(Action{
    "Menu/File/Export/Map...",
    "Map...",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { context.mapWindow().exportDocumentAsMap(); },
    [](const auto& context) { return context.hasDocument(); },
    std::nullopt,
    std::nullopt,
    "Exports the current map to a .map file. Layers marked Omit From Export "
    "will be omitted.",
  }));

  /* ========== File Menu (Associated Resources) ========== */
  fileMenu.addSeparator();
  fileMenu.addItem(addAction(Action{
    "Menu/File/Load Point File...",
    "Load Point File...",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { context.mapWindow().loadPointFile(); },
    [](const auto& context) { return context.hasDocument(); },
  }));
  fileMenu.addItem(addAction(Action{
    "Menu/File/Reload Point File",
    "Reload Point File",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { context.mapWindow().reloadPointFile(); },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().canReloadPointFile();
    },
  }));
  fileMenu.addItem(addAction(Action{
    "Menu/File/Unload Point File",
    "Unload Point File",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { context.mapWindow().unloadPointFile(); },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().canUnloadPointFile();
    },
  }));
  fileMenu.addSeparator();
  fileMenu.addItem(addAction(Action{
    "Menu/File/Load Portal File...",
    "Load Portal File...",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { context.mapWindow().loadPortalFile(); },
    [](const auto& context) { return context.hasDocument(); },
  }));
  fileMenu.addItem(addAction(Action{
    "Menu/File/Reload Portal File",
    "Reload Portal File",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { context.mapWindow().reloadPortalFile(); },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().canReloadPortalFile();
    },
  }));
  fileMenu.addItem(addAction(Action{
    "Menu/File/Unload Portal File",
    "Unload Portal File",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { context.mapWindow().unloadPortalFile(); },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().canUnloadPortalFile();
    },
  }));
  fileMenu.addSeparator();
  fileMenu.addItem(addAction(Action{
    "Menu/File/Reload Material Collections",
    "Reload Material Collections",
    ActionContext::Any,
    KeySequence{"F5"},
    [](auto& context) { context.mapWindow().reloadMaterialCollections(); },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().canReloadMaterialCollections();
    },
  }));
  fileMenu.addItem(addAction(Action{
    "Menu/File/Reload Entity Definitions",
    "Reload Entity Definitions",
    ActionContext::Any,
    KeySequence{"F6"},
    [](auto& context) { context.mapWindow().reloadEntityDefinitions(); },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().canReloadEntityDefinitions();
    },
  }));
  fileMenu.addSeparator();
  fileMenu.addItem(addAction(Action{
    "Menu/File/Revert",
    "Revert Document",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { context.mapWindow().revertDocument(); },
    [](const auto& context) { return context.hasDocument(); },
    std::nullopt,
    std::nullopt,
    "Discards any unsaved changes and reloads the map file.",
  }));
  fileMenu.addItem(addAction(Action{
    "Menu/File/Close",
    "Close Document",
    ActionContext::Any,
    standardShortcut(StandardShortcut::Close),
    [](auto& context) { context.mapWindow().closeDocument(); },
    [](const auto& context) { return context.hasDocument(); },
  }));
}

void ActionManager::createEditMenu()
{ /* ========== Edit Menu ========== */
  auto& editMenu = createMainMenu("Edit");
  editMenu.addItem(
    addAction(Action{
      std::filesystem::path{"Menu/Edit/Undo"},
      "Undo",
      ActionContext::Any,
      standardShortcut(StandardShortcut::Undo),
      [](auto& context) { context.mapWindow().undo(); },
      [](const auto& context) {
        return context.hasDocument() && context.mapWindow().canUndo();
      },
    }),
    MenuEntryType::Undo);
  editMenu.addItem(
    addAction(Action{
      std::filesystem::path{"Menu/Edit/Redo"},
      "Redo",
      ActionContext::Any,
      standardShortcut(StandardShortcut::Redo),
      [](auto& context) { context.mapWindow().redo(); },
      [](const auto& context) {
        return context.hasDocument() && context.mapWindow().canRedo();
      },
    }),
    MenuEntryType::Redo);
  editMenu.addSeparator();
  editMenu.addItem(addAction(Action{
    "Menu/Edit/Repeat",
    "Repeat Last Commands",
    ActionContext::Any,
    KeySequence{"Ctrl+R"},
    [](auto& context) { context.mapWindow().repeatLastCommands(); },
    [](const auto& context) { return context.hasDocument(); },
  }));
  editMenu.addItem(addAction(Action{
    "Menu/Edit/Clear Repeatable Commands",
    "Clear Repeatable Commands",
    ActionContext::Any,
    KeySequence{"Ctrl+Shift+R"},
    [](auto& context) { context.mapWindow().clearRepeatableCommands(); },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().hasRepeatableCommands();
    },
  }));
  editMenu.addSeparator();
  editMenu.addItem(
    addAction(Action{
      std::filesystem::path{"Menu/Edit/Cut"},
      "Cut",
      ActionContext::Any,
      standardShortcut(StandardShortcut::Cut),
      [](auto& context) { context.mapWindow().cutSelection(); },
      [](const auto& context) {
        return context.hasDocument() && context.mapWindow().canCopySelection();
      },
    }),
    MenuEntryType::Cut);
  editMenu.addItem(
    addAction(Action{
      std::filesystem::path{"Menu/Edit/Copy"},
      "Copy",
      ActionContext::Any,
      standardShortcut(StandardShortcut::Copy),
      [](auto& context) { context.mapWindow().copySelection(); },
      [](const auto& context) {
        return context.hasDocument() && context.mapWindow().canCopySelection();
      },
    }),
    MenuEntryType::Copy);
  editMenu.addItem(
    addAction(Action{
      std::filesystem::path{"Menu/Edit/Paste"},
      "Paste",
      ActionContext::Any,
      standardShortcut(StandardShortcut::Paste),
      [](auto& context) { context.mapWindow().pasteAtCursorPosition(); },
      [](const auto& context) {
        return context.hasDocument() && context.mapWindow().canPaste();
      },
    }),
    MenuEntryType::Paste);
  editMenu.addItem(
    addAction(Action{
      std::filesystem::path{"Menu/Edit/Paste at Original Position"},
      "Paste at Original Position",
      ActionContext::Any,
      KeySequence{"Ctrl+Alt+V"},
      [](auto& context) { context.mapWindow().pasteAtOriginalPosition(); },
      [](const auto& context) {
        return context.hasDocument() && context.mapWindow().canPaste();
      },
    }),
    MenuEntryType::PasteAtOriginalPosition);
  editMenu.addItem(addAction(Action{
    "Menu/Edit/Duplicate",
    "Duplicate",
    ActionContext::Any,
    KeySequence{"Ctrl+D"},
    [](auto& context) { context.mapWindow().duplicateSelection(); },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().canDuplicateSelection();
    },
    std::filesystem::path{"DuplicateObjects.svg"},
  }));
  editMenu.addItem(addAction(Action{
    std::filesystem::path{"Menu/Edit/Delete"},
    "Delete",
    ActionContext::Any,
    standardShortcut(StandardShortcut::Delete),
    [](auto& context) { context.mapWindow().deleteSelection(); },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().canDeleteSelection();
    },
  }));
  editMenu.addSeparator();

  auto& transformMenu = editMenu.addMenu("Transform");
  transformMenu.addItem(addAction(Action{
    std::filesystem::path{"Controls/Map view/Flip objects horizontally"},
    "Flip Horizontally",
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyOrNoTool,
    KeySequence{"Ctrl+F"},
    [](auto& context) { context.mapView().flip(vm::direction::left); },
    [](const auto& context) {
      return context.hasDocument() && context.mapView().canFlip();
    },
    std::filesystem::path{"FlipHorizontally.svg"},
  }));
  transformMenu.addItem(addAction(Action{
    std::filesystem::path{"Controls/Map view/Flip objects vertically"},
    "Flip Vertically",
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyOrNoTool,
    KeySequence{"Ctrl+Alt+F"},
    [](auto& context) { context.mapView().flip(vm::direction::up); },
    [](const auto& context) {
      return context.hasDocument() && context.mapView().canFlip();
    },
    std::filesystem::path{"FlipVertically.svg"},
  }));
  transformMenu.addItem(addAction(Action{
    std::filesystem::path{"Menu/Edit/Move objects"},
    "Move...",
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyOrNoTool,
    KeySequence{"Ctrl+Alt+M"},
    [](auto& context) { context.mapWindow().moveSelectedObjects(); },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().canMoveSelectedObjects();
    },
  }));

  auto& csgMenu = editMenu.addMenu("CSG");
  csgMenu.addItem(addAction(Action{
    "Menu/Edit/CSG/Convex Merge",
    "Convex Merge",
    ActionContext::Any,
    KeySequence{"Ctrl+J"},
    [](auto& context) { context.mapWindow().csgConvexMerge(); },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().canDoCsgConvexMerge();
    },
  }));
  csgMenu.addItem(addAction(Action{
    "Menu/Edit/CSG/Subtract",
    "Subtract",
    ActionContext::Any,
    KeySequence{"Ctrl+K"},
    [](auto& context) { context.mapWindow().csgSubtract(); },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().canDoCsgSubtract();
    },
  }));
  csgMenu.addItem(addAction(Action{
    "Menu/Edit/CSG/Hollow",
    "Hollow",
    ActionContext::Any,
    KeySequence{"Ctrl+Shift+K"},
    [](auto& context) { context.mapWindow().csgHollow(); },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().canDoCsgHollow();
    },
  }));
  csgMenu.addItem(addAction(Action{
    "Menu/Edit/CSG/Intersect",
    "Intersect",
    ActionContext::Any,
    KeySequence{"Ctrl+L"},
    [](auto& context) { context.mapWindow().csgIntersect(); },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().canDoCsgIntersect();
    },
  }));

  auto& vertexEditingMenu = editMenu.addMenu("Vertices");
  vertexEditingMenu.addItem(addAction(Action{
    "Menu/Edit/Snap Vertices to Integer",
    "Snap Vertices to Integer",
    ActionContext::Any,
    KeySequence{"Ctrl+Shift+V"},
    [](auto& context) { context.mapWindow().snapVerticesToInteger(); },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().canSnapVertices();
    },
  }));
  vertexEditingMenu.addItem(addAction(Action{
    "Menu/Edit/Snap Vertices to Grid",
    "Snap Vertices to Grid",
    ActionContext::Any,
    KeySequence{"Ctrl+Alt+Shift+V"},
    [](auto& context) { context.mapWindow().snapVerticesToGrid(); },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().canSnapVertices();
    },
  }));

  auto& patchEditingMenu = editMenu.addMenu("Patches");
  patchEditingMenu.addItem(addAction(Action{
    "Menu/Edit/Convert Selection to Patches",
    "Convert Selection to Patches",
    ActionContext::Any,
    KeySequence{"Ctrl+P"},
    [](auto& context) { context.mapWindow().convertSelectionToPatches(); },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().canConvertSelectionToPatches();
    },
  }));

  auto& materialsMenu = editMenu.addMenu("Materials");
  materialsMenu.addItem(addAction(Action{
    "Menu/Edit/Texture Lock",
    "Texture Lock",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { context.mapWindow().toggleAlignmentLock(); },
    [](const auto& context) { return context.hasDocument(); },
    [](const auto&) { return pref(Preferences::AlignmentLock); },
    std::filesystem::path{"AlignmentLock.svg"},
  }));
  materialsMenu.addItem(addAction(Action{
    "Menu/Edit/UV Lock",
    "UV Lock",
    ActionContext::Any,
    KeySequence{"U"},
    [](auto& context) { context.mapWindow().toggleUvLock(); },
    [](const auto& context) { return context.hasDocument(); },
    [](const auto&) { return pref(Preferences::UvLock); },
    std::filesystem::path{"UVLock.svg"},
  }));
  materialsMenu.addSeparator();
  materialsMenu.addItem(addAction(Action{
    "Menu/Edit/Replace Material...",
    "Replace Material...",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { context.mapWindow().replaceMaterial(); },
    [](const auto& context) { return context.hasDocument(); },
  }));
}

void ActionManager::createSelectionMenu()
{
  auto& selectionMenu = createMainMenu("Selection");
  selectionMenu.addItem(addAction(Action{
    "Menu/Edit/Select All",
    "Select All",
    ActionContext::Any,
    standardShortcut(StandardShortcut::SelectAll),
    [](auto& context) { context.mapWindow().selectAll(); },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().canSelect();
    },
  }));
  selectionMenu.addItem(addAction(Action{
    std::filesystem::path{"Menu/Edit/Invert Selection"},
    "Invert Selection",
    ActionContext::Any,
    KeySequence{"Ctrl+Alt+A"},
    [](auto& context) { context.mapWindow().selectInverse(); },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().canSelectInverse();
    },
  }));
  selectionMenu.addItem(addAction(Action{
    std::filesystem::path{"Menu/Edit/Deselect All"},
    "Deselect All",
    ActionContext::Any,
    KeySequence{"Ctrl+Shift+A"},
    [](auto& context) { context.mapWindow().selectNone(); },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().canDeselect();
    },
  }));
  selectionMenu.addSeparator();
  selectionMenu.addItem(addAction(Action{
    "Menu/Edit/Select Siblings",
    "Select Siblings",
    ActionContext::Any,
    KeySequence{"Ctrl+B"},
    [](auto& context) { context.mapWindow().selectSiblings(); },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().canSelectSiblings();
    },
  }));
  selectionMenu.addItem(addAction(Action{
    "Menu/Edit/Select Touching",
    "Select Touching",
    ActionContext::Any,
    KeySequence{"Ctrl+T"},
    [](auto& context) { context.mapWindow().selectTouching(); },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().canSelectByBrush();
    },
  }));
  selectionMenu.addItem(addAction(Action{
    "Menu/Edit/Select Inside",
    "Select Inside",
    ActionContext::Any,
    KeySequence{"Ctrl+E"},
    [](auto& context) { context.mapWindow().selectInside(); },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().canSelectByBrush();
    },
  }));
  selectionMenu.addItem(addAction(Action{
    std::filesystem::path{"Menu/Edit/Select Tall"},
    "Select Tall",
    ActionContext::Any,
    KeySequence{"Ctrl+Shift+E"},
    [](auto& context) { context.mapWindow().selectTall(); },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().canSelectTall();
    },
  }));
  selectionMenu.addItem(addAction(Action{
    "Menu/Edit/Select by Line Number",
    "Select by Line Number...",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { context.mapWindow().selectByLineNumber(); },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().canSelect();
    },
  }));
}

void ActionManager::createGroupsMenu()
{
  auto& groupsMenu = createMainMenu("Groups");
  groupsMenu.addItem(addAction(Action{
    std::filesystem::path{"Menu/Edit/Group"},
    "Group Selected Objects",
    ActionContext::Any,
    KeySequence{"Ctrl+G"},
    [](auto& context) { context.mapWindow().groupSelectedObjects(); },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().canGroupSelectedObjects();
    },
  }));
  groupsMenu.addItem(addAction(Action{
    "Menu/Edit/Ungroup",
    "Ungroup Selected Objects",
    ActionContext::Any,
    KeySequence{"Ctrl+Shift+G"},
    [](auto& context) { context.mapWindow().ungroupSelectedObjects(); },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().canUngroupSelectedObjects();
    },
  }));
  groupsMenu.addItem(addAction(Action{
    "Menu/Edit/Rename Groups",
    "Rename Selected Groups",
    ActionContext::Any,
    KeySequence{"Ctrl+Alt+G"},
    [](auto& context) { context.mapWindow().renameSelectedGroups(); },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().canRenameSelectedGroups();
    },
  }));
  groupsMenu.addSeparator();

  groupsMenu.addItem(addAction(Action{
    std::filesystem::path{"Menu/Edit/Create Linked Duplicate"},
    "Create Linked Duplicate",
    ActionContext::Any,
    KeySequence{"Ctrl+Shift+D"},
    [](auto& context) { createLinkedDuplicate(context.map()); },
    [](const auto& context) {
      return context.hasDocument() && canCreateLinkedDuplicate(context.map());
    },
  }));
  groupsMenu.addItem(addAction(Action{
    std::filesystem::path{"Menu/Edit/Select Linked Groups"},
    "Select Linked Groups",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { selectLinkedGroups(context.map()); },
    [](const auto& context) {
      return context.hasDocument() && canSelectLinkedGroups(context.map());
    },
  }));
  groupsMenu.addItem(addAction(Action{
    std::filesystem::path{"Menu/Edit/Separate Linked Groups"},
    "Separate Selected Groups",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { separateSelectedLinkedGroups(context.map()); },
    [](const auto& context) {
      return context.hasDocument() && canSeparateSelectedLinkedGroups(context.map());
    },
  }));
  groupsMenu.addItem(addAction(Action{
    std::filesystem::path{"Menu/Edit/Extract Linked Groups"},
    "Extract Selected Objects",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { extractLinkedGroups(context.map()); },
    [](const auto& context) {
      return context.hasDocument() && mdl::canExtractLinkedGroups(context.map());
    },
  }));
  groupsMenu.addItem(addAction(Action{
    std::filesystem::path{"Menu/Edit/Clear Protected Properties"},
    "Clear Protected Properties",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { clearProtectedEntityProperties(context.map()); },
    [](const auto& context) {
      return context.hasDocument() && canClearProtectedEntityProperties(context.map());
    },
  }));
}

void ActionManager::createToolsMenu()
{
  auto& toolsMenu = createMainMenu("Tools");
  toolsMenu.addItem(addAction(Action{
    "Menu/Edit/Tools/Brush Tool",
    "Brush Tool",
    ActionContext::Any,
    KeySequence{"B"},
    [](auto& context) { context.mapWindow().toolBox().toggleAssembleBrushTool(); },
    [](const auto& context) {
      return context.hasDocument()
             && context.mapWindow().toolBox().canToggleAssembleBrushTool();
    },
    [](const auto& context) {
      return context.hasDocument()
             && context.mapWindow().toolBox().assembleBrushToolActive();
    },
    std::filesystem::path{"BrushTool.svg"},
  }));
  toolsMenu.addItem(addAction(Action{
    "Menu/Edit/Tools/Clip Tool",
    "Clip Tool",
    ActionContext::Any,
    KeySequence{"C"},
    [](auto& context) { context.mapWindow().toolBox().toggleClipTool(); },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().toolBox().canToggleClipTool();
    },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().toolBox().clipToolActive();
    },
    std::filesystem::path{"ClipTool.svg"},
  }));
  toolsMenu.addItem(addAction(Action{
    "Menu/Edit/Tools/Rotate Tool",
    "Rotate Tool",
    ActionContext::Any,
    KeySequence{"R"},
    [](auto& context) { context.mapWindow().toolBox().toggleRotateTool(); },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().toolBox().canToggleRotateTool();
    },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().toolBox().rotateToolActive();
    },
    std::filesystem::path{"RotateTool.svg"},
  }));
  toolsMenu.addItem(addAction(Action{
    "Menu/Edit/Tools/Sweep Tool",
    "Sweep Tool",
    ActionContext::Any,
    KeySequence{"Y"},
    [](auto& context) { context.mapWindow().toolBox().toggleSweepTool(); },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().toolBox().canToggleSweepTool();
    },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().toolBox().sweepToolActive();
    },
    std::filesystem::path{"SweepTool.svg"},
  }));
  toolsMenu.addItem(addAction(Action{
    "Menu/Edit/Tools/Scale Tool",
    "Scale Tool",
    ActionContext::Any,
    KeySequence{"T"},
    [](auto& context) { context.mapWindow().toolBox().toggleScaleTool(); },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().toolBox().canToggleScaleTool();
    },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().toolBox().scaleToolActive();
    },
    std::filesystem::path{"ScaleTool.svg"},
  }));
  toolsMenu.addItem(addAction(Action{
    "Menu/Edit/Tools/Shear Tool",
    "Shear Tool",
    ActionContext::Any,
    KeySequence{"G"},
    [](auto& context) { context.mapWindow().toolBox().toggleShearTool(); },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().toolBox().canToggleShearTool();
    },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().toolBox().shearToolActive();
    },
    std::filesystem::path{"ShearTool.svg"},
  }));
  toolsMenu.addItem(addAction(Action{
    "Menu/Edit/Tools/Vertex Tool",
    "Vertex Tool",
    ActionContext::Any,
    KeySequence{"V"},
    [](auto& context) { context.mapWindow().toolBox().toggleVertexTool(); },
    [](const auto& context) {
      return context.hasDocument()
             && context.mapWindow().toolBox().canToggleAnyVertexTool();
    },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().toolBox().vertexToolActive();
    },
    std::filesystem::path{"VertexTool.svg"},
  }));
  toolsMenu.addItem(addAction(Action{
    "Menu/Edit/Tools/Edge Tool",
    "Edge Tool",
    ActionContext::Any,
    KeySequence{"E"},
    [](auto& context) { context.mapWindow().toolBox().toggleEdgeTool(); },
    [](const auto& context) {
      return context.hasDocument()
             && context.mapWindow().toolBox().canToggleAnyVertexTool();
    },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().toolBox().edgeToolActive();
    },
    std::filesystem::path{"EdgeTool.svg"},
  }));
  toolsMenu.addItem(addAction(Action{
    "Menu/Edit/Tools/Face Tool",
    "Face Tool",
    ActionContext::Any,
    KeySequence{"F"},
    [](auto& context) { context.mapWindow().toolBox().toggleFaceTool(); },
    [](const auto& context) {
      return context.hasDocument()
             && context.mapWindow().toolBox().canToggleAnyVertexTool();
    },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().toolBox().faceToolActive();
    },
    std::filesystem::path{"FaceTool.svg"},
  }));
  toolsMenu.addItem(addAction(Action{
    "Menu/Edit/Tools/Control Point Tool",
    "Control Point Tool",
    ActionContext::Any,
    KeySequence{"P"},
    [](auto& context) { context.mapWindow().toolBox().toggleControlPointTool(); },
    [](const auto& context) {
      return context.hasDocument()
             && context.mapWindow().toolBox().canToggleControlPointTool();
    },
    [](const auto& context) {
      return context.hasDocument()
             && context.mapWindow().toolBox().controlPointToolActive();
    },
    std::filesystem::path{"ControlPointTool.svg"},
  }));
  toolsMenu.addItem(addAction(Action{
    "Controls/Map view/Deactivate current tool",
    "Deactivate Current Tool",
    ActionContext::Any,
    KeySequence{"Shift+Esc"},
    [](auto& context) { context.mapView().deactivateCurrentTool(); },
    [](const auto& context) { return context.hasDocument(); },
    [](const auto& context) {
      return context.hasDocument() && !context.mapWindow().toolBox().anyModalToolActive();
    },
    std::filesystem::path{"NoTool.svg"},
  }));
}

void ActionManager::createViewMenu()
{
  auto& viewMenu = createMainMenu("View");
  auto& gridMenu = viewMenu.addMenu("Grid");
  gridMenu.addItem(addAction(Action{
    "Menu/View/Grid/Show Grid",
    "Show Grid",
    ActionContext::Any,
    KeySequence{"0"},
    [](auto& context) { context.mapWindow().toggleShowGrid(); },
    [](const auto& context) { return context.hasDocument(); },
    [](const auto& context) {
      return context.hasDocument() && context.map().grid().visible();
    },
  }));
  gridMenu.addItem(addAction(Action{
    "Menu/View/Grid/Snap to Grid",
    "Snap to Grid",
    ActionContext::Any,
    KeySequence{"Alt+0"},
    [](auto& context) { context.mapWindow().toggleSnapToGrid(); },
    [](const auto& context) { return context.hasDocument(); },
    [](const auto& context) {
      return context.hasDocument() && context.map().grid().snap();
    },
  }));
  gridMenu.addItem(addAction(Action{
    "Menu/View/Grid/Increase Grid Size",
    "Increase Grid Size",
    ActionContext::Any,
    KeySequence{"+"},
    [](auto& context) { context.mapWindow().incGridSize(); },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().canIncGridSize();
    },
  }));
  gridMenu.addItem(addAction(Action{
    "Menu/View/Grid/Decrease Grid Size",
    "Decrease Grid Size",
    ActionContext::Any,
    KeySequence{"-"},
    [](auto& context) { context.mapWindow().decGridSize(); },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().canDecGridSize();
    },
  }));
  gridMenu.addSeparator();
  gridMenu.addItem(addAction(Action{
    "Menu/View/Grid/Set Grid Size 0.125",
    "Set Grid Size 0.125",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { context.mapWindow().setGridSize(-3); },
    [](const auto& context) { return context.hasDocument(); },
    [](const auto& context) {
      return context.hasDocument() && context.map().grid().size() == -3;
    },
  }));
  gridMenu.addItem(addAction(Action{
    "Menu/View/Grid/Set Grid Size 0.25",
    "Set Grid Size 0.25",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { context.mapWindow().setGridSize(-2); },
    [](const auto& context) { return context.hasDocument(); },
    [](const auto& context) {
      return context.hasDocument() && context.map().grid().size() == -2;
    },
  }));
  gridMenu.addItem(addAction(Action{
    "Menu/View/Grid/Set Grid Size 0.5",
    "Set Grid Size 0.5",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { context.mapWindow().setGridSize(-1); },
    [](const auto& context) { return context.hasDocument(); },
    [](const auto& context) {
      return context.hasDocument() && context.map().grid().size() == -1;
    },
  }));
  gridMenu.addItem(addAction(Action{
    "Menu/View/Grid/Set Grid Size 1",
    "Set Grid Size 1",
    ActionContext::Any,
    KeySequence{"1"},
    [](auto& context) { context.mapWindow().setGridSize(0); },
    [](const auto& context) { return context.hasDocument(); },
    [](const auto& context) {
      return context.hasDocument() && context.map().grid().size() == 0;
    },
  }));
  gridMenu.addItem(addAction(Action{
    "Menu/View/Grid/Set Grid Size 2",
    "Set Grid Size 2",
    ActionContext::Any,
    KeySequence{"2"},
    [](auto& context) { context.mapWindow().setGridSize(1); },
    [](const auto& context) { return context.hasDocument(); },
    [](const auto& context) {
      return context.hasDocument() && context.map().grid().size() == 1;
    },
  }));
  gridMenu.addItem(addAction(Action{
    "Menu/View/Grid/Set Grid Size 4",
    "Set Grid Size 4",
    ActionContext::Any,
    KeySequence{"3"},
    [](auto& context) { context.mapWindow().setGridSize(2); },
    [](const auto& context) { return context.hasDocument(); },
    [](const auto& context) {
      return context.hasDocument() && context.map().grid().size() == 2;
    },
  }));
  gridMenu.addItem(addAction(Action{
    "Menu/View/Grid/Set Grid Size 8",
    "Set Grid Size 8",
    ActionContext::Any,
    KeySequence{"4"},
    [](auto& context) { context.mapWindow().setGridSize(3); },
    [](const auto& context) { return context.hasDocument(); },
    [](const auto& context) {
      return context.hasDocument() && context.map().grid().size() == 3;
    },
  }));
  gridMenu.addItem(addAction(Action{
    "Menu/View/Grid/Set Grid Size 16",
    "Set Grid Size 16",
    ActionContext::Any,
    KeySequence{"5"},
    [](auto& context) { context.mapWindow().setGridSize(4); },
    [](const auto& context) { return context.hasDocument(); },
    [](const auto& context) {
      return context.hasDocument() && context.map().grid().size() == 4;
    },
  }));
  gridMenu.addItem(addAction(Action{
    "Menu/View/Grid/Set Grid Size 32",
    "Set Grid Size 32",
    ActionContext::Any,
    KeySequence{"6"},
    [](auto& context) { context.mapWindow().setGridSize(5); },
    [](const auto& context) { return context.hasDocument(); },
    [](const auto& context) {
      return context.hasDocument() && context.map().grid().size() == 5;
    },
  }));
  gridMenu.addItem(addAction(Action{
    "Menu/View/Grid/Set Grid Size 64",
    "Set Grid Size 64",
    ActionContext::Any,
    KeySequence{"7"},
    [](auto& context) { context.mapWindow().setGridSize(6); },
    [](const auto& context) { return context.hasDocument(); },
    [](const auto& context) {
      return context.hasDocument() && context.map().grid().size() == 6;
    },
  }));
  gridMenu.addItem(addAction(Action{
    "Menu/View/Grid/Set Grid Size 128",
    "Set Grid Size 128",
    ActionContext::Any,
    KeySequence{"8"},
    [](auto& context) { context.mapWindow().setGridSize(7); },
    [](const auto& context) { return context.hasDocument(); },
    [](const auto& context) {
      return context.hasDocument() && context.map().grid().size() == 7;
    },
  }));
  gridMenu.addItem(addAction(Action{
    "Menu/View/Grid/Set Grid Size 256",
    "Set Grid Size 256",
    ActionContext::Any,
    KeySequence{"9"},
    [](auto& context) { context.mapWindow().setGridSize(8); },
    [](const auto& context) { return context.hasDocument(); },
    [](const auto& context) {
      return context.hasDocument() && context.map().grid().size() == 8;
    },
  }));

  auto& cameraMenu = viewMenu.addMenu("Camera");
  cameraMenu.addItem(addAction(Action{
    "Menu/View/Camera/Move to Next Point",
    "Move Camera to Next Point",
    ActionContext::Any,
    KeySequence{"."},
    [](auto& context) { context.mapWindow().moveCameraToNextPoint(); },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().canMoveCameraToNextPoint();
    },
  }));
  cameraMenu.addItem(addAction(Action{
    "Menu/View/Camera/Move to Previous Point",
    "Move Camera to Previous Point",
    ActionContext::Any,
    KeySequence{","},
    [](auto& context) { context.mapWindow().moveCameraToPreviousPoint(); },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().canMoveCameraToPreviousPoint();
    },
  }));
  cameraMenu.addItem(addAction(Action{
    "Menu/View/Camera/Reset 2D Cameras",
    "Reset 2D Cameras",
    ActionContext::Any,
    KeySequence{"Ctrl+Shift+U"},
    [](auto& context) { context.mapWindow().reset2dCameras(); },
    [](const auto& context) {
      return context.hasDocument() && !pref(Preferences::Link2DCameras);
    },
  }));
  cameraMenu.addItem(addAction(Action{
    "Menu/View/Camera/Focus on Selection",
    "Focus Camera on Selection",
    ActionContext::Any,
    KeySequence{"Ctrl+U"},
    [](auto& context) { context.mapWindow().focusCameraOnSelection(); },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().canFocusCamera();
    },
  }));
  cameraMenu.addItem(addAction(Action{
    "Menu/View/Camera/Move Camera to...",
    "Move Camera to...",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { context.mapWindow().moveCameraToPosition(); },
    [](const auto& context) { return context.hasDocument(); },
  }));

  viewMenu.addSeparator();
  viewMenu.addItem(addAction(Action{
    "Menu/View/Isolate",
    "Isolate Selection",
    ActionContext::Any,
    KeySequence{"Ctrl+I"},
    [](auto& context) { context.mapWindow().isolateSelection(); },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().canIsolateSelection();
    },
  }));
  viewMenu.addItem(addAction(Action{
    "Menu/View/Hide",
    "Hide Selection",
    ActionContext::Any,
    KeySequence{"Ctrl+Alt+I"},
    [](auto& context) { context.mapWindow().hideSelection(); },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().canHideSelection();
    },
  }));
  viewMenu.addItem(addAction(Action{
    "Menu/View/Show All",
    "Show All",
    ActionContext::Any,
    KeySequence{"Ctrl+Shift+I"},
    [](auto& context) { context.mapWindow().showAll(); },
    [](const auto& context) { return context.hasDocument(); },
  }));
  viewMenu.addSeparator();
  viewMenu.addItem(addAction(Action{
    "Menu/View/Switch to Map Inspector",
    "Show Map Inspector",
    ActionContext::Any,
    KeySequence{"Ctrl+1"},
    [](auto& context) { context.mapWindow().switchToInspectorPage(InspectorPage::Map); },
    [](const auto& context) { return context.hasDocument(); },
  }));
  viewMenu.addItem(addAction(Action{
    "Menu/View/Switch to Entity Inspector",
    "Show Entity Inspector",
    ActionContext::Any,
    KeySequence{"Ctrl+2"},
    [](auto& context) {
      context.mapWindow().switchToInspectorPage(InspectorPage::Entity);
    },
    [](const auto& context) { return context.hasDocument(); },
  }));
  viewMenu.addItem(addAction(Action{
    "Menu/View/Switch to Face Inspector",
    "Show Face Inspector",
    ActionContext::Any,
    KeySequence{"Ctrl+3"},
    [](auto& context) { context.mapWindow().switchToInspectorPage(InspectorPage::Face); },
    [](const auto& context) { return context.hasDocument(); },
  }));
  viewMenu.addSeparator();
  viewMenu.addItem(addAction(Action{
    "Menu/View/Toggle Toolbar",
    "Toggle Toolbar",
    ActionContext::Any,
    KeySequence{"Ctrl+Alt+T"},
    [](auto& context) { context.mapWindow().toggleToolbar(); },
    [](const auto& context) { return context.hasDocument(); },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().toolbarVisible();
    },
  }));
  viewMenu.addItem(addAction(Action{
    "Menu/View/Toggle Info Panel",
    "Toggle Info Panel",
    ActionContext::Any,
    KeySequence{"Ctrl+4"},
    [](auto& context) { context.mapWindow().toggleInfoPanel(); },
    [](const auto& context) { return context.hasDocument(); },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().infoPanelVisible();
    },
  }));
  viewMenu.addItem(addAction(Action{
    "Menu/View/Toggle Inspector",
    "Toggle Inspector",
    ActionContext::Any,
    KeySequence{"Ctrl+5"},
    [](auto& context) { context.mapWindow().toggleInspector(); },
    [](const auto& context) { return context.hasDocument(); },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().inspectorVisible();
    },
  }));
  viewMenu.addItem(addAction(Action{
    "Menu/View/Maximize Current View",
    "Maximize Current View",
    ActionContext::Any,
#if defined(Q_OS_MACOS)
    // Command + Space opens Spotlight so we can't use it, so use Ctrl + Space instead.
    KeySequence{"Meta+Space"},
#else
    KeySequence{"Ctrl+Space"},
#endif
    [](auto& context) { context.mapWindow().toggleMaximizeCurrentView(); },
    [](const auto& context) { return context.hasDocument(); },
    [](const auto& context) {
      return context.hasDocument() && context.mapWindow().currentViewMaximized();
    },
  }));
  viewMenu.addSeparator();
  viewMenu.addItem(addAction(Action{
    "Menu/File/Preferences...",
    "Preferences...",
    ActionContext::Any,
    standardShortcut(StandardShortcut::Preferences),
    [](auto& context) { context.appController().showPreferences(); },
    [](const auto&) { return true; },
  }));
}

void ActionManager::createRunMenu()
{
  auto& runMenu = createMainMenu("Run");
  runMenu.addItem(addAction(Action{
    "Menu/Run/Compile...",
    "Compile Map...",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { context.mapWindow().showCompileDialog(); },
    [](const auto& context) { return context.hasDocument(); },
  }));
  runMenu.addItem(addAction(Action{
    "Menu/Run/Launch...",
    "Launch Engine...",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { context.mapWindow().showLaunchEngineDialog(); },
    [](const auto& context) { return context.hasDocument(); },
  }));
  runMenu.addSeparator();
  runMenu.addItem(
    addAction(Action{
      "Menu/Run/Rerun...",
      "Re-run compilation...",
      ActionContext::Any,
      KeySequence{},
      [](auto& context) { context.mapWindow().rerunLastCompilation(); },
      [](const auto& context) {
        return context.hasDocument() && context.mapWindow().hasLastCompilationProfile();
      },
    }),
    MenuEntryType::Rerun);
}

void ActionManager::createDebugMenu()
{
#ifndef NDEBUG
  auto& debugMenu = createMainMenu("Debug");
  debugMenu.addItem(addAction(Action{
    "Menu/Debug/Print Vertices",
    "Print Vertices to Console",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { context.mapWindow().debugPrintVertices(); },
    [](const auto& context) { return context.hasDocument(); },
  }));
  debugMenu.addItem(addAction(Action{
    "Menu/Debug/Create Brush...",
    "Create Brush...",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { context.mapWindow().debugCreateBrush(); },
    [](const auto& context) { return context.hasDocument(); },
  }));
  debugMenu.addItem(addAction(Action{
    "Menu/Debug/Create Cube...",
    "Create Cube...",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { context.mapWindow().debugCreateCube(); },
    [](const auto& context) { return context.hasDocument(); },
  }));
  debugMenu.addItem(addAction(Action{
    "Menu/Debug/Crash...",
    "Crash...",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { context.mapWindow().debugCrash(); },
    [](const auto& context) { return context.hasDocument(); },
  }));
  debugMenu.addItem(addAction(Action{
    "Menu/Debug/Throw Exception During Command",
    "Throw Exception During Command",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { context.mapWindow().debugThrowExceptionDuringCommand(); },
    [](const auto& context) { return context.hasDocument(); },
  }));
  debugMenu.addItem(addAction(Action{
    "Menu/Debug/Show Crash Report Dialog",
    "Show Crash Report Dialog...",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { context.appController().debugShowCrashReportDialog(); },
    [](const auto&) { return true; },
  }));
  debugMenu.addItem(addAction(Action{
    "Menu/Debug/Set Window Size...",
    "Set Window Size...",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { context.mapWindow().debugSetWindowSize(); },
    [](const auto& context) { return context.hasDocument(); },
  }));
  debugMenu.addItem(addAction(Action{
    "Menu/Debug/Show Palette...",
    "Show Palette...",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { context.mapWindow().debugShowPalette(); },
    [](const auto& context) { return context.hasDocument(); },
  }));
#endif
}

void ActionManager::createHelpMenu()
{
  auto& helpMenu = createMainMenu("Help");
  helpMenu.addItem(addAction(Action{
    std::filesystem::path{"Menu/Help/TrenchBroom Manual"},
    "TrenchBroom Manual",
    ActionContext::Any,
    standardShortcut(StandardShortcut::HelpContents),
    [](auto& context) { context.appController().showManual(); },
    [](const auto&) { return true; },
  }));
  helpMenu.addItem(addAction(Action{
    "Menu/File/About TrenchBroom",
    "About TrenchBroom",
    ActionContext::Any,
    KeySequence{},
    [](auto& context) { context.appController().showAboutDialog(); },
    [](const auto&) { return true; },
  }));
}

Menu& ActionManager::createMainMenu(std::string name)
{
  return m_mainMenu.emplace_back(Menu{std::move(name), MenuEntryType::None, {}});
}

void ActionManager::createToolbar()
{
  m_toolBar.addItem(existingAction("Controls/Map view/Deactivate current tool"));
  m_toolBar.addItem(existingAction("Menu/Edit/Tools/Brush Tool"));
  m_toolBar.addItem(existingAction("Menu/Edit/Tools/Clip Tool"));
  m_toolBar.addItem(existingAction("Menu/Edit/Tools/Vertex Tool"));
  m_toolBar.addItem(existingAction("Menu/Edit/Tools/Edge Tool"));
  m_toolBar.addItem(existingAction("Menu/Edit/Tools/Face Tool"));
  m_toolBar.addItem(existingAction("Menu/Edit/Tools/Control Point Tool"));
  m_toolBar.addItem(existingAction("Menu/Edit/Tools/Rotate Tool"));
  m_toolBar.addItem(existingAction("Menu/Edit/Tools/Sweep Tool"));
  m_toolBar.addItem(existingAction("Menu/Edit/Tools/Scale Tool"));
  m_toolBar.addItem(existingAction("Menu/Edit/Tools/Shear Tool"));
  m_toolBar.addSeparator();
  m_toolBar.addItem(existingAction("Menu/Edit/Duplicate"));
  m_toolBar.addItem(existingAction("Controls/Map view/Flip objects horizontally"));
  m_toolBar.addItem(existingAction("Controls/Map view/Flip objects vertically"));
  m_toolBar.addSeparator();
  m_toolBar.addItem(existingAction("Menu/Edit/Texture Lock"));
  m_toolBar.addItem(existingAction("Menu/Edit/UV Lock"));
  m_toolBar.addSeparator();
}

Action& ActionManager::existingAction(const std::filesystem::path& preferencePath)
{
  auto it = m_actions.find(preferencePath);
  contract_assert(it != m_actions.end());

  return it->second;
}

Action& ActionManager::addAction(Action action)
{
  auto path = action.preference().path;
  auto [it, didInsert] = m_actions.emplace(std::move(path), std::move(action));
  contract_assert(didInsert);

  return it->second;
}

} // namespace tb::ui
