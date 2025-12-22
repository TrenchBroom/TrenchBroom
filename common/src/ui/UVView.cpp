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

#include "UVView.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "gl/ActiveShader.h"
#include "gl/Material.h"
#include "gl/Shaders.h"
#include "gl/Texture.h"
#include "gl/VboManager.h"
#include "gl/VertexType.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushFaceHandle.h"
#include "mdl/Grid.h"
#include "mdl/Map.h"
#include "render/Camera.h"
#include "render/EdgeRenderer.h"
#include "render/FaceRenderer.h"
#include "render/PrimType.h"
#include "render/RenderBatch.h"
#include "render/RenderContext.h"
#include "render/RenderUtils.h"
#include "render/Renderable.h"
#include "render/VertexArray.h"
#include "ui/MapDocument.h"
#include "ui/UVCameraTool.h"
#include "ui/UVOffsetTool.h"
#include "ui/UVOriginTool.h"
#include "ui/UVRotateTool.h"
#include "ui/UVScaleTool.h"
#include "ui/UVShearTool.h"

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

  const UVViewHelper& m_helper;
  render::VertexArray m_vertexArray;

public:
  explicit RenderMaterial(const UVViewHelper& helper)
    : m_helper{helper}
    , m_vertexArray{render::VertexArray::move(getVertices())}
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

private:
  void doPrepareVertices(gl::VboManager& vboManager) override
  {
    m_vertexArray.prepare(vboManager);
  }

  void doRender(render::RenderContext& renderContext) override
  {
    const auto& offset = m_helper.face()->attributes().offset();
    const auto& scale = m_helper.face()->attributes().scale();
    const auto toTex = m_helper.face()->toUVCoordSystemMatrix(offset, scale, true);

    const auto* material = m_helper.face()->material();
    contract_assert(material != nullptr);

    const auto* texture = material->texture();
    contract_assert(texture != nullptr);

    material->activate(renderContext.minFilterMode(), renderContext.magFilterMode());

    auto shader =
      gl::ActiveShader{renderContext.shaderManager(), gl::Shaders::UVViewShader};
    shader.set("ApplyMaterial", true);
    shader.set("Color", texture->averageColor());
    shader.set("Brightness", pref(Preferences::Brightness));
    shader.set("RenderGrid", true);
    shader.set("GridSizes", texture->sizef());
    shader.set("GridColor", vm::vec4f{render::gridColorForMaterial(material), 0.6f});
    shader.set("DpiScale", renderContext.dpiScale());
    shader.set("GridScales", scale);
    shader.set("GridMatrix", vm::mat4x4f{toTex});
    shader.set("GridDivider", vm::vec2f{m_helper.subDivisions()});
    shader.set("CameraZoom", m_helper.cameraZoom());
    shader.set("Material", 0);

    m_vertexArray.render(render::PrimType::Quads);

    material->deactivate();
  }
};

} // namespace

const mdl::HitType::Type UVView::FaceHitType = mdl::HitType::freeType();

UVView::UVView(MapDocument& document, GLContextManager& contextManager)
  : RenderView{contextManager}
  , m_document{document}
  , m_helper{m_camera}
{
  setToolBox(m_toolBox);
  createTools();
  m_toolBox.disable();
  connectObservers();
}

void UVView::setSubDivisions(const vm::vec2i& subDivisions)
{
  m_helper.setSubDivisions(subDivisions);
  update();
}

bool UVView::event(QEvent* event)
{
  if (event->type() == QEvent::WindowDeactivate)
  {
    cancelDrag();
  }

  return RenderView::event(event);
}

void UVView::createTools()
{
  addToolController(std::make_unique<UVRotateTool>(m_document, m_helper));
  addToolController(std::make_unique<UVOriginTool>(m_helper));
  addToolController(std::make_unique<UVScaleTool>(m_document, m_helper));
  addToolController(std::make_unique<UVShearTool>(m_document, m_helper));
  addToolController(std::make_unique<UVOffsetTool>(m_document, m_helper));
  addToolController(std::make_unique<UVCameraTool>(m_camera));
}

void UVView::connectObservers()
{
  auto& map = m_document.map();

  m_notifierConnection +=
    m_document.documentWasLoadedNotifier.connect(this, &UVView::documentDidChange);
  m_notifierConnection +=
    m_document.documentDidChangeNotifier.connect(this, &UVView::documentDidChange);
  m_notifierConnection +=
    map.grid().gridDidChangeNotifier.connect(this, &UVView::gridDidChange);

  auto& prefs = PreferenceManager::instance();
  m_notifierConnection +=
    prefs.preferenceDidChangeNotifier.connect(this, &UVView::preferenceDidChange);

  m_notifierConnection +=
    m_camera.cameraDidChangeNotifier.connect(this, &UVView::cameraDidChange);
}

void UVView::documentDidChange()
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

void UVView::gridDidChange()
{
  update();
}

void UVView::preferenceDidChange(const std::filesystem::path&)
{
  update();
}

void UVView::cameraDidChange(const render::Camera*)
{
  update();
}

void UVView::updateViewport(int x, int y, int width, int height)
{
  if (m_camera.setViewport({x, y, width, height}))
  {
    m_helper.cameraViewportChanged();
  }
}

void UVView::renderContents()
{
  if (m_helper.valid())
  {
    auto renderContext = render::RenderContext{
      render::RenderMode::Render2D, m_camera, fontManager(), shaderManager()};
    renderContext.setFilterMode(
      pref(Preferences::TextureMinFilter), pref(Preferences::TextureMagFilter));

    auto renderBatch = render::RenderBatch{vboManager()};
    renderContext.setDpiScale(float(window()->devicePixelRatioF()));

    setupGL(renderContext);
    renderMaterial(renderContext, renderBatch);
    renderFace(renderContext, renderBatch);
    renderToolBox(renderContext, renderBatch);
    renderUVAxes(renderContext, renderBatch);

    renderBatch.render(renderContext);
  }
}

bool UVView::shouldRenderFocusIndicator() const
{
  return false;
}

const Color& UVView::getBackgroundColor()
{
  return pref(Preferences::BrowserBackgroundColor);
}

void UVView::setupGL(render::RenderContext& renderContext)
{
  const auto& viewport = renderContext.camera().viewport();
  const auto r = devicePixelRatioF();
  const auto x = int(viewport.x * r);
  const auto y = int(viewport.y * r);
  const auto width = int(viewport.width * r);
  const auto height = int(viewport.height * r);

  glAssert(glViewport(x, y, width, height));

  if (pref(Preferences::EnableMSAA))
  {
    glAssert(glEnable(GL_MULTISAMPLE));
  }
  else
  {
    glAssert(glDisable(GL_MULTISAMPLE));
  }

  glAssert(glEnable(GL_BLEND));
  glAssert(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
  glAssert(glShadeModel(GL_SMOOTH));
  glAssert(glDisable(GL_DEPTH_TEST));
}

void UVView::renderMaterial(render::RenderContext&, render::RenderBatch& renderBatch)
{
  if (getTexture(m_helper.face()->material()))
  {
    renderBatch.addOneShot(new RenderMaterial{m_helper});
  }
}

void UVView::renderFace(render::RenderContext&, render::RenderBatch& renderBatch)
{
  using Vertex = gl::VertexTypes::P3::Vertex;

  contract_pre(m_helper.valid());

  auto edgeVertices = m_helper.face()->vertices()
                      | std::views::transform([](const auto* vertex) {
                          return Vertex{vm::vec3f{vertex->position()}};
                        })
                      | kdl::ranges::to<std::vector>();

  auto edgeRenderer = render::DirectEdgeRenderer{
    render::VertexArray::move(std::move(edgeVertices)), render::PrimType::LineLoop};

  const auto edgeColor = RgbaF{1.0f, 1.0f, 1.0f, 1.0f};
  edgeRenderer.renderOnTop(renderBatch, edgeColor, 2.5f);
}

void UVView::renderUVAxes(render::RenderContext&, render::RenderBatch& renderBatch)
{
  using Vertex = gl::VertexTypes::P3C4::Vertex;

  contract_pre(m_helper.valid());

  const auto& normal = m_helper.face()->boundary().normal;
  const auto uAxis = vm::vec3f{
    m_helper.face()->uAxis() - vm::dot(m_helper.face()->uAxis(), normal) * normal};
  const auto vAxis = vm::vec3f{
    m_helper.face()->vAxis() - vm::dot(m_helper.face()->vAxis(), normal) * normal};
  const auto center = vm::vec3f{m_helper.face()->boundsCenter()};

  const auto length = 32.0f / m_helper.cameraZoom();

  auto edgeRenderer = render::DirectEdgeRenderer{
    render::VertexArray::move(std::vector{
      Vertex{center, pref(Preferences::XAxisColor).to<RgbaF>().toVec()},
      Vertex{center + length * uAxis, pref(Preferences::XAxisColor).to<RgbaF>().toVec()},
      Vertex{center, pref(Preferences::YAxisColor).to<RgbaF>().toVec()},
      Vertex{center + length * vAxis, pref(Preferences::YAxisColor).to<RgbaF>().toVec()},
    }),
    render::PrimType::Lines};
  edgeRenderer.renderOnTop(renderBatch, 2.0f);
}

void UVView::renderToolBox(
  render::RenderContext& renderContext, render::RenderBatch& renderBatch)
{
  renderTools(renderContext, renderBatch);
}

void UVView::processEvent(const KeyEvent& event)
{
  ToolBoxConnector::processEvent(event);
}

void UVView::processEvent(const MouseEvent& event)
{
  ToolBoxConnector::processEvent(event);
}

void UVView::processEvent(const ScrollEvent& event)
{
  ToolBoxConnector::processEvent(event);
}

void UVView::processEvent(const GestureEvent& event)
{
  ToolBoxConnector::processEvent(event);
}

void UVView::processEvent(const CancelEvent& event)
{
  ToolBoxConnector::processEvent(event);
}

PickRequest UVView::pickRequest(const float x, const float y) const
{
  return PickRequest{vm::ray3d{m_camera.pickRay(x, y)}, m_camera};
}

mdl::PickResult UVView::pick(const vm::ray3d& pickRay) const
{
  auto pickResult = mdl::PickResult::byDistance();
  if (m_helper.valid())
  {
    if (const auto distance = m_helper.face()->intersectWithRay(pickRay))
    {
      const auto hitPoint = vm::point_at_distance(pickRay, *distance);
      pickResult.addHit({UVView::FaceHitType, *distance, hitPoint, m_helper.face()});
    }
  }
  return pickResult;
}

} // namespace tb::ui
