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

#pragma once

#include "FloatType.h"
#include "Model/BrushFace.h"
#include "Model/BrushNode.h"
#include "Model/HitType.h"
#include "Model/PickResult.h"
#include "Renderer/Camera.h"

#include <kdl/vector_set.h>

#include <vecmath/segment.h>

#include <iterator>
#include <map>
#include <vector>

namespace TrenchBroom {
namespace Renderer {
class Camera;
}

namespace View {
class Grid;

class VertexHandleManagerBase {
public:
  virtual ~VertexHandleManagerBase();

public:
  /**
   * Adds all handles of the the given range of brushes to this handle manager.
   *
   * @tparam I the type of the given range iterators
   * @param cur the beginning of the range
   * @param end the end of the range
   */
  template <typename I> void addHandles(I cur, I end) {
    while (cur != end) {
      addHandles(*cur);
      ++cur;
    }
  }

  /**
   * Adds all handles of the given brush to this handle manager.
   *
   * @param brushNode the brush whose handles to add
   */
  virtual void addHandles(const Model::BrushNode* brushNode) = 0;

  /**
   * Removes all handles of the given range of brushes from this handle manager.
   *
   * @tparam I the type of the range iterators
   * @param cur the beginning of the range
   * @param end the end of the range
   */
  template <typename I> void removeHandles(I cur, I end) {
    while (cur != end) {
      removeHandles(*cur);
      ++cur;
    }
  }

  /**
   * Removes all handles of the given brush from this handle manager.
   *
   * @param brushNode the brush whose handles to remove
   */
  virtual void removeHandles(const Model::BrushNode* brushNode) = 0;
};

template <typename H> class VertexHandleManagerBaseT : public VertexHandleManagerBase {
public:
  using Handle = H;
  using HandleList = std::vector<H>;

private:
protected:
  /**
   * Represents the status of a handle, i.e., how many duplicates exist at the same coordinates and
   * whether or not all of these are selected.
   */
  struct HandleInfo {
    size_t count;
    bool selected;

    HandleInfo()
      : count(0)
      , selected(false) {}

    /**
     * Sets this handle to selected.
     *
     * @return true if and only if this handle was not previously selected
     */
    bool select() {
      const bool result = !selected;
      selected = true;
      return result;
    }

    /**
     * Sets this handle to deselected.
     *
     * @return true if and only if this handle was previously selected
     */
    bool deselect() {
      const bool result = selected;
      selected = false;
      return result;
    }

    /**
     * Toggles the selection state of this handle.
     *
     * @return true if and only if this handle was previously selected
     */
    bool toggle() {
      selected = !selected;
      return selected;
    }

    /**
     * Increments the number of handles at the same coordinates.
     */
    void inc() { ++count; }

    /**
     * Deccrements the number of handles at the same coordinates.
     */
    void dec() { --count; }
  };

  using HandleMap = std::map<H, HandleInfo>;
  using HandleEntry = typename HandleMap::value_type;

  /**
   * Maps a handle position to its info.
   */
  HandleMap m_handles;

  /**
   * The total number of selected handles, not counting duplicates.
   */
  size_t m_selectedHandleCount;

public:
  VertexHandleManagerBaseT()
    : m_selectedHandleCount(0) {}

  virtual ~VertexHandleManagerBaseT() {}

public:
  /**
   * Returns the hit type value of the picking hits reported by this manager.
   *
   * @return the hit type value
   */
  virtual Model::HitType::Type hitType() const = 0;

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
  size_t unselectedHandleCount() const { return totalHandleCount() - selectedHandleCount(); }

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
  HandleList allHandles() const {
    HandleList result;
    result.reserve(totalHandleCount());
    collectHandles(
      [](const HandleInfo& /* info */) {
        return true;
      },
      std::back_inserter(result));
    return result;
  }

  /**
   * Returns all selected handles contained in this manager.
   *
   * @return a list containing all selected handles
   */
  HandleList selectedHandles() const {
    HandleList result;
    result.reserve(selectedHandleCount());
    collectHandles(
      [](const HandleInfo& info) {
        return info.selected;
      },
      std::back_inserter(result));
    return result;
  }

  /**
   * Returns all unselected handles contained in this manager.
   *
   * @return a list containing all unselected handles
   */
  HandleList unselectedHandles() const {
    HandleList result;
    result.reserve(unselectedHandleCount());
    collectHandles(
      [](const HandleInfo& info) {
        return !info.selected;
      },
      std::back_inserter(result));
    return result;
  }

private:
  template <typename T, typename O> void collectHandles(const T& test, O out) const {
    for (const auto& [handle, info] : m_handles) {
      if (test(info)) {
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
   * @return true if and only if the given handle is contained in this manager and it is selected
   */
  bool selected(const Handle& handle) const {
    const auto it = m_handles.find(handle);
    if (it == std::end(m_handles))
      return false;
    return it->second.selected;
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
  void add(const Handle& handle) {
    m_handles[handle].inc(); // unknown value gets value constructed, which for HandleInfo means its
                             // default constructor is called
  }

  /**
   * Removes the given handle from this manager.
   *
   * @param handle the handle to remove
   * @return true if the given handle was contained in this manager (and therefore removed) and
   * false otherwise
   */
  bool remove(const Handle& handle) {
    const auto it = m_handles.find(handle);
    if (it != std::end(m_handles)) {
      HandleInfo& info = it->second;
      info.dec();

      if (info.count == 0) {
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
  void clear() {
    m_handles.clear();
    m_selectedHandleCount = 0;
  }

  /**
   * Selects the given range of handles.
   *
   * @tparam I the type of the range iterator
   * @param cur the beginning of the range
   * @param end the end of the range
   */
  template <typename I> void select(I cur, I end) {
    while (cur != end) {
      select(*cur);
      ++cur;
    }
  }

  /**
   * Selects the given handle. If the given handle is not contained in this manager or if it is
   * selected, nothing happens.
   *
   * @param handle the handle to select
   */
  void select(const Handle& handle) {
    forEachCloseHandle(handle, [this](HandleInfo& info) {
      select(info);
    });
  }

  /**
   * Deselects the given range of handles.
   *
   * @tparam I the type of the range iterators
   * @param cur the beginning of the range
   * @param end the end of the range
   */
  template <typename I> void deselect(I cur, I end) {
    while (cur != end) {
      deselect(*cur);
      ++cur;
    }
  }

  /**
   * Deselects the given handle. If the handle is not contained in this manager or if it is not
   * selected, nothing happens.
   *
   * @param handle the handle to deselect
   */
  void deselect(const Handle& handle) {
    forEachCloseHandle(handle, [this](HandleInfo& info) {
      deselect(info);
    });
  }

  /**
   * Deselects all currently selected handles
   */
  void deselectAll() {
    for (auto& [handle, info] : m_handles) {
      deselect(info);
    }
  }

  /**
   * Toggles the selection of the given range of handles.
   *
   * @tparam I the type of the range iterators
   * @param begin the beginning of the range
   * @param end the end of the range
   */
  template <typename I> void toggle(I begin, I end) {
    using SelectionState = std::map<Handle, bool>;
    SelectionState selectionState;

    for (auto cur = begin; cur != end; ++cur) {
      selectionState[*cur] = selected(*cur);
    }

    for (auto cur = begin; cur != end; ++cur) {
      if (selectionState[*cur]) {
        deselect(*cur);
      } else {
        select(*cur);
      }
    }
  }

private:
  template <typename F> void forEachCloseHandle(const H& otherHandle, F fun) {
    static const auto epsilon = 0.001 * 0.001;
    for (auto& [handle, info] : m_handles) {
      if (compare(otherHandle, handle, epsilon) == 0) {
        fun(info);
      }
    }
  }

  void select(HandleInfo& info) {
    if (info.select()) {
      assert(selectedHandleCount() < totalHandleCount());
      ++m_selectedHandleCount;
    }
  }

  void deselect(HandleInfo& info) {
    if (info.deselect()) {
      assert(m_selectedHandleCount > 0);
      --m_selectedHandleCount;
    }
  }

  void toggle(HandleInfo& info) {
    if (info.toggle()) {
      assert(selectedHandleCount() < totalHandleCount());
      ++m_selectedHandleCount;
    } else {
      assert(m_selectedHandleCount > 0);
      --m_selectedHandleCount;
    }
  }

public:
  /**
   * Applies the given picking test to all handles in this manager and adds all hits to the given
   * picking result.
   *
   * @tparam P the type of the picking test, which must be a unary function that maps a handle to a
   * picking hit
   * @param test the picking test to apply
   * @param pickResult the pick result to add hits to
   */
  template <typename P> void pick(const P& test, Model::PickResult& pickResult) const {
    for (const auto& [handle, info] : m_handles) {
      const auto hit = test(handle);
      if (hit.isMatch()) {
        pickResult.addHit(hit);
      }
    }
  }

public:
  /**
   * Finds and returns all brushes in the given range which are incident to the given handle.
   *
   * @tparam I the type of the range iterators
   * @param handle the handle
   * @param begin the beginning of the range of brushes
   * @param end the end of the range of brushes
   * @return a set of all brushes that are incident to the given handle
   */
  template <typename I>
  std::vector<Model::BrushNode*> findIncidentBrushes(const Handle& handle, I begin, I end) const {
    kdl::vector_set<Model::BrushNode*> result;
    findIncidentBrushes(handle, begin, end, std::inserter(result, result.end()));
    return result.release_data();
  }

  /**
   * Finds and returns all brushes in the given range which are incident to any handle in the given
   * range.
   *
   * @tparam I1 the type of range iterators for the range of handles
   * @tparam I2 the type of range iterators for the range of brushes
   * @param hBegin the beginning of the range of handles
   * @param hEnd the end of the range of handles
   * @param bBegin the beginning of the range of brushes
   * @param bEnd the end of the range of brushes
   * @return a set containing all incident brushes
   */
  template <typename I1, typename I2>
  std::vector<Model::BrushNode*> findIncidentBrushes(I1 hBegin, I1 hEnd, I2 bBegin, I2 bEnd) const {
    kdl::vector_set<Model::BrushNode*> result;
    auto out = std::inserter(result, std::end(result));
    for (auto hCur = hBegin; hCur != hEnd; ++hCur) {
      findIncidentBrushes(*hCur, bBegin, bEnd, out);
    }
    return result.release_data();
  }

  /**
   * Finds all brushes in the given range which are incident to the given handle.
   *
   * @tparam I the type of the range iterators
   * @tparam O an output iterator to append the resulting brushes to
   * @param handle the handle
   * @param begin the beginning of the range of brushes
   * @param end the end of the range of brushes
   * @param out an output iterator that accepts the incident brushes
   */
  template <typename I, typename O>
  void findIncidentBrushes(const Handle& handle, I begin, I end, O out) const {
    for (auto cur = begin; cur != end; ++cur) {
      if (isIncident(handle, *cur)) {
        out++ = *cur;
      }
    }
  }

private:
  /**
   * Checks whether the given brush is incident to the given handle.
   *
   * @param handle the handle to check
   * @param brushNode the brush to check
   * @return true if and only if the given brush is incident to the given handle
   */
  virtual bool isIncident(const Handle& handle, const Model::BrushNode* brushNode) const = 0;
};

/**
 * Manages vertex handles. A vertex handle is a 3D point.
 */
class VertexHandleManager : public VertexHandleManagerBaseT<vm::vec3> {
public:
  static const Model::HitType::Type HandleHitType;

public:
  using VertexHandleManagerBase::addHandles;
  using VertexHandleManagerBase::removeHandles;

public:
  /**
   * Picks all vertex handles hit by the given picking ray in the context of the given camera, and
   * adds the hits to the given picking result.
   *
   * @param pickRay the picking ray
   * @param camera the camera
   * @param pickResult the picking result to add the hits to
   */
  void pick(
    const vm::ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) const;

public:
  void addHandles(const Model::BrushNode* brushNode) override;
  void removeHandles(const Model::BrushNode* brushNode) override;

  Model::HitType::Type hitType() const override;

private:
  bool isIncident(const Handle& handle, const Model::BrushNode* brushNode) const override;
};

/**
 * Manages edge handles. An edge handle is a line segment given by two points. The edge handles are
 * not directly pickable. Instead of picking the line segment, the manager intersects the picking
 * ray with a sphere around the center point of each edge handle.
 *
 * Additionally, this manager can pick virtual handles. These virtual handles are points where the
 * edge handles intersect with a grid plane. Such handles are not added to this manager explicitly,
 * but are computed on the fly.
 */
class EdgeHandleManager : public VertexHandleManagerBaseT<vm::segment3> {
public:
  static const Model::HitType::Type HandleHitType;
  using HitType = std::tuple<vm::segment3, vm::vec3>;

public:
  using VertexHandleManagerBase::addHandles;
  using VertexHandleManagerBase::removeHandles;

public:
  /**
   * Picks a virtual handle at any position where an edge handle intersects with any grid plane.
   * These virtual handles are points, but they are computed on the fly from the edge handles
   * contained in this manager.
   *
   * @param pickRay the picking ray
   * @param camera the camera
   * @param grid the current grid
   * @param pickResult the picking result to add the hits to
   */
  void pickGridHandle(
    const vm::ray3& pickRay, const Renderer::Camera& camera, const Grid& grid,
    Model::PickResult& pickResult) const;

  /**
   * Picks the center point of the edge handles contained in this manager.
   *
   * @param pickRay the picking ray
   * @param camera the camera
   * @param pickResult the picking result to add the hits to
   */
  void pickCenterHandle(
    const vm::ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) const;

public:
  void addHandles(const Model::BrushNode* brushNode) override;
  void removeHandles(const Model::BrushNode* brushNode) override;

  Model::HitType::Type hitType() const override;

private:
  bool isIncident(const Handle& handle, const Model::BrushNode* brushNode) const override;
};

/**
 * Manages face handles. A face handle is a polygon given its vertices. The face handles are not
 * directly pickable. Instead of picking the polygon, the manager intersects the picking ray with a
 * sphere around the center point of each face handle.
 *
 * Additionally, this manager can pick virtual handles. These virtual handles are points where the
 * face handles intersect with two grid planes. Such handles are not added to this manager
 * explicitly, but are computed on the fly.
 */
class FaceHandleManager : public VertexHandleManagerBaseT<vm::polygon3> {
public:
  static const Model::HitType::Type HandleHitType;
  using HitType = std::tuple<vm::polygon3, vm::vec3>;

public:
  using VertexHandleManagerBase::addHandles;
  using VertexHandleManagerBase::removeHandles;

public:
  /**
   * Picks a virtual handle at any position where a face handle intersects with any two grid planes.
   * These virtual handles are points, but they are computed on the fly from the face handles
   * contained in this manager.
   *
   * @param pickRay the picking ray
   * @param camera the camera
   * @param grid the current grid
   * @param pickResult the picking result to add the hits to
   */
  void pickGridHandle(
    const vm::ray3& pickRay, const Renderer::Camera& camera, const Grid& grid,
    Model::PickResult& pickResult) const;

  /**
   * Picks the center point of the face handles contained in this manager.
   *
   * @param pickRay the picking ray
   * @param camera the camera
   * @param pickResult the picking result to add the hits to
   */
  void pickCenterHandle(
    const vm::ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) const;

public:
  void addHandles(const Model::BrushNode* brushNode) override;
  void removeHandles(const Model::BrushNode* brushNode) override;

  Model::HitType::Type hitType() const override;

private:
  bool isIncident(const Handle& handle, const Model::BrushNode* brushNode) const override;
};
} // namespace View
} // namespace TrenchBroom
