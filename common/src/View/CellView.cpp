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

#include "CellView.h"

#include <QDrag>
#include <QMimeData>
#include <QPropertyAnimation>
#include <QScrollBar>
#include <QToolTip>

#include "PreferenceManager.h"
#include "Preferences.h"
#include "View/CellLayout.h"
#include "View/RenderView.h"

#include <algorithm>

namespace TrenchBroom::View
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
    const auto x = float(event->localPos().x());
    const auto y = float(event->localPos().y() + top);
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
    const auto x = float(event->localPos().x());
    const auto y = float(event->localPos().y() + top);
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

void CellView::doRender()
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
void CellView::processEvent(const CancelEvent& /* event */) {}

} // namespace TrenchBroom::View
