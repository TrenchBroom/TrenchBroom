/*
 Copyright (C) 2026 Kristian Duske

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

#include "mdl/PickResult.h"

#include "kd/const_overload.h"
#include "kd/contracts.h"

#include <vm/polygon.h>
#include <vm/segment.h>
#include <vm/vec.h>

#include <algorithm>
#include <map>
#include <memory>
#include <ranges>
#include <typeindex>
#include <unordered_map>

#pragma once

namespace tb::mdl
{
class Node;

/**
 * Manages geometry handles (vertices, edges, faces, ...) extracted from nodes, and tracks
 * which handles are currently selected.
 *
 * Each distinct handle type (e.g. vertex handle, edge handle) must be registered via
 * registerHandleType() before handles of that type can be added. Multiple handle types
 * can be registered on the same manager; they are stored and queried independently.
 *
 * ## Clumping
 *
 * Handles that are spatially close to one another are grouped into clumps. Two handles
 * belong to the same clump when their distance is less than the clump distance configured
 * at construction time (default: 0.01). Selection operates at the clump level: selecting
 * any handle in a clump selects all handles in that clump, and deselecting any handle
 * deselects the whole clump.
 *
 * Clumps are formed and dissolved dynamically as handles are added and removed:
 * - When a handle is added, every existing clump whose nearest member is within the clump
 *   distance is merged into a single new clump together with the new handle. A merged
 *   clump is selected if any of the source clumps were selected.
 * - When a handle is removed, its clump is dissolved and the remaining handles are
 *   re-inserted one by one, potentially forming several smaller clumps. If the dissolved
 *   clump was selected, each newly formed clump is also marked as selected.
 *
 * This means that adding a "bridge" handle that is close to two previously separate
 * clumps will merge them into one, and removing it later will split them apart again.
 *
 * ## Handle types
 *
 * A handle type `H` must satisfy the following static interface:
 * - `static auto H::getHandles(const Node&)`: returns a range of handles for a node.
 * - `static double H::distance(const H& a, const H& b)`: returns the distance between
 *   two handles, used to determine whether they belong to the same clump.
 * - `auto H::pick(...)`: returns an `std::optional<Hit>` for a pick ray query.
 */
class NodeHandleManager
{
private:
  class HandleMapBase
  {
  public:
    virtual ~HandleMapBase();

    virtual void addHandles(const Node& node) = 0;
    virtual void removeHandles(const Node& node) = 0;
  };

  template <typename HandleType>
  class HandleMap : public HandleMapBase
  {
  private:
    struct HandleClump
    {
      std::map<HandleType, size_t> handles;
      size_t count = 0u;
      bool selected = false;

      void addHandle(const HandleType& handle, size_t handleCount = 1u)
      {
        handles[handle] += handleCount;
        count += handleCount;
      }

      void removeHandle(const HandleType& handle)
      {
        auto iHandle = handles.find(handle);
        contract_assert(iHandle != handles.end());

        --iHandle->second;
        if (iHandle->second == 0)
        {
          handles.erase(iHandle);
        }
        --count;
      }

      bool select() { return !std::exchange(selected, true); }
      bool deselect() { return std::exchange(selected, false); }

      bool shouldContain(const HandleType& handle, const double clumpDistance) const
      {
        const auto shouldClump = [&](const auto& candidate) {
          return HandleType::distance(handle, candidate) < clumpDistance;
        };

        return std::ranges::any_of(handles | std::views::keys, shouldClump);
      }
    };

    std::vector<std::unique_ptr<HandleClump>> m_handleClumps;
    std::map<HandleType, HandleClump*> m_handles;
    size_t numSelectedHandleClumps = 0;
    double m_clumpDistance;

  public:
    explicit HandleMap(const double clumpDistance)
      : m_clumpDistance{clumpDistance}
    {
    }

    size_t handleCount() const { return m_handles.size(); }

    size_t selectedHandleCount() const { return numSelectedHandleClumps; }

    bool anyHandleSelected() const { return numSelectedHandleClumps > 0u; }

    bool allHandlesSelected() const
    {
      return std::ranges::all_of(
        m_handles, [](const auto& entry) { return entry.second->selected; });
    }

    bool containsHandle(const HandleType& handle) const
    {
      return findHandleEntry(handle) != m_handles.end();
    }

    bool isHandleSelected(const HandleType& handle) const
    {
      const auto iEntry = findHandleEntry(handle);
      return iEntry != m_handles.end() && iEntry->second->selected;
    }

    auto allHandles() const { return m_handles | std::views::keys; }

    auto selectedHandles() const
    {
      const auto isSelected = [](const auto& entry) { return entry.second->selected; };
      return m_handles | std::views::filter(isSelected) | std::views::keys;
    }

    auto unselectedHandles() const
    {
      const auto isUnselected = [](const auto& entry) { return !entry.second->selected; };
      return m_handles | std::views::filter(isUnselected) | std::views::keys;
    }

    void selectHandle(const HandleType& handle)
    {
      auto iEntry = findHandleEntry(handle);
      contract_assert(iEntry != m_handles.end());

      selectHandleClump(*iEntry->second);
    }

    void deselectHandle(const HandleType& handle)
    {
      auto iEntry = findHandleEntry(handle);
      contract_assert(iEntry != m_handles.end());

      deselectHandleClump(*iEntry->second);
    }

    void toggleHandle(const HandleType& handle)
    {
      auto iEntry = findHandleEntry(handle);
      contract_assert(iEntry != m_handles.end());

      if (!iEntry->second->selected)
      {
        selectHandleClump(*iEntry->second);
      }
      else
      {
        deselectHandleClump(*iEntry->second);
      }
    }

    void deselectAllHandles()
    {
      for (auto& handleClump : m_handleClumps)
      {
        handleClump->selected = false;
      }
      numSelectedHandleClumps = 0u;
    }

    void clear()
    {
      m_handleClumps.clear();
      m_handles.clear();
      numSelectedHandleClumps = 0u;
    }

    template <typename... Args>
    void pick(PickResult& pickResult, Args&&... args) const
    {
      for (const auto& [handle, handleClump] : m_handles)
      {
        if (const auto hit = handle.pick(std::forward<Args>(args)...))
        {
          pickResult.addHit(*hit);
        }
      }
    }

    void addHandles(const Node& node) override
    {
      for (const auto& handle : HandleType::getHandles(node))
      {
        addHandle(handle);
      }
    }

    void removeHandles(const Node& node) override
    {
      for (const auto& handle : HandleType::getHandles(node))
      {
        removeHandle(handle);
      }
    }

  private:
    auto findHandleEntry(const HandleType& handle) const
    {
      auto iEntry = m_handles.find(handle);
      if (iEntry != m_handles.end())
      {
        return iEntry;
      }

      // A handle may be reconstructed after a geometric edit with small floating-point
      // differences, so fall back to the first handle in the same clump distance.
      return std::ranges::find_if(m_handles, [&](const auto& entry) {
        return HandleType::distance(handle, entry.first) < m_clumpDistance;
      });
    }

    auto findHandleEntry(const HandleType& handle)
    {
      return KDL_CONST_OVERLOAD(findHandleEntry(handle));
    }

    void addHandle(const HandleType& handle, const size_t count = 1u)
    {
      auto newHandleClump = std::make_unique<HandleClump>();
      newHandleClump->addHandle(handle, count);

      for (auto iHandleClump = m_handleClumps.begin();
           iHandleClump != m_handleClumps.end();)
      {
        auto& handleClump = *iHandleClump;
        if (handleClump->shouldContain(handle, m_clumpDistance))
        {
          for (const auto& [clumpedHandle, clumpedHandleCount] : handleClump->handles)
          {
            newHandleClump->addHandle(clumpedHandle, clumpedHandleCount);
          }
          newHandleClump->selected = newHandleClump->selected || handleClump->selected;

          iHandleClump = removeHandleClump(iHandleClump).first;
        }
        else
        {
          ++iHandleClump;
        }
      }

      addHandleClump(std::move(newHandleClump));
    }

    void removeHandle(const HandleType& handle)
    {
      auto iHandleClump = std::ranges::find_if(
        m_handleClumps,
        [&](const auto& handleClump) { return handleClump->handles.contains(handle); });
      contract_assert(iHandleClump != m_handleClumps.end());

      auto handleClump = removeHandleClump(iHandleClump).second;

      handleClump->removeHandle(handle);

      // reinsert the handles to re-form the clump (might have become disjoint and form
      // more than one clump)
      for (const auto& [remainingHandle, count] : handleClump->handles)
      {
        addHandle(remainingHandle, count);
      }

      if (handleClump->selected)
      {
        for (const auto& remainingHandle : handleClump->handles | std::views::keys)
        {
          selectHandle(remainingHandle);
        }
      }
    }

    void addHandleClump(std::unique_ptr<HandleClump> handleClump)
    {
      for (const auto& [handle, count] : handleClump->handles)
      {
        m_handles.emplace(handle, handleClump.get());
      }

      if (handleClump->selected)
      {
        ++numSelectedHandleClumps;
      }

      m_handleClumps.push_back(std::move(handleClump));
    }

    auto removeHandleClump(auto iHandleClump)
    {
      auto handleClump = std::move(*iHandleClump);
      if (handleClump->selected)
      {
        --numSelectedHandleClumps;
      }

      for (const auto& [handle, count] : handleClump->handles)
      {
        m_handles.erase(handle);
      }

      auto iNextHandleClump = m_handleClumps.erase(iHandleClump);
      return std::pair{iNextHandleClump, std::move(handleClump)};
    }

    void selectHandleClump(HandleClump& handleClump)
    {
      if (handleClump.select())
      {
        contract_assert(selectedHandleCount() < handleCount());
        ++numSelectedHandleClumps;
      }
    }

    void deselectHandleClump(HandleClump& handleClump)
    {
      if (handleClump.deselect())
      {
        contract_assert(selectedHandleCount() > 0);
        --numSelectedHandleClumps;
      }
    }
  };

  std::unordered_map<std::type_index, std::unique_ptr<HandleMapBase>> m_handleMaps;
  double m_clumpDistance;

public:
  /**
   * Creates a manager with the given clump distance.
   *
   * Handles whose pairwise distance is less than @p clumpDistance are grouped into a
   * single clump and selected or deselected together.
   *
   * @param clumpDistance the maximum distance at which two handles are considered part of
   * the same clump
   */
  explicit NodeHandleManager(const double clumpDistance = 0.01)
    : m_clumpDistance{clumpDistance}
  {
  }

  /**
   * Registers a handle type with this manager.
   *
   * Must be called once per handle type before handles of that type can be added. Calling
   * this more than once for the same type is a contract violation.
   *
   * @tparam HandleType the handle type to register
   */
  template <typename HandleType>
  void registerHandleType()
  {
    const auto inserted = m_handleMaps
                            .emplace(
                              std::type_index{typeid(HandleType)},
                              std::make_unique<HandleMap<HandleType>>(m_clumpDistance))
                            .second;
    contract_assert(inserted);
  }

  /**
   * Returns the number of distinct handles of the given type currently tracked.
   *
   * @tparam HandleType the handle type to query
   * @return the number of handles
   */
  template <typename HandleType>
  size_t handleCount() const
  {
    return getHandleMap<HandleType>().handleCount();
  }

  /**
   * Returns the number of currently selected clumps of the given handle type.
   *
   * Because selection is clump-based, this counts clumps, not individual handles. A
   * clump containing multiple handles counts as one.
   *
   * @tparam HandleType the handle type to query
   * @return the number of selected clumps
   */
  template <typename HandleType>
  size_t selectedHandleCount() const
  {
    return getHandleMap<HandleType>().selectedHandleCount();
  }

  /**
   * Returns whether any handle clump of the given type is selected.
   *
   * @tparam HandleType the handle type to query
   * @return true if at least one clump is selected
   */
  template <typename HandleType>
  bool anyHandleSelected() const
  {
    return getHandleMap<HandleType>().anyHandleSelected();
  }

  /**
   * Returns whether all handles of the given type are selected.
   *
   * @tparam HandleType the handle type to query
   * @return true if all handles are selected
   */
  template <typename HandleType>
  bool allHandlesSelected() const
  {
    return getHandleMap<HandleType>().allHandlesSelected();
  }

  /**
   * Returns whether the given handle is tracked by this manager.
   *
   * @tparam HandleType the handle type to query
   * @param handle the handle to look up
   * @return true if and only if the handle is currently tracked
   */
  template <typename HandleType>
  bool containsHandle(const HandleType& handle) const
  {
    return getHandleMap<HandleType>().containsHandle(handle);
  }

  /**
   * Returns whether the given handle is selected.
   *
   * @tparam HandleType the handle type to query
   * @param handle the handle to look up
   * @return true if and only if the handle exists and is selected
   */
  template <typename HandleType>
  bool isHandleSelected(const HandleType& handle) const
  {
    return getHandleMap<HandleType>().isHandleSelected(handle);
  }

  /**
   * Returns a view over all tracked handles of the given type.
   *
   * @tparam HandleType the handle type to query
   * @return a range of all handles
   */
  template <typename HandleType>
  auto allHandles() const
  {
    return getHandleMap<HandleType>().allHandles();
  }

  /**
   * Returns a view over all selected handles of the given type.
   *
   * A handle is considered selected if its clump is selected.
   *
   * @tparam HandleType the handle type to query
   * @return a range of selected handles
   */
  template <typename HandleType>
  auto selectedHandles() const
  {
    return getHandleMap<HandleType>().selectedHandles();
  }

  /**
   * Returns a view over all unselected handles of the given type.
   *
   * @tparam HandleType the handle type to query
   * @return a range of unselected handles
   */
  template <typename HandleType>
  auto unselectedHandles() const
  {
    return getHandleMap<HandleType>().unselectedHandles();
  }

  /**
   * Adds all handles extracted from each node in the given range.
   *
   * @tparam R a range of `const Node*`
   * @param nodes the nodes whose handles to add
   */
  template <typename HandleType, std::ranges::range R>
  void addHandles(const R& nodes)
  {
    for (const auto* node : nodes)
    {
      addHandles<HandleType>(*node);
    }
  }

  /**
   * Removes all handles extracted from each node in the given range.
   *
   * @tparam R a range of `const Node*`
   * @param nodes the nodes whose handles to remove
   */
  template <typename HandleType, std::ranges::range R>
  void removeHandles(const R& nodes)
  {
    for (const auto* node : nodes)
    {
      removeHandles<HandleType>(*node);
    }
  }

  /**
   * Adds all handles extracted from the given node to every registered handle type.
   *
   * @param node the node whose handles to add
   */
  template <typename HandleType>
  void addHandles(const Node& node)
  {
    getHandleMap<HandleType>().addHandles(node);
  }

  /**
   * Removes all handles extracted from the given node from every registered handle type.
   *
   * @param node the node whose handles to remove
   */
  template <typename HandleType>
  void removeHandles(const Node& node)
  {
    getHandleMap<HandleType>().removeHandles(node);
  }

  /**
   * Selects the clump containing the given handle.
   *
   * All handles in the same clump are selected together. Selecting an already-selected
   * clump has no effect.
   *
   * @tparam HandleType the handle type
   * @param handle a handle whose clump to select
   */
  template <typename HandleType>
  void selectHandle(const HandleType& handle)
  {
    getHandleMap<HandleType>().selectHandle(handle);
  }

  /**
   * Deselects the clump containing the given handle.
   *
   * All handles in the same clump are deselected together. Deselecting an
   * already-deselected clump has no effect.
   *
   * @tparam HandleType the handle type
   * @param handle a handle whose clump to deselect
   */
  template <typename HandleType>
  void deselectHandle(const HandleType& handle)
  {
    getHandleMap<HandleType>().deselectHandle(handle);
  }

  /**
   * Toggles the selection of the clump containing the given handle.
   *
   * @tparam HandleType the handle type
   * @param handle a handle whose clump to toggle
   */
  template <typename HandleType>
  void toggleHandle(const HandleType& handle)
  {
    getHandleMap<HandleType>().toggleHandle(handle);
  }

  /**
   * Selects all handles in the given range.
   *
   * @tparam HandleType the handle type
   * @tparam R a range of HandleType values
   * @param handles the handles to select
   */
  template <typename HandleType, std::ranges::range R>
  void selectHandles(const R& handles)
  {
    auto& handleMap = getHandleMap<HandleType>();
    for (const auto& handle : handles)
    {
      handleMap.selectHandle(handle);
    }
  }

  /**
   * Deselects all handles in the given range.
   *
   * @tparam HandleType the handle type
   * @tparam R a range of HandleType values
   * @param handles the handles to deselect
   */
  template <typename HandleType, std::ranges::range R>
  void deselectHandles(const R& handles)
  {
    auto& handleMap = getHandleMap<HandleType>();
    for (const auto& handle : handles)
    {
      handleMap.deselectHandle(handle);
    }
  }

  /**
   * Toggles all handles in the given range.
   *
   * @tparam HandleType the handle type
   * @tparam R a range of HandleType values
   * @param handles the handles to toggle
   */
  template <typename HandleType, std::ranges::range R>
  void toggleHandles(const R& handles)
  {
    auto& handleMap = getHandleMap<HandleType>();
    for (const auto& handle : handles)
    {
      handleMap.toggleHandle(handle);
    }
  }

  /**
   * Deselects all handles of the given type.
   *
   * @tparam HandleType the handle type
   */
  template <typename HandleType>
  void deselectAllHandles()
  {
    getHandleMap<HandleType>().deselectAllHandles();
  }

  /**
   * Clears all handles of the given type.
   *
   * @tparam HandleType the handle type
   */
  template <typename HandleType>
  void clear()
  {
    getHandleMap<HandleType>().clear();
  }

  /**
   * Performs a pick query for handles of the given type.
   *
   * Forwards all additional arguments to `HandleType::pick` and adds any resulting hits
   * to @p pickResult.
   *
   * @tparam HandleType the handle type to pick against
   * @param pickResult the pick result to add hits to
   * @param args additional arguments forwarded to the handle's pick function
   */
  template <typename HandleType, typename... Args>
  void pick(PickResult& pickResult, Args&&... args) const
  {
    getHandleMap<HandleType>().pick(pickResult, std::forward<Args>(args)...);
  }

private:
  template <typename HandleType>
  const HandleMap<HandleType>& getHandleMap() const
  {
    const auto iHandleMap = m_handleMaps.find(std::type_index{typeid(HandleType)});
    contract_assert(iHandleMap != m_handleMaps.end());
    return static_cast<const HandleMap<HandleType>&>(*iHandleMap->second);
  }

  template <typename HandleType>
  HandleMap<HandleType>& getHandleMap()
  {
    return KDL_CONST_OVERLOAD(getHandleMap<HandleType>());
  }
};

} // namespace tb::mdl
