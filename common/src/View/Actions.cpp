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

#include "Actions.h"

#include "Assets/EntityDefinition.h"
#include "Model/EntityProperties.h"
#include "Model/Tag.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "TrenchBroomApp.h"
#include "View/Grid.h"
#include "View/Inspector.h"
#include "View/MapDocument.h"
#include "View/MapFrame.h"
#include "View/MapViewBase.h"

#include "vecmath/util.h"

#include <QKeySequence>
#include <QString>

#include <cassert>
#include <set>
#include <string>

namespace TrenchBroom
{
namespace View
{
// ActionExecutionContext

ActionExecutionContext::ActionExecutionContext(MapFrame* mapFrame, MapViewBase* mapView)
  : m_actionContext(mapView != nullptr ? mapView->actionContext() : ActionContext::Any)
  , // cache here for performance reasons
  m_frame(mapFrame)
  , m_mapView(mapView)
{
  if (m_frame != nullptr)
  {
    assert(m_mapView != nullptr);
  }
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

MapViewBase* ActionExecutionContext::view()
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

// Action

Action::~Action() = default;

Action::Action(
  const IO::Path& preferencePath,
  const QString& label,
  const ActionContext::Type actionContext,
  const QKeySequence& defaultShortcut,
  const IO::Path& iconPath,
  const QString& statusTip)
  : m_label(label)
  , m_preferencePath(preferencePath)
  , m_actionContext(actionContext)
  , m_defaultShortcut(defaultShortcut)
  , m_iconPath(iconPath)
  , m_statusTip(statusTip)
{
}

const QString& Action::label() const
{
  return m_label;
}

const IO::Path& Action::preferencePath() const
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
  auto& pref = prefs.dynamicPreference(m_preferencePath, QKeySequence(m_defaultShortcut));
  return prefs.get(pref);
}

void Action::setKeySequence(const QKeySequence& keySequence) const
{
  auto& prefs = PreferenceManager::instance();
  auto& pref = prefs.dynamicPreference(m_preferencePath, QKeySequence(m_defaultShortcut));
  prefs.set(pref, keySequence);
}

void Action::resetKeySequence() const
{
  setKeySequence(m_defaultShortcut);
}

bool Action::hasIcon() const
{
  return !m_iconPath.isEmpty();
}

const IO::Path& Action::iconPath() const
{
  assert(hasIcon());
  return m_iconPath;
}

const QString& Action::statusTip() const
{
  return m_statusTip;
}

// MenuVisitor

MenuVisitor::~MenuVisitor() = default;

// MenuEntry

MenuEntry::MenuEntry(const MenuEntryType entryType)
  : m_entryType(entryType)
{
}

MenuEntry::~MenuEntry() = default;

MenuEntryType MenuEntry::entryType() const
{
  return m_entryType;
}

// MenuSeparatorItem

MenuSeparatorItem::MenuSeparatorItem()
  : MenuEntry(MenuEntryType::Menu_None)
{
}

void MenuSeparatorItem::accept(MenuVisitor& menuVisitor) const
{
  menuVisitor.visit(*this);
}

// MenuActionItem

MenuActionItem::MenuActionItem(const Action* action, const MenuEntryType entryType)
  : MenuEntry(entryType)
  , m_action(action)
{
}

const QString& MenuActionItem::label() const
{
  return m_action->label();
}

const Action& MenuActionItem::action() const
{
  return *m_action;
}

void MenuActionItem::accept(MenuVisitor& menuVisitor) const
{
  menuVisitor.visit(*this);
}

// Menu

Menu::Menu(const std::string& name, const MenuEntryType entryType)
  : MenuEntry(entryType)
  , m_name(name)
{
}

const std::string& Menu::name() const
{
  return m_name;
}

Menu& Menu::addMenu(const std::string& name, const MenuEntryType entryType)
{
  m_entries.emplace_back(std::make_unique<Menu>(name, entryType));
  return *static_cast<Menu*>(m_entries.back().get());
}

void Menu::addSeparator()
{
  m_entries.emplace_back(std::make_unique<MenuSeparatorItem>());
}

MenuActionItem& Menu::addItem(const Action* action, const MenuEntryType entryType)
{
  m_entries.emplace_back(std::make_unique<MenuActionItem>(action, entryType));
  return *static_cast<MenuActionItem*>(m_entries.back().get());
}

void Menu::accept(MenuVisitor& visitor) const
{
  visitor.visit(*this);
}

void Menu::visitEntries(MenuVisitor& visitor) const
{
  for (const auto& entry : m_entries)
  {
    entry->accept(visitor);
  }
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

std::vector<std::unique_ptr<Action>> ActionManager::createTagActions(
  const std::vector<Model::SmartTag>& tags) const
{
  std::vector<std::unique_ptr<Action>> result;

  for (const auto& tag : tags)
  {
    result.push_back(makeAction(
      IO::Path("Filters/Tags/" + tag.name() + "/Toggle Visible"),
      QObject::tr("Toggle %1 visible").arg(QString::fromStdString(tag.name())),
      ActionContext::Any,
      [&tag](ActionExecutionContext& context) { context.view()->toggleTagVisible(tag); },
      [](ActionExecutionContext& context) { return context.hasDocument(); }));
    if (tag.canEnable())
    {
      result.push_back(makeAction(
        IO::Path("Tags/" + tag.name() + "/Enable"),
        QObject::tr("Turn Selection into %1").arg(QString::fromStdString(tag.name())),
        ActionContext::AnyView | ActionContext::AnySelection | ActionContext::AnyOrNoTool,
        [&tag](ActionExecutionContext& context) { context.view()->enableTag(tag); },
        [](ActionExecutionContext& context) { return context.hasDocument(); }));
    }
    if (tag.canDisable())
    {
      result.push_back(makeAction(
        IO::Path("Tags/" + tag.name() + "/Disable"),
        QObject::tr("Turn Selection into non-%1").arg(QString::fromStdString(tag.name())),
        ActionContext::AnyView | ActionContext::AnySelection | ActionContext::AnyOrNoTool,
        [&tag](ActionExecutionContext& context) { context.view()->disableTag(tag); },
        [](ActionExecutionContext& context) { return context.hasDocument(); }));
    }
  }

  return result;
}

std::vector<std::unique_ptr<Action>> ActionManager::createEntityDefinitionActions(
  const std::vector<Assets::EntityDefinition*>& entityDefinitions) const
{
  std::vector<std::unique_ptr<Action>> result;

  for (const auto* definition : entityDefinitions)
  {
    result.push_back(makeAction(
      IO::Path("Entities/" + definition->name() + "/Toggle"),
      QObject::tr("Toggle %1 visible").arg(QString::fromStdString(definition->name())),
      ActionContext::Any,
      [definition](ActionExecutionContext& context) {
        context.view()->toggleEntityDefinitionVisible(definition);
      },
      [](ActionExecutionContext& context) { return context.hasDocument(); }));
    if (definition->name() != Model::EntityPropertyValues::WorldspawnClassname)
    {
      result.push_back(makeAction(
        IO::Path("Entities/" + definition->name() + "/Create"),
        QObject::tr("Create %1").arg(QString::fromStdString(definition->name())),
        ActionContext::Any,
        [definition](ActionExecutionContext& context) {
          context.view()->createEntity(definition);
        },
        [](ActionExecutionContext& context) { return context.hasDocument(); }));
    }
  }

  return result;
}

void ActionManager::visitMainMenu(MenuVisitor& visitor) const
{
  for (const auto& menu : m_mainMenu)
  {
    menu->accept(visitor);
  }
}

void ActionManager::visitToolBarActions(MenuVisitor& visitor) const
{
  if (m_toolBar != nullptr)
  {
    m_toolBar->accept(visitor);
  }
}

void ActionManager::visitMapViewActions(const ActionVisitor& visitor) const
{
  class Visitor : public MenuVisitor
  {
  public:
    std::set<const Action*> menuActions;

    void visit(const Menu& menu) override { menu.visitEntries(*this); }

    void visit(const MenuSeparatorItem&) override {}

    void visit(const MenuActionItem& item) override
    {
      const Action* tAction = &item.action();
      menuActions.insert(tAction);
    }
  };

  // Gather the set of all Actions that are used in menus/toolbars
  Visitor v;
  visitMainMenu(v);
  visitToolBarActions(v);

  for (const auto& [path, actionPtr] : m_actions)
  {
    unused(path);
    const Action* tAction = actionPtr.get();
    if (v.menuActions.find(tAction) == v.menuActions.end())
    {
      // This action is not used in a menu, so visit it
      visitor(*actionPtr);
    }
  }
}

const std::map<IO::Path, std::unique_ptr<Action>>& ActionManager::actionsMap() const
{
  return m_actions;
}

class ActionManager::ResetMenuVisitor : public MenuVisitor
{
  void visit(const Menu& menu) override { menu.visitEntries(*this); }
  void visit(const MenuSeparatorItem&) override {}
  void visit(const MenuActionItem& item) override { item.action().resetKeySequence(); }
};

void ActionManager::resetAllKeySequences() const
{
  ResetMenuVisitor menuVisitor;
  visitMainMenu(menuVisitor);
  visitToolBarActions(menuVisitor);

  auto actionVisitor = [](const Action& action) { action.resetKeySequence(); };
  visitMapViewActions(actionVisitor);
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
  createAction(
    IO::Path("Controls/Map view/Create brush"),
    QObject::tr("Create Brush"),
    ActionContext::View3D | ActionContext::AnyOrNoSelection
      | ActionContext::CreateComplexBrushTool,
    QKeySequence(Qt::Key_Return),
    [](ActionExecutionContext& context) { context.view()->createComplexBrush(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->createComplexBrushToolActive();
    });
  createAction(
    IO::Path("Controls/Map view/Create primitive brush"),
    QObject::tr("Create Primitive Brush"),
    ActionContext::View3D | ActionContext::AnyOrNoSelection
    | ActionContext::CreateComplexBrushTool,
    QKeySequence(Qt::Key_Return),
    [](ActionExecutionContext &context) { context.view()->createPrimitiveBrush(); },
    [](ActionExecutionContext &context) {
      return context.hasDocument() && context.frame()->createPrimitiveBrushToolActive();
    });
  createAction(
    IO::Path("Controls/Map view/Toggle clip side"),
    QObject::tr("Toggle Clip Side"),
    ActionContext::AnyView | ActionContext::AnyOrNoSelection | ActionContext::ClipTool,
    QKeySequence(Qt::CTRL + Qt::Key_Return),
    [](ActionExecutionContext& context) { context.view()->toggleClipSide(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->clipToolActive();
    });
  createAction(
    IO::Path("Controls/Map view/Perform clip"),
    QObject::tr("Perform Clip"),
    ActionContext::AnyView | ActionContext::AnyOrNoSelection | ActionContext::ClipTool,
    QKeySequence(Qt::Key_Return),
    [](ActionExecutionContext& context) { context.view()->performClip(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->clipToolActive();
    });

  /* ========== Translation ========== */
  // applies to objects, vertices, handles (e.g. rotation center)
  // these preference paths are structured like "action in 2D view; action in 3D view"
  createAction(
    IO::Path("Controls/Map view/Move objects up; Move objects forward"),
    QObject::tr("Move Forward"),
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyVertexTool
      | ActionContext::RotateTool | ActionContext::NoTool,
    QKeySequence(Qt::Key_Up),
    [](ActionExecutionContext& context) { context.view()->move(vm::direction::forward); },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/Move objects down; Move objects backward"),
    QObject::tr("Move Backward"),
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyVertexTool
      | ActionContext::RotateTool | ActionContext::NoTool,
    QKeySequence(Qt::Key_Down),
    [](ActionExecutionContext& context) {
      context.view()->move(vm::direction::backward);
    },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/Move objects left"),
    QObject::tr("Move Left"),
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyVertexTool
      | ActionContext::RotateTool | ActionContext::NoTool,
    QKeySequence(Qt::Key_Left),
    [](ActionExecutionContext& context) { context.view()->move(vm::direction::left); },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/Move objects right"),
    QObject::tr("Move Right"),
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyVertexTool
      | ActionContext::RotateTool | ActionContext::NoTool,
    QKeySequence(Qt::Key_Right),
    [](ActionExecutionContext& context) { context.view()->move(vm::direction::right); },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/Move objects backward; Move objects up"),
    QObject::tr("Move Up"),
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyVertexTool
      | ActionContext::RotateTool | ActionContext::NoTool,
    QKeySequence(Qt::Key_PageUp),
    [](ActionExecutionContext& context) { context.view()->move(vm::direction::up); },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/Move objects forward; Move objects down"),
    QObject::tr("Move Down"),
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyVertexTool
      | ActionContext::RotateTool | ActionContext::NoTool,
    QKeySequence(Qt::Key_PageDown),
    [](ActionExecutionContext& context) { context.view()->move(vm::direction::down); },
    [](ActionExecutionContext& context) { return context.hasDocument(); });

  /* ========== Duplication ========== */
  // these preference paths are structured like "action in 2D view; action in 3D view"
  createAction(
    IO::Path("Controls/Map view/Duplicate and move objects up; Duplicate and move "
             "objects forward"),
    QObject::tr("Duplicate and Move Forward"),
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyOrNoTool,
    QKeySequence(Qt::CTRL + Qt::Key_Up),
    [](ActionExecutionContext& context) {
      context.view()->duplicateAndMoveObjects(vm::direction::forward);
    },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/Duplicate and move objects down; Duplicate and move "
             "objects backward"),
    QObject::tr("Duplicate and Move Backward"),
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyOrNoTool,
    QKeySequence(Qt::CTRL + Qt::Key_Down),
    [](ActionExecutionContext& context) {
      context.view()->duplicateAndMoveObjects(vm::direction::backward);
    },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/Duplicate and move objects left"),
    QObject::tr("Duplicate and Move Left"),
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyOrNoTool,
    QKeySequence(Qt::CTRL + Qt::Key_Left),
    [](ActionExecutionContext& context) {
      context.view()->duplicateAndMoveObjects(vm::direction::left);
    },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/Duplicate and move objects right"),
    QObject::tr("Duplicate and Move Right"),
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyOrNoTool,
    QKeySequence(Qt::CTRL + Qt::Key_Right),
    [](ActionExecutionContext& context) {
      context.view()->duplicateAndMoveObjects(vm::direction::right);
    },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/Duplicate and move objects backward; Duplicate and move "
             "objects up"),
    QObject::tr("Duplicate and Move Up"),
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyOrNoTool,
    QKeySequence(Qt::CTRL + Qt::Key_PageUp),
    [](ActionExecutionContext& context) {
      context.view()->duplicateAndMoveObjects(vm::direction::up);
    },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/Duplicate and move objects forward; Duplicate and move "
             "objects down"),
    QObject::tr("Duplicate and Move Down"),
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyOrNoTool,
    QKeySequence(Qt::CTRL + Qt::Key_PageDown),
    [](ActionExecutionContext& context) {
      context.view()->duplicateAndMoveObjects(vm::direction::down);
    },
    [](ActionExecutionContext& context) { return context.hasDocument(); });

  /* ========== Rotation ========== */
  // applies to objects, vertices, handles (e.g. rotation center)
  createAction(
    IO::Path("Controls/Map view/Roll objects clockwise"),
    QObject::tr("Roll Clockwise"),
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::RotateTool
      | ActionContext::NoTool,
    QKeySequence(Qt::ALT + Qt::Key_Up),
    [](ActionExecutionContext& context) {
      context.view()->rotateObjects(vm::rotation_axis::roll, true);
    },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/Roll objects counter-clockwise"),
    QObject::tr("Roll Counter-clockwise"),
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::RotateTool
      | ActionContext::NoTool,
    QKeySequence(Qt::ALT + Qt::Key_Down),
    [](ActionExecutionContext& context) {
      context.view()->rotateObjects(vm::rotation_axis::roll, false);
    },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/Yaw objects clockwise"),
    QObject::tr("Yaw Clockwise"),
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::RotateTool
      | ActionContext::NoTool,
    QKeySequence(Qt::ALT + Qt::Key_Left),
    [](ActionExecutionContext& context) {
      context.view()->rotateObjects(vm::rotation_axis::yaw, true);
    },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/Yaw objects counter-clockwise"),
    QObject::tr("Yaw Counter-clockwise"),
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::RotateTool
      | ActionContext::NoTool,
    QKeySequence(Qt::ALT + Qt::Key_Right),
    [](ActionExecutionContext& context) {
      context.view()->rotateObjects(vm::rotation_axis::yaw, false);
    },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/Pitch objects clockwise"),
    QObject::tr("Pitch Clockwise"),
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::RotateTool
      | ActionContext::NoTool,
    QKeySequence(Qt::ALT + Qt::Key_PageUp),
    [](ActionExecutionContext& context) {
      context.view()->rotateObjects(vm::rotation_axis::pitch, true);
    },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/Pitch objects counter-clockwise"),
    QObject::tr("Pitch Counter-clockwise"),
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::RotateTool
      | ActionContext::NoTool,
    QKeySequence(Qt::ALT + Qt::Key_PageDown),
    [](ActionExecutionContext& context) {
      context.view()->rotateObjects(vm::rotation_axis::pitch, false);
    },
    [](ActionExecutionContext& context) { return context.hasDocument(); });

  /* ========== Texturing ========== */
  createAction(
    IO::Path("Controls/Map view/Move textures up"),
    QObject::tr("Move Textures Up"),
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    QKeySequence(Qt::Key_Up),
    [](ActionExecutionContext& context) {
      context.view()->moveTextures(
        vm::direction::up, MapViewBase::TextureActionMode::Normal);
    },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/Move textures up (coarse)"),
    QObject::tr("Move Textures Up (Coarse)"),
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    QKeySequence(Qt::SHIFT + Qt::Key_Up),
    [](ActionExecutionContext& context) {
      context.view()->moveTextures(
        vm::direction::up, MapViewBase::TextureActionMode::Coarse);
    },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/Move textures up (fine)"),
    QObject::tr("Move Textures Up (Fine)"),
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    QKeySequence(Qt::CTRL + Qt::Key_Up),
    [](ActionExecutionContext& context) {
      context.view()->moveTextures(
        vm::direction::up, MapViewBase::TextureActionMode::Fine);
    },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/Move textures down"),
    QObject::tr("Move Textures Down"),
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    QKeySequence(Qt::Key_Down),
    [](ActionExecutionContext& context) {
      context.view()->moveTextures(
        vm::direction::down, MapViewBase::TextureActionMode::Normal);
    },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/Move textures down (coarse)"),
    QObject::tr("Move Textures Down (Coarse)"),
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    QKeySequence(Qt::SHIFT + Qt::Key_Down),
    [](ActionExecutionContext& context) {
      context.view()->moveTextures(
        vm::direction::down, MapViewBase::TextureActionMode::Coarse);
    },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/Move textures down (fine)"),
    QObject::tr("Move Textures Down (Fine)"),
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    QKeySequence(Qt::CTRL + Qt::Key_Down),
    [](ActionExecutionContext& context) {
      context.view()->moveTextures(
        vm::direction::down, MapViewBase::TextureActionMode::Fine);
    },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/Move textures left"),
    QObject::tr("Move Textures Left"),
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    QKeySequence(Qt::Key_Left),
    [](ActionExecutionContext& context) {
      context.view()->moveTextures(
        vm::direction::left, MapViewBase::TextureActionMode::Normal);
    },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/Move textures left (coarse)"),
    QObject::tr("Move Textures Left (Coarse)"),
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    QKeySequence(Qt::SHIFT + Qt::Key_Left),
    [](ActionExecutionContext& context) {
      context.view()->moveTextures(
        vm::direction::left, MapViewBase::TextureActionMode::Coarse);
    },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/Move textures left (fine)"),
    QObject::tr("Move Textures Left (Fine)"),
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    QKeySequence(Qt::CTRL + Qt::Key_Left),
    [](ActionExecutionContext& context) {
      context.view()->moveTextures(
        vm::direction::left, MapViewBase::TextureActionMode::Fine);
    },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/Move textures right"),
    QObject::tr("Move Textures Right"),
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    QKeySequence(Qt::Key_Right),
    [](ActionExecutionContext& context) {
      context.view()->moveTextures(
        vm::direction::right, MapViewBase::TextureActionMode::Normal);
    },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/Move textures right (coarse)"),
    QObject::tr("Move Textures Right (Coarse)"),
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    QKeySequence(Qt::SHIFT + Qt::Key_Right),
    [](ActionExecutionContext& context) {
      context.view()->moveTextures(
        vm::direction::right, MapViewBase::TextureActionMode::Coarse);
    },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/Move textures right (fine)"),
    QObject::tr("Move Textures Right (Fine)"),
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    QKeySequence(Qt::CTRL + Qt::Key_Right),
    [](ActionExecutionContext& context) {
      context.view()->moveTextures(
        vm::direction::right, MapViewBase::TextureActionMode::Fine);
    },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/Rotate textures clockwise"),
    QObject::tr("Rotate Textures Clockwise"),
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    QKeySequence(Qt::Key_PageUp),
    [](ActionExecutionContext& context) {
      context.view()->rotateTextures(true, MapViewBase::TextureActionMode::Normal);
    },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/Rotate textures clockwise (coarse)"),
    QObject::tr("Rotate Textures Clockwise (Coarse)"),
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    QKeySequence(Qt::SHIFT + Qt::Key_PageUp),
    [](ActionExecutionContext& context) {
      context.view()->rotateTextures(true, MapViewBase::TextureActionMode::Coarse);
    },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/Rotate textures clockwise (fine)"),
    QObject::tr("Rotate Textures Clockwise (Fine)"),
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    QKeySequence(Qt::CTRL + Qt::Key_PageUp),
    [](ActionExecutionContext& context) {
      context.view()->rotateTextures(true, MapViewBase::TextureActionMode::Fine);
    },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/Rotate textures counter-clockwise"),
    QObject::tr("Rotate Textures Counter-clockwise"),
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    QKeySequence(Qt::Key_PageDown),
    [](ActionExecutionContext& context) {
      context.view()->rotateTextures(false, MapViewBase::TextureActionMode::Normal);
    },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/Rotate textures counter-clockwise (coarse)"),
    QObject::tr("Rotate Textures Counter-clockwise (Coarse)"),
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    QKeySequence(Qt::SHIFT + Qt::Key_PageDown),
    [](ActionExecutionContext& context) {
      context.view()->rotateTextures(false, MapViewBase::TextureActionMode::Coarse);
    },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/Rotate textures counter-clockwise (fine)"),
    QObject::tr("Rotate Textures Counter-clockwise (Fine)"),
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    QKeySequence(Qt::CTRL + Qt::Key_PageDown),
    [](ActionExecutionContext& context) {
      context.view()->rotateTextures(false, MapViewBase::TextureActionMode::Fine);
    },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/Reveal in texture browser"),
    QObject::tr("Reveal in texture browser"),
    ActionContext::View3D | ActionContext::AnySelection | ActionContext::AnyOrNoTool,
    QKeySequence(),
    [](ActionExecutionContext& context) { context.frame()->revealTexture(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/Flip textures horizontally"),
    QObject::tr("Flip textures horizontally"),
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    QKeySequence(Qt::CTRL + Qt::Key_F),
    [](ActionExecutionContext& context) {
      context.view()->flipTextures(vm::direction::right);
    },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/Flip textures vertically"),
    QObject::tr("Flip textures vertically"),
    ActionContext::View3D | ActionContext::FaceSelection | ActionContext::AnyOrNoTool,
    QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_F),
    [](ActionExecutionContext& context) {
      context.view()->flipTextures(vm::direction::up);
    },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/Reset texture alignment"),
    QObject::tr("Reset texture alignment"),
    ActionContext::AnyView | ActionContext::AnySelection | ActionContext::AnyOrNoTool,
    QKeySequence(Qt::SHIFT + Qt::Key_R),
    [](ActionExecutionContext& context) { context.view()->resetTextures(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/Reset texture alignment to world aligned"),
    QObject::tr("Reset texture alignment to world aligned"),
    ActionContext::AnyView | ActionContext::AnySelection | ActionContext::AnyOrNoTool,
    QKeySequence(Qt::SHIFT + Qt::ALT + Qt::Key_R),
    [](ActionExecutionContext& context) { context.view()->resetTexturesToWorld(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); });

  /* ========== Tag Actions ========== */
  createAction(
    IO::Path("Controls/Map view/Make structural"),
    QObject::tr("Make Structural"),
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyOrNoTool,
    QKeySequence(Qt::ALT + Qt::Key_S),
    [](ActionExecutionContext& context) { context.view()->makeStructural(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); });

  /* ========== View / Filter Actions ========== */
  createAction(
    IO::Path("Controls/Map view/View Filter > Toggle show entity classnames"),
    QObject::tr("Toggle Show Entity Classnames"),
    ActionContext::Any,
    QKeySequence(),
    [](ActionExecutionContext& context) { context.view()->toggleShowEntityClassnames(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/View Filter > Toggle show group bounds"),
    QObject::tr("Toggle Show Group Bounds"),
    ActionContext::Any,
    QKeySequence(),
    [](ActionExecutionContext& context) { context.view()->toggleShowGroupBounds(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/View Filter > Toggle show brush entity bounds"),
    QObject::tr("Toggle Show Brush Entity Bounds"),
    ActionContext::Any,
    QKeySequence(),
    [](ActionExecutionContext& context) {
      context.view()->toggleShowBrushEntityBounds();
    },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/View Filter > Toggle show point entity bounds"),
    QObject::tr("Toggle Show Point Entity Bounds"),
    ActionContext::Any,
    QKeySequence(),
    [](ActionExecutionContext& context) {
      context.view()->toggleShowPointEntityBounds();
    },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/View Filter > Toggle show point entities"),
    QObject::tr("Toggle Show Point Entities"),
    ActionContext::Any,
    QKeySequence(),
    [](ActionExecutionContext& context) { context.view()->toggleShowPointEntities(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/View Filter > Toggle show point entity models"),
    QObject::tr("Toggle Show Point Entity Models"),
    ActionContext::Any,
    QKeySequence(),
    [](ActionExecutionContext& context) {
      context.view()->toggleShowPointEntityModels();
    },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/View Filter > Toggle show brushes"),
    QObject::tr("Toggle Show Brushes"),
    ActionContext::Any,
    QKeySequence(),
    [](ActionExecutionContext& context) { context.view()->toggleShowBrushes(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/View Filter > Show textures"),
    QObject::tr("Show Textures"),
    ActionContext::Any,
    QKeySequence(),
    [](ActionExecutionContext& context) { context.view()->showTextures(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/View Filter > Hide textures"),
    QObject::tr("Hide Textures"),
    ActionContext::Any,
    QKeySequence(),
    [](ActionExecutionContext& context) { context.view()->hideTextures(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/View Filter > Hide faces"),
    QObject::tr("Hide Faces"),
    ActionContext::Any,
    QKeySequence(),
    [](ActionExecutionContext& context) { context.view()->hideFaces(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/View Filter > Shade faces"),
    QObject::tr("Toggle Shade Faces"),
    ActionContext::Any,
    QKeySequence(),
    [](ActionExecutionContext& context) { context.view()->toggleShadeFaces(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/View Filter > Use fog"),
    QObject::tr("Toggle Show Fog"),
    ActionContext::Any,
    QKeySequence(),
    [](ActionExecutionContext& context) { context.view()->toggleShowFog(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/View Filter > Show edges"),
    QObject::tr("Toggle Show Edges"),
    ActionContext::Any,
    QKeySequence(),
    [](ActionExecutionContext& context) { context.view()->toggleShowEdges(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/View Filter > Show all entity links"),
    QObject::tr("Show All Entity Links"),
    ActionContext::Any,
    QKeySequence(),
    [](ActionExecutionContext& context) { context.view()->showAllEntityLinks(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/View Filter > Show transitively selected entity links"),
    QObject::tr("Show Transitively Selected Entity Links"),
    ActionContext::Any,
    QKeySequence(),
    [](ActionExecutionContext& context) {
      context.view()->showTransitivelySelectedEntityLinks();
    },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/View Filter > Show directly selected entity links"),
    QObject::tr("Show Directly Selected Entity Links"),
    ActionContext::Any,
    QKeySequence(),
    [](ActionExecutionContext& context) {
      context.view()->showDirectlySelectedEntityLinks();
    },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/View Filter > Hide entity links"),
    QObject::tr("Hide All Entity Links"),
    ActionContext::Any,
    QKeySequence(),
    [](ActionExecutionContext& context) { context.view()->hideAllEntityLinks(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); });

  /* ========== Misc Actions ========== */
  createAction(
    IO::Path("Controls/Map view/Cycle map view"),
    QObject::tr("Cycle View"),
    ActionContext::Any,
    QKeySequence(Qt::Key_Space),
    [](ActionExecutionContext& context) { context.view()->cycleMapView(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/Reset camera zoom"),
    QObject::tr("Reset Camera Zoom"),
    ActionContext::View3D | ActionContext::AnyOrNoTool | ActionContext::AnyOrNoSelection,
    QKeySequence(Qt::SHIFT + Qt::Key_Escape),
    [](ActionExecutionContext& context) { context.view()->resetCameraZoom(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
  createAction(
    IO::Path("Controls/Map view/Cancel"),
    QObject::tr("Cancel"),
    ActionContext::Any,
    QKeySequence(Qt::Key_Escape),
    [](ActionExecutionContext& context) { context.view()->cancel(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); });
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
  fileMenu.addItem(createMenuAction(
    IO::Path("Menu/File/New"),
    QObject::tr("New Document"),
    QKeySequence::New,
    [](ActionExecutionContext&) {
      auto& app = TrenchBroomApp::instance();
      app.newDocument();
    },
    [](ActionExecutionContext&) { return true; }));
  fileMenu.addSeparator();
  fileMenu.addItem(createMenuAction(
    IO::Path("Menu/File/Open..."),
    QObject::tr("Open Document..."),
    QKeySequence::Open,
    [](ActionExecutionContext&) {
      auto& app = TrenchBroomApp::instance();
      app.openDocument();
    },
    [](ActionExecutionContext&) { return true; }));
  fileMenu.addMenu("Open Recent", MenuEntryType::Menu_RecentDocuments);
  fileMenu.addSeparator();
  fileMenu.addItem(createMenuAction(
    IO::Path("Menu/File/Save"),
    QObject::tr("Save Document"),
    QKeySequence::Save,
    [](ActionExecutionContext& context) { context.frame()->saveDocument(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); }));
  fileMenu.addItem(createMenuAction(
    IO::Path("Menu/File/Save as..."),
    QObject::tr("Save Document as..."),
    QKeySequence::SaveAs,
    [](ActionExecutionContext& context) { context.frame()->saveDocumentAs(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); }));

  auto& exportMenu = fileMenu.addMenu("Export");
  exportMenu.addItem(createMenuAction(
    IO::Path("Menu/File/Export/Wavefront OBJ..."),
    QObject::tr("Wavefront OBJ..."),
    0,
    [](ActionExecutionContext& context) { context.frame()->exportDocumentAsObj(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); }));
  exportMenu.addItem(createMenuAction(
    IO::Path("Menu/File/Export/Map..."),
    QObject::tr("Map..."),
    0,
    [](ActionExecutionContext& context) { context.frame()->exportDocumentAsMap(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); },
    IO::Path(),
    QObject::tr("Exports the current map to a .map file. Layers marked Omit From Export "
                "will be omitted.")));

  /* ========== File Menu (Associated Resources) ========== */
  fileMenu.addSeparator();
  fileMenu.addItem(createMenuAction(
    IO::Path("Menu/File/Load Point File..."),
    QObject::tr("Load Point File..."),
    0,
    [](ActionExecutionContext& context) { context.frame()->loadPointFile(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); }));
  fileMenu.addItem(createMenuAction(
    IO::Path("Menu/File/Reload Point File"),
    QObject::tr("Reload Point File"),
    0,
    [](ActionExecutionContext& context) { context.frame()->reloadPointFile(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->canReloadPointFile();
    }));
  fileMenu.addItem(createMenuAction(
    IO::Path("Menu/File/Unload Point File"),
    QObject::tr("Unload Point File"),
    0,
    [](ActionExecutionContext& context) { context.frame()->unloadPointFile(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->canUnloadPointFile();
    }));
  fileMenu.addSeparator();
  fileMenu.addItem(createMenuAction(
    IO::Path("Menu/File/Load Portal File..."),
    QObject::tr("Load Portal File..."),
    0,
    [](ActionExecutionContext& context) { context.frame()->loadPortalFile(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); }));
  fileMenu.addItem(createMenuAction(
    IO::Path("Menu/File/Reload Portal File"),
    QObject::tr("Reload Portal File"),
    0,
    [](ActionExecutionContext& context) { context.frame()->reloadPortalFile(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->canReloadPortalFile();
    }));
  fileMenu.addItem(createMenuAction(
    IO::Path("Menu/File/Unload Portal File"),
    QObject::tr("Unload Portal File"),
    0,
    [](ActionExecutionContext& context) { context.frame()->unloadPortalFile(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->canUnloadPortalFile();
    }));
  fileMenu.addSeparator();
  fileMenu.addItem(createMenuAction(
    IO::Path("Menu/File/Reload Texture Collections"),
    QObject::tr("Reload Texture Collections"),
    Qt::Key_F5,
    [](ActionExecutionContext& context) { context.frame()->reloadTextureCollections(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); }));
  fileMenu.addItem(createMenuAction(
    IO::Path("Menu/File/Reload Entity Definitions"),
    QObject::tr("Reload Entity Definitions"),
    Qt::Key_F6,
    [](ActionExecutionContext& context) { context.frame()->reloadEntityDefinitions(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); }));
  fileMenu.addSeparator();
  fileMenu.addItem(createMenuAction(
    IO::Path("Menu/File/Revert"),
    QObject::tr("Revert Document"),
    0,
    [](ActionExecutionContext& context) { context.frame()->revertDocument(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); },
    IO::Path(),
    QObject::tr("Discards any unsaved changes and reloads the map file.")));
  fileMenu.addItem(createMenuAction(
    IO::Path("Menu/File/Close"),
    QObject::tr("Close Document"),
    QKeySequence::Close,
    [](ActionExecutionContext& context) { context.frame()->closeDocument(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); }));
}

void ActionManager::createEditMenu()
{ /* ========== Edit Menu ========== */
  auto& editMenu = createMainMenu("Edit");
  editMenu.addItem(
    createMenuAction(
      IO::Path("Menu/Edit/Undo"),
      QObject::tr("Undo"),
      QKeySequence::Undo,
      [](ActionExecutionContext& context) { context.frame()->undo(); },
      [](ActionExecutionContext& context) {
        return context.hasDocument() && context.frame()->canUndo();
      }),
    MenuEntryType::Menu_Undo);
  editMenu.addItem(
    createMenuAction(
      IO::Path("Menu/Edit/Redo"),
      QObject::tr("Redo"),
      QKeySequence::Redo,
      [](ActionExecutionContext& context) { context.frame()->redo(); },
      [](ActionExecutionContext& context) {
        return context.hasDocument() && context.frame()->canRedo();
      }),
    MenuEntryType::Menu_Redo);
  editMenu.addSeparator();
  editMenu.addItem(createMenuAction(
    IO::Path("Menu/Edit/Repeat"),
    QObject::tr("Repeat Last Commands"),
    Qt::CTRL + Qt::Key_R,
    [](ActionExecutionContext& context) { context.frame()->repeatLastCommands(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); }));
  editMenu.addItem(createMenuAction(
    IO::Path("Menu/Edit/Clear Repeatable Commands"),
    QObject::tr("Clear Repeatable Commands"),
    Qt::CTRL + Qt::SHIFT + Qt::Key_R,
    [](ActionExecutionContext& context) { context.frame()->clearRepeatableCommands(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->hasRepeatableCommands();
    }));
  editMenu.addSeparator();
  editMenu.addItem(
    createMenuAction(
      IO::Path("Menu/Edit/Cut"),
      QObject::tr("Cut"),
      QKeySequence::Cut,
      [](ActionExecutionContext& context) { context.frame()->cutSelection(); },
      [](ActionExecutionContext& context) {
        return context.hasDocument() && context.frame()->canCopySelection();
      }),
    MenuEntryType::Menu_Cut);
  editMenu.addItem(
    createMenuAction(
      IO::Path("Menu/Edit/Copy"),
      QObject::tr("Copy"),
      QKeySequence::Copy,
      [](ActionExecutionContext& context) { context.frame()->copySelection(); },
      [](ActionExecutionContext& context) {
        return context.hasDocument() && context.frame()->canCopySelection();
      }),
    MenuEntryType::Menu_Copy);
  editMenu.addItem(
    createMenuAction(
      IO::Path("Menu/Edit/Paste"),
      QObject::tr("Paste"),
      QKeySequence::Paste,
      [](ActionExecutionContext& context) { context.frame()->pasteAtCursorPosition(); },
      [](ActionExecutionContext& context) {
        return context.hasDocument() && context.frame()->canPaste();
      }),
    MenuEntryType::Menu_Paste);
  editMenu.addItem(
    createMenuAction(
      IO::Path("Menu/Edit/Paste at Original Position"),
      QObject::tr("Paste at Original Position"),
      Qt::CTRL + Qt::ALT + Qt::Key_V,
      [](ActionExecutionContext& context) { context.frame()->pasteAtOriginalPosition(); },
      [](ActionExecutionContext& context) {
        return context.hasDocument() && context.frame()->canPaste();
      }),
    MenuEntryType::Menu_PasteAtOriginalPosition);
  editMenu.addItem(createMenuAction(
    IO::Path("Menu/Edit/Duplicate"),
    QObject::tr("Duplicate"),
    Qt::CTRL + Qt::Key_D,
    [](ActionExecutionContext& context) { context.frame()->duplicateSelection(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->canDuplicateSelectino();
    },
    IO::Path("DuplicateObjects.svg")));
  editMenu.addItem(createAction(
    IO::Path("Menu/Edit/Delete"),
    QObject::tr("Delete"),
    ActionContext::Any,
    QKeySequence(
#ifdef __APPLE__
      Qt::Key_Backspace
#else
      QKeySequence::Delete
#endif
      ),
    [](ActionExecutionContext& context) { context.frame()->deleteSelection(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->canDeleteSelection();
    }));
  editMenu.addSeparator();
  editMenu.addItem(createMenuAction(
    IO::Path("Menu/Edit/Select All"),
    QObject::tr("Select All"),
    QKeySequence::SelectAll,
    [](ActionExecutionContext& context) { context.frame()->selectAll(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->canSelect();
    }));
  editMenu.addItem(createMenuAction(
    IO::Path("Menu/Edit/Select Siblings"),
    QObject::tr("Select Siblings"),
    Qt::CTRL + Qt::Key_B,
    [](ActionExecutionContext& context) { context.frame()->selectSiblings(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->canSelectSiblings();
    }));
  editMenu.addItem(createMenuAction(
    IO::Path("Menu/Edit/Select Touching"),
    QObject::tr("Select Touching"),
    Qt::CTRL + Qt::Key_T,
    [](ActionExecutionContext& context) { context.frame()->selectTouching(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->canSelectByBrush();
    }));
  editMenu.addItem(createMenuAction(
    IO::Path("Menu/Edit/Select Inside"),
    QObject::tr("Select Inside"),
    Qt::CTRL + Qt::Key_E,
    [](ActionExecutionContext& context) { context.frame()->selectInside(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->canSelectByBrush();
    }));
  editMenu.addItem(createAction(
    IO::Path("Menu/Edit/Select Tall"),
    QObject::tr("Select Tall"),
    ActionContext::Any,
    QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_E),
    [](ActionExecutionContext& context) { context.frame()->selectTall(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->canSelectTall();
    }));
  editMenu.addItem(createMenuAction(
    IO::Path("Menu/Edit/Select by Line Number"),
    QObject::tr("Select by Line Number..."),
    0,
    [](ActionExecutionContext& context) { context.frame()->selectByLineNumber(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->canSelect();
    }));
  editMenu.addItem(createAction(
    IO::Path("Menu/Edit/Select Inverse"),
    QObject::tr("Select Inverse"),
    ActionContext::Any,
    QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_A),
    [](ActionExecutionContext& context) { context.frame()->selectInverse(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->canSelectInverse();
    }));
  editMenu.addItem(createAction(
    IO::Path("Menu/Edit/Select None"),
    QObject::tr("Select None"),
    ActionContext::Any,
    QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_A),
    [](ActionExecutionContext& context) { context.frame()->selectNone(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->canDeselect();
    }));
  editMenu.addSeparator();
  editMenu.addItem(createAction(
    IO::Path("Menu/Edit/Group"),
    QObject::tr("Group Selected Objects"),
    ActionContext::Any,
    QKeySequence(Qt::CTRL + Qt::Key_G),
    [](ActionExecutionContext& context) { context.frame()->groupSelectedObjects(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->canGroupSelectedObjects();
    }));
  editMenu.addItem(createMenuAction(
    IO::Path("Menu/Edit/Ungroup"),
    QObject::tr("Ungroup Selected Objects"),
    Qt::CTRL + Qt::SHIFT + Qt::Key_G,
    [](ActionExecutionContext& context) { context.frame()->ungroupSelectedObjects(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->canUngroupSelectedObjects();
    }));
  editMenu.addSeparator();

  editMenu.addItem(createAction(
    IO::Path("Menu/Edit/Create Linked Duplicate"),
    QObject::tr("Create Linked Duplicate"),
    ActionContext::Any,
    QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_D),
    [](ActionExecutionContext& context) { context.document()->createLinkedDuplicate(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.document()->canCreateLinkedDuplicate();
    }));
  editMenu.addItem(createAction(
    IO::Path("Menu/Edit/Select Linked Groups"),
    QObject::tr("Select Linked Groups"),
    ActionContext::Any,
    0,
    [](ActionExecutionContext& context) { context.document()->selectLinkedGroups(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.document()->canSelectLinkedGroups();
    }));
  editMenu.addItem(createAction(
    IO::Path("Menu/Edit/Separate Linked Groups"),
    QObject::tr("Separate Selected Groups"),
    ActionContext::Any,
    0,
    [](ActionExecutionContext& context) { context.document()->separateLinkedGroups(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.document()->canSeparateLinkedGroups();
    }));
  editMenu.addItem(createAction(
    IO::Path("Menu/Edit/Clear Protected Properties"),
    QObject::tr("Clear Protected Properties"),
    ActionContext::Any,
    0,
    [](ActionExecutionContext& context) {
      context.document()->clearProtectedProperties();
    },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.document()->canClearProtectedProperties();
    }));
  editMenu.addSeparator();

  editMenu.addItem(createAction(
    IO::Path("Controls/Map view/Flip objects horizontally"),
    QObject::tr("Flip Horizontally"),
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyOrNoTool,
    QKeySequence(Qt::CTRL + Qt::Key_F),
    [](ActionExecutionContext& context) {
      context.view()->flipObjects(vm::direction::left);
    },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.view() && context.view()->canFlipObjects();
    },
    IO::Path("FlipHorizontally.svg")));
  editMenu.addItem(createAction(
    IO::Path("Controls/Map view/Flip objects vertically"),
    QObject::tr("Flip Vertically"),
    ActionContext::AnyView | ActionContext::NodeSelection | ActionContext::AnyOrNoTool,
    QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_F),
    [](ActionExecutionContext& context) {
      context.view()->flipObjects(vm::direction::up);
    },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.view() && context.view()->canFlipObjects();
    },
    IO::Path("FlipVertically.svg")));
  editMenu.addSeparator();

  auto& toolMenu = editMenu.addMenu("Tools");
  toolMenu.addItem(createMenuAction(
    IO::Path("Menu/Edit/Tools/Brush Tool"),
    QObject::tr("Brush Tool"),
    Qt::Key_B,
    [](ActionExecutionContext& context) {
      context.frame()->toggleCreateComplexBrushTool();
    },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->canToggleCreateComplexBrushTool();
    },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->createComplexBrushToolActive();
    },
    IO::Path("BrushTool.svg")));
  toolMenu.addItem(createMenuAction(
    IO::Path("Menu/Edit/Tools/Primitive Brush Tool"),
    QObject::tr("Primitive Brush Tool"),
    Qt::Key_P,
    [](ActionExecutionContext &context) {
      context.frame()->toggleCreatePrimitiveBrushTool();
    },
    [](ActionExecutionContext &context) {
      return context.hasDocument() && context.frame()->canToggleCreatePrimitiveBrushTool();
    },
    [](ActionExecutionContext &context) {
      return context.hasDocument() && context.frame()->createPrimitiveBrushToolActive();
    },
    IO::Path("PrimitiveBrushTool.svg")));
  toolMenu.addItem(createMenuAction(
    IO::Path("Menu/Edit/Tools/Clip Tool"),
    QObject::tr("Clip Tool"),
    Qt::Key_C,
    [](ActionExecutionContext& context) { context.frame()->toggleClipTool(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->canToggleClipTool();
    },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->clipToolActive();
    },
    IO::Path("ClipTool.svg")));
  toolMenu.addItem(createMenuAction(
    IO::Path("Menu/Edit/Tools/Rotate Tool"),
    QObject::tr("Rotate Tool"),
    Qt::Key_R,
    [](ActionExecutionContext& context) { context.frame()->toggleRotateObjectsTool(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->canToggleRotateObjectsTool();
    },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->rotateObjectsToolActive();
    },
    IO::Path("RotateTool.svg")));
  toolMenu.addItem(createMenuAction(
    IO::Path("Menu/Edit/Tools/Scale Tool"),
    QObject::tr("Scale Tool"),
    Qt::Key_T,
    [](ActionExecutionContext& context) { context.frame()->toggleScaleObjectsTool(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->canToggleScaleObjectsTool();
    },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->scaleObjectsToolActive();
    },
    IO::Path("ScaleTool.svg")));
  toolMenu.addItem(createMenuAction(
    IO::Path("Menu/Edit/Tools/Shear Tool"),
    QObject::tr("Shear Tool"),
    Qt::Key_G,
    [](ActionExecutionContext& context) { context.frame()->toggleShearObjectsTool(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->canToggleShearObjectsTool();
    },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->shearObjectsToolActive();
    },
    IO::Path("ShearTool.svg")));
  toolMenu.addItem(createMenuAction(
    IO::Path("Menu/Edit/Tools/Vertex Tool"),
    QObject::tr("Vertex Tool"),
    Qt::Key_V,
    [](ActionExecutionContext& context) { context.frame()->toggleVertexTool(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->canToggleVertexTool();
    },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->vertexToolActive();
    },
    IO::Path("VertexTool.svg")));
  toolMenu.addItem(createMenuAction(
    IO::Path("Menu/Edit/Tools/Edge Tool"),
    QObject::tr("Edge Tool"),
    Qt::Key_E,
    [](ActionExecutionContext& context) { context.frame()->toggleEdgeTool(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->canToggleEdgeTool();
    },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->edgeToolActive();
    },
    IO::Path("EdgeTool.svg")));
  toolMenu.addItem(createMenuAction(
    IO::Path("Menu/Edit/Tools/Face Tool"),
    QObject::tr("Face Tool"),
    Qt::Key_F,
    [](ActionExecutionContext& context) { context.frame()->toggleFaceTool(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->canToggleFaceTool();
    },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->faceToolActive();
    },
    IO::Path("FaceTool.svg")));
  toolMenu.addItem(createMenuAction(
    IO::Path("Menu/Edit/Tools/Make Primitive"),
    QObject::tr("Make Primitive"),
    0,
    [](ActionExecutionContext &context) { context.frame()->showPrimitiveDialog(); },
    [](ActionExecutionContext &context) { return context.hasDocument(); }));
  toolMenu.addItem(createMenuAction(
    IO::Path("Controls/Map view/Deactivate current tool"),
    QObject::tr("Deactivate Current Tool"),
    Qt::CTRL + Qt::Key_Escape,
    [](ActionExecutionContext& context) { context.view()->deactivateTool(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && !context.frame()->anyToolActive();
    },
    IO::Path("NoTool.svg")));

  auto& csgMenu = editMenu.addMenu("CSG");
  csgMenu.addItem(createMenuAction(
    IO::Path("Menu/Edit/CSG/Convex Merge"),
    QObject::tr("Convex Merge"),
    Qt::CTRL + Qt::Key_J,
    [](ActionExecutionContext& context) { context.frame()->csgConvexMerge(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->canDoCsgConvexMerge();
    }));
  csgMenu.addItem(createMenuAction(
    IO::Path("Menu/Edit/CSG/Subtract"),
    QObject::tr("Subtract"),
    Qt::CTRL + Qt::Key_K,
    [](ActionExecutionContext& context) { context.frame()->csgSubtract(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->canDoCsgSubtract();
    }));
  csgMenu.addItem(createMenuAction(
    IO::Path("Menu/Edit/CSG/Hollow"),
    QObject::tr("Hollow"),
    Qt::CTRL + Qt::SHIFT + Qt::Key_K,
    [](ActionExecutionContext& context) { context.frame()->csgHollow(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->canDoCsgHollow();
    }));
  csgMenu.addItem(createMenuAction(
    IO::Path("Menu/Edit/CSG/Intersect"),
    QObject::tr("Intersect"),
    Qt::CTRL + Qt::Key_L,
    [](ActionExecutionContext& context) { context.frame()->csgIntersect(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->canDoCsgIntersect();
    }));

  editMenu.addSeparator();
  editMenu.addItem(createMenuAction(
    IO::Path("Menu/Edit/Snap Vertices to Integer"),
    QObject::tr("Snap Vertices to Integer"),
    Qt::CTRL + Qt::SHIFT + Qt::Key_V,
    [](ActionExecutionContext& context) { context.frame()->snapVerticesToInteger(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->canSnapVertices();
    }));
  editMenu.addItem(createMenuAction(
    IO::Path("Menu/Edit/Snap Vertices to Grid"),
    QObject::tr("Snap Vertices to Grid"),
    Qt::CTRL + Qt::ALT + Qt::SHIFT + Qt::Key_V,
    [](ActionExecutionContext& context) { context.frame()->snapVerticesToGrid(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->canSnapVertices();
    }));
  editMenu.addSeparator();
  editMenu.addItem(createMenuAction(
    IO::Path("Menu/Edit/Texture Lock"),
    QObject::tr("Texture Lock"),
    0,
    [](ActionExecutionContext& context) { context.frame()->toggleTextureLock(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); },
    [](ActionExecutionContext&) { return pref(Preferences::TextureLock); },
    IO::Path("TextureLock.svg")));
  editMenu.addItem(createMenuAction(
    IO::Path("Menu/Edit/UV Lock"),
    QObject::tr("UV Lock"),
    Qt::Key_U,
    [](ActionExecutionContext& context) { context.frame()->toggleUVLock(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); },
    [](ActionExecutionContext&) { return pref(Preferences::UVLock); },
    IO::Path("UVLock.svg")));
  editMenu.addSeparator();
  editMenu.addItem(createMenuAction(
    IO::Path("Menu/Edit/Replace Texture..."),
    QObject::tr("Replace Texture..."),
    0,
    [](ActionExecutionContext& context) { context.frame()->replaceTexture(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); }));
}

void ActionManager::createViewMenu()
{
  auto& viewMenu = createMainMenu("View");
  auto& gridMenu = viewMenu.addMenu("Grid");
  gridMenu.addItem(createMenuAction(
    IO::Path("Menu/View/Grid/Show Grid"),
    QObject::tr("Show Grid"),
    Qt::Key_0,
    [](ActionExecutionContext& context) { context.frame()->toggleShowGrid(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.document()->grid().visible();
    }));
  gridMenu.addItem(createMenuAction(
    IO::Path("Menu/View/Grid/Snap to Grid"),
    QObject::tr("Snap to Grid"),
    Qt::ALT + Qt::Key_0,
    [](ActionExecutionContext& context) { context.frame()->toggleSnapToGrid(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.document()->grid().snap();
    }));
  gridMenu.addItem(createMenuAction(
    IO::Path("Menu/View/Grid/Increase Grid Size"),
    QObject::tr("Increase Grid Size"),
    Qt::Key_Plus,
    [](ActionExecutionContext& context) { context.frame()->incGridSize(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->canIncGridSize();
    }));
  gridMenu.addItem(createMenuAction(
    IO::Path("Menu/View/Grid/Decrease Grid Size"),
    QObject::tr("Decrease Grid Size"),
    Qt::Key_Minus,
    [](ActionExecutionContext& context) { context.frame()->decGridSize(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->canDecGridSize();
    }));
  gridMenu.addSeparator();
  gridMenu.addItem(createMenuAction(
    IO::Path("Menu/View/Grid/Set Grid Size 0.125"),
    QObject::tr("Set Grid Size 0.125"),
    0,
    [](ActionExecutionContext& context) { context.frame()->setGridSize(-3); },
    [](ActionExecutionContext& context) { return context.hasDocument(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.document()->grid().size() == -3;
    }));
  gridMenu.addItem(createMenuAction(
    IO::Path("Menu/View/Grid/Set Grid Size 0.25"),
    QObject::tr("Set Grid Size 0.25"),
    0,
    [](ActionExecutionContext& context) { context.frame()->setGridSize(-2); },
    [](ActionExecutionContext& context) { return context.hasDocument(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.document()->grid().size() == -2;
    }));
  gridMenu.addItem(createMenuAction(
    IO::Path("Menu/View/Grid/Set Grid Size 0.5"),
    QObject::tr("Set Grid Size 0.5"),
    0,
    [](ActionExecutionContext& context) { context.frame()->setGridSize(-1); },
    [](ActionExecutionContext& context) { return context.hasDocument(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.document()->grid().size() == -1;
    }));
  gridMenu.addItem(createMenuAction(
    IO::Path("Menu/View/Grid/Set Grid Size 1"),
    QObject::tr("Set Grid Size 1"),
    Qt::Key_1,
    [](ActionExecutionContext& context) { context.frame()->setGridSize(0); },
    [](ActionExecutionContext& context) { return context.hasDocument(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.document()->grid().size() == 0;
    }));
  gridMenu.addItem(createMenuAction(
    IO::Path("Menu/View/Grid/Set Grid Size 2"),
    QObject::tr("Set Grid Size 2"),
    Qt::Key_2,
    [](ActionExecutionContext& context) { context.frame()->setGridSize(1); },
    [](ActionExecutionContext& context) { return context.hasDocument(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.document()->grid().size() == 1;
    }));
  gridMenu.addItem(createMenuAction(
    IO::Path("Menu/View/Grid/Set Grid Size 4"),
    QObject::tr("Set Grid Size 4"),
    Qt::Key_3,
    [](ActionExecutionContext& context) { context.frame()->setGridSize(2); },
    [](ActionExecutionContext& context) { return context.hasDocument(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.document()->grid().size() == 2;
    }));
  gridMenu.addItem(createMenuAction(
    IO::Path("Menu/View/Grid/Set Grid Size 8"),
    QObject::tr("Set Grid Size 8"),
    Qt::Key_4,
    [](ActionExecutionContext& context) { context.frame()->setGridSize(3); },
    [](ActionExecutionContext& context) { return context.hasDocument(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.document()->grid().size() == 3;
    }));
  gridMenu.addItem(createMenuAction(
    IO::Path("Menu/View/Grid/Set Grid Size 16"),
    QObject::tr("Set Grid Size 16"),
    Qt::Key_5,
    [](ActionExecutionContext& context) { context.frame()->setGridSize(4); },
    [](ActionExecutionContext& context) { return context.hasDocument(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.document()->grid().size() == 4;
    }));
  gridMenu.addItem(createMenuAction(
    IO::Path("Menu/View/Grid/Set Grid Size 32"),
    QObject::tr("Set Grid Size 32"),
    Qt::Key_6,
    [](ActionExecutionContext& context) { context.frame()->setGridSize(5); },
    [](ActionExecutionContext& context) { return context.hasDocument(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.document()->grid().size() == 5;
    }));
  gridMenu.addItem(createMenuAction(
    IO::Path("Menu/View/Grid/Set Grid Size 64"),
    QObject::tr("Set Grid Size 64"),
    Qt::Key_7,
    [](ActionExecutionContext& context) { context.frame()->setGridSize(6); },
    [](ActionExecutionContext& context) { return context.hasDocument(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.document()->grid().size() == 6;
    }));
  gridMenu.addItem(createMenuAction(
    IO::Path("Menu/View/Grid/Set Grid Size 128"),
    QObject::tr("Set Grid Size 128"),
    Qt::Key_8,
    [](ActionExecutionContext& context) { context.frame()->setGridSize(7); },
    [](ActionExecutionContext& context) { return context.hasDocument(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.document()->grid().size() == 7;
    }));
  gridMenu.addItem(createMenuAction(
    IO::Path("Menu/View/Grid/Set Grid Size 256"),
    QObject::tr("Set Grid Size 256"),
    Qt::Key_9,
    [](ActionExecutionContext& context) { context.frame()->setGridSize(8); },
    [](ActionExecutionContext& context) { return context.hasDocument(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.document()->grid().size() == 8;
    }));

  auto& cameraMenu = viewMenu.addMenu("Camera");
  cameraMenu.addItem(createMenuAction(
    IO::Path("Menu/View/Camera/Move to Next Point"),
    QObject::tr("Move Camera to Next Point"),
    Qt::Key_Period,
    [](ActionExecutionContext& context) { context.frame()->moveCameraToNextPoint(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->canMoveCameraToNextPoint();
    }));
  cameraMenu.addItem(createMenuAction(
    IO::Path("Menu/View/Camera/Move to Previous Point"),
    QObject::tr("Move Camera to Previous Point"),
    Qt::Key_Comma,
    [](ActionExecutionContext& context) { context.frame()->moveCameraToPreviousPoint(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->canMoveCameraToPreviousPoint();
    }));
  cameraMenu.addItem(createMenuAction(
    IO::Path("Menu/View/Camera/Focus on Selection"),
    QObject::tr("Focus Camera on Selection"),
    Qt::CTRL + Qt::Key_U,
    [](ActionExecutionContext& context) { context.frame()->focusCameraOnSelection(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->canFocusCamera();
    }));
  cameraMenu.addItem(createMenuAction(
    IO::Path("Menu/View/Camera/Move Camera to..."),
    QObject::tr("Move Camera to..."),
    0,
    [](ActionExecutionContext& context) { context.frame()->moveCameraToPosition(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); }));

  viewMenu.addSeparator();
  viewMenu.addItem(createMenuAction(
    IO::Path("Menu/View/Isolate"),
    QObject::tr("Isolate Selection"),
    Qt::CTRL + Qt::Key_I,
    [](ActionExecutionContext& context) { context.frame()->isolateSelection(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->canIsolateSelection();
    }));
  viewMenu.addItem(createMenuAction(
    IO::Path("Menu/View/Hide"),
    QObject::tr("Hide Selection"),
    Qt::CTRL + Qt::ALT + Qt::Key_I,
    [](ActionExecutionContext& context) { context.frame()->hideSelection(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->canHideSelection();
    }));
  viewMenu.addItem(createMenuAction(
    IO::Path("Menu/View/Show All"),
    QObject::tr("Show All"),
    Qt::CTRL + Qt::SHIFT + Qt::Key_I,
    [](ActionExecutionContext& context) { context.frame()->showAll(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); }));
  viewMenu.addSeparator();
  viewMenu.addItem(createMenuAction(
    IO::Path("Menu/View/Switch to Map Inspector"),
    QObject::tr("Show Map Inspector"),
    Qt::CTRL + Qt::Key_1,
    [](ActionExecutionContext& context) {
      context.frame()->switchToInspectorPage(InspectorPage::Map);
    },
    [](ActionExecutionContext& context) { return context.hasDocument(); }));
  viewMenu.addItem(createMenuAction(
    IO::Path("Menu/View/Switch to Entity Inspector"),
    QObject::tr("Show Entity Inspector"),
    Qt::CTRL + Qt::Key_2,
    [](ActionExecutionContext& context) {
      context.frame()->switchToInspectorPage(InspectorPage::Entity);
    },
    [](ActionExecutionContext& context) { return context.hasDocument(); }));
  viewMenu.addItem(createMenuAction(
    IO::Path("Menu/View/Switch to Face Inspector"),
    QObject::tr("Show Face Inspector"),
    Qt::CTRL + Qt::Key_3,
    [](ActionExecutionContext& context) {
      context.frame()->switchToInspectorPage(InspectorPage::Face);
    },
    [](ActionExecutionContext& context) { return context.hasDocument(); }));
  viewMenu.addSeparator();
  viewMenu.addItem(createMenuAction(
    IO::Path("Menu/View/Toggle Toolbar"),
    QObject::tr("Toggle Toolbar"),
    Qt::CTRL + Qt::ALT + Qt::Key_T,
    [](ActionExecutionContext& context) { context.frame()->toggleToolbar(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->toolbarVisible();
    }));
  viewMenu.addItem(createMenuAction(
    IO::Path("Menu/View/Toggle Info Panel"),
    QObject::tr("Toggle Info Panel"),
    Qt::CTRL + Qt::Key_4,
    [](ActionExecutionContext& context) { context.frame()->toggleInfoPanel(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->infoPanelVisible();
    }));
  viewMenu.addItem(createMenuAction(
    IO::Path("Menu/View/Toggle Inspector"),
    QObject::tr("Toggle Inspector"),
    Qt::CTRL + Qt::Key_5,
    [](ActionExecutionContext& context) { context.frame()->toggleInspector(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->inspectorVisible();
    }));
  viewMenu.addItem(createMenuAction(
    IO::Path("Menu/View/Maximize Current View"),
    QObject::tr("Maximize Current View"),
#ifdef Q_OS_MACOS
    // Command + Space opens Spotlight so we can't use it, so use Ctrl + Space instead.
    Qt::META + Qt::Key_Space,
#else
    Qt::CTRL + Qt::Key_Space,
#endif
    [](ActionExecutionContext& context) { context.frame()->toggleMaximizeCurrentView(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); },
    [](ActionExecutionContext& context) {
      return context.hasDocument() && context.frame()->currentViewMaximized();
    }));
  viewMenu.addSeparator();
  viewMenu.addItem(createMenuAction(
    IO::Path("Menu/File/Preferences..."),
    QObject::tr("Preferences..."),
    QKeySequence::Preferences,
    [](ActionExecutionContext&) {
      auto& app = TrenchBroomApp::instance();
      app.showPreferences();
    },
    [](ActionExecutionContext&) { return true; }));
}

void ActionManager::createRunMenu()
{
  auto& runMenu = createMainMenu("Run");
  runMenu.addItem(createMenuAction(
    IO::Path("Menu/Run/Compile..."),
    QObject::tr("Compile Map..."),
    0,
    [](ActionExecutionContext& context) { context.frame()->showCompileDialog(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); }));
  runMenu.addItem(createMenuAction(
    IO::Path("Menu/Run/Launch..."),
    QObject::tr("Launch Engine..."),
    0,
    [](ActionExecutionContext& context) { context.frame()->showLaunchEngineDialog(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); }));
}

void ActionManager::createDebugMenu()
{
#ifndef NDEBUG
  auto& debugMenu = createMainMenu("Debug");
  debugMenu.addItem(createMenuAction(
    IO::Path("Menu/Debug/Print Vertices"),
    QObject::tr("Print Vertices to Console"),
    0,
    [](ActionExecutionContext& context) { context.frame()->debugPrintVertices(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); }));
  debugMenu.addItem(createMenuAction(
    IO::Path("Menu/Debug/Create Brush..."),
    QObject::tr("Create Brush..."),
    0,
    [](ActionExecutionContext& context) { context.frame()->debugCreateBrush(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); }));
  debugMenu.addItem(createMenuAction(
    IO::Path("Menu/Debug/Create Cube..."),
    QObject::tr("Create Cube..."),
    0,
    [](ActionExecutionContext& context) { context.frame()->debugCreateCube(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); }));
  debugMenu.addItem(createMenuAction(
    IO::Path("Menu/Debug/Clip Brush..."),
    QObject::tr("Clip Brush..."),
    0,
    [](ActionExecutionContext& context) { context.frame()->debugClipBrush(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); }));
  debugMenu.addItem(createMenuAction(
    IO::Path("Menu/Debug/Crash..."),
    QObject::tr("Crash..."),
    0,
    [](ActionExecutionContext& context) { context.frame()->debugCrash(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); }));
  debugMenu.addItem(createMenuAction(
    IO::Path("Menu/Debug/Throw Exception During Command"),
    QObject::tr("Throw Exception During Command"),
    0,
    [](ActionExecutionContext& context) {
      context.frame()->debugThrowExceptionDuringCommand();
    },
    [](ActionExecutionContext& context) { return context.hasDocument(); }));
  debugMenu.addItem(createMenuAction(
    IO::Path("Menu/Debug/Show Crash Report Dialog"),
    QObject::tr("Show Crash Report Dialog..."),
    0,
    [](ActionExecutionContext&) {
      auto& app = TrenchBroomApp::instance();
      app.debugShowCrashReportDialog();
    },
    [](ActionExecutionContext&) { return true; }));
  debugMenu.addItem(createMenuAction(
    IO::Path("Menu/Debug/Set Window Size..."),
    QObject::tr("Set Window Size..."),
    0,
    [](ActionExecutionContext& context) { context.frame()->debugSetWindowSize(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); }));
  debugMenu.addItem(createMenuAction(
    IO::Path("Menu/Debug/Show Palette..."),
    QObject::tr("Show Palette..."),
    0,
    [](ActionExecutionContext& context) { context.frame()->debugShowPalette(); },
    [](ActionExecutionContext& context) { return context.hasDocument(); }));
#endif
}

void ActionManager::createHelpMenu()
{
  auto& helpMenu = createMainMenu("Help");
  helpMenu.addItem(createAction(
    IO::Path("Menu/Help/TrenchBroom Manual"),
    QObject::tr("TrenchBroom Manual"),
    ActionContext::Any,
    QKeySequence(QKeySequence::HelpContents),
    [](ActionExecutionContext&) {
      auto& app = TrenchBroomApp::instance();
      app.showManual();
    },
    [](ActionExecutionContext&) { return true; }));
  helpMenu.addItem(createMenuAction(
    IO::Path("Menu/File/About TrenchBroom"),
    QObject::tr("About TrenchBroom"),
    0,
    [](ActionExecutionContext&) {
      auto& app = TrenchBroomApp::instance();
      app.showAboutDialog();
    },
    [](ActionExecutionContext&) { return true; }));
}

Menu& ActionManager::createMainMenu(const std::string& name)
{
  auto menu = std::make_unique<Menu>(name, MenuEntryType::Menu_None);
  auto* result = menu.get();
  m_mainMenu.emplace_back(std::move(menu));
  return *result;
}

void ActionManager::createToolbar()
{
  m_toolBar = std::make_unique<Menu>("Toolbar", MenuEntryType::Menu_None);
  m_toolBar->addItem(
    existingAction(IO::Path("Controls/Map view/Deactivate current tool")));
  m_toolBar->addItem(existingAction(IO::Path("Menu/Edit/Tools/Brush Tool")));
  m_toolBar->addItem(existingAction(IO::Path("Menu/Edit/Tools/Primitive Brush Tool")));
  m_toolBar->addItem(existingAction(IO::Path("Menu/Edit/Tools/Clip Tool")));
  m_toolBar->addItem(existingAction(IO::Path("Menu/Edit/Tools/Vertex Tool")));
  m_toolBar->addItem(existingAction(IO::Path("Menu/Edit/Tools/Edge Tool")));
  m_toolBar->addItem(existingAction(IO::Path("Menu/Edit/Tools/Face Tool")));
  m_toolBar->addItem(existingAction(IO::Path("Menu/Edit/Tools/Rotate Tool")));
  m_toolBar->addItem(existingAction(IO::Path("Menu/Edit/Tools/Scale Tool")));
  m_toolBar->addItem(existingAction(IO::Path("Menu/Edit/Tools/Shear Tool")));
  m_toolBar->addSeparator();
  m_toolBar->addItem(existingAction(IO::Path("Menu/Edit/Duplicate")));
  m_toolBar->addItem(
    existingAction(IO::Path("Controls/Map view/Flip objects horizontally")));
  m_toolBar->addItem(
    existingAction(IO::Path("Controls/Map view/Flip objects vertically")));
  m_toolBar->addSeparator();
  m_toolBar->addItem(existingAction(IO::Path("Menu/Edit/Texture Lock")));
  m_toolBar->addItem(existingAction(IO::Path("Menu/Edit/UV Lock")));
  m_toolBar->addSeparator();
}

const Action* ActionManager::existingAction(const IO::Path& preferencePath) const
{
  auto it = m_actions.find(preferencePath);
  ensure(it != m_actions.end(), "couldn't find action");
  return it->second.get();
}
} // namespace View
} // namespace TrenchBroom
