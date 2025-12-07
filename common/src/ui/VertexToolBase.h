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

#include "Logger.h"
#include "NotifierConnection.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushNode.h"
#include "mdl/BrushVertexCommands.h"
#include "mdl/CommandProcessor.h"
#include "mdl/Game.h"
#include "mdl/GameConfig.h"
#include "mdl/Hit.h"
#include "mdl/Map.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Polyhedron.h"
#include "mdl/Polyhedron3.h"
#include "mdl/SelectionChange.h"
#include "mdl/Transaction.h"
#include "mdl/TransactionScope.h"
#include "mdl/VertexHandleManager.h"
#include "mdl/WorldNode.h"
#include "render/RenderBatch.h"
#include "render/RenderService.h"
#include "ui/Lasso.h"
#include "ui/MapDocument.h"
#include "ui/Tool.h"

#include "kd/contracts.h"
#include "kd/overload.h"
#include "kd/ranges/to.h"
#include "kd/result.h"
#include "kd/set_temp.h"
#include "kd/string_utils.h"
#include "kd/vector_utils.h"

#include "vm/vec.h"
#include "vm/vec_io.h" // IWYU pragma: keep

#include <map>
#include <ranges>
#include <string>
#include <vector>

namespace tb
{
namespace mdl
{
class Grid;
class PickResult;
} // namespace mdl

namespace render
{
class Camera;
}

namespace ui
{
class Lasso;
class MapDocument;

template <typename H>
class VertexToolBase : public Tool
{
public:
  enum class MoveResult
  {
    Continue,
    Deny,
    Cancel
  };

protected:
  MapDocument& m_document;

private:
  size_t m_changeCount = 0;
  size_t m_ignoreChangeNotifications = 0;
  NotifierConnection m_notifierConnection;

protected:
  H m_dragHandlePosition;
  bool m_dragging = false;

protected:
  explicit VertexToolBase(MapDocument& document)
    : Tool{false}
    , m_document{document}
  {
  }

public:
  ~VertexToolBase() override = default;

public:
  const mdl::Grid& grid() const { return m_document.map().grid(); }

  const std::vector<mdl::BrushNode*>& selectedBrushes() const
  {
    return m_document.map().selection().brushes;
  }

public:
  template <typename M, typename H2>
    requires(!std::ranges::range<H2>)
  std::vector<mdl::BrushNode*> findIncidentBrushes(
    const M& manager, const H2& handle) const
  {
    return manager.findIncidentBrushes(handle, selectedBrushes());
  }

  template <typename M, std::ranges::range R>
  std::vector<mdl::BrushNode*> findIncidentBrushes(
    const M& manager, const R& handles) const
  {
    const auto& brushes = selectedBrushes();
    auto result = std::vector<mdl::BrushNode*>{};
    auto out = std::back_inserter(result);

    for (const auto& handle : handles)
    {
      manager.findIncidentBrushes(handle, brushes, out);
    }

    return kdl::vec_sort_and_remove_duplicates(std::move(result));
  }

  virtual void pick(
    const vm::ray3d& pickRay,
    const render::Camera& camera,
    mdl::PickResult& pickResult) const = 0;

public: // Handle selection
  bool select(const std::vector<mdl::Hit>& hits, const bool addToSelection)
  {
    contract_pre(!hits.empty());

    if (const auto& firstHit = hits.front(); firstHit.type() == handleManager().hitType())
    {
      if (!addToSelection)
      {
        handleManager().deselectAll();
      }

      // Count the number of hit handles which are selected already.
      size_t selected = 0u;
      for (const auto& hit : hits)
      {
        if (handleManager().selected(hit.target<H>()))
        {
          ++selected;
        }
      }

      if (selected < hits.size())
      {
        for (const auto& hit : hits)
        {
          handleManager().select(hit.target<H>());
        }
      }
      else if (addToSelection)
      {
        // The user meant to deselect a selected handle.
        for (const auto& hit : hits)
        {
          handleManager().deselect(hit.target<H>());
        }
      }
    }
    refreshViews();
    notifyToolHandleSelectionChanged();
    return true;
  }

  void select(const Lasso& lasso, const bool modifySelection)
  {
    auto selectedHandles = std::vector<H>{};
    lasso.selected(handleManager().allHandles(), std::back_inserter(selectedHandles));

    if (!modifySelection)
    {
      handleManager().deselectAll();
    }
    handleManager().toggle(selectedHandles);

    refreshViews();
    notifyToolHandleSelectionChanged();
  }

  bool selected(const mdl::Hit& hit) const
  {
    return handleManager().selected(hit.target<H>());
  }

  virtual bool deselectAll()
  {
    if (handleManager().anySelected())
    {
      handleManager().deselectAll();
      refreshViews();
      notifyToolHandleSelectionChanged();
      return true;
    }
    return false;
  }

public:
  using HandleManager = mdl::VertexHandleManagerBaseT<H>;
  virtual HandleManager& handleManager() = 0;
  virtual const HandleManager& handleManager() const = 0;

public: // performing moves
  virtual std::tuple<vm::vec3d, vm::vec3d> handlePositionAndHitPoint(
    const std::vector<mdl::Hit>& hits) const = 0;

  virtual bool startMove(const std::vector<mdl::Hit>& hits)
  {
    contract_pre(!hits.empty());

    // Delesect all handles if any of the hit handles is not already selected.
    for (const auto& hit : hits)
    {
      const H handle = getHandlePosition(hit);
      if (!handleManager().selected(handle))
      {
        handleManager().deselectAll();
        break;
      }
    }

    // Now select all of the hit handles.
    for (const auto& hit : hits)
    {
      const H handle = getHandlePosition(hit);
      if (hit.hasType(handleManager().hitType()))
      {
        handleManager().select(handle);
      }
    }
    refreshViews();

    m_document.map().startTransaction(actionName(), mdl::TransactionScope::LongRunning);

    m_dragHandlePosition = getHandlePosition(hits.front());
    m_dragging = true;
    ++m_ignoreChangeNotifications;
    return true;
  }

  virtual MoveResult move(const vm::vec3d& delta) = 0;

  virtual void endMove()
  {
    m_document.map().commitTransaction();
    m_dragging = false;
    --m_ignoreChangeNotifications;
  }

  virtual void cancelMove()
  {
    m_document.map().cancelTransaction();
    m_dragging = false;
    --m_ignoreChangeNotifications;
  }

  virtual bool allowAbsoluteSnapping() const
  {
    // override in VertexTool
    return false;
  }

public: // csg convex merge
  bool canDoCsgConvexMerge() { return handleManager().selectedHandleCount() > 1; }

  void csgConvexMerge()
  {
    auto vertices = std::vector<vm::vec3d>{};
    const auto handles = handleManager().selectedHandles();
    H::get_vertices(std::begin(handles), std::end(handles), std::back_inserter(vertices));

    const auto polyhedron = mdl::Polyhedron3{vertices};
    if (!polyhedron.polyhedron() || !polyhedron.closed())
    {
      return;
    }

    auto& map = m_document.map();
    auto game = map.game();

    const auto builder = mdl::BrushBuilder{
      map.world()->mapFormat(),
      map.worldBounds(),
      game->config().faceAttribsConfig.defaults};
    builder.createBrush(polyhedron, map.currentMaterialName())
      | kdl::transform([&](auto b) {
          for (const auto* selectedBrushNode : map.selection().brushes)
          {
            b.cloneFaceAttributesFrom(selectedBrushNode->brush());
          }

          auto* newParent = parentForNodes(map, map.selection().nodes);
          auto transaction = mdl::Transaction{map, "CSG Convex Merge"};
          deselectAll();
          if (addNodes(map, {{newParent, {new mdl::BrushNode{std::move(b)}}}}).empty())
          {
            transaction.cancel();
            return;
          }
          transaction.commit();
        })
      | kdl::transform_error(
        [&](auto e) { map.logger().error() << "Could not create brush: " << e.msg; });
  }

  virtual H getHandlePosition(const mdl::Hit& hit) const
  {
    contract_pre(hit.isMatch());
    contract_pre(hit.hasType(handleManager().hitType()));

    return hit.target<H>();
  }

  virtual std::string actionName() const = 0;

public:
  void moveSelection(const vm::vec3d& delta)
  {
    const auto ignoreChangeNotifications = kdl::inc_temp{m_ignoreChangeNotifications};

    auto transaction = mdl::Transaction{m_document.map(), actionName()};
    move(delta);
    transaction.commit();
  }

  bool canRemoveSelection() const { return handleManager().selectedHandleCount() > 0; }

public: // rendering
  void renderHandles(
    render::RenderContext& renderContext, render::RenderBatch& renderBatch) const
  {
    auto renderService = render::RenderService{renderContext, renderBatch};
    if (!handleManager().allSelected())
    {
      renderHandles(
        handleManager().unselectedHandles(),
        renderService,
        pref(Preferences::HandleColor));
    }
    if (handleManager().anySelected())
    {
      renderHandles(
        handleManager().selectedHandles(),
        renderService,
        pref(Preferences::SelectedHandleColor));
    }
  }

  void renderDragHandle(
    render::RenderContext& renderContext, render::RenderBatch& renderBatch) const
  {
    renderHandle(
      renderContext,
      renderBatch,
      m_dragHandlePosition,
      pref(Preferences::SelectedHandleColor));
  }

  template <typename HH>
  void renderHandle(
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch,
    const HH& handle) const
  {
    renderHandle(renderContext, renderBatch, handle, pref(Preferences::HandleColor));
  }

  void renderDragHighlight(
    render::RenderContext& renderContext, render::RenderBatch& renderBatch) const
  {
    renderHighlight(renderContext, renderBatch, m_dragHandlePosition);
  }

  void renderDragGuide(
    render::RenderContext& renderContext, render::RenderBatch& renderBatch) const
  {
    renderGuide(renderContext, renderBatch, m_dragHandlePosition);
  }

  template <typename HH>
  void renderHandles(
    const std::vector<HH>& handles,
    render::RenderService& renderService,
    const Color& color) const
  {
    renderService.setForegroundColor(color);
    renderService.renderHandles(kdl::vec_static_cast<typename HH::float_type>(handles));
  }

  template <typename HH>
  void renderHandle(
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch,
    const HH& handle,
    const Color& color) const
  {
    auto renderService = render::RenderService{renderContext, renderBatch};
    renderService.setForegroundColor(color);
    renderService.renderHandle(typename HH::float_type(handle));
  }

  template <typename HH>
  void renderHighlight(
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch,
    const HH& handle) const
  {
    auto renderService = render::RenderService{renderContext, renderBatch};
    renderService.setForegroundColor(pref(Preferences::SelectedHandleColor));
    renderService.renderHandleHighlight(typename HH::float_type(handle));
  }

  void renderHighlight(
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch,
    const vm::vec3d& handle) const
  {
    auto renderService = render::RenderService{renderContext, renderBatch};
    renderService.setForegroundColor(pref(Preferences::SelectedHandleColor));
    renderService.renderHandleHighlight(vm::vec3f{handle});

    renderService.setForegroundColor(pref(Preferences::SelectedInfoOverlayTextColor));
    renderService.setBackgroundColor(
      pref(Preferences::SelectedInfoOverlayBackgroundColor));
    renderService.renderString(kdl::str_to_string(handle), vm::vec3f{handle});
  }

  template <typename HH>
  void renderGuide(
    render::RenderContext&, render::RenderBatch&, const HH& /* position */) const
  {
  }

  virtual void renderGuide(
    render::RenderContext&, render::RenderBatch&, const vm::vec3d& /* position */) const
  {
  }

protected: // Tool interface
  bool doActivate() override
  {
    m_changeCount = 0;
    connectObservers();

    handleManager().clear();
    handleManager().addHandles(selectedBrushes());

    return true;
  }

  bool doDeactivate() override
  {
    m_notifierConnection.disconnect();
    handleManager().clear();
    return true;
  }

private: // Observers and state management
  void connectObservers()
  {
    auto& map = m_document.map();

    m_notifierConnection +=
      map.selectionDidChangeNotifier.connect(this, &VertexToolBase::selectionDidChange);
    m_notifierConnection +=
      map.nodesWillChangeNotifier.connect(this, &VertexToolBase::nodesWillChange);
    m_notifierConnection +=
      map.nodesDidChangeNotifier.connect(this, &VertexToolBase::nodesDidChange);

    auto& commandProcessor = map.commandProcessor();
    m_notifierConnection +=
      commandProcessor.commandDoNotifier.connect(this, &VertexToolBase::commandDo);
    m_notifierConnection +=
      commandProcessor.commandDoneNotifier.connect(this, &VertexToolBase::commandDone);
    m_notifierConnection += commandProcessor.commandDoFailedNotifier.connect(
      this, &VertexToolBase::commandDoFailed);
    m_notifierConnection +=
      commandProcessor.commandUndoNotifier.connect(this, &VertexToolBase::commandUndo);
    m_notifierConnection += commandProcessor.commandUndoneNotifier.connect(
      this, &VertexToolBase::commandUndone);
    m_notifierConnection += commandProcessor.commandUndoFailedNotifier.connect(
      this, &VertexToolBase::commandUndoFailed);
  }

  void commandDo(mdl::Command& command) { commandDoOrUndo(command); }

  void commandDone(mdl::Command& command) { commandDoneOrUndoFailed(command); }

  void commandDoFailed(mdl::Command& command) { commandDoFailedOrUndone(command); }

  void commandUndo(mdl::UndoableCommand& command) { commandDoOrUndo(command); }

  void commandUndone(mdl::UndoableCommand& command) { commandDoFailedOrUndone(command); }

  void commandUndoFailed(mdl::UndoableCommand& command)
  {
    commandDoneOrUndoFailed(command);
  }

  void commandDoOrUndo(mdl::Command& command)
  {
    if (auto* vertexCommand = dynamic_cast<mdl::BrushVertexCommandT<H>*>(&command))
    {
      deselectHandles();
      removeHandles(*vertexCommand);
      ++m_ignoreChangeNotifications;
    }
  }

  void commandDoneOrUndoFailed(mdl::Command& command)
  {
    if (auto* vertexCommand = dynamic_cast<mdl::BrushVertexCommandT<H>*>(&command))
    {
      addHandles(*vertexCommand);
      selectNewHandlePositions(*vertexCommand);
      --m_ignoreChangeNotifications;
    }
  }

  void commandDoFailedOrUndone(mdl::Command& command)
  {
    if (auto* vertexCommand = dynamic_cast<mdl::BrushVertexCommandT<H>*>(&command))
    {
      addHandles(*vertexCommand);
      selectOldHandlePositions(*vertexCommand);
      --m_ignoreChangeNotifications;
    }
  }

  void selectionDidChange(const mdl::SelectionChange& selectionChange)
  {
    addHandles(selectionChange.selectedNodes);
    removeHandles(selectionChange.deselectedNodes);
  }

  void nodesWillChange(const std::vector<mdl::Node*>& nodes)
  {
    if (m_ignoreChangeNotifications == 0u)
    {
      const auto selectedNodes =
        nodes | std::views::filter([](const auto* node) { return node->selected(); })
        | kdl::ranges::to<std::vector>();
      removeHandles(selectedNodes);
    }
  }

  void nodesDidChange(const std::vector<mdl::Node*>& nodes)
  {
    if (m_ignoreChangeNotifications == 0u)
    {
      const auto selectedNodes =
        nodes | std::views::filter([](const auto* node) { return node->selected(); })
        | kdl::ranges::to<std::vector>();
      addHandles(selectedNodes);
    }
  }

protected:
  virtual void deselectHandles() { handleManager().deselectAll(); }

  virtual void addHandles(mdl::BrushVertexCommandT<H>& command)
  {
    command.addHandles(handleManager());
  }

  virtual void removeHandles(mdl::BrushVertexCommandT<H>& command)
  {
    command.removeHandles(handleManager());
  }

  virtual void selectNewHandlePositions(mdl::BrushVertexCommandT<H>& command)
  {
    command.selectNewHandlePositions(handleManager());
  }

  virtual void selectOldHandlePositions(mdl::BrushVertexCommandT<H>& command)
  {
    command.selectOldHandlePositions(handleManager());
  }

  template <typename HT>
  void addHandles(
    const std::vector<mdl::Node*>& nodes,
    mdl::VertexHandleManagerBaseT<HT>& handleManager)
  {
    for (const auto* node : nodes)
    {
      node->accept(kdl::overload(
        [](const mdl::WorldNode*) {},
        [](const mdl::LayerNode*) {},
        [](const mdl::GroupNode*) {},
        [](const mdl::EntityNode*) {},
        [&](const mdl::BrushNode* brush) { handleManager.addHandles(brush); },
        [](const mdl::PatchNode*) {}));
    }
  }

  template <typename HT>
  void removeHandles(
    const std::vector<mdl::Node*>& nodes,
    mdl::VertexHandleManagerBaseT<HT>& handleManager)
  {
    for (const auto* node : nodes)
    {
      node->accept(kdl::overload(
        [](const mdl::WorldNode*) {},
        [](const mdl::LayerNode*) {},
        [](const mdl::GroupNode*) {},
        [](const mdl::EntityNode*) {},
        [&](const mdl::BrushNode* brush) { handleManager.removeHandles(brush); },
        [](const mdl::PatchNode*) {}));
    }
  }

  virtual void addHandles(const std::vector<mdl::Node*>& nodes)
  {
    addHandles(nodes, handleManager());
  }

  virtual void removeHandles(const std::vector<mdl::Node*>& nodes)
  {
    removeHandles(nodes, handleManager());
  }
};

} // namespace ui
} // namespace tb
