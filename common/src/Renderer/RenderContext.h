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
#include "Renderer/Transformation.h"

#include <vecmath/bbox.h>

namespace TrenchBroom
{
namespace Renderer
{
class Camera;
class FontManager;
class ShaderManager;

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
  const Camera& m_camera;
  Transformation m_transformation;
  FontManager& m_fontManager;
  ShaderManager& m_shaderManager;

  // settings for any map rendering view
  bool m_showTextures;
  bool m_showFaces;
  bool m_showEdges;
  bool m_shadeFaces;

  bool m_showPointEntities;
  bool m_showPointEntityModels;
  bool m_showEntityClassnames;

  bool m_showGroupBounds;
  bool m_showBrushEntityBounds;
  bool m_showPointEntityBounds;

  bool m_showFog;

  bool m_showGrid;
  FloatType m_gridSize;
  float m_dpiScale;

  bool m_hideSelection;
  bool m_tintSelection;

  ShowSelectionGuide m_showSelectionGuide;
  vm::bbox3f m_sofMapBounds;

public:
  RenderContext(
    RenderMode renderMode,
    const Camera& camera,
    FontManager& fontManager,
    ShaderManager& shaderManager);

  bool render2D() const;
  bool render3D() const;

  const Camera& camera() const;
  Transformation& transformation();
  FontManager& fontManager();
  ShaderManager& shaderManager();

  bool showTextures() const;
  void setShowTextures(bool showTextures);

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

  FloatType gridSize() const;
  void setGridSize(FloatType gridSize);

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

private:
  RenderContext(const RenderContext& other);
  RenderContext& operator=(const RenderContext& other);
};
} // namespace Renderer
} // namespace TrenchBroom
