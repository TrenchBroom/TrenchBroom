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

#include "Macros.h"
#include "gl/GL.h"
#include "render/Transformation.h"

#include "vm/bbox.h"

namespace tb
{
namespace gl
{
class Camera;
class FontManager;
class ShaderManager;
} // namespace gl

namespace render
{

enum class RenderMode
{
  Render3D,
  Render2D
};

class RenderContext
{
private:
  enum class ShowSelectionGuide
  {
    Show,
    Hide,
    ForceShow,
    ForceHide
  };

  // general context for any rendering view
  RenderMode m_renderMode;
  const gl::Camera& m_camera;
  Transformation m_transformation;
  gl::FontManager& m_fontManager;
  gl::ShaderManager& m_shaderManager;

  int m_textureMinFilter = GL_NEAREST_MIPMAP_NEAREST;
  int m_textureMagFilter = GL_NEAREST;

  // settings for any map rendering view
  bool m_showMaterials = true;
  bool m_showFaces = true;
  bool m_showEdges = true;
  bool m_shadeFaces = true;

  bool m_showPointEntities = true;
  bool m_showPointEntityModels = true;
  bool m_showEntityClassnames = true;

  bool m_showGroupBounds = true;
  bool m_showBrushEntityBounds = true;
  bool m_showPointEntityBounds = true;

  bool m_showFog = false;

  bool m_showGrid = true;
  double m_gridSize = 4;
  float m_dpiScale = 1.0;

  bool m_hideSelection = false;
  bool m_tintSelection = false;

  ShowSelectionGuide m_showSelectionGuide = ShowSelectionGuide::Hide;
  vm::bbox3f m_softMapBounds;

public:
  RenderContext(
    RenderMode renderMode,
    const gl::Camera& camera,
    gl::FontManager& fontManager,
    gl::ShaderManager& shaderManager);

  deleteCopyAndMove(RenderContext);

  bool render2D() const;
  bool render3D() const;

  const gl::Camera& camera() const;
  Transformation& transformation();
  gl::FontManager& fontManager();
  gl::ShaderManager& shaderManager();

  int minFilterMode() const;
  int magFilterMode() const;
  void setFilterMode(int minFilter, int magFilter);

  bool showMaterials() const;
  void setShowMaterials(bool showMaterials);

  bool showFaces() const;
  void setShowFaces(bool showFaces);

  bool showEdges() const;
  void setShowEdges(bool showEdges);

  bool shadeFaces() const;
  void setShadeFaces(bool shadeFaces);

  bool showPointEntities() const;
  void setShowPointEntities(bool showPointEntities);

  bool showPointEntityModels() const;
  void setShowPointEntityModels(bool showPointEntityModels);

  bool showEntityClassnames() const;
  void setShowEntityClassnames(bool showEntityClassnames);

  bool showGroupBounds() const;
  void setShowGroupBounds(bool showGroupBounds);

  bool showBrushEntityBounds() const;
  void setShowBrushEntityBounds(bool showBrushEntityBounds);

  bool showPointEntityBounds() const;
  void setShowPointEntityBounds(bool showPointEntityBounds);

  bool showFog() const;
  void setShowFog(bool showFog);

  bool showGrid() const;
  void setShowGrid(bool showGrid);

  const vm::bbox3f& softMapBounds() const;
  void setSoftMapBounds(const vm::bbox3f& softMapBounds);

  double gridSize() const;
  void setGridSize(double gridSize);

  float dpiScale() const;
  void setDpiScale(float dpiScale);

  bool hideSelection() const;
  void setHideSelection();

  bool tintSelection() const;
  void clearTintSelection();

  bool showSelectionGuide() const;
  void setShowSelectionGuide();
  void setHideSelectionGuide();
  void setForceShowSelectionGuide();
  void setForceHideSelectionGuide();

private:
  void setShowSelectionGuide(ShowSelectionGuide showSelectionGuide);
};

} // namespace render
} // namespace tb
