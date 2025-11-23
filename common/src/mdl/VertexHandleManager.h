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

#include "mdl/BrushNode.h"
#include "mdl/HitType.h"
#include "mdl/PickResult.h"
#include "render/Camera.h"

#include "kd/contracts.h"
#include "kd/map_utils.h"
#include "kd/ranges/to.h"
#include "kd/vector_utils.h"

#include <iterator>
#include <map>
#include <ranges>
#include <vector>

namespace tb
{
namespace render
{
class Camera;
}

namespace mdl
{
class Grid;

class VertexHandleManagerBase
{
public:
  virtual ~VertexHandleManagerBase();

public:
  /**
   * Adds all handles of the the given range of brushes to this handle manager.
   *
   * @tparam R the type of the given range
   * @param handles the handles to add
   */
  template <std::ranges::range R>
  void addHandles(const R& handles)
  {
    for (const auto& handle : handles)
    {
      addHandles(handle);
    }
  }

  /**
   * Adds all handles of the given brush to this handle manager.
   *
   * @param brushNode the brush whose handles to add
   */
  virtual void addHandles(const BrushNode* brushNode) = 0;

  /**
   * Removes all handles of the given range of brushes from this handle manager.
   *
   * @tparam R the type of the given range
   * @param handles the handles to remove
   */
  template <std::ranges::range R>
  void removeHandles(const R& handles)
  {
    for (const auto& handle : handles)
    {
      removeHandles(handle);
    }
  }

  /**
   * Removes all handles of the given brush from this handle manager.
   *
   * @param brushNode the brush whose handles to remove
   */
  virtual void removeHandles(const BrushNode* brushNode) = 0;
};

template <typename H>
class VertexHandleManagerBaseT : public VertexHandleManagerBase
{
public:
  using Handle = H;

protected:
  /**
   * Represents the status of a handle, i.e., how many duplicates exist at the same
   * coordinates and whether or not all of these are selected.
   */
  struct HandleInfo
  {
    size_t count = 0;
    bool selected = false;

    /**
     * Sets this handle to selected.
     *
     * @return true if and only if this handle was not previously selected
     */
    bool select() { return !std::exchange(selected, true); }

    /**
     * Sets this handle to deselected.
     *
     * @return true if and only if this handle was previously selected
     */
    bool deselect() { return std::exchange(selected, false); }

    /**
     * Toggles the selection state of this handle.
     *
     * @return true if and only if this handle was previously selected
     */
    bool toggle() { return std::exchange(selected, !selected); }

    /**
     * Increments the number of handles at the same coordinates.
     */
    void inc() { ++count; }

    /**
     * Deccrements the number of handles at the same coordinates.
     */
    void dec() { --count; }
  };

  /**
   * Maps a handle position to its info.
   */
  std::map<H, HandleInfo> m_handles;

  /**
   * The total number of selected handles, not counting duplicates.
   */
  size_t m_selectedHandleCount = 0;

public:
  ~VertexHandleManagerBaseT() override = default;

public:
  /**
   * Returns the hit type value of the picking hits reported by this manager.
   *
   * @return the hit type value
   */
  virtual HitType::Type hitType() const = 0;

  /**
   * The total number of selected handles, not counting duplicates.
   *
   * @return the total number of selected handles
   */
  size_t selectedHandleCount() const { return m_selectedHandleCount; }

  /**
   * The total number of unselected handles, not counting duplicates.
   *
   * @return the total number of unselected handles
   */
  size_t unselectedHandleCount() const
  {
    return totalHandleCount() - selectedHandleCount();
  }

  /**
   * The total number of handles, selected or not, not counting duplicates.
   *
   * @return the total number of handles
   */
  size_t totalHandleCount() const { return m_handles.size(); }

public:
  /**
   * Returns all handles contained in this manager.
   *
   * @return a list containing all handles
   */
  auto allHandles() const
  {
    auto result = std::vector<Handle>{};
    result.reserve(totalHandleCount());
    collectHandles([](const auto&) { return true; }, std::back_inserter(result));
    return result;
  }

  /**
   * Returns all selected handles contained in this manager.
   *
   * @return a list containing all selected handles
   */
  auto selectedHandles() const
  {
    auto result = std::vector<Handle>{};
    result.reserve(selectedHandleCount());
    collectHandles(
      [](const auto& info) { return info.selected; }, std::back_inserter(result));
    return result;
  }

  /**
   * Returns all unselected handles contained in this manager.
   *
   * @return a list containing all unselected handles
   */
  auto unselectedHandles() const
  {
    auto result = std::vector<Handle>{};
    result.reserve(unselectedHandleCount());
    collectHandles(
      [](const auto& info) { return !info.selected; }, std::back_inserter(result));
    return result;
  }

private:
  template <typename T, typename O>
  void collectHandles(const T& test, O out) const
  {
    for (const auto& [handle, info] : m_handles)
    {
      if (test(info))
      {
        out++ = handle;
      }
    }
  }

public:
  /**
   * Indicates whether the given handle is contained in this manager.
   *
   * @param handle the handle to check
   * @return true if and only if the given handle is contained in this manager
   */
  bool contains(const Handle& handle) const { return m_handles.count(handle) > 0; }

  /**
   * Indicates whether the given handle is selected.
   *
   * @param handle the handle to check
   * @return true if and only if the given handle is contained in this manager and it is
   * selected
   */
  bool selected(const Handle& handle) const
  {
    const auto it = m_handles.find(handle);
    return it != m_handles.end() && it->second.selected;
  }

  /**
   * Indicates whether any handle is currently selected.
   *
   * @return true if and only if any handle is selected
   */
  bool anySelected() const { return selectedHandleCount() > 0; }

  /**
   * Indicates whether all handles are currently selected
   * @return true if and only if all handles are selected
   */
  bool allSelected() const { return selectedHandleCount() == totalHandleCount(); }

public:
  /**
   * Adds the given handle to this manager.
   *
   * @param handle the handle to add
   */
  void add(const Handle& handle)
  {
    m_handles[handle].inc(); // unknown value gets value constructed, which for HandleInfo
                             // means its default constructor is called
  }

  /**
   * Removes the given handle from this manager.
   *
   * @param handle the handle to remove
   * @return true if the given handle was contained in this manager (and therefore
   * removed) and false otherwise
   */
  bool remove(const Handle& handle)
  {
    if (const auto it = m_handles.find(handle); it != m_handles.end())
    {
      auto& info = it->second;
      info.dec();

      if (info.count == 0)
      {
        deselect(info);
        m_handles.erase(it);
      }
      return true;
    }

    return false;
  }

  /**
   * Removes all handles from this manager.
   */
  void clear()
  {
    m_handles.clear();
    m_selectedHandleCount = 0;
  }

  /**
   * Selects the given range of handles.
   *
   * @tparam R the type of the given range
   * @param handles the handles to select
   */
  template <std::ranges::range R>
  void select(const R& handles)
  {
    for (const auto& handle : handles)
    {
      select(handle);
    }
  }

  /**
   * Selects the given handle. If the given handle is not contained in this manager or if
   * it is selected, nothing happens.
   *
   * @param handle the handle to select
   */
  void select(const Handle& handle)
  {
    forEachCloseHandle(handle, [&](auto& info) { select(info); });
  }

  /**
   * Deselects the given range of handles.
   *
   * @tparam R the type of the given range
   * @param handles the handles to deselect
   */
  template <std::ranges::range R>
  void deselect(const R& handles)
  {
    for (const auto& handle : handles)
    {
      deselect(handle);
    }
  }

  /**
   * Deselects the given handle. If the handle is not contained in this manager or if it
   * is not selected, nothing happens.
   *
   * @param handle the handle to deselect
   */
  void deselect(const Handle& handle)
  {
    forEachCloseHandle(handle, [&](auto& info) { deselect(info); });
  }

  /**
   * Deselects all currently selected handles
   */
  void deselectAll()
  {
    for (auto& [handle, info] : m_handles)
    {
      deselect(info);
    }
  }

  /**
   * Toggles the selection of the given range of handles.
   *
   * @tparam R the type of the given range
   * @param handles the handles to toggle
   */
  template <std::ranges::range R>
  void toggle(const R& handles)
  {
    const auto selectionState = handles | std::views::transform([&](const auto& handle) {
                                  return std::pair{handle, selected(handle)};
                                })
                                | kdl::ranges::to<std::map<Handle, bool>>();

    for (const auto& handle : handles)
    {
      if (kdl::map_find_or_default(selectionState, handle, false))
      {
        deselect(handle);
      }
      else
      {
        select(handle);
      }
    }
  }

private:
  template <typename F>
  void forEachCloseHandle(const H& otherHandle, F fun)
  {
    static const auto epsilon = 0.001 * 0.001;
    for (auto& [handle, info] : m_handles)
    {
      if (compare(otherHandle, handle, epsilon) == 0)
      {
        fun(info);
      }
    }
  }

  void select(HandleInfo& info)
  {
    if (info.select())
    {
      contract_assert(selectedHandleCount() < totalHandleCount());

      ++m_selectedHandleCount;
    }
  }

  void deselect(HandleInfo& info)
  {
    if (info.deselect())
    {
      contract_assert(m_selectedHandleCount > 0);

      --m_selectedHandleCount;
    }
  }

  void toggle(HandleInfo& info)
  {
    if (info.toggle())
    {
      contract_assert(selectedHandleCount() < totalHandleCount());

      ++m_selectedHandleCount;
    }
    else
    {
      contract_assert(m_selectedHandleCount > 0);

      --m_selectedHandleCount;
    }
  }

public:
  /**
   * Applies the given picking test to all handles in this manager and adds all hits to
   * the given picking result.
   *
   * @tparam P the type of the picking test, which must be a unary function that maps a
   * handle to a picking hit
   * @param test the picking test to apply
   * @param pickResult the pick result to add hits to
   */
  template <typename P>
  void pick(const P& test, PickResult& pickResult) const
  {
    for (const auto& [handle, info] : m_handles)
    {
      const auto hit = test(handle);
      if (hit.isMatch())
      {
        pickResult.addHit(hit);
      }
    }
  }

public:
  /**
   * Finds and returns all brushes in the given range which are incident to the given
   * handle.
   *
   * @tparam R the type of the given brush range
   * @param handle the handle
   * @param brushes the handles to add
   * @return a set of all brushes that are incident to the given handle
   */
  template <std::ranges::range R>
  std::vector<BrushNode*> findIncidentBrushes(
    const Handle& handle, const R& brushes) const
  {
    auto result = std::vector<BrushNode*>{};
    findIncidentBrushes(handle, brushes, std::back_inserter(result));
    return kdl::vec_sort_and_remove_duplicates(std::move(result));
  }

  /**
   * Finds and returns all brushes in the given range which are incident to any handle
   * in the given range.
   *
   * @tparam HandleRange the type of the range of handles
   * @tparam BrushRange the type of the range of brushes
   * @param handles the range of handles
   * @param brushes the range of brushes
   * @return a vector containing all incident brushes
   */
  template <std::ranges::range HandleRange, std::ranges::range BrushRange>
  std::vector<BrushNode*> findIncidentBrushes(
    const HandleRange& handles, const BrushRange& brushes) const
  {
    auto result = std::vector<BrushNode*>{};
    auto out = std::back_inserter(result);

    for (const auto& handle : handles)
    {
      findIncidentBrushes(handle, brushes, out);
    }
    return kdl::vec_sort_and_remove_duplicates(std::move(result));
  }

  /**
   * Finds all brushes in the given range which are incident to the given handle.
   *
   * @tparam R the type of the given range of brushes
   * @tparam O an output iterator to append the resulting brushes to
   * @param handle the handle
   * @param brushes the range of brushes
   * @param out an output iterator that accepts the incident brushes
   */
  template <std::ranges::range R, typename O>
  void findIncidentBrushes(const Handle& handle, const R& brushes, O out) const
  {
    std::ranges::copy_if(
      brushes, out, [&](const auto& brush) { return isIncident(handle, brush); });
  }

private:
  /**
   * Checks whether the given brush is incident to the given handle.
   *
   * @param handle the handle to check
   * @param brushNode the brush to check
   * @return true if and only if the given brush is incident to the given handle
   */
  virtual bool isIncident(const Handle& handle, const BrushNode* brushNode) const = 0;
};

/**
 * Manages vertex handles. A vertex handle is a 3D point.
 */
class VertexHandleManager : public VertexHandleManagerBaseT<vm::vec3d>
{
public:
  static const HitType::Type HandleHitType;

public:
  using VertexHandleManagerBase::addHandles;
  using VertexHandleManagerBase::removeHandles;

public:
  /**
   * Picks all vertex handles hit by the given picking ray in the context of the given
   * camera, and adds the hits to the given picking result.
   *
   * @param pickRay the picking ray
   * @param camera the camera
   * @param pickResult the picking result to add the hits to
   */
  void pick(
    const vm::ray3d& pickRay, const render::Camera& camera, PickResult& pickResult) const;

public:
  void addHandles(const BrushNode* brushNode) override;
  void removeHandles(const BrushNode* brushNode) override;

  HitType::Type hitType() const override;

private:
  bool isIncident(const Handle& handle, const BrushNode* brushNode) const override;
};

/**
 * Manages edge handles. An edge handle is a line segment given by two points. The edge
 * handles are not directly pickable. Instead of picking the line segment, the manager
 * intersects the picking ray with a sphere around the center point of each edge handle.
 *
 * Additionally, this manager can pick virtual handles. These virtual handles are points
 * where the edge handles intersect with a grid plane. Such handles are not added to
 * this manager explicitly, but are computed on the fly.
 */
class EdgeHandleManager : public VertexHandleManagerBaseT<vm::segment3d>
{
public:
  static const HitType::Type HandleHitType;
  using HitData = std::tuple<vm::segment3d, vm::vec3d>;

public:
  using VertexHandleManagerBase::addHandles;
  using VertexHandleManagerBase::removeHandles;

public:
  /**
   * Picks a virtual handle at any position where an edge handle intersects with any
   * grid plane. These virtual handles are points, but they are computed on the fly from
   * the edge handles contained in this manager.
   *
   * @param pickRay the picking ray
   * @param camera the camera
   * @param grid the current grid
   * @param pickResult the picking result to add the hits to
   */
  void pickGridHandle(
    const vm::ray3d& pickRay,
    const render::Camera& camera,
    const Grid& grid,
    PickResult& pickResult) const;

  /**
   * Picks the center point of the edge handles contained in this manager.
   *
   * @param pickRay the picking ray
   * @param camera the camera
   * @param pickResult the picking result to add the hits to
   */
  void pickCenterHandle(
    const vm::ray3d& pickRay, const render::Camera& camera, PickResult& pickResult) const;

public:
  void addHandles(const BrushNode* brushNode) override;
  void removeHandles(const BrushNode* brushNode) override;

  HitType::Type hitType() const override;

private:
  bool isIncident(const Handle& handle, const BrushNode* brushNode) const override;
};

/**
 * Manages face handles. A face handle is a polygon given its vertices. The face handles
 * are not directly pickable. Instead of picking the polygon, the manager intersects the
 * picking ray with a sphere around the center point of each face handle.
 *
 * Additionally, this manager can pick virtual handles. These virtual handles are points
 * where the face handles intersect with two grid planes. Such handles are not added to
 * this manager explicitly, but are computed on the fly.
 */
class FaceHandleManager : public VertexHandleManagerBaseT<vm::polygon3d>
{
public:
  static const HitType::Type HandleHitType;
  using HitData = std::tuple<vm::polygon3d, vm::vec3d>;

public:
  using VertexHandleManagerBase::addHandles;
  using VertexHandleManagerBase::removeHandles;

public:
  /**
   * Picks a virtual handle at any position where a face handle intersects with any two
   * grid planes. These virtual handles are points, but they are computed on the fly
   * from the face handles contained in this manager.
   *
   * @param pickRay the picking ray
   * @param camera the camera
   * @param grid the current grid
   * @param pickResult the picking result to add the hits to
   */
  void pickGridHandle(
    const vm::ray3d& pickRay,
    const render::Camera& camera,
    const Grid& grid,
    PickResult& pickResult) const;

  /**
   * Picks the center point of the face handles contained in this manager.
   *
   * @param pickRay the picking ray
   * @param camera the camera
   * @param pickResult the picking result to add the hits to
   */
  void pickCenterHandle(
    const vm::ray3d& pickRay, const render::Camera& camera, PickResult& pickResult) const;

public:
  void addHandles(const BrushNode* brushNode) override;
  void removeHandles(const BrushNode* brushNode) override;

  HitType::Type hitType() const override;

private:
  bool isIncident(const Handle& handle, const BrushNode* brushNode) const override;
};

} // namespace mdl
} // namespace tb
