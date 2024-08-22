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

#include "EntityModel.h"

#include "Assets/MaterialCollection.h"
#include "Assets/Texture.h"
#include "Renderer/IndexRangeMap.h"
#include "Renderer/MaterialIndexRangeMap.h"
#include "Renderer/MaterialIndexRangeRenderer.h"
#include "Renderer/PrimType.h"

#include "kdl/reflection_impl.h"
#include "kdl/vector_utils.h"

#include "vm/bbox.h"
#include "vm/forward.h"
#include "vm/intersection.h"

#include <fmt/format.h>

#include <ranges>
#include <string>

namespace TrenchBroom::Assets
{

std::ostream& operator<<(std::ostream& lhs, const PitchType rhs)
{
  switch (rhs)
  {
  case PitchType::Normal:
    lhs << "Normal";
    break;
  case PitchType::MdlInverted:
    lhs << "MdlInverted";
    break;
  }
  return lhs;
}

std::ostream& operator<<(std::ostream& lhs, const Orientation rhs)
{
  switch (rhs)
  {
  case Orientation::ViewPlaneParallelUpright:
    lhs << "ViewPlaneParallelUpright";
    break;
  case Orientation::FacingUpright:
    lhs << "FacingUpright";
    break;
  case Orientation::ViewPlaneParallel:
    lhs << "ViewPlaneParallel";
    break;
  case Orientation::Oriented:
    lhs << "Oriented";
    break;
  case Orientation::ViewPlaneParallelOriented:
    lhs << "ViewPlaneParallelOriented";
    break;
  }
  return lhs;
}


// EntityModelFrame

kdl_reflect_impl(EntityModelFrame);

EntityModelFrame::EntityModelFrame(
  const size_t index, std::string name, const vm::bbox3f& bounds)
  : m_index{index}
  , m_name{std::move(name)}
  , m_bounds{bounds}
  , m_spacialTree{16.0f}
{
}

size_t EntityModelFrame::index() const
{
  return m_index;
}

size_t EntityModelFrame::skinOffset() const
{
  return m_skinOffset;
}

void EntityModelFrame::setSkinOffset(const size_t skinOffset)
{
  m_skinOffset = skinOffset;
}

const std::string& EntityModelFrame::name() const
{
  return m_name;
}

const vm::bbox3f& EntityModelFrame::bounds() const
{
  return m_bounds;
}

std::optional<float> EntityModelFrame::intersect(const vm::ray3f& ray) const
{
  auto closestDistance = std::optional<float>{};

  const auto candidates = m_spacialTree.find_intersectors(ray);
  for (const auto triNum : candidates)
  {
    const auto& p1 = m_tris[triNum * 3 + 0];
    const auto& p2 = m_tris[triNum * 3 + 1];
    const auto& p3 = m_tris[triNum * 3 + 2];
    closestDistance =
      vm::safe_min(closestDistance, vm::intersect_ray_triangle(ray, p1, p2, p3));
  }

  return closestDistance;
}

void EntityModelFrame::addToSpacialTree(
  const std::vector<EntityModelVertex>& vertices,
  const Renderer::PrimType primType,
  const size_t index,
  const size_t count)
{
  switch (primType)
  {
  case Renderer::PrimType::Points:
  case Renderer::PrimType::Lines:
  case Renderer::PrimType::LineStrip:
  case Renderer::PrimType::LineLoop:
    break;
  case Renderer::PrimType::Triangles: {
    assert(count % 3 == 0);
    m_tris.reserve(m_tris.size() + count);
    for (size_t i = 0; i < count; i += 3)
    {
      auto bounds = vm::bbox3f::builder{};
      const auto& p1 = Renderer::getVertexComponent<0>(vertices[index + i + 0]);
      const auto& p2 = Renderer::getVertexComponent<0>(vertices[index + i + 1]);
      const auto& p3 = Renderer::getVertexComponent<0>(vertices[index + i + 2]);
      bounds.add(p1);
      bounds.add(p2);
      bounds.add(p3);

      const auto triIndex = m_tris.size() / 3u;
      m_tris.push_back(p1);
      m_tris.push_back(p2);
      m_tris.push_back(p3);
      m_spacialTree.insert(bounds.bounds(), triIndex);
    }
    break;
  }
  case Renderer::PrimType::Polygon:
  case Renderer::PrimType::TriangleFan: {
    assert(count > 2);
    m_tris.reserve(m_tris.size() + (count - 2) * 3);

    const auto& p1 = Renderer::getVertexComponent<0>(vertices[index]);
    for (size_t i = 1; i < count - 1; ++i)
    {
      auto bounds = vm::bbox3f::builder{};
      const auto& p2 = Renderer::getVertexComponent<0>(vertices[index + i]);
      const auto& p3 = Renderer::getVertexComponent<0>(vertices[index + i + 1]);
      bounds.add(p1);
      bounds.add(p2);
      bounds.add(p3);

      const auto triIndex = m_tris.size() / 3u;
      m_tris.push_back(p1);
      m_tris.push_back(p2);
      m_tris.push_back(p3);
      m_spacialTree.insert(bounds.bounds(), triIndex);
    }
    break;
  }
  case Renderer::PrimType::Quads:
  case Renderer::PrimType::QuadStrip:
  case Renderer::PrimType::TriangleStrip: {
    assert(count > 2);
    m_tris.reserve(m_tris.size() + (count - 2) * 3);
    for (size_t i = 0; i < count - 2; ++i)
    {
      auto bounds = vm::bbox3f::builder{};
      const auto& p1 = Renderer::getVertexComponent<0>(vertices[index + i + 0]);
      const auto& p2 = Renderer::getVertexComponent<0>(vertices[index + i + 1]);
      const auto& p3 = Renderer::getVertexComponent<0>(vertices[index + i + 2]);
      bounds.add(p1);
      bounds.add(p2);
      bounds.add(p3);

      const auto triIndex = m_tris.size() / 3u;
      if (i % 2 == 0)
      {
        m_tris.push_back(p1);
        m_tris.push_back(p2);
        m_tris.push_back(p3);
      }
      else
      {
        m_tris.push_back(p1);
        m_tris.push_back(p3);
        m_tris.push_back(p2);
      }
      m_spacialTree.insert(bounds.bounds(), triIndex);
    }
    break;
  }
    switchDefault();
  }
}

// EntityModelData::Mesh

/**
 * The mesh associated with a frame and a surface.
 */
class EntityModelMesh
{
protected:
  std::vector<EntityModelVertex> m_vertices;

  kdl_reflect_inline_empty(EntityModelMesh);

  /**
   * Creates a new frame mesh that uses the given vertices.
   *
   * @param vertices the vertices
   */
  explicit EntityModelMesh(std::vector<EntityModelVertex> vertices)
    : m_vertices{std::move(vertices)}
  {
  }

public:
  virtual ~EntityModelMesh() = default;

public:
  /**
   * Returns a renderer that renders this mesh with the given material.
   *
   * @param skin the material to use when rendering the mesh
   * @return the renderer
   */
  std::unique_ptr<Renderer::MaterialIndexRangeRenderer> buildRenderer(
    const Material* skin) const
  {
    const auto vertexArray = Renderer::VertexArray::ref(m_vertices);
    return doBuildRenderer(skin, vertexArray);
  }

private:
  /**
   * Creates and returns the actual mesh renderer
   *
   * @param skin the skin to use when rendering the mesh
   * @param vertices the vertices associated with this mesh
   * @return the renderer
   */
  virtual std::unique_ptr<Renderer::MaterialIndexRangeRenderer> doBuildRenderer(
    const Material* skin, const Renderer::VertexArray& vertices) const = 0;
};

// EntityModelData::IndexedMesh

namespace
{

/**
 * A model frame mesh for indexed rendering. Stores vertices and vertex indices.
 */
class EntityModelIndexedMesh : public EntityModelMesh
{
private:
  Renderer::IndexRangeMap m_indices;

  kdl_reflect_inline_empty(EntityModelIndexedMesh);

public:
  /**
   * Creates a new frame mesh with the given vertices and indices.
   *
   * @param frame the frame to which this mesh belongs
   * @param vertices the vertices
   * @param indices the indices
   */
  EntityModelIndexedMesh(
    EntityModelFrame& frame,
    std::vector<EntityModelVertex> vertices,
    Renderer::IndexRangeMap indices)
    : EntityModelMesh{std::move(vertices)}
    , m_indices{std::move(indices)}
  {
    m_indices.forEachPrimitive(
      [&](const Renderer::PrimType primType, const size_t index, const size_t count) {
        frame.addToSpacialTree(m_vertices, primType, index, count);
      });
  }

private:
  std::unique_ptr<Renderer::MaterialIndexRangeRenderer> doBuildRenderer(
    const Material* skin, const Renderer::VertexArray& vertices) const override
  {
    const Renderer::MaterialIndexRangeMap indices(skin, m_indices);
    return std::make_unique<Renderer::MaterialIndexRangeRenderer>(vertices, indices);
  }
};

// EntityModelMaterialMesh

/**
 * A model frame mesh for per material indexed rendering. Stores vertices and per material
 * indices.
 */
class EntityModelMaterialMesh : public EntityModelMesh
{
private:
  Renderer::MaterialIndexRangeMap m_indices;

  kdl_reflect_inline_empty(EntityModelMaterialMesh);

public:
  /**
   * Creates a new frame mesh with the given vertices and per material indices.
   *
   * @param frame the frame to which this mesh belongs
   * @param vertices the vertices
   * @param indices the per material indices
   */
  EntityModelMaterialMesh(
    EntityModelFrame& frame,
    std::vector<EntityModelVertex> vertices,
    Renderer::MaterialIndexRangeMap indices)
    : EntityModelMesh{std::move(vertices)}
    , m_indices{std::move(indices)}
  {
    m_indices.forEachPrimitive([&](
                                 const Material* /* material */,
                                 const Renderer::PrimType primType,
                                 const size_t index,
                                 const size_t count) {
      frame.addToSpacialTree(m_vertices, primType, index, count);
    });
  }

private:
  std::unique_ptr<Renderer::MaterialIndexRangeRenderer> doBuildRenderer(
    const Material* /* skin */, const Renderer::VertexArray& vertices) const override
  {
    return std::make_unique<Renderer::MaterialIndexRangeRenderer>(vertices, m_indices);
  }
};

} // namespace

// EntityModelSurface

kdl_reflect_impl(EntityModelSurface);

EntityModelSurface::EntityModelSurface(std::string name, const size_t frameCount)
  : m_name{std::move(name)}
  , m_meshes{frameCount}
  , m_skins{std::make_unique<MaterialCollection>()}
{
}

EntityModelSurface::~EntityModelSurface() = default;

const std::string& EntityModelSurface::name() const
{
  return m_name;
}

void EntityModelSurface::upload(const bool glContextAvailable)
{
  for (auto& material : m_skins->materials())
  {
    if (auto* texture = material.texture())
    {
      texture->upload(glContextAvailable);
    }
  }
}

void EntityModelSurface::drop(const bool glContextAvailable)
{
  for (auto& material : m_skins->materials())
  {
    if (auto* texture = material.texture())
    {
      texture->drop(glContextAvailable);
    }
  }
}

void EntityModelSurface::addMesh(
  EntityModelFrame& frame,
  std::vector<EntityModelVertex> vertices,
  Renderer::IndexRangeMap indices)
{
  assert(frame.index() < frameCount());
  m_meshes[frame.index()] = std::make_unique<EntityModelIndexedMesh>(
    frame, std::move(vertices), std::move(indices));
}

void EntityModelSurface::addMesh(
  EntityModelFrame& frame,
  std::vector<EntityModelVertex> vertices,
  Renderer::MaterialIndexRangeMap indices)
{
  assert(frame.index() < frameCount());
  m_meshes[frame.index()] = std::make_unique<EntityModelMaterialMesh>(
    frame, std::move(vertices), std::move(indices));
}

void EntityModelSurface::setSkins(std::vector<Material> skins)
{
  m_skins = std::make_unique<MaterialCollection>(std::move(skins));
}

size_t EntityModelSurface::frameCount() const
{
  return m_meshes.size();
}

size_t EntityModelSurface::skinCount() const
{
  return m_skins->materialCount();
}

const Material* EntityModelSurface::skin(const std::string& name) const
{
  return m_skins->materialByName(name);
}

const Material* EntityModelSurface::skin(const size_t index) const
{
  return m_skins->materialByIndex(index);
}

std::unique_ptr<Renderer::MaterialIndexRangeRenderer> EntityModelSurface::buildRenderer(
  const size_t skinIndex, const size_t frameIndex) const
{
  assert(frameIndex < frameCount());
  assert(skinIndex < skinCount());

  return m_meshes[frameIndex] ? m_meshes[frameIndex]->buildRenderer(skin(skinIndex))
                              : nullptr;
}

// EntityModelData

kdl_reflect_impl(EntityModelData);

EntityModelData::EntityModelData(const PitchType pitchType, const Orientation orientation)
  : m_pitchType{pitchType}
  , m_orientation{orientation}
{
}

PitchType EntityModelData::pitchType() const
{
  return m_pitchType;
}

Orientation EntityModelData::orientation() const
{
  return m_orientation;
}

std::unique_ptr<Renderer::MaterialRenderer> EntityModelData::buildRenderer(
  const size_t skinIndex, const size_t frameIndex) const
{
  auto renderers = std::vector<std::unique_ptr<Renderer::MaterialIndexRangeRenderer>>{};
  if (frameIndex >= frameCount())
  {
    return nullptr;
  }

  const auto& frame = this->frame(frameIndex);
  const auto actualSkinIndex = skinIndex + frame->skinOffset();
  for (auto& surface : m_surfaces)
  {
    // If an out of range skin is requested, use the first skin as a fallback
    const auto correctedSkinIndex =
      actualSkinIndex < surface.skinCount() ? actualSkinIndex : 0;
    if (auto renderer = surface.buildRenderer(correctedSkinIndex, frameIndex))
    {
      renderers.push_back(std::move(renderer));
    }
  }
  return !renderers.empty() ? std::make_unique<Renderer::MultiMaterialIndexRangeRenderer>(
           std::move(renderers))
                            : nullptr;
}

vm::bbox3f EntityModelData::bounds(const size_t frameIndex) const
{
  return frameIndex < m_frames.size() ? m_frames[frameIndex].bounds() : vm::bbox3f{8.0f};
}

void EntityModelData::upload(const bool glContextAvailable)
{
  for (auto& surface : m_surfaces)
  {
    surface.upload(glContextAvailable);
  }
}

void EntityModelData::drop(const bool glContextAvailable)
{
  for (auto& surface : m_surfaces)
  {
    surface.drop(glContextAvailable);
  }
}

EntityModelFrame& EntityModelData::addFrame(std::string name, const vm::bbox3f& bounds)
{
  return m_frames.emplace_back(frameCount(), std::move(name), bounds);
}

EntityModelSurface& EntityModelData::addSurface(std::string name, const size_t frameCount)
{
  return m_surfaces.emplace_back(std::move(name), frameCount);
}

size_t EntityModelData::frameCount() const
{
  return m_frames.size();
}

size_t EntityModelData::surfaceCount() const
{
  return m_surfaces.size();
}

const std::vector<EntityModelFrame>& EntityModelData::frames() const
{
  return m_frames;
}

std::vector<EntityModelFrame>& EntityModelData::frames()
{
  return m_frames;
}

const std::vector<EntityModelSurface>& EntityModelData::surfaces() const
{
  return m_surfaces;
}

const EntityModelFrame* EntityModelData::frame(const std::string& name) const
{
  const auto it = std::ranges::find_if(
    m_frames, [&](const auto& frame) { return frame.name() == name; });
  return it != m_frames.end() ? &*it : nullptr;
}

const EntityModelFrame* EntityModelData::frame(const size_t index) const
{
  return index < frameCount() ? &m_frames[index] : nullptr;
}

const EntityModelSurface& EntityModelData::surface(const size_t index) const
{
  if (index >= surfaceCount())
  {
    throw std::out_of_range{"Surface index is out of bounds"};
  }
  return m_surfaces[index];
}

EntityModelSurface& EntityModelData::surface(const size_t index)
{
  return const_cast<EntityModelSurface&>(
    const_cast<const EntityModelData&>(*this).surface(index));
}

const EntityModelSurface* EntityModelData::surface(const std::string& name) const
{
  const auto it = std::ranges::find_if(
    m_surfaces, [&](const auto& surface) { return surface.name() == name; });
  return it != m_surfaces.end() ? &*it : nullptr;
}

kdl_reflect_impl(EntityModel);

EntityModel::EntityModel(
  std::string name, std::shared_ptr<EntityModelDataResource> dataResource)
  : m_name{std::move(name)}
  , m_dataResource{std::move(dataResource)}
{
}

const std::string& EntityModel::name() const
{
  return m_name;
}

const EntityModelData* EntityModel::data() const
{
  return m_dataResource->get();
}

EntityModelData* EntityModel::data()
{
  return m_dataResource->get();
}

const EntityModelDataResource& EntityModel::dataResource() const
{
  return *m_dataResource;
}

} // namespace TrenchBroom::Assets
