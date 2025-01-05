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

#include "Actions.h"

#include <QKeySequence>
#include <QString>

#include "PreferenceManager.h"
#include "Preferences.h"
#include "TrenchBroomApp.h"
#include "mdl/EntityDefinition.h"
#include "mdl/EntityProperties.h"
#include "mdl/Tag.h"
#include "ui/Grid.h" // IWYU pragma: keep
#include "ui/Inspector.h"
#include "ui/MapDocument.h"
#include "ui/MapFrame.h"
#include "ui/MapViewBase.h"

#include "vm/util.h"

#include <cassert>
#include <string>
#include <unordered_set>

namespace tb::ui
{
// ActionExecutionContext

ActionExecutionContext::ActionExecutionContext(MapFrame* mapFrame, MapViewBase* mapView)
  : m_actionContext(mapView != nullptr ? mapView->actionContext() : ActionContext::Any)
  , // cache here for performance reasons
  m_frame{mapFrame}
  , m_mapView{mapView}
{
  assert(m_frame == nullptr || m_mapView != nullptr);
}

bool ActionExecutionContext::hasDocument() const
{
  return m_frame != nullptr;
}

bool ActionExecutionContext::hasActionContext(
  const ActionContext::Type actionContext) const
{
  if (actionContext == ActionContext::Any || m_actionContext == ActionContext::Any)
  {
    return true;
  }

  if (hasDocument())
  {
    return actionContextMatches(m_actionContext, actionContext);
  }
  return false;
}

MapFrame* ActionExecutionContext::frame()
{
  assert(hasDocument());
  return m_frame;
}

const MapFrame* ActionExecutionContext::frame() const
{
  assert(hasDocument());
  return m_frame;
}

MapViewBase* ActionExecutionContext::view()
{
  assert(hasDocument());
  assert(m_mapView != nullptr);
  return m_mapView;
}

const MapViewBase* ActionExecutionContext::view() const
{
  assert(hasDocument());
  assert(m_mapView != nullptr);
  return m_mapView;
}

MapDocument* ActionExecutionContext::document()
{
  assert(hasDocument());
  return m_frame->document().get();
}

const MapDocument* ActionExecutionContext::document() const
{
  assert(hasDocument());
  return m_frame->document().get();
}

// Action

Action::Action(
  std::filesystem::path preferencePath,
  QString label,
  const ActionContext::Type actionContext,
  QKeySequence defaultShortcut,
  ExecuteFn execute,
  EnabledFn enabled,
  std::optional<CheckedFn> checked,
  std::optional<std::filesystem::path> iconPath,
  std::optional<QString> statusTip)
  : m_label{std::move(label)}
  , m_preferencePath{std::move(preferencePath)}
  , m_actionContext{actionContext}
  , m_defaultShortcut{defaultShortcut}
  , m_execute{std::move(execute)}
  , m_enabled{std::move(enabled)}
  , m_checked{std::move(checked)}
  , m_iconPath{std::move(iconPath)}
  , m_statusTip{std::move(statusTip)}
{
}

Action::Action(
  std::filesystem::path preferencePath,
  QString label,
  const ActionContext::Type actionContext,
  QKeySequence defaultShortcut,
  ExecuteFn execute,
  EnabledFn enabled,
  std::optional<std::filesystem::path> iconPath,
  std::optional<QString> statusTip)
  : Action{
      std::move(preferencePath),
      std::move(label),
      actionContext,
      defaultShortcut,
      std::move(execute),
      std::move(enabled),
      std::nullopt,
      std::move(iconPath),
      std::move(statusTip)}
{
}

Action::Action(
  std::filesystem::path preferencePath,
  QString label,
  ActionContext::Type actionContext,
  ExecuteFn execute,
  EnabledFn enabled)
  : Action{
      std::move(preferencePath),
      std::move(label),
      actionContext,
      QKeySequence{},
      std::move(execute),
      std::move(enabled),
      std::nullopt,
      std::nullopt,
      std::nullopt}
{
}

const QString& Action::label() const
{
  return m_label;
}

const std::filesystem::path& Action::preferencePath() const
{
  return m_preferencePath;
}

ActionContext::Type Action::actionContext() const
{
  return m_actionContext;
}

QKeySequence Action::keySequence() const
{
  auto& prefs = PreferenceManager::instance();
  auto& pref = prefs.dynamicPreference(m_preferencePath, QKeySequence{m_defaultShortcut});
  return prefs.get(pref);
}

void Action::setKeySequence(const QKeySequence& keySequence) const
{
  auto& prefs = PreferenceManager::instance();
  auto& pref = prefs.dynamicPreference(m_preferencePath, QKeySequence{m_defaultShortcut});
  prefs.set(pref, keySequence);
}

void Action::resetKeySequence() const
{
  setKeySequence(m_defaultShortcut);
}

void Action::execute(ActionExecutionContext& context) const
{
  if (enabled(context))
  {
    m_execute(context);
  }
}

bool Action::enabled(const ActionExecutionContext& context) const
{
  return context.hasActionContext(m_actionContext) && m_enabled(context);
}

bool Action::checkable() const
{
  return m_checked != std::nullopt;
}

bool Action::checked(const ActionExecutionContext& context) const
{
  return m_checked && (*m_checked)(context);
}

const std::optional<std::filesystem::path>& Action::iconPath() const
{
  return m_iconPath;
}

const std::optional<QString>& Action::statusTip() const
{
  return m_statusTip;
}

void Menu::addSeparator()
{
  entries.emplace_back(MenuSeparator{});
}

const Action& Menu::addItem(const Action& action, const MenuEntryType entryType_)
{
  entries.emplace_back(MenuAction{action, entryType_});
  return action;
}

Menu& Menu::addMenu(std::string name_, const MenuEntryType entryType_)
{
  return std::get<Menu>(entries.emplace_back(Menu{std::move(name_), entryType_, {}}));
}

// ActionManager

ActionManager::ActionManager()
{
  initialize();
}

const ActionManager& ActionManager::instance()
{
  static const auto instance = ActionManager();
  return instance;
}

std::vector<Action> ActionManager::createTagActions(
  const std::vector<mdl::SmartTag>& tags) const
{
  std::vector<Action> result;

  for (const auto& tag : tags)
  {
    result.emplace_back(
      std::filesystem::path{"Filters/Tags/" + tag.name() + "/Toggle Visible"},
      QObject::tr("Toggle %1 visible").arg(QString::fromStdString(tag.name())),
      ActionContext::Any,
      [&tag](auto& context) { context.view()->toggleTagVisible(tag); },
      [](const auto& context) { return context.hasDocument(); });
    if (tag.canEnable())
    {
      result.emplace_back(
        std::filesystem::path{"Tags/" + tag.name() + "/Enable"},
        QObject::tr("Turn Selection into %1").arg(QString::fromStdString(tag.name())),
        ActionContext::AnyView | ActionContext::AnySelection | ActionContext::AnyOrNoTool,
        [&tag](auto& context) { context.view()->enableTag(tag); },
        [](const auto& context) { return context.hasDocument(); });
    }
    if (tag.canDisable())
    {
      result.emplace_back(
        std::filesystem::path{"Tags/" + tag.name() + "/Disable"},
        QObject::tr("Turn Selection into non-%1").arg(QString::fromStdString(tag.name())),
        ActionContext::AnyView | ActionContext::AnySelection | ActionContext::AnyOrNoTool,
        [&tag](auto& context) { context.view()->disableTag(tag); },
        [](const auto& context) { return context.hasDocument(); });
    }
  }

  return result;
}

std::vector<Action> ActionManager::createEntityDefinitionActions(
  const std::vector<mdl::EntityDefinition*>& entityDefinitions) const
{
  std::vector<Action> result;

  for (const auto* definition : entityDefinitions)
  {
    result.emplace_back(
      std::filesystem::path{"Entities/" + definition->name() + "/Toggle"},
      QObject::tr("Toggle %1 visible").arg(QString::fromStdString(definition->name())),
      ActionContext::Any,
      [definition](auto& context) {
        context.view()->toggleEntityDefinitionVisible(definition);
      },
      [](const auto& context) { return context.hasDocument(); });
    if (definition->name() != mdl::EntityPropertyValues::WorldspawnClassname)
    {
      result.emplace_back(
        std::filesystem::path{"Entities/" + definition->name() + "/Create"},
        QObject::tr("Create %1").arg(QString::fromStdString(definition->name())),
        ActionContext::Any,
        [definition](auto& context) { context.view()->createEntity(definition); },
        [](const auto& context) { return context.hasDocument(); });
    }
  }

  return result;
}

void ActionManager::visitMapViewActions(const ActionVisitor& visitor) const
{
  // Gather the set of all Actions that are used in menus/toolbars

  auto menuActions = std::unordered_set<const Action*>{};
  const auto collectActionsVisitor = kdl::overload(
    [](const MenuSeparator&) {},
    [&](const MenuAction& actionItem) { menuActions.insert(&actionItem.action); },
    [](const auto& thisLambda, const Menu& menu) { menu.visitEntries(thisLambda); });

  visitMainMenu(collectActionsVisitor);
  visitToolBar(collectActionsVisitor);

  for (const auto& [path, tAction] : m_actions)
  {
    unused(path);
    if (menuActions.count(&tAction) == 0)
    {
      // This action is not used in a menu, so visit it
      visitor(tAction);
    }
  }
}

const std::unordered_map<std::filesystem::path, Action, kdl::path_hash>& ActionManager::
  actionsMap() const
{
  return m_actions;
}

void ActionManager::resetAllKeySequences() const
{
  const auto resetVisitor = kdl::overload(
    [](const MenuSeparator&) {},
    [](const MenuAction& actionItem) { actionItem.action.resetKeySequence(); },
    [](const auto& thisLambda, const Menu& menu) { menu.visitEntries(thisLambda); });

  visitMainMenu(resetVisitor);
  visitToolBar(resetVisitor);

  visitMapViewActions([](const auto& action) { action.resetKeySequence(); });
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
    QObject::tr("Create Brush"),
    ActionContext::View3D | ActionContext::AnyOrNoSelection
      | ActionContext::AssembleBrushTool,
    QKeySequence{Qt::Key_Return},
    [](auto& context) { context.view()->assembleBrush(); },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->assembleBrushToolActive();
    },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Toggle clip side"},
    QObject::tr("Toggle Clip Side"),
    ActionContext::AnyView | ActionContext::AnyOrNoSelection | ActionContext::ClipTool,
    QKeySequence{Qt::CTRL | Qt::Key_Return},
    [](auto& context) { context.view()->toggleClipSide(); },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->clipToolActive();
    },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Perform clip"},
    QObject::tr("Perform Clip"),
    ActionContext::AnyView | ActionContext::AnyOrNoSelection | ActionContext::ClipTool,
    QKeySequence{Qt::Key_Return},
    [](auto& context) { context.view()->performClip(); },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->clipToolActive();
    },
  });

  /* ========== Translation ========== */
  // applies to objects, vertices, handles (e.g. rotation center)
  // these preference paths are structured like "action in 2D view; action in 3D view"
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Move objects up; Move objects forward"},
    QObject::tr("Move Forward"),
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyVertexTool
      | ActionContext::RotateTool | ActionContext::NoTool,
    QKeySequence{Qt::Key_Up},
    [](auto& context) { context.view()->move(vm::direction::forward); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Move objects down; Move objects backward"},
    QObject::tr("Move Backward"),
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyVertexTool
      | ActionContext::RotateTool | ActionContext::NoTool,
    QKeySequence{Qt::Key_Down},
    [](auto& context) { context.view()->move(vm::direction::backward); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Move objects left"},
    QObject::tr("Move Left"),
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyVertexTool
      | ActionContext::RotateTool | ActionContext::NoTool,
    QKeySequence{Qt::Key_Left},
    [](auto& context) { context.view()->move(vm::direction::left); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Move objects right"},
    QObject::tr("Move Right"),
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyVertexTool
      | ActionContext::RotateTool | ActionContext::NoTool,
    QKeySequence{Qt::Key_Right},
    [](auto& context) { context.view()->move(vm::direction::right); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Move objects backward; Move objects up"},
    QObject::tr("Move Up"),
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyVertexTool
      | ActionContext::RotateTool | ActionContext::NoTool,
    QKeySequence{Qt::Key_PageUp},
    [](auto& context) { context.view()->move(vm::direction::up); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Move objects forward; Move objects down"},
    QObject::tr("Move Down"),
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyVertexTool
      | ActionContext::RotateTool | ActionContext::NoTool,
    QKeySequence{Qt::Key_PageDown},
    [](auto& context) { context.view()->move(vm::direction::down); },
    [](const auto& context) { return context.hasDocument(); },
  });

  /* ========== Duplication ========== */
  // these preference paths are structured like "action in 2D view; action in 3D view"
  addAction(Action{
    std::filesystem::path(
      "Controls/Map view/Duplicate and move objects up; Duplicate and move "
      "objects forward"),
    QObject::tr("Duplicate and Move Forward"),
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyOrNoTool,
    QKeySequence{Qt::CTRL | Qt::Key_Up},
    [](auto& context) {
      context.view()->duplicateAndMoveObjects(vm::direction::forward);
    },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path(
      "Controls/Map view/Duplicate and move objects down; Duplicate and move "
      "objects backward"),
    QObject::tr("Duplicate and Move Backward"),
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyOrNoTool,
    QKeySequence{Qt::CTRL | Qt::Key_Down},
    [](auto& context) {
      context.view()->duplicateAndMoveObjects(vm::direction::backward);
    },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Duplicate and move objects left"},
    QObject::tr("Duplicate and Move Left"),
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyOrNoTool,
    QKeySequence{Qt::CTRL | Qt::Key_Left},
    [](auto& context) { context.view()->duplicateAndMoveObjects(vm::direction::left); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Duplicate and move objects right"},
    QObject::tr("Duplicate and Move Right"),
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyOrNoTool,
    QKeySequence{Qt::CTRL | Qt::Key_Right},
    [](auto& context) { context.view()->duplicateAndMoveObjects(vm::direction::right); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path(
      "Controls/Map view/Duplicate and move objects backward; Duplicate and move "
      "objects up"),
    QObject::tr("Duplicate and Move Up"),
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyOrNoTool,
    QKeySequence{Qt::CTRL | Qt::Key_PageUp},
    [](auto& context) { context.view()->duplicateAndMoveObjects(vm::direction::up); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path(
      "Controls/Map view/Duplicate and move objects forward; Duplicate and move "
      "objects down"),
    QObject::tr("Duplicate and Move Down"),
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyOrNoTool,
    QKeySequence{Qt::CTRL | Qt::Key_PageDown},
    [](auto& context) { context.view()->duplicateAndMoveObjects(vm::direction::down); },
    [](const auto& context) { return context.hasDocument(); },
  });

  /* ========== Rotation ========== */
  // applies to objects, vertices, handles (e.g. rotation center)
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Roll objects clockwise"},
    QObject::tr("Roll Clockwise"),
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::RotateTool
      | ActionContext::NoTool,
    QKeySequence{Qt::ALT | Qt::Key_Up},
    [](auto& context) { context.view()->rotateObjects(vm::rotation_axis::roll, true); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Roll objects counter-clockwise"},
    QObject::tr("Roll Counter-clockwise"),
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::RotateTool
      | ActionContext::NoTool,
    QKeySequence{Qt::ALT | Qt::Key_Down},
    [](auto& context) { context.view()->rotateObjects(vm::rotation_axis::roll, false); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Yaw objects clockwise"},
    QObject::tr("Yaw Clockwise"),
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::RotateTool
      | ActionContext::NoTool,
    QKeySequence{Qt::ALT | Qt::Key_Left},
    [](auto& context) { context.view()->rotateObjects(vm::rotation_axis::yaw, true); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Yaw objects counter-clockwise"},
    QObject::tr("Yaw Counter-clockwise"),
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::RotateTool
      | ActionContext::NoTool,
    QKeySequence{Qt::ALT | Qt::Key_Right},
    [](auto& context) { context.view()->rotateObjects(vm::rotation_axis::yaw, false); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Pitch objects clockwise"},
    QObject::tr("Pitch Clockwise"),
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::RotateTool
      | ActionContext::NoTool,
    QKeySequence{Qt::ALT | Qt::Key_PageUp},
    [](auto& context) { context.view()->rotateObjects(vm::rotation_axis::pitch, true); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Pitch objects counter-clockwise"},
    QObject::tr("Pitch Counter-clockwise"),
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::RotateTool
      | ActionContext::NoTool,
    QKeySequence{Qt::ALT | Qt::Key_PageDown},
    [](auto& context) { context.view()->rotateObjects(vm::rotation_axis::pitch, false); },
    [](const auto& context) { return context.hasDocument(); },
  });

  /* ========== Texturing ========== */
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Move textures up"},
    QObject::tr("Move Textures Up"),
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    QKeySequence{Qt::Key_Up},
    [](auto& context) {
      context.view()->moveUV(vm::direction::up, MapViewBase::UVActionMode::Normal);
    },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Move textures up (coarse)"},
    QObject::tr("Move Textures Up (Coarse)"),
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    QKeySequence{Qt::SHIFT | Qt::Key_Up},
    [](auto& context) {
      context.view()->moveUV(vm::direction::up, MapViewBase::UVActionMode::Coarse);
    },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Move textures up (fine)"},
    QObject::tr("Move Textures Up (Fine)"),
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    QKeySequence{Qt::CTRL | Qt::Key_Up},
    [](auto& context) {
      context.view()->moveUV(vm::direction::up, MapViewBase::UVActionMode::Fine);
    },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Move textures down"},
    QObject::tr("Move Textures Down"),
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    QKeySequence{Qt::Key_Down},
    [](auto& context) {
      context.view()->moveUV(vm::direction::down, MapViewBase::UVActionMode::Normal);
    },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Move textures down (coarse)"},
    QObject::tr("Move Textures Down (Coarse)"),
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    QKeySequence{Qt::SHIFT | Qt::Key_Down},
    [](auto& context) {
      context.view()->moveUV(vm::direction::down, MapViewBase::UVActionMode::Coarse);
    },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Move textures down (fine)"},
    QObject::tr("Move Textures Down (Fine)"),
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    QKeySequence{Qt::CTRL | Qt::Key_Down},
    [](auto& context) {
      context.view()->moveUV(vm::direction::down, MapViewBase::UVActionMode::Fine);
    },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Move textures left"},
    QObject::tr("Move Textures Left"),
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    QKeySequence{Qt::Key_Left},
    [](auto& context) {
      context.view()->moveUV(vm::direction::left, MapViewBase::UVActionMode::Normal);
    },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Move textures left (coarse)"},
    QObject::tr("Move Textures Left (Coarse)"),
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    QKeySequence{Qt::SHIFT | Qt::Key_Left},
    [](auto& context) {
      context.view()->moveUV(vm::direction::left, MapViewBase::UVActionMode::Coarse);
    },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Move textures left (fine)"},
    QObject::tr("Move Textures Left (Fine)"),
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    QKeySequence{Qt::CTRL | Qt::Key_Left},
    [](auto& context) {
      context.view()->moveUV(vm::direction::left, MapViewBase::UVActionMode::Fine);
    },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Move textures right"},
    QObject::tr("Move Textures Right"),
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    QKeySequence{Qt::Key_Right},
    [](auto& context) {
      context.view()->moveUV(vm::direction::right, MapViewBase::UVActionMode::Normal);
    },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Move textures right (coarse)"},
    QObject::tr("Move Textures Right (Coarse)"),
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    QKeySequence{Qt::SHIFT | Qt::Key_Right},
    [](auto& context) {
      context.view()->moveUV(vm::direction::right, MapViewBase::UVActionMode::Coarse);
    },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Move textures right (fine)"},
    QObject::tr("Move Textures Right (Fine)"),
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    QKeySequence{Qt::CTRL | Qt::Key_Right},
    [](auto& context) {
      context.view()->moveUV(vm::direction::right, MapViewBase::UVActionMode::Fine);
    },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Rotate textures clockwise"},
    QObject::tr("Rotate Textures Clockwise"),
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    QKeySequence{Qt::Key_PageUp},
    [](auto& context) {
      context.view()->rotateUV(true, MapViewBase::UVActionMode::Normal);
    },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Rotate textures clockwise (coarse)"},
    QObject::tr("Rotate Textures Clockwise (Coarse)"),
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    QKeySequence{Qt::SHIFT | Qt::Key_PageUp},
    [](auto& context) {
      context.view()->rotateUV(true, MapViewBase::UVActionMode::Coarse);
    },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Rotate textures clockwise (fine)"},
    QObject::tr("Rotate Textures Clockwise (Fine)"),
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    QKeySequence{Qt::CTRL | Qt::Key_PageUp},
    [](auto& context) {
      context.view()->rotateUV(true, MapViewBase::UVActionMode::Fine);
    },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Rotate textures counter-clockwise"},
    QObject::tr("Rotate Textures Counter-clockwise"),
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    QKeySequence{Qt::Key_PageDown},
    [](auto& context) {
      context.view()->rotateUV(false, MapViewBase::UVActionMode::Normal);
    },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Rotate textures counter-clockwise (coarse)"},
    QObject::tr("Rotate Textures Counter-clockwise (Coarse)"),
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    QKeySequence{Qt::SHIFT | Qt::Key_PageDown},
    [](auto& context) {
      context.view()->rotateUV(false, MapViewBase::UVActionMode::Coarse);
    },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Rotate textures counter-clockwise (fine)"},
    QObject::tr("Rotate Textures Counter-clockwise (Fine)"),
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    QKeySequence{Qt::CTRL | Qt::Key_PageDown},
    [](auto& context) {
      context.view()->rotateUV(false, MapViewBase::UVActionMode::Fine);
    },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Reveal in texture browser"},
    QObject::tr("Reveal in texture browser"),
    ActionContext::View3D | ActionContext::AnySelection | ActionContext::AnyOrNoTool,
    QKeySequence{},
    [](auto& context) { context.frame()->revealMaterial(); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Flip textures horizontally"},
    QObject::tr("Flip textures horizontally"),
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    QKeySequence{Qt::CTRL | Qt::Key_F},
    [](auto& context) { context.view()->flipUV(vm::direction::right); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Flip textures vertically"},
    QObject::tr("Flip textures vertically"),
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    QKeySequence{Qt::CTRL | Qt::ALT | Qt::Key_F},
    [](auto& context) { context.view()->flipUV(vm::direction::up); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Reset texture alignment"},
    QObject::tr("Reset texture alignment"),
    ActionContext::AnyView | ActionContext::AnySelection | ActionContext::AnyOrNoTool,
    QKeySequence{Qt::SHIFT | Qt::Key_R},
    [](auto& context) { context.view()->resetUV(); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Reset texture alignment to world aligned"},
    QObject::tr("Reset texture alignment to world aligned"),
    ActionContext::AnyView | ActionContext::AnySelection | ActionContext::AnyOrNoTool,
    QKeySequence{Qt::SHIFT | Qt::ALT | Qt::Key_R},
    [](auto& context) { context.view()->resetUVToWorld(); },
    [](const auto& context) { return context.hasDocument(); },
  });

  /* ========== Tag Actions ========== */
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Make structural"},
    QObject::tr("Make Structural"),
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyOrNoTool,
    QKeySequence{Qt::ALT | Qt::Key_S},
    [](auto& context) { context.view()->makeStructural(); },
    [](const auto& context) { return context.hasDocument(); },
  });

  /* ========== View / Filter Actions ========== */
  addAction(Action{
    std::filesystem::path{
      "Controls/Map view/View Filter > Toggle show entity classnames"},
    QObject::tr("Toggle Show Entity Classnames"),
    ActionContext::Any,
    QKeySequence{},
    [](auto& context) { context.view()->toggleShowEntityClassnames(); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/View Filter > Toggle show group bounds"},
    QObject::tr("Toggle Show Group Bounds"),
    ActionContext::Any,
    QKeySequence{},
    [](auto& context) { context.view()->toggleShowGroupBounds(); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{
      "Controls/Map view/View Filter > Toggle show brush entity bounds"},
    QObject::tr("Toggle Show Brush Entity Bounds"),
    ActionContext::Any,
    QKeySequence{},
    [](auto& context) { context.view()->toggleShowBrushEntityBounds(); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{
      "Controls/Map view/View Filter > Toggle show point entity bounds"},
    QObject::tr("Toggle Show Point Entity Bounds"),
    ActionContext::Any,
    QKeySequence{},
    [](auto& context) { context.view()->toggleShowPointEntityBounds(); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/View Filter > Toggle show point entities"},
    QObject::tr("Toggle Show Point Entities"),
    ActionContext::Any,
    QKeySequence{},
    [](auto& context) { context.view()->toggleShowPointEntities(); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{
      "Controls/Map view/View Filter > Toggle show point entity models"},
    QObject::tr("Toggle Show Point Entity Models"),
    ActionContext::Any,
    QKeySequence{},
    [](auto& context) { context.view()->toggleShowPointEntityModels(); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/View Filter > Toggle show brushes"},
    QObject::tr("Toggle Show Brushes"),
    ActionContext::Any,
    QKeySequence{},
    [](auto& context) { context.view()->toggleShowBrushes(); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/View Filter > Show textures"},
    QObject::tr("Show Textures"),
    ActionContext::Any,
    QKeySequence{},
    [](auto& context) { context.view()->showMaterials(); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/View Filter > Hide textures"},
    QObject::tr("Hide Textures"),
    ActionContext::Any,
    QKeySequence{},
    [](auto& context) { context.view()->hideMaterials(); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/View Filter > Hide faces"},
    QObject::tr("Hide Faces"),
    ActionContext::Any,
    QKeySequence{},
    [](auto& context) { context.view()->hideFaces(); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/View Filter > Shade faces"},
    QObject::tr("Toggle Shade Faces"),
    ActionContext::Any,
    QKeySequence{},
    [](auto& context) { context.view()->toggleShadeFaces(); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/View Filter > Use fog"},
    QObject::tr("Toggle Show Fog"),
    ActionContext::Any,
    QKeySequence{},
    [](auto& context) { context.view()->toggleShowFog(); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/View Filter > Show edges"},
    QObject::tr("Toggle Show Edges"),
    ActionContext::Any,
    QKeySequence{},
    [](auto& context) { context.view()->toggleShowEdges(); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/View Filter > Show all entity links"},
    QObject::tr("Show All Entity Links"),
    ActionContext::Any,
    QKeySequence{},
    [](auto& context) { context.view()->showAllEntityLinks(); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{
      "Controls/Map view/View Filter > Show transitively selected entity links"},
    QObject::tr("Show Transitively Selected Entity Links"),
    ActionContext::Any,
    QKeySequence{},
    [](auto& context) { context.view()->showTransitivelySelectedEntityLinks(); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{
      "Controls/Map view/View Filter > Show directly selected entity links"},
    QObject::tr("Show Directly Selected Entity Links"),
    ActionContext::Any,
    QKeySequence{},
    [](auto& context) { context.view()->showDirectlySelectedEntityLinks(); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/View Filter > Hide entity links"},
    QObject::tr("Hide All Entity Links"),
    ActionContext::Any,
    QKeySequence{},
    [](auto& context) { context.view()->hideAllEntityLinks(); },
    [](const auto& context) { return context.hasDocument(); },
  });

  /* ========== Misc Actions ========== */
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Cycle map view"},
    QObject::tr("Cycle View"),
    ActionContext::Any,
    QKeySequence{Qt::Key_Space},
    [](auto& context) { context.view()->cycleMapView(); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Reset camera zoom"},
    QObject::tr("Reset Camera Zoom"),
    ActionContext::View3D | ActionContext::AnyOrNoTool | ActionContext::AnyOrNoSelection,
    QKeySequence{Qt::CTRL | Qt::ALT | Qt::Key_Z},
    [](auto& context) { context.view()->resetCameraZoom(); },
    [](const auto& context) { return context.hasDocument(); },
  });
  addAction(Action{
    std::filesystem::path{"Controls/Map view/Cancel"},
    QObject::tr("Cancel"),
    ActionContext::Any,
    QKeySequence{Qt::Key_Escape},
    [](auto& context) { context.view()->cancel(); },
    [](const auto& context) { return context.hasDocument(); },
  });
}

void ActionManager::createMenu()
{
  createFileMenu();
  createEditMenu();
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
    QObject::tr("New Document"),
    ActionContext::Any,
    QKeySequence::New,
    [](auto&) {
      auto& app = TrenchBroomApp::instance();
      app.newDocument();
    },
    [](const auto&) { return true; },
  }));
  fileMenu.addSeparator();
  fileMenu.addItem(addAction(Action{
    "Menu/File/Open...",
    QObject::tr("Open Document..."),
    ActionContext::Any,
    QKeySequence::Open,
    [](auto&) {
      auto& app = TrenchBroomApp::instance();
      app.openDocument();
    },
    [](const auto&) { return true; },
  }));
  fileMenu.addMenu("Open Recent", MenuEntryType::RecentDocuments);
  fileMenu.addSeparator();
  fileMenu.addItem(addAction(Action{
    "Menu/File/Save",
    QObject::tr("Save Document"),
    ActionContext::Any,
    QKeySequence::Save,
    [](auto& context) { context.frame()->saveDocument(); },
    [](const auto& context) { return context.hasDocument(); },
  }));
  fileMenu.addItem(addAction(Action{
    "Menu/File/Save as...",
    QObject::tr("Save Document as..."),
    ActionContext::Any,
    QKeySequence::SaveAs,
    [](auto& context) { context.frame()->saveDocumentAs(); },
    [](const auto& context) { return context.hasDocument(); },
  }));

  auto& exportMenu = fileMenu.addMenu("Export");
  exportMenu.addItem(addAction(Action{
    "Menu/File/Export/Wavefront OBJ...",
    QObject::tr("Wavefront OBJ..."),
    ActionContext::Any,
    QKeySequence{},
    [](auto& context) { context.frame()->exportDocumentAsObj(); },
    [](const auto& context) { return context.hasDocument(); },
  }));
  exportMenu.addItem(addAction(Action{
    "Menu/File/Export/Map...",
    QObject::tr("Map..."),
    ActionContext::Any,
    QKeySequence{},
    [](auto& context) { context.frame()->exportDocumentAsMap(); },
    [](const auto& context) { return context.hasDocument(); },
    std::nullopt,
    QObject::tr("Exports the current map to a .map file. Layers marked Omit From Export "
                "will be omitted."),
  }));

  /* ========== File Menu (Associated Resources) ========== */
  fileMenu.addSeparator();
  fileMenu.addItem(addAction(Action{
    "Menu/File/Load Point File...",
    QObject::tr("Load Point File..."),
    ActionContext::Any,
    QKeySequence{},
    [](auto& context) { context.frame()->loadPointFile(); },
    [](const auto& context) { return context.hasDocument(); },
  }));
  fileMenu.addItem(addAction(Action{
    "Menu/File/Reload Point File",
    QObject::tr("Reload Point File"),
    ActionContext::Any,
    QKeySequence{},
    [](auto& context) { context.frame()->reloadPointFile(); },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->canReloadPointFile();
    },
  }));
  fileMenu.addItem(addAction(Action{
    "Menu/File/Unload Point File",
    QObject::tr("Unload Point File"),
    ActionContext::Any,
    QKeySequence{},
    [](auto& context) { context.frame()->unloadPointFile(); },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->canUnloadPointFile();
    },
  }));
  fileMenu.addSeparator();
  fileMenu.addItem(addAction(Action{
    "Menu/File/Load Portal File...",
    QObject::tr("Load Portal File..."),
    ActionContext::Any,
    QKeySequence{},
    [](auto& context) { context.frame()->loadPortalFile(); },
    [](const auto& context) { return context.hasDocument(); },
  }));
  fileMenu.addItem(addAction(Action{
    "Menu/File/Reload Portal File",
    QObject::tr("Reload Portal File"),
    ActionContext::Any,
    QKeySequence{},
    [](auto& context) { context.frame()->reloadPortalFile(); },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->canReloadPortalFile();
    },
  }));
  fileMenu.addItem(addAction(Action{
    "Menu/File/Unload Portal File",
    QObject::tr("Unload Portal File"),
    ActionContext::Any,
    QKeySequence{},
    [](auto& context) { context.frame()->unloadPortalFile(); },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->canUnloadPortalFile();
    },
  }));
  fileMenu.addSeparator();
  fileMenu.addItem(addAction(Action{
    "Menu/File/Reload Texture Collections",
    QObject::tr("Reload Texture Collections"),
    ActionContext::Any,
    QKeySequence{Qt::Key_F5},
    [](auto& context) { context.frame()->reloadMaterialCollections(); },
    [](const auto& context) { return context.hasDocument(); },
  }));
  fileMenu.addItem(addAction(Action{
    "Menu/File/Reload Entity Definitions",
    QObject::tr("Reload Entity Definitions"),
    ActionContext::Any,
    QKeySequence{Qt::Key_F6},
    [](auto& context) { context.frame()->reloadEntityDefinitions(); },
    [](const auto& context) { return context.hasDocument(); },
  }));
  fileMenu.addSeparator();
  fileMenu.addItem(addAction(Action{
    "Menu/File/Revert",
    QObject::tr("Revert Document"),
    ActionContext::Any,
    QKeySequence{},
    [](auto& context) { context.frame()->revertDocument(); },
    [](const auto& context) { return context.hasDocument(); },
    std::nullopt,
    QObject::tr("Discards any unsaved changes and reloads the map file."),
  }));
  fileMenu.addItem(addAction(Action{
    "Menu/File/Close",
    QObject::tr("Close Document"),
    ActionContext::Any,
    QKeySequence::Close,
    [](auto& context) { context.frame()->closeDocument(); },
    [](const auto& context) { return context.hasDocument(); },
  }));
}

void ActionManager::createEditMenu()
{ /* ========== Edit Menu ========== */
  auto& editMenu = createMainMenu("Edit");
  editMenu.addItem(
    addAction(Action{
      std::filesystem::path{"Menu/Edit/Undo"},
      QObject::tr("Undo"),
      ActionContext::Any,
      QKeySequence::Undo,
      [](auto& context) { context.frame()->undo(); },
      [](const auto& context) {
        return context.hasDocument() && context.frame()->canUndo();
      },
    }),
    MenuEntryType::Undo);
  editMenu.addItem(
    addAction(Action{
      std::filesystem::path{"Menu/Edit/Redo"},
      QObject::tr("Redo"),
      ActionContext::Any,
      QKeySequence::Redo,
      [](auto& context) { context.frame()->redo(); },
      [](const auto& context) {
        return context.hasDocument() && context.frame()->canRedo();
      },
    }),
    MenuEntryType::Redo);
  editMenu.addSeparator();
  editMenu.addItem(addAction(Action{
    "Menu/Edit/Repeat",
    QObject::tr("Repeat Last Commands"),
    ActionContext::Any,
    QKeySequence{Qt::CTRL | Qt::Key_R},
    [](auto& context) { context.frame()->repeatLastCommands(); },
    [](const auto& context) { return context.hasDocument(); },
  }));
  editMenu.addItem(addAction(Action{
    "Menu/Edit/Clear Repeatable Commands",
    QObject::tr("Clear Repeatable Commands"),
    ActionContext::Any,
    QKeySequence{Qt::CTRL | Qt::SHIFT | Qt::Key_R},
    [](auto& context) { context.frame()->clearRepeatableCommands(); },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->hasRepeatableCommands();
    },
  }));
  editMenu.addSeparator();
  editMenu.addItem(
    addAction(Action{
      std::filesystem::path{"Menu/Edit/Cut"},
      QObject::tr("Cut"),
      ActionContext::Any,
      QKeySequence::Cut,
      [](auto& context) { context.frame()->cutSelection(); },
      [](const auto& context) {
        return context.hasDocument() && context.frame()->canCopySelection();
      },
    }),
    MenuEntryType::Cut);
  editMenu.addItem(
    addAction(Action{
      std::filesystem::path{"Menu/Edit/Copy"},
      QObject::tr("Copy"),
      ActionContext::Any,
      QKeySequence::Copy,
      [](auto& context) { context.frame()->copySelection(); },
      [](const auto& context) {
        return context.hasDocument() && context.frame()->canCopySelection();
      },
    }),
    MenuEntryType::Copy);
  editMenu.addItem(
    addAction(Action{
      std::filesystem::path{"Menu/Edit/Paste"},
      QObject::tr("Paste"),
      ActionContext::Any,
      QKeySequence::Paste,
      [](auto& context) { context.frame()->pasteAtCursorPosition(); },
      [](const auto& context) {
        return context.hasDocument() && context.frame()->canPaste();
      },
    }),
    MenuEntryType::Paste);
  editMenu.addItem(
    addAction(Action{
      std::filesystem::path{"Menu/Edit/Paste at Original Position"},
      QObject::tr("Paste at Original Position"),
      ActionContext::Any,
      QKeySequence{Qt::CTRL | Qt::ALT | Qt::Key_V},
      [](auto& context) { context.frame()->pasteAtOriginalPosition(); },
      [](const auto& context) {
        return context.hasDocument() && context.frame()->canPaste();
      },
    }),
    MenuEntryType::PasteAtOriginalPosition);
  editMenu.addItem(addAction(Action{
    "Menu/Edit/Duplicate",
    QObject::tr("Duplicate"),
    ActionContext::Any,
    QKeySequence{Qt::CTRL | Qt::Key_D},
    [](auto& context) { context.frame()->duplicateSelection(); },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->canDuplicateSelectino();
    },
    std::filesystem::path{"DuplicateObjects.svg"},
  }));
  editMenu.addItem(addAction(Action{
    std::filesystem::path{"Menu/Edit/Delete"},
    QObject::tr("Delete"),
    ActionContext::Any,
    QKeySequence{
#ifdef __APPLE__
      Qt::Key_Backspace
#else
      QKeySequence::Delete
#endif
    },
    [](auto& context) { context.frame()->deleteSelection(); },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->canDeleteSelection();
    },
  }));
  editMenu.addSeparator();
  editMenu.addItem(addAction(Action{
    "Menu/Edit/Select All",
    QObject::tr("Select All"),
    ActionContext::Any,
    QKeySequence::SelectAll,
    [](auto& context) { context.frame()->selectAll(); },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->canSelect();
    },
  }));
  editMenu.addItem(addAction(Action{
    "Menu/Edit/Select Siblings",
    QObject::tr("Select Siblings"),
    ActionContext::Any,
    QKeySequence{Qt::CTRL | Qt::Key_B},
    [](auto& context) { context.frame()->selectSiblings(); },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->canSelectSiblings();
    },
  }));
  editMenu.addItem(addAction(Action{
    "Menu/Edit/Select Touching",
    QObject::tr("Select Touching"),
    ActionContext::Any,
    QKeySequence{Qt::CTRL | Qt::Key_T},
    [](auto& context) { context.frame()->selectTouching(); },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->canSelectByBrush();
    },
  }));
  editMenu.addItem(addAction(Action{
    "Menu/Edit/Select Inside",
    QObject::tr("Select Inside"),
    ActionContext::Any,
    QKeySequence{Qt::CTRL | Qt::Key_E},
    [](auto& context) { context.frame()->selectInside(); },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->canSelectByBrush();
    },
  }));
  editMenu.addItem(addAction(Action{
    std::filesystem::path{"Menu/Edit/Select Tall"},
    QObject::tr("Select Tall"),
    ActionContext::Any,
    QKeySequence{Qt::CTRL | Qt::SHIFT | Qt::Key_E},
    [](auto& context) { context.frame()->selectTall(); },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->canSelectTall();
    },
  }));
  editMenu.addItem(addAction(Action{
    "Menu/Edit/Select by Line Number",
    QObject::tr("Select by Line Number..."),
    ActionContext::Any,
    QKeySequence{},
    [](auto& context) { context.frame()->selectByLineNumber(); },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->canSelect();
    },
  }));
  editMenu.addItem(addAction(Action{
    std::filesystem::path{"Menu/Edit/Select Inverse"},
    QObject::tr("Select Inverse"),
    ActionContext::Any,
    QKeySequence{Qt::CTRL | Qt::ALT | Qt::Key_A},
    [](auto& context) { context.frame()->selectInverse(); },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->canSelectInverse();
    },
  }));
  editMenu.addItem(addAction(Action{
    std::filesystem::path{"Menu/Edit/Select None"},
    QObject::tr("Select None"),
    ActionContext::Any,
    QKeySequence{Qt::CTRL | Qt::SHIFT | Qt::Key_A},
    [](auto& context) { context.frame()->selectNone(); },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->canDeselect();
    },
  }));
  editMenu.addSeparator();
  editMenu.addItem(addAction(Action{
    std::filesystem::path{"Menu/Edit/Group"},
    QObject::tr("Group Selected Objects"),
    ActionContext::Any,
    QKeySequence{Qt::CTRL | Qt::Key_G},
    [](auto& context) { context.frame()->groupSelectedObjects(); },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->canGroupSelectedObjects();
    },
  }));
  editMenu.addItem(addAction(Action{
    "Menu/Edit/Ungroup",
    QObject::tr("Ungroup Selected Objects"),
    ActionContext::Any,
    QKeySequence{Qt::CTRL | Qt::SHIFT | Qt::Key_G},
    [](auto& context) { context.frame()->ungroupSelectedObjects(); },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->canUngroupSelectedObjects();
    },
  }));
  editMenu.addSeparator();

  editMenu.addItem(addAction(Action{
    std::filesystem::path{"Menu/Edit/Create Linked Duplicate"},
    QObject::tr("Create Linked Duplicate"),
    ActionContext::Any,
    QKeySequence{Qt::CTRL | Qt::SHIFT | Qt::Key_D},
    [](auto& context) { context.document()->createLinkedDuplicate(); },
    [](const auto& context) {
      return context.hasDocument() && context.document()->canCreateLinkedDuplicate();
    },
  }));
  editMenu.addItem(addAction(Action{
    std::filesystem::path{"Menu/Edit/Select Linked Groups"},
    QObject::tr("Select Linked Groups"),
    ActionContext::Any,
    QKeySequence{},
    [](auto& context) { context.document()->selectLinkedGroups(); },
    [](const auto& context) {
      return context.hasDocument() && context.document()->canSelectLinkedGroups();
    },
  }));
  editMenu.addItem(addAction(Action{
    std::filesystem::path{"Menu/Edit/Separate Linked Groups"},
    QObject::tr("Separate Selected Groups"),
    ActionContext::Any,
    QKeySequence{},
    [](auto& context) { context.document()->separateLinkedGroups(); },
    [](const auto& context) {
      return context.hasDocument() && context.document()->canSeparateLinkedGroups();
    },
  }));
  editMenu.addItem(addAction(Action{
    std::filesystem::path{"Menu/Edit/Clear Protected Properties"},
    QObject::tr("Clear Protected Properties"),
    ActionContext::Any,
    QKeySequence{},
    [](auto& context) { context.document()->clearProtectedProperties(); },
    [](const auto& context) {
      return context.hasDocument() && context.document()->canClearProtectedProperties();
    },
  }));
  editMenu.addSeparator();

  editMenu.addItem(addAction(Action{
    std::filesystem::path{"Controls/Map view/Flip objects horizontally"},
    QObject::tr("Flip Horizontally"),
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyOrNoTool,
    QKeySequence{Qt::CTRL | Qt::Key_F},
    [](auto& context) { context.view()->flipObjects(vm::direction::left); },
    [](const auto& context) {
      return context.hasDocument() && context.view() && context.view()->canFlipObjects();
    },
    std::filesystem::path{"FlipHorizontally.svg"},
  }));
  editMenu.addItem(addAction(Action{
    std::filesystem::path{"Controls/Map view/Flip objects vertically"},
    QObject::tr("Flip Vertically"),
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyOrNoTool,
    QKeySequence{Qt::CTRL | Qt::ALT | Qt::Key_F},
    [](auto& context) { context.view()->flipObjects(vm::direction::up); },
    [](const auto& context) {
      return context.hasDocument() && context.view() && context.view()->canFlipObjects();
    },
    std::filesystem::path{"FlipVertically.svg"},
  }));
  editMenu.addItem(addAction(Action{
    std::filesystem::path{"Menu/Edit/Move objects"},
    QObject::tr("Move..."),
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyOrNoTool,
    QKeySequence{Qt::CTRL | Qt::ALT | Qt::Key_M},
    [](auto& context) { context.frame()->moveSelectedObjects(); },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->canMoveSelectedObjects();
    },
  }));
  editMenu.addSeparator();

  auto& toolMenu = editMenu.addMenu("Tools");
  toolMenu.addItem(addAction(Action{
    "Menu/Edit/Tools/Brush Tool",
    QObject::tr("Brush Tool"),
    ActionContext::Any,
    QKeySequence{Qt::Key_B},
    [](auto& context) { context.frame()->toggleAssembleBrushTool(); },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->canToggleAssembleBrushTool();
    },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->assembleBrushToolActive();
    },
    std::filesystem::path{"BrushTool.svg"},
  }));
  toolMenu.addItem(addAction(Action{
    "Menu/Edit/Tools/Clip Tool",
    QObject::tr("Clip Tool"),
    ActionContext::Any,
    QKeySequence{Qt::Key_C},
    [](auto& context) { context.frame()->toggleClipTool(); },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->canToggleClipTool();
    },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->clipToolActive();
    },
    std::filesystem::path{"ClipTool.svg"},
  }));
  toolMenu.addItem(addAction(Action{
    "Menu/Edit/Tools/Rotate Tool",
    QObject::tr("Rotate Tool"),
    ActionContext::Any,
    QKeySequence{Qt::Key_R},
    [](auto& context) { context.frame()->toggleRotateObjectsTool(); },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->canToggleRotateObjectsTool();
    },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->rotateObjectsToolActive();
    },
    std::filesystem::path{"RotateTool.svg"},
  }));
  toolMenu.addItem(addAction(Action{
    "Menu/Edit/Tools/Scale Tool",
    QObject::tr("Scale Tool"),
    ActionContext::Any,
    QKeySequence{Qt::Key_T},
    [](auto& context) { context.frame()->toggleScaleObjectsTool(); },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->canToggleScaleObjectsTool();
    },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->scaleObjectsToolActive();
    },
    std::filesystem::path{"ScaleTool.svg"},
  }));
  toolMenu.addItem(addAction(Action{
    "Menu/Edit/Tools/Shear Tool",
    QObject::tr("Shear Tool"),
    ActionContext::Any,
    QKeySequence{Qt::Key_G},
    [](auto& context) { context.frame()->toggleShearObjectsTool(); },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->canToggleShearObjectsTool();
    },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->shearObjectsToolActive();
    },
    std::filesystem::path{"ShearTool.svg"},
  }));
  toolMenu.addItem(addAction(Action{
    "Menu/Edit/Tools/Vertex Tool",
    QObject::tr("Vertex Tool"),
    ActionContext::Any,
    QKeySequence{Qt::Key_V},
    [](auto& context) { context.frame()->toggleVertexTool(); },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->canToggleVertexTool();
    },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->vertexToolActive();
    },
    std::filesystem::path{"VertexTool.svg"},
  }));
  toolMenu.addItem(addAction(Action{
    "Menu/Edit/Tools/Edge Tool",
    QObject::tr("Edge Tool"),
    ActionContext::Any,
    QKeySequence{Qt::Key_E},
    [](auto& context) { context.frame()->toggleEdgeTool(); },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->canToggleEdgeTool();
    },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->edgeToolActive();
    },
    std::filesystem::path{"EdgeTool.svg"},
  }));
  toolMenu.addItem(addAction(Action{
    "Menu/Edit/Tools/Face Tool",
    QObject::tr("Face Tool"),
    ActionContext::Any,
    QKeySequence{Qt::Key_F},
    [](auto& context) { context.frame()->toggleFaceTool(); },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->canToggleFaceTool();
    },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->faceToolActive();
    },
    std::filesystem::path{"FaceTool.svg"},
  }));
  toolMenu.addItem(addAction(Action{
    "Controls/Map view/Deactivate current tool",
    QObject::tr("Deactivate Current Tool"),
    ActionContext::Any,
    QKeySequence{Qt::SHIFT | Qt::Key_Escape},
    [](auto& context) { context.view()->deactivateTool(); },
    [](const auto& context) { return context.hasDocument(); },
    [](const auto& context) {
      return context.hasDocument() && !context.frame()->anyToolActive();
    },
    std::filesystem::path{"NoTool.svg"},
  }));

  auto& csgMenu = editMenu.addMenu("CSG");
  csgMenu.addItem(addAction(Action{
    "Menu/Edit/CSG/Convex Merge",
    QObject::tr("Convex Merge"),
    ActionContext::Any,
    QKeySequence{Qt::CTRL | Qt::Key_J},
    [](auto& context) { context.frame()->csgConvexMerge(); },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->canDoCsgConvexMerge();
    },
  }));
  csgMenu.addItem(addAction(Action{
    "Menu/Edit/CSG/Subtract",
    QObject::tr("Subtract"),
    ActionContext::Any,
    QKeySequence{Qt::CTRL | Qt::Key_K},
    [](auto& context) { context.frame()->csgSubtract(); },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->canDoCsgSubtract();
    },
  }));
  csgMenu.addItem(addAction(Action{
    "Menu/Edit/CSG/Hollow",
    QObject::tr("Hollow"),
    ActionContext::Any,
    QKeySequence{Qt::CTRL | Qt::SHIFT | Qt::Key_K},
    [](auto& context) { context.frame()->csgHollow(); },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->canDoCsgHollow();
    },
  }));
  csgMenu.addItem(addAction(Action{
    "Menu/Edit/CSG/Intersect",
    QObject::tr("Intersect"),
    ActionContext::Any,
    QKeySequence{Qt::CTRL | Qt::Key_L},
    [](auto& context) { context.frame()->csgIntersect(); },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->canDoCsgIntersect();
    },
  }));

  editMenu.addSeparator();
  editMenu.addItem(addAction(Action{
    "Menu/Edit/Snap Vertices to Integer",
    QObject::tr("Snap Vertices to Integer"),
    ActionContext::Any,
    QKeySequence{Qt::CTRL | Qt::SHIFT | Qt::Key_V},
    [](auto& context) { context.frame()->snapVerticesToInteger(); },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->canSnapVertices();
    },
  }));
  editMenu.addItem(addAction(Action{
    "Menu/Edit/Snap Vertices to Grid",
    QObject::tr("Snap Vertices to Grid"),
    ActionContext::Any,
    QKeySequence{Qt::CTRL | Qt::ALT | Qt::SHIFT | Qt::Key_V},
    [](auto& context) { context.frame()->snapVerticesToGrid(); },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->canSnapVertices();
    },
  }));
  editMenu.addSeparator();
  editMenu.addItem(addAction(Action{
    "Menu/Edit/Texture Lock",
    QObject::tr("Texture Lock"),
    ActionContext::Any,
    QKeySequence{},
    [](auto& context) { context.frame()->toggleAlignmentLock(); },
    [](const auto& context) { return context.hasDocument(); },
    [](const auto&) { return pref(Preferences::AlignmentLock); },
    std::filesystem::path{"AlignmentLock.svg"},
  }));
  editMenu.addItem(addAction(Action{
    "Menu/Edit/UV Lock",
    QObject::tr("UV Lock"),
    ActionContext::Any,
    QKeySequence{Qt::Key_U},
    [](auto& context) { context.frame()->toggleUVLock(); },
    [](const auto& context) { return context.hasDocument(); },
    [](const auto&) { return pref(Preferences::UVLock); },
    std::filesystem::path{"UVLock.svg"},
  }));
  editMenu.addSeparator();
  editMenu.addItem(addAction(Action{
    "Menu/Edit/Replace Texture...",
    QObject::tr("Replace Texture..."),
    ActionContext::Any,
    QKeySequence{},
    [](auto& context) { context.frame()->replaceMaterial(); },
    [](const auto& context) { return context.hasDocument(); },
  }));
}

void ActionManager::createViewMenu()
{
  auto& viewMenu = createMainMenu("View");
  auto& gridMenu = viewMenu.addMenu("Grid");
  gridMenu.addItem(addAction(Action{
    "Menu/View/Grid/Show Grid",
    QObject::tr("Show Grid"),
    ActionContext::Any,
    QKeySequence{Qt::Key_0},
    [](auto& context) { context.frame()->toggleShowGrid(); },
    [](const auto& context) { return context.hasDocument(); },
    [](const auto& context) {
      return context.hasDocument() && context.document()->grid().visible();
    },
  }));
  gridMenu.addItem(addAction(Action{
    "Menu/View/Grid/Snap to Grid",
    QObject::tr("Snap to Grid"),
    ActionContext::Any,
    QKeySequence{Qt::ALT | Qt::Key_0},
    [](auto& context) { context.frame()->toggleSnapToGrid(); },
    [](const auto& context) { return context.hasDocument(); },
    [](const auto& context) {
      return context.hasDocument() && context.document()->grid().snap();
    },
  }));
  gridMenu.addItem(addAction(Action{
    "Menu/View/Grid/Increase Grid Size",
    QObject::tr("Increase Grid Size"),
    ActionContext::Any,
    QKeySequence{Qt::Key_Plus},
    [](auto& context) { context.frame()->incGridSize(); },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->canIncGridSize();
    },
  }));
  gridMenu.addItem(addAction(Action{
    "Menu/View/Grid/Decrease Grid Size",
    QObject::tr("Decrease Grid Size"),
    ActionContext::Any,
    QKeySequence{Qt::Key_Minus},
    [](auto& context) { context.frame()->decGridSize(); },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->canDecGridSize();
    },
  }));
  gridMenu.addSeparator();
  gridMenu.addItem(addAction(Action{
    "Menu/View/Grid/Set Grid Size 0.125",
    QObject::tr("Set Grid Size 0.125"),
    ActionContext::Any,
    QKeySequence{},
    [](auto& context) { context.frame()->setGridSize(-3); },
    [](const auto& context) { return context.hasDocument(); },
    [](const auto& context) {
      return context.hasDocument() && context.document()->grid().size() == -3;
    },
  }));
  gridMenu.addItem(addAction(Action{
    "Menu/View/Grid/Set Grid Size 0.25",
    QObject::tr("Set Grid Size 0.25"),
    ActionContext::Any,
    QKeySequence{},
    [](auto& context) { context.frame()->setGridSize(-2); },
    [](const auto& context) { return context.hasDocument(); },
    [](const auto& context) {
      return context.hasDocument() && context.document()->grid().size() == -2;
    },
  }));
  gridMenu.addItem(addAction(Action{
    "Menu/View/Grid/Set Grid Size 0.5",
    QObject::tr("Set Grid Size 0.5"),
    ActionContext::Any,
    QKeySequence{},
    [](auto& context) { context.frame()->setGridSize(-1); },
    [](const auto& context) { return context.hasDocument(); },
    [](const auto& context) {
      return context.hasDocument() && context.document()->grid().size() == -1;
    },
  }));
  gridMenu.addItem(addAction(Action{
    "Menu/View/Grid/Set Grid Size 1",
    QObject::tr("Set Grid Size 1"),
    ActionContext::Any,
    QKeySequence{Qt::Key_1},
    [](auto& context) { context.frame()->setGridSize(0); },
    [](const auto& context) { return context.hasDocument(); },
    [](const auto& context) {
      return context.hasDocument() && context.document()->grid().size() == 0;
    },
  }));
  gridMenu.addItem(addAction(Action{
    "Menu/View/Grid/Set Grid Size 2",
    QObject::tr("Set Grid Size 2"),
    ActionContext::Any,
    QKeySequence{Qt::Key_2},
    [](auto& context) { context.frame()->setGridSize(1); },
    [](const auto& context) { return context.hasDocument(); },
    [](const auto& context) {
      return context.hasDocument() && context.document()->grid().size() == 1;
    },
  }));
  gridMenu.addItem(addAction(Action{
    "Menu/View/Grid/Set Grid Size 4",
    QObject::tr("Set Grid Size 4"),
    ActionContext::Any,
    QKeySequence{Qt::Key_3},
    [](auto& context) { context.frame()->setGridSize(2); },
    [](const auto& context) { return context.hasDocument(); },
    [](const auto& context) {
      return context.hasDocument() && context.document()->grid().size() == 2;
    },
  }));
  gridMenu.addItem(addAction(Action{
    "Menu/View/Grid/Set Grid Size 8",
    QObject::tr("Set Grid Size 8"),
    ActionContext::Any,
    QKeySequence{Qt::Key_4},
    [](auto& context) { context.frame()->setGridSize(3); },
    [](const auto& context) { return context.hasDocument(); },
    [](const auto& context) {
      return context.hasDocument() && context.document()->grid().size() == 3;
    },
  }));
  gridMenu.addItem(addAction(Action{
    "Menu/View/Grid/Set Grid Size 16",
    QObject::tr("Set Grid Size 16"),
    ActionContext::Any,
    QKeySequence{Qt::Key_5},
    [](auto& context) { context.frame()->setGridSize(4); },
    [](const auto& context) { return context.hasDocument(); },
    [](const auto& context) {
      return context.hasDocument() && context.document()->grid().size() == 4;
    },
  }));
  gridMenu.addItem(addAction(Action{
    "Menu/View/Grid/Set Grid Size 32",
    QObject::tr("Set Grid Size 32"),
    ActionContext::Any,
    QKeySequence{Qt::Key_6},
    [](auto& context) { context.frame()->setGridSize(5); },
    [](const auto& context) { return context.hasDocument(); },
    [](const auto& context) {
      return context.hasDocument() && context.document()->grid().size() == 5;
    },
  }));
  gridMenu.addItem(addAction(Action{
    "Menu/View/Grid/Set Grid Size 64",
    QObject::tr("Set Grid Size 64"),
    ActionContext::Any,
    QKeySequence{Qt::Key_7},
    [](auto& context) { context.frame()->setGridSize(6); },
    [](const auto& context) { return context.hasDocument(); },
    [](const auto& context) {
      return context.hasDocument() && context.document()->grid().size() == 6;
    },
  }));
  gridMenu.addItem(addAction(Action{
    "Menu/View/Grid/Set Grid Size 128",
    QObject::tr("Set Grid Size 128"),
    ActionContext::Any,
    QKeySequence{Qt::Key_8},
    [](auto& context) { context.frame()->setGridSize(7); },
    [](const auto& context) { return context.hasDocument(); },
    [](const auto& context) {
      return context.hasDocument() && context.document()->grid().size() == 7;
    },
  }));
  gridMenu.addItem(addAction(Action{
    "Menu/View/Grid/Set Grid Size 256",
    QObject::tr("Set Grid Size 256"),
    ActionContext::Any,
    QKeySequence{Qt::Key_9},
    [](auto& context) { context.frame()->setGridSize(8); },
    [](const auto& context) { return context.hasDocument(); },
    [](const auto& context) {
      return context.hasDocument() && context.document()->grid().size() == 8;
    },
  }));

  auto& cameraMenu = viewMenu.addMenu("Camera");
  cameraMenu.addItem(addAction(Action{
    "Menu/View/Camera/Move to Next Point",
    QObject::tr("Move Camera to Next Point"),
    ActionContext::Any,
    QKeySequence{Qt::Key_Period},
    [](auto& context) { context.frame()->moveCameraToNextPoint(); },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->canMoveCameraToNextPoint();
    },
  }));
  cameraMenu.addItem(addAction(Action{
    "Menu/View/Camera/Move to Previous Point",
    QObject::tr("Move Camera to Previous Point"),
    ActionContext::Any,
    QKeySequence{Qt::Key_Comma},
    [](auto& context) { context.frame()->moveCameraToPreviousPoint(); },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->canMoveCameraToPreviousPoint();
    },
  }));
  cameraMenu.addItem(addAction(Action{
    "Menu/View/Camera/Reset 2D Cameras",
    QObject::tr("Reset 2D Cameras"),
    ActionContext::Any,
    QKeySequence{Qt::CTRL | Qt::SHIFT | Qt::Key_U},
    [](auto& context) { context.frame()->reset2dCameras(); },
    [](const auto& context) {
      return context.hasDocument() && !pref(Preferences::Link2DCameras);
    },
  }));
  cameraMenu.addItem(addAction(Action{
    "Menu/View/Camera/Focus on Selection",
    QObject::tr("Focus Camera on Selection"),
    ActionContext::Any,
    QKeySequence{Qt::CTRL | Qt::Key_U},
    [](auto& context) { context.frame()->focusCameraOnSelection(); },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->canFocusCamera();
    },
  }));
  cameraMenu.addItem(addAction(Action{
    "Menu/View/Camera/Move Camera to...",
    QObject::tr("Move Camera to..."),
    ActionContext::Any,
    QKeySequence{},
    [](auto& context) { context.frame()->moveCameraToPosition(); },
    [](const auto& context) { return context.hasDocument(); },
  }));

  viewMenu.addSeparator();
  viewMenu.addItem(addAction(Action{
    "Menu/View/Isolate",
    QObject::tr("Isolate Selection"),
    ActionContext::Any,
    QKeySequence{Qt::CTRL | Qt::Key_I},
    [](auto& context) { context.frame()->isolateSelection(); },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->canIsolateSelection();
    },
  }));
  viewMenu.addItem(addAction(Action{
    "Menu/View/Hide",
    QObject::tr("Hide Selection"),
    ActionContext::Any,
    QKeySequence{Qt::CTRL | Qt::ALT | Qt::Key_I},
    [](auto& context) { context.frame()->hideSelection(); },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->canHideSelection();
    },
  }));
  viewMenu.addItem(addAction(Action{
    "Menu/View/Show All",
    QObject::tr("Show All"),
    ActionContext::Any,
    QKeySequence{Qt::CTRL | Qt::SHIFT | Qt::Key_I},
    [](auto& context) { context.frame()->showAll(); },
    [](const auto& context) { return context.hasDocument(); },
  }));
  viewMenu.addSeparator();
  viewMenu.addItem(addAction(Action{
    "Menu/View/Switch to Map Inspector",
    QObject::tr("Show Map Inspector"),
    ActionContext::Any,
    QKeySequence{Qt::CTRL | Qt::Key_1},
    [](auto& context) { context.frame()->switchToInspectorPage(InspectorPage::Map); },
    [](const auto& context) { return context.hasDocument(); },
  }));
  viewMenu.addItem(addAction(Action{
    "Menu/View/Switch to Entity Inspector",
    QObject::tr("Show Entity Inspector"),
    ActionContext::Any,
    QKeySequence{Qt::CTRL | Qt::Key_2},
    [](auto& context) { context.frame()->switchToInspectorPage(InspectorPage::Entity); },
    [](const auto& context) { return context.hasDocument(); },
  }));
  viewMenu.addItem(addAction(Action{
    "Menu/View/Switch to Face Inspector",
    QObject::tr("Show Face Inspector"),
    ActionContext::Any,
    QKeySequence{Qt::CTRL | Qt::Key_3},
    [](auto& context) { context.frame()->switchToInspectorPage(InspectorPage::Face); },
    [](const auto& context) { return context.hasDocument(); },
  }));
  viewMenu.addSeparator();
  viewMenu.addItem(addAction(Action{
    "Menu/View/Toggle Toolbar",
    QObject::tr("Toggle Toolbar"),
    ActionContext::Any,
    QKeySequence{Qt::CTRL | Qt::ALT | Qt::Key_T},
    [](auto& context) { context.frame()->toggleToolbar(); },
    [](const auto& context) { return context.hasDocument(); },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->toolbarVisible();
    },
  }));
  viewMenu.addItem(addAction(Action{
    "Menu/View/Toggle Info Panel",
    QObject::tr("Toggle Info Panel"),
    ActionContext::Any,
    QKeySequence{Qt::CTRL | Qt::Key_4},
    [](auto& context) { context.frame()->toggleInfoPanel(); },
    [](const auto& context) { return context.hasDocument(); },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->infoPanelVisible();
    },
  }));
  viewMenu.addItem(addAction(Action{
    "Menu/View/Toggle Inspector",
    QObject::tr("Toggle Inspector"),
    ActionContext::Any,
    QKeySequence{Qt::CTRL | Qt::Key_5},
    [](auto& context) { context.frame()->toggleInspector(); },
    [](const auto& context) { return context.hasDocument(); },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->inspectorVisible();
    },
  }));
  viewMenu.addItem(addAction(Action{
    "Menu/View/Maximize Current View",
    QObject::tr("Maximize Current View"),
    ActionContext::Any,
#ifdef Q_OS_MACOS
    // Command + Space opens Spotlight so we can't use it, so use Ctrl + Space instead.
    QKeySequence{Qt::META | Qt::Key_Space},
#else
    QKeySequence{Qt::CTRL | Qt::Key_Space},
#endif
    [](auto& context) { context.frame()->toggleMaximizeCurrentView(); },
    [](const auto& context) { return context.hasDocument(); },
    [](const auto& context) {
      return context.hasDocument() && context.frame()->currentViewMaximized();
    },
  }));
  viewMenu.addSeparator();
  viewMenu.addItem(addAction(Action{
    "Menu/File/Preferences...",
    QObject::tr("Preferences..."),
    ActionContext::Any,
    QKeySequence::Preferences,
    [](auto&) {
      auto& app = TrenchBroomApp::instance();
      app.showPreferences();
    },
    [](const auto&) { return true; },
  }));
}

void ActionManager::createRunMenu()
{
  auto& runMenu = createMainMenu("Run");
  runMenu.addItem(addAction(Action{
    "Menu/Run/Compile...",
    QObject::tr("Compile Map..."),
    ActionContext::Any,
    QKeySequence{},
    [](auto& context) { context.frame()->showCompileDialog(); },
    [](const auto& context) { return context.hasDocument(); },
  }));
  runMenu.addItem(addAction(Action{
    "Menu/Run/Launch...",
    QObject::tr("Launch Engine..."),
    ActionContext::Any,
    QKeySequence{},
    [](auto& context) { context.frame()->showLaunchEngineDialog(); },
    [](const auto& context) { return context.hasDocument(); },
  }));
}

void ActionManager::createDebugMenu()
{
#ifndef NDEBUG
  auto& debugMenu = createMainMenu("Debug");
  debugMenu.addItem(addAction(Action{
    "Menu/Debug/Print Vertices",
    QObject::tr("Print Vertices to Console"),
    ActionContext::Any,
    QKeySequence{},
    [](auto& context) { context.frame()->debugPrintVertices(); },
    [](const auto& context) { return context.hasDocument(); },
  }));
  debugMenu.addItem(addAction(Action{
    "Menu/Debug/Create Brush...",
    QObject::tr("Create Brush..."),
    ActionContext::Any,
    QKeySequence{},
    [](auto& context) { context.frame()->debugCreateBrush(); },
    [](const auto& context) { return context.hasDocument(); },
  }));
  debugMenu.addItem(addAction(Action{
    "Menu/Debug/Create Cube...",
    QObject::tr("Create Cube..."),
    ActionContext::Any,
    QKeySequence{},
    [](auto& context) { context.frame()->debugCreateCube(); },
    [](const auto& context) { return context.hasDocument(); },
  }));
  debugMenu.addItem(addAction(Action{
    "Menu/Debug/Clip Brush...",
    QObject::tr("Clip Brush..."),
    ActionContext::Any,
    QKeySequence{},
    [](auto& context) { context.frame()->debugClipBrush(); },
    [](const auto& context) { return context.hasDocument(); },
  }));
  debugMenu.addItem(addAction(Action{
    "Menu/Debug/Crash...",
    QObject::tr("Crash..."),
    ActionContext::Any,
    QKeySequence{},
    [](auto& context) { context.frame()->debugCrash(); },
    [](const auto& context) { return context.hasDocument(); },
  }));
  debugMenu.addItem(addAction(Action{
    "Menu/Debug/Throw Exception During Command",
    QObject::tr("Throw Exception During Command"),
    ActionContext::Any,
    QKeySequence{},
    [](auto& context) { context.frame()->debugThrowExceptionDuringCommand(); },
    [](const auto& context) { return context.hasDocument(); },
  }));
  debugMenu.addItem(addAction(Action{
    "Menu/Debug/Show Crash Report Dialog",
    QObject::tr("Show Crash Report Dialog..."),
    ActionContext::Any,
    QKeySequence{},
    [](auto&) {
      auto& app = TrenchBroomApp::instance();
      app.debugShowCrashReportDialog();
    },
    [](const auto&) { return true; },
  }));
  debugMenu.addItem(addAction(Action{
    "Menu/Debug/Set Window Size...",
    QObject::tr("Set Window Size..."),
    ActionContext::Any,
    QKeySequence{},
    [](auto& context) { context.frame()->debugSetWindowSize(); },
    [](const auto& context) { return context.hasDocument(); },
  }));
  debugMenu.addItem(addAction(Action{
    "Menu/Debug/Show Palette...",
    QObject::tr("Show Palette..."),
    ActionContext::Any,
    QKeySequence{},
    [](auto& context) { context.frame()->debugShowPalette(); },
    [](const auto& context) { return context.hasDocument(); },
  }));
#endif
}

void ActionManager::createHelpMenu()
{
  auto& helpMenu = createMainMenu("Help");
  helpMenu.addItem(addAction(Action{
    std::filesystem::path{"Menu/Help/TrenchBroom Manual"},
    QObject::tr("TrenchBroom Manual"),
    ActionContext::Any,
    QKeySequence{QKeySequence::HelpContents},
    [](auto&) {
      auto& app = TrenchBroomApp::instance();
      app.showManual();
    },
    [](const auto&) { return true; },
  }));
  helpMenu.addItem(addAction(Action{
    "Menu/File/About TrenchBroom",
    QObject::tr("About TrenchBroom"),
    ActionContext::Any,
    QKeySequence{},
    [](auto&) {
      auto& app = TrenchBroomApp::instance();
      app.showAboutDialog();
    },
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
  m_toolBar.addItem(existingAction("Menu/Edit/Tools/Rotate Tool"));
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

const Action& ActionManager::existingAction(
  const std::filesystem::path& preferencePath) const
{
  auto it = m_actions.find(preferencePath);
  ensure(it != m_actions.end(), "couldn't find action");
  return it->second;
}

const Action& ActionManager::addAction(Action action)
{
  auto [it, didInsert] = m_actions.insert({action.preferencePath(), std::move(action)});
  ensure(didInsert, "duplicate action");
  return it->second;
}

namespace
{

struct ActionConflictCmp
{
  bool operator()(const Action* lhs, const Action* rhs) const
  {
    if (actionContextMatches(lhs->actionContext(), rhs->actionContext()))
    {
      // if the two have the same sequence, they would be in conflict, so we compare the
      // sequences
      const auto lhsKeySequence = lhs->keySequence();
      const auto rhsKeySequence = rhs->keySequence();
      return lhsKeySequence < rhsKeySequence;
    }
    // otherwise, we just compare by the action context
    return lhs->actionContext() < rhs->actionContext();
  }
};

} // namespace

std::vector<size_t> findConflicts(const std::vector<const Action*>& actions)
{
  auto entries = std::map<const Action*, size_t, ActionConflictCmp>{};
  auto conflicts = std::vector<size_t>{};

  for (size_t i = 0; i < actions.size(); ++i)
  {
    const auto& action = *actions[i];
    const auto keySequence = action.keySequence();
    if (keySequence.count() > 0)
    {
      const auto [it, noConflict] = entries.emplace(&action, i);
      if (!noConflict)
      {
        // found a duplicate, so there are conflicts
        const auto otherIndex = it->second;
        conflicts.emplace_back(otherIndex);
        conflicts.emplace_back(i);
      }
    }
  }

  return conflicts;
}

} // namespace tb::ui
