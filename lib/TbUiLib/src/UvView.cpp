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

#include "ui/UvView.h"

#include "base/PreferenceManager.h"
#include "gl/ActiveShader.h"
#include "gl/Camera.h"
#include "gl/GlInterface.h"
#include "gl/Material.h"
#include "gl/PrimType.h"
#include "gl/Shaders.h"
#include "gl/Texture.h"
#include "gl/VboManager.h"
#include "gl/VertexArray.h"
#include "gl/VertexType.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushFaceHandle.h"
#include "mdl/Grid.h"
#include "mdl/Map.h"
#include "prefs/Preferences.h"
#include "render/EdgeRenderer.h"
#include "render/FaceRenderer.h"
#include "render/RenderBatch.h"
#include "render/RenderContext.h"
#include "render/Renderable.h"
#include "ui/MapDocument.h"
#include "ui/UvCameraTool.h"
#include "ui/UvOffsetTool.h"
#include "ui/UvOriginTool.h"
#include "ui/UvRotateTool.h"
#include "ui/UvScaleTool.h"
#include "ui/UvShearTool.h"

#include "kd/contracts.h"
#include "kd/ranges/to.h"

#include <memory>
#include <ranges>
#include <vector>

namespace tb::ui
{

namespace
{

class RenderMaterial : public render::DirectRenderable
{
private:
  using Vertex = gl::VertexTypes::P3NT2::Vertex;

  const UvViewHelper& m_helper;
  gl::VertexArray m_vertexArray;

public:
  explicit RenderMaterial(const UvViewHelper& helper)
    : m_helper{helper}
    , m_vertexArray{gl::VertexArray::move(getVertices())}
  {
  }

private:
  std::vector<Vertex> getVertices() const
  {
    const auto normal = vm::vec3f{m_helper.face()->boundary().normal};

    const auto& camera = m_helper.camera();
    const auto& v = camera.zoomedViewport();
    const auto w2 = float(v.width) / 2.0f;
    const auto h2 = float(v.height) / 2.0f;

    const auto& p = camera.position();
    const auto& r = camera.right();
    const auto& u = camera.up();

    const auto pos1 = -w2 * r + h2 * u + p;
    const auto pos2 = +w2 * r + h2 * u + p;
    const auto pos3 = +w2 * r - h2 * u + p;
    const auto pos4 = -w2 * r - h2 * u + p;

    return {
      Vertex{pos1, normal, m_helper.face()->uvCoords(vm::vec3d(pos1))},
      Vertex{pos2, normal, m_helper.face()->uvCoords(vm::vec3d(pos2))},
      Vertex{pos3, normal, m_helper.face()->uvCoords(vm::vec3d(pos3))},
      Vertex{pos4, normal, m_helper.face()->uvCoords(vm::vec3d(pos4))},
    };
  }

  void prepare(gl::Gl& gl, gl::VboManager& vboManager) override
  {
    m_vertexArray.prepare(gl, vboManager);
  }

  void render(render::RenderContext& renderContext) override
  {
    auto& gl = renderContext.gl();

    const auto uvAttributes = m_helper.face()->uvAttributes();
    const auto toTex =
      m_helper.face()->toUvCoordSystemMatrix(uvAttributes.offset, uvAttributes.scale);

    const auto* material = m_helper.face()->material();
    contract_assert(material != nullptr);

    const auto* texture = material->texture();
    contract_assert(texture != nullptr);

    auto shader =
      gl::ActiveShader{gl, renderContext.shaderManager(), gl::Shaders::UvViewShader};
    shader.set("ApplyMaterial", true);
    shader.set("Color", texture->averageColor());
    shader.set("Brightness", pref(Preferences::Brightness));
    shader.set("RenderGrid", true);
    shader.set("GridSizes", texture->sizef());
    shader.set("GridAlpha", pref(Preferences::GridAlpha));
    shader.set("DpiScale", renderContext.dpiScale());
    shader.set("GridScales", uvAttributes.scale);
    shader.set("GridMatrix", vm::mat4x4f{toTex});
    shader.set("GridDivider", vm::vec2f{m_helper.subDivisions()});
    shader.set("CameraZoom", m_helper.camera().zoom());
    shader.set("Material", 0);

    if (m_vertexArray.setup(gl, shader.program()))
    {
      material->activate(
        gl, renderContext.minFilterMode(), renderContext.magFilterMode());

      m_vertexArray.render(gl, gl::PrimType::Quads);
      m_vertexArray.cleanup(gl, shader.program());

      material->deactivate(gl);
    }
  }
};

// Chooses a light overlay color on dark textures and a dark one on bright textures, so UV
// view overlays stay visible on any texture (mirrors the adaptive grid). The UV view
// shows a single face, so a per-texture decision from its average color is stable.
RgbaF adaptiveOverlayColor(const gl::Texture* texture, const float alpha)
{
  auto luma = 0.0f;
  if (texture)
  {
    const auto avg = texture->averageColor().to<RgbF>();
    luma = pref(Preferences::Brightness)
           * (0.299f * avg.get<ColorChannel::r>() + 0.587f * avg.get<ColorChannel::g>()
              + 0.114f * avg.get<ColorChannel::b>());
  }
  const auto shade = luma < 0.5f ? 0.9f : 0.1f;
  return RgbaF{shade, shade, shade, alpha};
}

} // namespace

const mdl::HitType::Type UvView::FaceHitType = mdl::HitType::freeType();

UvView::UvView(AppController& appController, MapDocument& document)
  : RenderView{appController}
  , m_document{document}
  , m_helper{m_camera}
{
  setToolBox(m_toolBox);
  createTools();
  m_toolBox.disable();
  connectObservers();
}

void UvView::setSubDivisions(const vm::vec2i& subDivisions)
{
  m_helper.setSubDivisions(subDivisions);
  update();
}

bool UvView::event(QEvent* event)
{
  if (event->type() == QEvent::WindowDeactivate)
  {
    cancelDrag();
  }

  return RenderView::event(event);
}

void UvView::createTools()
{
  addToolController(std::make_unique<UvRotateTool>(m_document, m_helper));
  addToolController(std::make_unique<UvOriginTool>(m_helper));
  addToolController(std::make_unique<UvScaleTool>(m_document, m_helper));
  addToolController(std::make_unique<UvShearTool>(m_document, m_helper));
  addToolController(std::make_unique<UvOffsetTool>(m_document, m_helper));
  addToolController(std::make_unique<UvCameraTool>(m_camera));
}

void UvView::connectObservers()
{
  m_notifierConnection += m_document.documentWasLoadedNotifier.connect([&] { reload(); });
  m_notifierConnection += m_document.documentDidChangeNotifier.connect([&] { reload(); });
  m_notifierConnection +=
    m_document.selectionDidChangeNotifier.connect([&](const auto&) { reload(); });
  m_notifierConnection += m_document.gridDidChangeNotifier.connect([&] { update(); });

  auto& prefs = PreferenceManager::instance();
  m_notifierConnection +=
    prefs.preferenceDidChangeNotifier.connect([&](const auto&) { update(); });

  m_notifierConnection +=
    m_camera.cameraDidChangeNotifier.connect([&](const auto&) { update(); });
}

void UvView::reload()
{
  const auto faces = m_document.map().selection().brushFaces;
  if (faces.size() != 1)
  {
    m_helper.setFaceHandle(std::nullopt);
  }
  else
  {
    m_helper.setFaceHandle(faces.back());
  }

  if (m_helper.valid())
  {
    m_toolBox.enable();
  }
  else
  {
    m_toolBox.disable();
  }

  update();
}

void UvView::updateViewport(int x, int y, int width, int height)
{
  if (m_camera.setViewport({x, y, width, height}))
  {
    m_helper.cameraViewportChanged();
  }
}

void UvView::renderContents(gl::Gl& gl)
{
  if (m_helper.valid())
  {
    auto renderContext = render::RenderContext{
      gl, render::RenderMode::Render2D, m_camera, fontManager(), shaderManager()};
    renderContext.setFilterMode(
      pref(Preferences::TextureMinFilter), pref(Preferences::TextureMagFilter));

    auto renderBatch = render::RenderBatch{vboManager()};
    renderContext.setDpiScale(float(window()->devicePixelRatioF()));

    setupGL(renderContext);
    renderMaterial(renderContext, renderBatch);
    renderFace(renderContext, renderBatch);
    renderToolBox(renderContext, renderBatch);
    renderUvAxes(renderContext, renderBatch);

    renderBatch.render(renderContext);
  }
}

bool UvView::shouldRenderFocusIndicator() const
{
  return false;
}

const Color& UvView::getBackgroundColor()
{
  return pref(Preferences::BrowserBackgroundColor);
}

void UvView::setupGL(render::RenderContext& renderContext)
{
  auto& gl = renderContext.gl();

  const auto& viewport = renderContext.camera().viewport();
  const auto r = devicePixelRatioF();
  const auto x = int(viewport.x * r);
  const auto y = int(viewport.y * r);
  const auto width = int(viewport.width * r);
  const auto height = int(viewport.height * r);

  gl.viewport(x, y, width, height);

  if (pref(Preferences::EnableMSAA))
  {
    gl.enable(GL_MULTISAMPLE);
  }
  else
  {
    gl.disable(GL_MULTISAMPLE);
  }

  gl.enable(GL_BLEND);
  gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  gl.shadeModel(GL_SMOOTH);
  gl.disable(GL_DEPTH_TEST);
}

void UvView::renderMaterial(render::RenderContext&, render::RenderBatch& renderBatch)
{
  if (getTexture(m_helper.face()->material()))
  {
    renderBatch.addOneShot(new RenderMaterial{m_helper});
  }
}

void UvView::renderFace(render::RenderContext&, render::RenderBatch& renderBatch)
{
  using Vertex = gl::VertexTypes::P3::Vertex;

  contract_pre(m_helper.valid());

  auto edgeVertices = m_helper.face()->vertices()
                      | std::views::transform([](const auto* vertex) {
                          return Vertex{vm::vec3f{vertex->position()}};
                        })
                      | kdl::ranges::to<std::vector>();

  auto edgeRenderer = render::DirectEdgeRenderer{
    gl::VertexArray::move(std::move(edgeVertices)), gl::PrimType::LineLoop};

  const auto edgeColor =
    adaptiveOverlayColor(getTexture(m_helper.face()->material()), 1.0f);
  edgeRenderer.renderOnTop(renderBatch, edgeColor, 2.5f);
}

void UvView::renderUvAxes(render::RenderContext&, render::RenderBatch& renderBatch)
{
  using Vertex = gl::VertexTypes::P3C4::Vertex;

  contract_pre(m_helper.valid());

  const auto& normal = m_helper.face()->boundary().normal;
  const auto uAxis = vm::vec3f{
    m_helper.face()->uAxis() - vm::dot(m_helper.face()->uAxis(), normal) * normal};
  const auto vAxis = vm::vec3f{
    m_helper.face()->vAxis() - vm::dot(m_helper.face()->vAxis(), normal) * normal};
  const auto center = vm::vec3f{m_helper.face()->boundsCenter()};

  const auto length = 32.0f / m_helper.camera().zoom();

  auto edgeRenderer = render::DirectEdgeRenderer{
    gl::VertexArray::move(std::vector{
      Vertex{center, pref(Preferences::XAxisColor).to<RgbaF>().toVec()},
      Vertex{center + length * uAxis, pref(Preferences::XAxisColor).to<RgbaF>().toVec()},
      Vertex{center, pref(Preferences::YAxisColor).to<RgbaF>().toVec()},
      Vertex{center + length * vAxis, pref(Preferences::YAxisColor).to<RgbaF>().toVec()},
    }),
    gl::PrimType::Lines};
  edgeRenderer.renderOnTop(renderBatch, 2.0f);
}

void UvView::renderToolBox(
  render::RenderContext& renderContext, render::RenderBatch& renderBatch)
{
  renderTools(renderContext, renderBatch);
}

void UvView::processEvent(const KeyEvent& event)
{
  ToolBoxConnector::processEvent(event);
}

void UvView::processEvent(const MouseEvent& event)
{
  ToolBoxConnector::processEvent(event);
}

void UvView::processEvent(const ScrollEvent& event)
{
  ToolBoxConnector::processEvent(event);
}

void UvView::processEvent(const GestureEvent& event)
{
  ToolBoxConnector::processEvent(event);
}

void UvView::processEvent(const CancelEvent& event)
{
  ToolBoxConnector::processEvent(event);
}

PickRequest UvView::pickRequest(const float x, const float y) const
{
  return PickRequest{vm::ray3d{m_camera.pickRay(x, y)}, m_camera};
}

mdl::PickResult UvView::pick(const vm::ray3d& pickRay) const
{
  auto pickResult = mdl::PickResult::byDistance();
  if (m_helper.valid())
  {
    if (const auto distance = m_helper.face()->intersectWithRay(pickRay))
    {
      const auto hitPoint = vm::point_at_distance(pickRay, *distance);
      pickResult.addHit({UvView::FaceHitType, *distance, hitPoint, m_helper.face()});
    }
  }
  return pickResult;
}

} // namespace tb::ui
