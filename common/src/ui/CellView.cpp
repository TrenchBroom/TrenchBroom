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

#include "CellView.h"

#include <QDrag>
#include <QMimeData>
#include <QPropertyAnimation>
#include <QScrollBar>
#include <QToolTip>

#include "PreferenceManager.h"
#include "Preferences.h"
#include "render/ActiveShader.h"
#include "render/FontDescriptor.h"
#include "render/FontManager.h"
#include "render/GLVertexType.h"
#include "render/PrimType.h"
#include "render/Shaders.h"
#include "render/TextureFont.h"
#include "render/Transformation.h"
#include "render/VertexArray.h"
#include "ui/CellLayout.h"
#include "ui/RenderView.h"

#include "kdl/skip_iterator.h"

#include "vm/mat_ext.h"

#include <algorithm>
#include <map>

namespace tb::ui
{

void CellView::updateScrollBar()
{
  if (m_scrollBar)
  {
    const auto thumbSize = size().height();
    const auto range = int(m_layout.height());
    m_scrollBar->setMinimum(0);
    m_scrollBar->setMaximum(std::max(range - thumbSize, 0));
    m_scrollBar->setPageStep(thumbSize);
    m_scrollBar->setSingleStep(int(m_layout.minCellHeight()));
  }
}

void CellView::initLayout()
{
  doInitLayout(m_layout);
  m_layoutInitialized = true;
}

void CellView::reloadLayout()
{
  initLayout(); // always initialize the layout when reloading

  m_layout.clear();
  doReloadLayout(m_layout);
  updateScrollBar();

  m_valid = true;
}

void CellView::validate()
{
  if (!m_valid)
  {
    reloadLayout();
  }
}

CellView::CellView(GLContextManager& contextManager, QScrollBar* scrollBar)
  : RenderView{contextManager}
  , m_scrollBar{scrollBar}
{
  if (m_scrollBar)
  {
    connect(
      m_scrollBar,
      &QAbstractSlider::actionTriggered,
      this,
      &CellView::onScrollBarActionTriggered);
    connect(
      m_scrollBar,
      &QAbstractSlider::valueChanged,
      this,
      &CellView::onScrollBarValueChanged);
  }
}

void CellView::invalidate()
{
  m_valid = false;
}

void CellView::clear()
{
  m_layout.clear();
  doClear();
  m_valid = true;
}

void CellView::resizeEvent(QResizeEvent* event)
{
  validate();
  m_layout.setWidth(float(size().width()));
  updateScrollBar();

  RenderView::resizeEvent(event);
}

void CellView::scrollToCellInternal(const Cell& cell)
{
  const auto visibleRect = this->visibleRect();
  const auto top = int(cell.cellBounds().top());
  const auto bottom = int(cell.cellBounds().bottom());

  if (top >= visibleRect.top() && bottom <= visibleRect.bottom())
  {
    return;
  }

  const auto rowMargin = int(m_layout.rowMargin());
  const auto newPosition = top < visibleRect.top()
                             ? top - rowMargin
                             : visibleRect.top() + bottom - visibleRect.bottom();

  auto* animation = new QPropertyAnimation{m_scrollBar, "sliderPosition"};
  animation->setDuration(300);
  animation->setEasingCurve(QEasingCurve::InOutQuad);
  animation->setStartValue(m_scrollBar->sliderPosition());
  animation->setEndValue(newPosition);
  animation->start();
}

void CellView::onScrollBarValueChanged()
{
  update();
}

/**
 * QAbstractSlider::actionTriggered listener. Overrides the default movement increments
 * for the scrollbar up/down /page up/page down arrows.
 */
void CellView::onScrollBarActionTriggered(int action)
{
  validate();
  const auto top = float(m_scrollBar->value());
  const auto height = float(size().height());

  // NOTE: We call setSliderPosition(), not setValue()
  // see: https://doc.qt.io/archives/qt-4.8/qabstractslider.html#actionTriggered
  switch (action)
  {
  case QAbstractSlider::SliderSingleStepAdd:
    m_scrollBar->setSliderPosition(int(m_layout.rowPosition(top, 1))); // line down
    break;
  case QAbstractSlider::SliderSingleStepSub:
    m_scrollBar->setSliderPosition(int(m_layout.rowPosition(top, -1))); // line up
    break;
  case QAbstractSlider::SliderPageStepAdd:
    m_scrollBar->setSliderPosition(
      int(m_layout.rowPosition(top + height, 0))); // page down
    break;
  case QAbstractSlider::SliderPageStepSub:
    m_scrollBar->setSliderPosition(int(m_layout.rowPosition(top - height, 0))); // page up
    break;
  default:
    break;
  }
}

// CellView

void CellView::mousePressEvent(QMouseEvent* event)
{
  validate();
  if (event->button() == Qt::LeftButton)
  {
    m_potentialDrag = true;
  }
  else if (event->button() == Qt::RightButton)
  {
    if (event->modifiers() & Qt::AltModifier)
    {
      m_lastMousePos = event->pos();
    }
  }
}

void CellView::mouseReleaseEvent(QMouseEvent* event)
{
  validate();
  if (event->button() == Qt::LeftButton)
  {
    const auto top = m_scrollBar ? m_scrollBar->value() : 0;
    const auto x = float(event->position().x());
    const auto y = float(event->position().y() + top);
    doLeftClick(m_layout, x, y);
  }
}

void CellView::mouseMoveEvent(QMouseEvent* event)
{
  validate();
  if (event->buttons() & Qt::LeftButton)
  {
    if (m_potentialDrag)
    {
      startDrag(event);
      m_potentialDrag = false;
    }
  }
  else if ((event->buttons() & Qt::RightButton) && (event->modifiers() & Qt::AltModifier))
  {
    scroll(event);
  }

  m_lastMousePos = event->pos();
}

void CellView::wheelEvent(QWheelEvent* event)
{
  const auto pixelDelta = event->pixelDelta();
  const auto angleDelta = event->angleDelta();

  if (!pixelDelta.isNull())
  {
    scrollBy(pixelDelta.y());
  }
  else if (!angleDelta.isNull())
  {
    scrollBy(angleDelta.y());
  }
  event->accept();
}

bool CellView::event(QEvent* event)
{
  if (event->type() == QEvent::ToolTip)
  {
    return updateTooltip(static_cast<QHelpEvent*>(event));
  }
  return QWidget::event(event);
}

void CellView::contextMenuEvent(QContextMenuEvent* event)
{
  validate();
  const auto top = m_scrollBar ? m_scrollBar->value() : 0;
  const auto x = float(event->pos().x());
  const auto y = float(event->pos().y() + top);
  doContextMenu(m_layout, x, y, event);
}

void CellView::startDrag(const QMouseEvent* event)
{
  validate();
  if (dndEnabled())
  {
    const auto top = m_scrollBar ? m_scrollBar->value() : 0;
    const auto x = float(event->position().x());
    const auto y = float(event->position().y() + top);
    if (const Cell* cell = m_layout.cellAt(x, y))
    {
      /*
       wxImage* feedbackImage = dndImage(*cell);
       int xOffset = event.GetX() - int(cell->itemBounds().left());
       int yOffset = event.GetY() - int(cell->itemBounds().top()) + top;
       */

      const auto dropData = dndData(*cell);

      auto* mimeData = new QMimeData{};
      mimeData->setText(dropData);

      auto* drag = new QDrag{this};
      drag->setMimeData(mimeData);

      drag->exec(Qt::CopyAction);
    }
  }
}

void CellView::scroll(const QMouseEvent* event)
{
  const auto mousePosition = event->pos();
  const auto delta = mousePosition.y() - m_lastMousePos.y();

  scrollBy(delta);
}

void CellView::scrollBy(const int deltaY)
{
  validate();
  if (m_scrollBar)
  {
    const auto newThumbPosition = m_scrollBar->value() - deltaY;
    m_scrollBar->setValue(newThumbPosition);
    update();
  }
}

bool CellView::updateTooltip(QHelpEvent* event)
{
  validate();
  const auto top = m_scrollBar ? m_scrollBar->value() : 0;
  const auto x = float(event->pos().x());
  const auto y = float(event->pos().y() + top);

  // see: https://doc.qt.io/qt-5/qtwidgets-widgets-tooltips-example.html
  if (const auto* cell = m_layout.cellAt(x, y))
  {
    QToolTip::showText(event->globalPos(), tooltip(*cell));
  }
  else
  {
    QToolTip::hideText();
    event->ignore();
  }
  return true;
}

QRect CellView::visibleRect() const
{
  const auto top = m_scrollBar ? m_scrollBar->value() : 0;
  return QRect{QPoint{0, top}, size()};
}

void CellView::renderContents()
{
  validate();
  if (!m_layoutInitialized)
  {
    initLayout();
  }

  const auto r = devicePixelRatioF();
  const auto viewportWidth = int(width() * r);
  const auto viewportHeight = int(height() * r);
  glAssert(glViewport(0, 0, viewportWidth, viewportHeight));

  setupGL();

  // NOTE: These are in points, while the glViewport call above is
  // in pixels
  const auto visibleRect = this->visibleRect();

  const auto y = float(visibleRect.y());
  const auto h = float(visibleRect.height());

  doRender(m_layout, y, h);

  const auto viewLeft = float(0);
  const auto viewTop = float(size().height());
  const auto viewRight = float(size().width());
  const auto viewBottom = float(0);

  const auto transformation = render::Transformation{
    vm::ortho_matrix(-1.0f, 1.0f, viewLeft, viewTop, viewRight, viewBottom),
    vm::view_matrix(vm::vec3f{0, 0, -1}, vm::vec3f{0, 1, 0})
      * vm::translation_matrix(vm::vec3f{0.0f, 0.0f, 0.1f})};

  glAssert(glDisable(GL_DEPTH_TEST));
  glAssert(glFrontFace(GL_CCW));
  renderTitleBackgrounds(y, h);
  renderTitleStrings(y, h);
}

void CellView::setupGL()
{
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
  glAssert(glEnable(GL_CULL_FACE));
  glAssert(glEnable(GL_DEPTH_TEST));
  glAssert(glDepthFunc(GL_LEQUAL));
  glAssert(glShadeModel(GL_SMOOTH));
}

void CellView::renderTitleBackgrounds(float y, float height)
{
  using Vertex = render::GLVertexTypes::P2::Vertex;
  auto vertices = std::vector<Vertex>{};

  for (const auto& group : m_layout.groups())
  {
    if (group.intersectsY(y, height) && !group.title().empty())
    {
      const auto titleBounds = m_layout.titleBoundsForVisibleRect(group, y, height);
      vertices.emplace_back(
        vm::vec2f{titleBounds.left(), height - (titleBounds.top() - y)});
      vertices.emplace_back(
        vm::vec2f{titleBounds.left(), height - (titleBounds.bottom() - y)});
      vertices.emplace_back(
        vm::vec2f{titleBounds.right(), height - (titleBounds.bottom() - y)});
      vertices.emplace_back(
        vm::vec2f{titleBounds.right(), height - (titleBounds.top() - y)});
    }
  }

  auto shader =
    render::ActiveShader{shaderManager(), render::Shaders::VaryingPUniformCShader};
  shader.set("Color", pref(Preferences::BrowserGroupBackgroundColor));

  auto vertexArray = render::VertexArray::move(std::move(vertices));
  vertexArray.prepare(vboManager());
  vertexArray.render(render::PrimType::Quads);
}

namespace
{

auto collectStringVertices(
  CellLayout& layout, const float y, const float height, render::FontManager& fontManager)
{
  using TextVertex = render::GLVertexTypes::P2UV2C4::Vertex;

  auto defaultFont = render::FontDescriptor{
    pref(Preferences::RendererFontPath()), size_t(pref(Preferences::BrowserFontSize))};

  const auto textColor = std::vector<Color>{pref(Preferences::BrowserTextColor)};
  const auto subTextColor = std::vector<Color>{pref(Preferences::BrowserSubTextColor)};

  auto stringVertices = std::map<render::FontDescriptor, std::vector<TextVertex>>{};
  for (const auto& group : layout.groups())
  {
    if (group.intersectsY(y, height))
    {
      const auto& groupTitle = group.title();
      if (!groupTitle.empty())
      {
        const auto titleBounds = layout.titleBoundsForVisibleRect(group, y, height);
        const auto offset = vm::vec2f(
          titleBounds.left() + 2.0f,
          height - (titleBounds.top() - y) - titleBounds.height);

        auto& font = fontManager.font(defaultFont);
        const auto quads = font.quads(groupTitle, false, offset);
        const auto titleVertices = TextVertex::toList(
          quads.size() / 2,
          kdl::skip_iterator{std::begin(quads), std::end(quads), 0, 2},
          kdl::skip_iterator{std::begin(quads), std::end(quads), 1, 2},
          kdl::skip_iterator{std::begin(textColor), std::end(textColor), 0, 0});
        auto& vertices = stringVertices[defaultFont];
        vertices.insert(
          std::end(vertices), std::begin(titleVertices), std::end(titleVertices));
      }

      for (const auto& row : group.rows())
      {
        if (row.intersectsY(y, height))
        {
          for (const auto& cell : row.cells())
          {
            const auto& title = cell.title();
            const auto bounds = cell.titleBounds();
            const auto fontDescriptor =
              fontManager.selectFontSize(defaultFont, title, bounds.width, 6);
            const auto& font = fontManager.font(fontDescriptor);
            const auto size = font.measure(title);

            const auto x =
              bounds.left() + std::max((bounds.width - size.x()) / 2.0f, 0.0f);

            // y is relative to top, but OpenGL coords are relative to bottom, so invert
            const auto yOffset = vm::vec2f{x, y + height - bounds.bottom()};

            const auto quads = font.quads(title, false, yOffset);
            const auto vertices = TextVertex::toList(
              quads.size() / 2,
              kdl::skip_iterator{std::begin(quads), std::end(quads), 0, 2},
              kdl::skip_iterator{std::begin(quads), std::end(quads), 1, 2},
              kdl::skip_iterator{std::begin(textColor), std::end(textColor), 0, 0});

            stringVertices[fontDescriptor] =
              kdl::vec_concat(std::move(stringVertices[fontDescriptor]), vertices);
          }
        }
      }
    }
  }

  return stringVertices;
}

} // namespace

void CellView::renderTitleStrings(float y, float height)
{
  using StringRendererMap = std::map<render::FontDescriptor, render::VertexArray>;
  auto stringRenderers = StringRendererMap{};

  for (const auto& [descriptor, vertices] :
       collectStringVertices(m_layout, y, height, fontManager()))
  {
    stringRenderers[descriptor] = render::VertexArray::ref(vertices);
    stringRenderers[descriptor].prepare(vboManager());
  }

  auto shader = render::ActiveShader{shaderManager(), render::Shaders::ColoredTextShader};
  shader.set("Texture", 0);

  for (auto& [descriptor, vertexArray] : stringRenderers)
  {
    auto& font = fontManager().font(descriptor);
    font.activate();
    vertexArray.render(render::PrimType::Quads);
    font.deactivate();
  }
}

void CellView::doClear() {}
void CellView::doLeftClick(Layout&, float, float) {}
void CellView::doContextMenu(Layout&, float, float, QContextMenuEvent*) {}

bool CellView::dndEnabled()
{
  return false;
}

QPixmap CellView::dndImage(const Cell&)
{
  assert(false);
  return QPixmap{};
}

QString CellView::dndData(const Cell&)
{
  assert(false);
  return "";
}

QString CellView::tooltip(const Cell&)
{
  return "";
}

void CellView::processEvent(const KeyEvent& /* event */) {}
void CellView::processEvent(const MouseEvent& /* event */) {}
void CellView::processEvent(const ScrollEvent& /* event */) {}
void CellView::processEvent(const GestureEvent& /* event */) {}
void CellView::processEvent(const CancelEvent& /* event */) {}

} // namespace tb::ui
