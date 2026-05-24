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
#include "mdl/GameConfig.h"
#include "mdl/GameInfo.h"
#include "mdl/Hit.h"
#include "mdl/HitFilter.h"
#include "mdl/Map.h"
#include "mdl/Map_Nodes.h"
#include "mdl/NodeHandleManager.h"
#include "mdl/NodeHandles.h"
#include "mdl/Polyhedron.h"
#include "mdl/Polyhedron3.h"
#include "mdl/SelectionChange.h"
#include "mdl/Transaction.h"
#include "mdl/TransactionScope.h"
#include "mdl/WorldNode.h"
#include "render/RenderBatch.h"
#include "render/RenderService.h"
#include "ui/InputState.h"
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
#include <type_traits>
#include <vector>

namespace tb
{
namespace gl
{
class Camera;
}

namespace mdl
{
class Grid;
class PickResult;
} // namespace mdl

namespace ui
{
class Lasso;
class MapDocument;

template <typename HandleType>
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
  typename HandleType::Position m_dragHandlePosition;
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
  template <typename SomeHandleType>
    requires(!std::ranges::range<SomeHandleType>)
  std::vector<mdl::BrushNode*> findIncidentBrushes(const SomeHandleType& handle) const
  {
    const auto hasHandle = [&](const auto* brushNode) {
      const auto brushHandles = SomeHandleType::getHandles(*brushNode);
      return std::ranges::find(brushHandles, handle) != brushHandles.end();
    };

    auto result =
      selectedBrushes() | std::views::filter(hasHandle) | kdl::ranges::to<std::vector>();
    return kdl::vec_sort_and_remove_duplicates(std::move(result));
  }

  template <std::ranges::range R>
  std::vector<mdl::BrushNode*> findIncidentBrushes(const R& handles) const
  {
    using SomeHandleType = std::remove_cvref_t<std::ranges::range_value_t<R>>;

    const auto hasHandle = [&](const auto* brushNode, const auto& handle) {
      const auto brushHandles = SomeHandleType::getHandles(*brushNode);
      return std::ranges::find(handles, handle) != handles.end();
    };

    const auto hasAnyHandle = [&](const auto* brushNode) {
      return std::ranges::any_of(
        handles, [&](const auto& handle) { return hasHandle(brushNode, handle); });
    };

    auto result = selectedBrushes() | std::views::filter(hasAnyHandle)
                  | kdl::ranges::to<std::vector>();
    return kdl::vec_sort_and_remove_duplicates(std::move(result));
  }

  virtual void pick(
    const vm::ray3d& pickRay,
    const gl::Camera& camera,
    double handleRadius,
    mdl::PickResult& pickResult) const = 0;

  virtual mdl::Hit findDraggableHandle(
    const InputState& inputState, const mdl::HitType::Type hitType) const
  {
    using namespace mdl::HitFilters;

    const auto hits = inputState.pickResult().all(type(hitType));
    if (!hits.empty())
    {
      for (const auto& hit : hits)
      {
        if (selected(hit))
        {
          return hit;
        }
      }
      return inputState.pickResult().first(type(hitType));
    }
    return mdl::Hit::NoHit;
  }

  virtual std::vector<mdl::Hit> collectDraggableHandles(
    const InputState& inputState, const mdl::HitType::Type hitType) const
  {
    using namespace mdl::HitFilters;
    return inputState.pickResult().all(type(hitType));
  }

public: // Handle selection
  bool select(const std::vector<mdl::Hit>& hits, const bool addToSelection)
  {
    contract_pre(!hits.empty());

    if (const auto& firstHit = hits.front(); firstHit.hasType(HandleType::HandleHitType))
    {
      if (!addToSelection)
      {
        handleManager().template deselectAllHandles<HandleType>();
      }

      // Count the number of hit handles which are selected already.
      size_t selected = 0u;
      for (const auto& hit : hits)
      {
        if (handleManager().template isHandleSelected<HandleType>(
              hit.target<HandleType>()))
        {
          ++selected;
        }
      }

      if (selected < hits.size())
      {
        for (const auto& hit : hits)
        {
          handleManager().template selectHandle<HandleType>(hit.target<HandleType>());
        }
      }
      else if (addToSelection)
      {
        // The user meant to deselect a selected handle.
        for (const auto& hit : hits)
        {
          handleManager().template deselectHandle<HandleType>(hit.target<HandleType>());
        }
      }
    }
    refreshViews();
    notifyToolHandleSelectionChanged();
    return true;
  }

  void select(const Lasso& lasso, const bool modifySelection)
  {
    auto selectedHandles = std::vector<HandleType>{};
    auto allHandles = handleManager().template allHandles<HandleType>();
    lasso.selected(allHandles, std::back_inserter(selectedHandles));

    if (!modifySelection)
    {
      handleManager().template deselectAllHandles<HandleType>();
    }
    handleManager().template toggleHandles<HandleType>(selectedHandles);

    refreshViews();
    notifyToolHandleSelectionChanged();
  }

  bool selected(const mdl::Hit& hit) const
  {
    return handleManager().template isHandleSelected<HandleType>(
      hit.target<HandleType>());
  }

  virtual bool deselectAll()
  {
    if (handleManager().template anyHandleSelected<HandleType>())
    {
      handleManager().template deselectAllHandles<HandleType>();
      refreshViews();
      notifyToolHandleSelectionChanged();
      return true;
    }
    return false;
  }

public:
  mdl::NodeHandleManager& handleManager() { return m_document.map().nodeHandles(); }

  const mdl::NodeHandleManager& handleManager() const
  {
    return m_document.map().nodeHandles();
  }

public: // performing moves
  virtual std::tuple<vm::vec3d, vm::vec3d> handlePositionAndHitPoint(
    const std::vector<mdl::Hit>& hits) const = 0;

  virtual bool startMove(const std::vector<mdl::Hit>& hits)
  {
    contract_pre(!hits.empty());

    auto matchingHits = hits | std::views::filter([](const auto& hit) {
                          return hit.hasType(HandleType::HandleHitType);
                        });

    // Delesect all handles if any of the hit handles is not already selected.
    if (std::ranges::any_of(matchingHits, [&](const auto& hit) {
          return !handleManager().template isHandleSelected<HandleType>(
            hit.template target<HandleType>());
        }))
    {
      handleManager().template deselectAllHandles<HandleType>();
    }

    // Now select all of the hit handles.
    for (const auto& hit : matchingHits)
    {
      handleManager().template selectHandle<HandleType>(
        hit.template target<HandleType>());
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
  bool canDoCsgConvexMerge()
  {
    return handleManager().template selectedHandleCount<HandleType>() > 1;
  }

  void csgConvexMerge()
  {

    auto handles = handleManager().template selectedHandles<HandleType>();
    const auto vertices = HandleType::getVertices(handles);

    const auto polyhedron = mdl::Polyhedron3{vertices};
    if (!polyhedron.polyhedron() || !polyhedron.closed())
    {
      return;
    }

    auto& map = m_document.map();

    const auto builder = mdl::BrushBuilder{
      map.worldNode().mapFormat(),
      map.worldBounds(),
      map.gameInfo().gameConfig.faceAttribsConfig.defaults};
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

  virtual HandleType::Position getHandlePosition(const mdl::Hit& hit) const
  {
    contract_pre(hit.isMatch());
    contract_pre(hit.hasType(HandleType::HandleHitType));

    return hit.target<HandleType>().position;
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

  bool canRemoveSelection() const
  {
    return handleManager().template selectedHandleCount<HandleType>() > 0;
  }

public: // rendering
  void renderHandles(
    render::RenderContext& renderContext, render::RenderBatch& renderBatch) const
  {
    auto renderService = render::RenderService{renderContext, renderBatch};
    if (!handleManager().template allHandlesSelected<HandleType>())
    {
      const auto handlePositions = HandleType::getPositions(
        handleManager().template unselectedHandles<HandleType>());
      renderHandles(handlePositions, renderService, pref(Preferences::HandleColor));
    }
    if (handleManager().template anyHandleSelected<HandleType>())
    {
      const auto handlePositions =
        HandleType::getPositions(handleManager().template selectedHandles<HandleType>());
      renderHandles(
        handlePositions, renderService, pref(Preferences::SelectedHandleColor));
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

  template <typename HandlePosition>
  void renderHandles(
    const std::vector<HandlePosition>& handles,
    render::RenderService& renderService,
    const Color& color) const
  {
    renderService.setForegroundColor(color);
    renderService.renderHandles(
      kdl::vec_static_cast<typename HandlePosition::float_type>(handles));
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

    handleManager().template clear<HandleType>();
    handleManager().template addHandles<HandleType>(selectedBrushes());

    return true;
  }

  bool doDeactivate() override
  {
    m_notifierConnection.disconnect();
    handleManager().template clear<HandleType>();
    return true;
  }

private: // Observers and state management
  void connectObservers()
  {
    m_notifierConnection += m_document.selectionDidChangeNotifier.connect(
      this, &VertexToolBase::selectionDidChange);
    m_notifierConnection +=
      m_document.nodesWillChangeNotifier.connect(this, &VertexToolBase::nodesWillChange);
    m_notifierConnection +=
      m_document.nodesDidChangeNotifier.connect(this, &VertexToolBase::nodesDidChange);

    m_notifierConnection +=
      m_document.commandDoNotifier.connect(this, &VertexToolBase::commandDo);
    m_notifierConnection +=
      m_document.commandDoneNotifier.connect(this, &VertexToolBase::commandDone);
    m_notifierConnection +=
      m_document.commandDoFailedNotifier.connect(this, &VertexToolBase::commandDoFailed);
    m_notifierConnection +=
      m_document.commandUndoNotifier.connect(this, &VertexToolBase::commandUndo);
    m_notifierConnection +=
      m_document.commandUndoneNotifier.connect(this, &VertexToolBase::commandUndone);
    m_notifierConnection += m_document.commandUndoFailedNotifier.connect(
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
    if (
      auto* vertexCommand = dynamic_cast<mdl::BrushVertexCommandT<HandleType>*>(&command))
    {
      deselectHandles();
      removeHandles(*vertexCommand);
      ++m_ignoreChangeNotifications;
    }
  }

  void commandDoneOrUndoFailed(mdl::Command& command)
  {
    if (
      auto* vertexCommand = dynamic_cast<mdl::BrushVertexCommandT<HandleType>*>(&command))
    {
      addHandles(*vertexCommand);
      selectNewHandlePositions(*vertexCommand);
      --m_ignoreChangeNotifications;
    }
  }

  void commandDoFailedOrUndone(mdl::Command& command)
  {
    if (
      auto* vertexCommand = dynamic_cast<mdl::BrushVertexCommandT<HandleType>*>(&command))
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
  virtual void deselectHandles()
  {
    handleManager().template deselectAllHandles<HandleType>();
  }

  virtual void addHandles(mdl::BrushVertexCommandT<HandleType>& command)
  {
    command.addHandles(handleManager());
  }

  virtual void removeHandles(mdl::BrushVertexCommandT<HandleType>& command)
  {
    command.removeHandles(handleManager());
  }

  virtual void selectNewHandlePositions(mdl::BrushVertexCommandT<HandleType>& command)
  {
    command.selectNewHandlePositions(handleManager());
  }

  virtual void selectOldHandlePositions(mdl::BrushVertexCommandT<HandleType>& command)
  {
    command.selectOldHandlePositions(handleManager());
  }

  virtual void addHandles(const std::vector<mdl::Node*>& nodes)
  {
    handleManager().template addHandles<HandleType>(nodes);
  }

  virtual void removeHandles(const std::vector<mdl::Node*>& nodes)
  {
    handleManager().template removeHandles<HandleType>(nodes);
  }
};

} // namespace ui
} // namespace tb
