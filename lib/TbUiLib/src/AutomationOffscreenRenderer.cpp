/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include "ui/AutomationOffscreenRenderer.h"

#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFramebufferObjectFormat>
#include <QOpenGLFunctions_2_1>
#include <QSurface>
#include <QThread>

#include "base/Color.h"
#include "base/PreferenceManager.h"
#include "gl/GlManager.h"
#include "gl/OrthographicCamera.h"
#include "gl/PerspectiveCamera.h"
#include "gl/ResourceManager.h"
#include "mdl/Grid.h"
#include "mdl/Map.h"
#include "prefs/Preferences.h"
#include "render/GridRenderer.h"
#include "render/MapRenderer.h"
#include "render/RenderBatch.h"
#include "render/RenderContext.h"
#include "ui/AutomationRenderRequest.h"
#include "ui/GlFunctions.h"
#include "ui/GlQt.h"
#include "ui/MapDocument.h"

#include "vm/util.h"

#include <exception>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace tb::ui
{
namespace
{

class CurrentContextGuard
{
private:
  QOpenGLContext& m_context;
  QOpenGLContext* m_previousContext;
  QSurface* m_previousSurface;
  bool m_current = false;

public:
  CurrentContextGuard(QOpenGLContext& context, QOffscreenSurface& surface)
    : m_context{context}
    , m_previousContext{QOpenGLContext::currentContext()}
    , m_previousSurface{m_previousContext != nullptr ? m_previousContext->surface() : nullptr}
    , m_current{context.makeCurrent(&surface)}
  {
  }

  ~CurrentContextGuard()
  {
    if (!m_current)
    {
      return;
    }
    m_context.doneCurrent();
    if (m_previousContext != nullptr && m_previousSurface != nullptr)
    {
      m_previousContext->makeCurrent(m_previousSurface);
    }
  }

  bool current() const { return m_current; }
};

AutomationOffscreenRenderResult failure(
  const AutomationOffscreenRenderError error,
  const QString& message,
  const size_t revision)
{
  return {error, message, {}, std::nullopt, revision};
}

bool validRequest(const automation::AutomationRenderRequest& request)
{
  if (
    !automation::isValidImageSize(request.size) || request.camera.nearPlane <= 0.0
    || request.camera.farPlane <= request.camera.nearPlane
    || vm::is_zero(request.camera.direction, vm::Cd::almost_zero())
    || vm::is_zero(request.camera.up, vm::Cd::almost_zero()))
  {
    return false;
  }
  if (request.camera.projection == automation::AutomationProjection::Perspective)
  {
    return request.camera.verticalFov && *request.camera.verticalFov > 0.0
           && *request.camera.verticalFov < 180.0 && !request.camera.zoom;
  }
  return request.camera.zoom && *request.camera.zoom > 0.0 && !request.camera.verticalFov;
}

void configureContext(
  render::RenderContext& context,
  const mdl::Map& map,
  const automation::AutomationRenderRequest& request)
{
  context.setFilterMode(
    pref(Preferences::TextureMinFilter), pref(Preferences::TextureMagFilter));
  context.setShowMaterials(true);
  context.setShowFaces(true);
  context.setShowEdges(request.overlays.brushEdges);
  context.setShadeFaces(pref(Preferences::ShadeFaces));
  context.setShowPointEntities(pref(Preferences::ShowPointEntities));
  context.setShowPointEntityModels(pref(Preferences::ShowPointEntityModels));
  context.setShowEntityClassnames(pref(Preferences::ShowEntityClassnames));
  context.setShowGroupBounds(pref(Preferences::ShowGroupBounds));
  context.setShowBrushEntityBounds(pref(Preferences::ShowBrushEntityBounds));
  context.setShowPointEntityBounds(pref(Preferences::ShowPointEntityBounds));
  context.setShowFog(pref(Preferences::ShowFog));
  context.setShowGrid(request.overlays.grid);
  context.setGridSize(map.grid().actualSize());
  context.setDpiScale(1.0f);
  context.setSoftMapBounds({});
  if (!request.overlays.selection)
  {
    context.setHideSelection();
  }
}

template <typename Camera>
void render(
  gl::Gl& gl,
  gl::GlManager& glManager,
  MapDocument& document,
  Camera& camera,
  const automation::AutomationRenderRequest& request)
{
  auto renderContext = render::RenderContext{
    gl,
    render::RenderMode::Render3D,
    camera,
    glManager.fontManager(),
    glManager.shaderManager()};
  configureContext(renderContext, document.map(), request);

  gl.viewport(0, 0, request.size.width, request.size.height);
  gl.disable(GL_MULTISAMPLE);
  gl.enable(GL_BLEND);
  gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  gl.shadeModel(GL_SMOOTH);

  auto renderBatch = render::RenderBatch{glManager.vboManager()};
  if constexpr (std::is_same_v<Camera, gl::OrthographicCamera>)
  {
    if (request.overlays.grid)
    {
      renderBatch.addOneShot(
        new render::GridRenderer{camera, document.map().worldBounds()});
    }
  }
  document.mapRenderer().render(renderContext, renderBatch);
  renderBatch.render(renderContext);
}

template <typename Camera>
std::optional<automation::AutomationDepthImage> readDepth(
  QOpenGLFunctions_2_1& functions,
  const automation::AutomationRenderRequest& request,
  const Camera&)
{
  if (!request.outputs.depth)
  {
    return std::nullopt;
  }

  const auto width = request.size.width;
  const auto height = request.size.height;
  auto normalizedDepth = std::vector<float>(static_cast<size_t>(width * height));
  auto previousPackAlignment = GLint{};
  functions.glGetIntegerv(GL_PACK_ALIGNMENT, &previousPackAlignment);
  functions.glPixelStorei(GL_PACK_ALIGNMENT, 1);
  functions.glReadPixels(
    0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, normalizedDepth.data());
  functions.glPixelStorei(GL_PACK_ALIGNMENT, previousPackAlignment);
  if (functions.glGetError() != GL_NO_ERROR)
  {
    throw std::runtime_error{"Could not read the automation depth buffer"};
  }

  auto result = automation::AutomationDepthImage{
    request.size, std::vector<float>(static_cast<size_t>(width * height))};
  const auto nearPlane = static_cast<float>(request.camera.nearPlane);
  const auto farPlane = static_cast<float>(request.camera.farPlane);
  for (auto y = 0; y < height; ++y)
  {
    for (auto x = 0; x < width; ++x)
    {
      const auto sourceIndex = static_cast<size_t>((height - 1 - y) * width + x);
      const auto destinationIndex = static_cast<size_t>(y * width + x);
      const auto depth = normalizedDepth[sourceIndex];
      if (depth >= 1.0f)
      {
        result.values[destinationIndex] = std::numeric_limits<float>::infinity();
      }
      else if constexpr (std::is_same_v<Camera, gl::PerspectiveCamera>)
      {
        const auto clipDepth = 2.0f * depth - 1.0f;
        result.values[destinationIndex] =
          2.0f * nearPlane * farPlane
          / (farPlane + nearPlane - clipDepth * (farPlane - nearPlane));
      }
      else
      {
        result.values[destinationIndex] = nearPlane + depth * (farPlane - nearPlane);
      }
    }
  }
  return result;
}

struct FramebufferOutput
{
  QImage image;
  std::optional<automation::AutomationDepthImage> depth;
};

template <typename Camera>
FramebufferOutput renderToFramebuffer(
  gl::Gl& gl,
  QOpenGLFunctions_2_1& functions,
  gl::GlManager& glManager,
  MapDocument& document,
  Camera& camera,
  const automation::AutomationRenderRequest& request,
  QOpenGLFramebufferObject& framebuffer)
{
  framebuffer.bind();
  const auto background = pref(Preferences::BackgroundColor).to<RgbaF>();
  gl.clearColor(
    background.get<ColorChannel::r>(),
    background.get<ColorChannel::g>(),
    background.get<ColorChannel::b>(),
    1.0f);
  gl.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  render(gl, glManager, document, camera, request);
  const auto image = framebuffer.toImage(true);
  const auto depth = readDepth(functions, request, camera);
  framebuffer.release();
  return {image, std::move(depth)};
}

template <typename Camera>
void configureCamera(Camera& camera, const automation::AutomationRenderRequest& request)
{
  camera.setNearPlane(static_cast<float>(request.camera.nearPlane));
  camera.setFarPlane(static_cast<float>(request.camera.farPlane));
  camera.setViewport({0, 0, request.size.width, request.size.height});
  camera.moveTo(vm::vec3f{request.camera.position});
  camera.setDirection(vm::vec3f{request.camera.direction}, vm::vec3f{request.camera.up});
}

} // namespace

AutomationOffscreenRenderer::AutomationOffscreenRenderer(
  QOpenGLContext& context, QOffscreenSurface& surface, gl::GlManager& glManager)
  : m_context{context}
  , m_surface{surface}
  , m_glManager{glManager}
{
}

AutomationOffscreenRenderResult AutomationOffscreenRenderer::capture(
  MapDocument& document, const automation::AutomationRenderRequest& request)
{
  const auto revision = document.map().modificationCount();
  if (!validRequest(request))
  {
    return failure(
      AutomationOffscreenRenderError::InvalidRequest, "Invalid render request", revision);
  }
  if (!m_context.isValid() || !m_surface.isValid())
  {
    return failure(
      AutomationOffscreenRenderError::ContextUnavailable,
      "Automation offscreen context or surface is invalid",
      revision);
  }
  if (m_context.thread() != QThread::currentThread())
  {
    return failure(
      AutomationOffscreenRenderError::ContextUnavailable,
      "Automation offscreen context must be used from its owning Qt thread",
      revision);
  }

  auto currentContext = CurrentContextGuard{m_context, m_surface};
  if (!currentContext.current())
  {
    return failure(
      AutomationOffscreenRenderError::ContextUnavailable,
      "Could not make the automation offscreen context current",
      revision);
  }

  try
  {
    auto& functions = getGlFunctions("AutomationOffscreenRenderer::capture", &m_context);
    auto gl = GlQt{functions};
    if (!m_glManager.initialized())
    {
      m_glManager.initialize(gl);
    }
    if (m_glManager.resourceManager().needsProcessing())
    {
      return failure(
        AutomationOffscreenRenderError::ResourceNotReady,
        "OpenGL resources are pending processing",
        revision);
    }

    auto format = QOpenGLFramebufferObjectFormat{};
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    format.setSamples(0);
    auto framebuffer =
      QOpenGLFramebufferObject{request.size.width, request.size.height, format};
    if (!framebuffer.isValid())
    {
      return failure(
        AutomationOffscreenRenderError::FramebufferUnavailable,
        "Could not create the automation framebuffer",
        revision);
    }

    auto output = FramebufferOutput{};
    if (request.camera.projection == automation::AutomationProjection::Perspective)
    {
      auto camera = gl::PerspectiveCamera{};
      configureCamera(camera, request);
      camera.setFov(static_cast<float>(*request.camera.verticalFov));
      output = renderToFramebuffer(
        gl, functions, m_glManager, document, camera, request, framebuffer);
    }
    else
    {
      auto camera = gl::OrthographicCamera{};
      configureCamera(camera, request);
      camera.setZoom(static_cast<float>(*request.camera.zoom));
      output = renderToFramebuffer(
        gl, functions, m_glManager, document, camera, request, framebuffer);
    }

    if (document.map().modificationCount() != revision)
    {
      return failure(
        AutomationOffscreenRenderError::DocumentChanged,
        "Document changed while rendering",
        revision);
    }
    if (output.image.isNull())
    {
      return failure(
        AutomationOffscreenRenderError::RenderFailed,
        "Automation framebuffer readback failed",
        revision);
    }
    return {
      AutomationOffscreenRenderError::None,
      {},
      std::move(output.image),
      std::move(output.depth),
      revision};
  }
  catch (const std::exception& error)
  {
    return failure(
      AutomationOffscreenRenderError::RenderFailed,
      QString::fromUtf8(error.what()),
      revision);
  }
}

} // namespace tb::ui
