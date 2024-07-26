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

#include "Assets/EntityModel_Forward.h"
#include "octree.h"

#include "kdl/reflection_decl.h"

#include "vm/bbox.h"
#include "vm/forward.h"

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace TrenchBroom
{
template <typename T, typename U>
class octree;
}

namespace TrenchBroom::Renderer
{
enum class PrimType;
class IndexRangeMap;
class MaterialIndexRangeMap;
class MaterialIndexRangeRenderer;
class MaterialRenderer;
} // namespace TrenchBroom::Renderer

namespace TrenchBroom::Assets
{
class Material;
class MaterialCollection;

enum class PitchType
{
  Normal,
  MdlInverted
};

std::ostream& operator<<(std::ostream& lhs, PitchType rhs);

/**
 * Controls the orientation of an entity model.
 *
 * See
 * https://github.com/ericwa/Quakespasm/blob/7e7e13f9335697f8e94d1631fdf60ecdddb7498f/quakespasm/Quake/r_sprite.c#L82
 */
enum class Orientation
{
  /** Faces view plane, up is towards the heavens. */
  ViewPlaneParallelUpright,
  /** Faces camera origin, up is towards the heavens. */
  FacingUpright,
  /** Faces view plane, up is towards the top of the screen. */
  ViewPlaneParallel,
  /** Pitch yaw roll are independent of camera. */
  Oriented,
  /** Faces view plane, but obeys roll value. */
  ViewPlaneParallelOriented,
};

std::ostream& operator<<(std::ostream& lhs, Orientation rhs);

/**
 * One frame of the model. Since frames are loaded on demand, each frame has two possible
 * states: loaded and unloaded. These states are modeled as subclasses of this class.
 */
class EntityModelFrame
{
private:
  size_t m_index;
  std::string m_name;
  vm::bbox3f m_bounds;
  size_t m_skinOffset = 0;

  // For hit testing
  std::vector<vm::vec3f> m_tris;
  using TriNum = size_t;
  using SpacialTree = octree<float, TriNum>;
  SpacialTree m_spacialTree;

  kdl_reflect_decl(EntityModelFrame, m_index, m_name, m_bounds, m_skinOffset);

public:
  /**
   * Creates a new frame with the given index.
   *
   * @param index the index of this frame
   */
  explicit EntityModelFrame(size_t index, std::string name, const vm::bbox3f& bounds);

  /**
   * Returns the index of this frame.
   *
   * @return the index
   */
  size_t index() const;

  /**
   * Returns the skin offset of this frame.
   */
  size_t skinOffset() const;

  /**
   * Sets the skin offset of this frame
   */
  void setSkinOffset(size_t skinOffset);

  /**
   * Returns this frame's name.
   *
   * @return the name
   */
  const std::string& name() const;

  /**
   * Returns this frame's bounding box.
   *
   * @return the bounding box
   */
  const vm::bbox3f& bounds() const;

  /**
   * Intersects this frame with the given ray and returns the point of intersection.
   *
   * @param ray the ray to intersect
   * @return the distance to the point of intersection or nullopt if the given ray does
   * not intersect this frame
   */
  std::optional<float> intersect(const vm::ray3f& ray) const;

  /**
   * Adds the given primitives to the spacial tree for this frame.
   *
   * @param vertices the vertices
   * @param primType the primitive type
   * @param index the index of the first primitive's first vertex in the given vertex
   * array
   * @param count the number of vertices that make up the primitive(s)
   */
  void addToSpacialTree(
    const std::vector<EntityModelVertex>& vertices,
    Renderer::PrimType primType,
    size_t index,
    size_t count);
};

class EntityModelMesh;

/**
 * A model surface represents an individual part of a model. MDL and MD2 models use only
 * one surface, while more complex model formats such as MD3 contain multiple surfaces
 * with one skin per surface.
 *
 * Each surface contains per frame meshes. The number of per frame meshes should match the
 * number of frames in the model.
 */
class EntityModelSurface
{
private:
  std::string m_name;
  std::vector<std::unique_ptr<EntityModelMesh>> m_meshes;
  std::unique_ptr<MaterialCollection> m_skins;

  kdl_reflect_decl(EntityModelSurface, m_name, m_meshes, m_skins);

public:
  /**
   * Creates a new surface with the given name.
   *
   * @param name the surface's name
   * @param frameCount the number of frames
   */
  EntityModelSurface(std::string name, size_t frameCount);

  moveOnly(EntityModelSurface);

  ~EntityModelSurface();

  /**
   * Returns the name of this surface.
   *
   * @return the name of this surface
   */
  const std::string& name() const;

  /**
   * Prepares the skin materials of this surface for rendering.
   *
   * @param minFilter the minification filter (GL_TEXTURE_MIN_FILTER)
   * @param magFilter the magnification filter (GL_TEXTURE_MIN_FILTER)
   */
  void prepare(int minFilter, int magFilter);

  /**
   * Sets the minification and magnification filters for the skin materials of this
   * surface.
   *
   * @param minFilter the minification filter (GL_TEXTURE_MIN_FILTER)
   * @param magFilter the magnification filter (GL_TEXTURE_MIN_FILTER)
   */
  void setFilterMode(int minFilter, int magFilter);

  /**
   * Adds a new mesh to this surface.
   *
   * @param frame the frame which the mesh belongs to
   * @param vertices the mesh vertices
   * @param indices the vertex indices
   */
  void addMesh(
    EntityModelFrame& frame,
    std::vector<EntityModelVertex> vertices,
    Renderer::IndexRangeMap indices);

  /**
   * Adds a new material mesh to this surface.
   *
   * @param frame the frame which the mesh belongs to
   * @param vertices the mesh vertices
   * @param indices the per material vertex indices
   */
  void addMesh(
    EntityModelFrame& frame,
    std::vector<EntityModelVertex> vertices,
    Renderer::MaterialIndexRangeMap indices);

  /**
   * Sets the given materials as skins to this surface.
   *
   * @param skins the materials to set
   */
  void setSkins(std::vector<Material> skins);

  /**
   * Returns the number of frame meshes in this surface, should match the model's frame
   * count.
   *
   * @return the number of frame meshes
   */
  size_t frameCount() const;

  /**
   * Returns the number of skins of this surface.
   *
   * @return the number of skins
   */
  size_t skinCount() const;

  /**
   * Returns the skin with the given name.
   *
   * @param name the name of the skin to find
   * @return the skin with the given name, or null if no such skin was found
   */
  const Material* skin(const std::string& name) const;

  /**
   * Returns the skin with the given index.
   *
   * @param index the index of the skin to find
   * @return the skin with the given index, or null if the index is out of bounds
   */
  const Material* skin(size_t index) const;

  std::unique_ptr<Renderer::MaterialIndexRangeRenderer> buildRenderer(
    size_t skinIndex, size_t frameIndex) const;
};

/**
 * Manages all data necessary to render an entity model. Each model can have multiple
 * frames, and multiple surfaces. Each surface represents an independent mesh of
 * primitives such as triangles, and the corresponding materials. Every surface has a
 * separate mesh for each frame of the model.
 */
class EntityModelData
{
private:
  PitchType m_pitchType;
  Orientation m_orientation;
  std::vector<EntityModelFrame> m_frames;
  std::vector<EntityModelSurface> m_surfaces;
  bool m_prepared = false;

  kdl_reflect_decl(
    EntityModelData, m_pitchType, m_orientation, m_frames, m_surfaces, m_prepared);

public:
  EntityModelData(PitchType pitchType, Orientation orientation);

  /**
   * Returns this model's pitch type. The pitch type controls how a rotational
   * transformation matrix can be computed from an entity that uses this model.
   */
  PitchType pitchType() const;

  /**
   * Returns this model's orientation. The orientation controls how the model is oriented
   * in space depending on the camera position.
   */
  Orientation orientation() const;

  /**
   * Creates a renderer to render the given frame of the model using the skin with the
   * given index.
   *
   * @param skinIndex the index of the skin to use
   * @param frameIndex the index of the frame to render
   * @return the renderer
   */
  std::unique_ptr<Renderer::MaterialRenderer> buildRenderer(
    size_t skinIndex, size_t frameIndex) const;

  /**
   * Returns the bounds of the given frame of this model.
   *
   * @param frameIndex the index of the frame
   * @return the bounds of the frame
   */
  vm::bbox3f bounds(size_t frameIndex) const;

  /**
   * Indicates whether or not this model has been prepared for rendering.
   *
   * @return true if this model has been prepared and false otherwise
   */
  bool prepared() const;

  /**
   * Prepares this model for rendering by uploading its skin materials.
   *
   * @param minFilter the minification filter (GL_TEXTURE_MIN_FILTER)
   * @param magFilter the magnification filter (GL_TEXTURE_MIN_FILTER)
   */
  void prepare(int minFilter, int magFilter);

  /**
   * Sets the minification and magnification filters for the skin materials of this model.
   *
   * @param minFilter the minification filter (GL_TEXTURE_MIN_FILTER)
   * @param magFilter the magnification filter (GL_TEXTURE_MIN_FILTER)
   */
  void setFilterMode(int minFilter, int magFilter);

  /**
   * Adds a frame with the given name and bounds.
   *
   * @param name the frame name
   * @param bounds the frame bounds
   * @return the newly added frame
   */
  EntityModelFrame& addFrame(std::string name, const vm::bbox3f& bounds);

  /**
   * Adds a surface with the given name.
   *
   * @param name the surface name
   * @param frameCount the number of frames
   * @return the newly added surface
   */
  EntityModelSurface& addSurface(std::string name, size_t frameCount);

  /**
   * Returns the number of frames of this model.
   *
   * @return the number of frames
   */
  size_t frameCount() const;

  /**
   * Returns the number of surfaces of this model.
   *
   * @return the number of surfaces
   */
  size_t surfaceCount() const;

  /**
   * Returns all frames of this model.
   *
   * @return the frames
   */
  const std::vector<EntityModelFrame>& frames() const;

  /**
   * Returns all frames of this model.
   *
   * @return the frames
   */
  std::vector<EntityModelFrame>& frames();

  /**
   * Returns all surfaces of this model.
   *
   * @return the surfaces
   */
  const std::vector<EntityModelSurface>& surfaces() const;

  /**
   * Returns the frame with the given name.
   *
   * @param name the name of the frame to find
   * @return the frame with the given name or null if no such frame was found
   */
  const EntityModelFrame* frame(const std::string& name) const;

  /**
   * Returns the frame with the given index.
   *
   * @param index the index of the frame
   * @return the frame with the given index or null if the index is out of bounds
   */
  const EntityModelFrame* frame(size_t index) const;

  /**
   * Returns the surface with the given index.
   *
   * @param index the index of the surface to return
   * @return the surface with the given index
   * @throw std::out_of_range if the given index is out of bounds
   */
  const EntityModelSurface& surface(size_t index) const;

  /**
   * Returns the surface with the given index.
   *
   * @param index the index of the surface to return
   * @return the surface with the given index
   * @throw std::out_of_range if the given index is out of bounds
   */
  EntityModelSurface& surface(size_t index);

  /**
   * Returns the surface with the given name.
   *
   * @param name the name of the surface to find
   * @return the surface with the given name or null if no such surface was found
   */
  const EntityModelSurface* surface(const std::string& name) const;
};

class EntityModel
{
private:
  std::string m_name;
  EntityModelData m_data;

  kdl_reflect_decl(EntityModel, m_name, m_data);

public:
  EntityModel(std::string name, EntityModelData data);

  /**
   * Returns the name of this model.
   */
  const std::string& name() const;

  const EntityModelData* data() const;
  EntityModelData* data();
};

} // namespace TrenchBroom::Assets
