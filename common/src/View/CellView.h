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

#ifndef TrenchBroom_CellView_h
#define TrenchBroom_CellView_h

#include "Renderer/RenderUtils.h"
#include "Renderer/Transformation.h"
#include "Renderer/FontDescriptor.h"
#include "Preferences.h"
#include "View/CellLayout.h"
#include "View/RenderView.h"

#include <QScrollBar>

#include <algorithm>

namespace TrenchBroom {
    namespace View {
        class GLContextManager;

        class CellView : public RenderView {
        protected:
            using Layout = CellLayout;
            using Group = LayoutGroup;
            using Row = LayoutRow;
            using Cell = LayoutCell;
        private:
            Layout m_layout;
            Cell* m_selectedCell;
            bool m_layoutInitialized;
            
            bool m_valid;

            QScrollBar* m_scrollBar;
            QPoint m_lastMousePos;
            bool m_potentialDrag;

            void updateScrollBar() {
                if (m_scrollBar != nullptr) {
                    //int position = m_scrollBar->value(); // unused
                    int thumbSize = size().height();
                    int range = static_cast<int>(m_layout.height());
                    m_scrollBar->setRange(0, range);
                    //m_scrollBar->setValue(position); // no-op
                    m_scrollBar->setPageStep(thumbSize);
                }
            }

            void initLayout() {
                doInitLayout(m_layout);
                m_layoutInitialized = true;
            }

            void reloadLayout() {
                initLayout(); // always initialize the layout when reloading

                m_layout.clear();
                doReloadLayout(m_layout);
                updateScrollBar();

                m_valid = true;
            }
            
            void validate() {
                if (!m_valid)
                    reloadLayout();
            }
        public:
            CellView(GLContextManager& contextManager, QScrollBar* scrollBar = nullptr) :
            RenderView(contextManager),
            m_layoutInitialized(false),
            m_valid(false),
            m_scrollBar(scrollBar) {
                if (m_scrollBar != nullptr) {
                    connect(m_scrollBar, &QAbstractSlider::actionTriggered, this, &CellView::onScrollBarActionTriggered);
                    connect(m_scrollBar, &QAbstractSlider::valueChanged, this, &CellView::onScrollBarValueChanged);
                }
            }

            void invalidate() {
                m_valid = false;
            }
            
            void clear() {
                m_layout.clear();
                doClear();
                m_valid = true;
            }

            void resizeEvent(QResizeEvent* event) override {
                m_layout.setWidth(static_cast<float>(size().width()));
                updateScrollBar();

                RenderView::resizeEvent(event);
            }

            void onScrollBarValueChanged() {
                requestUpdate();
            }

            void onScrollBarActionTriggered(int action) {
                const auto top = static_cast<float>(m_scrollBar->value());
                const auto height = static_cast<float>(size().height());

                // NOTE: We call setSliderPosition(), not setValue()
                // see: https://doc.qt.io/archives/qt-4.8/qabstractslider.html#actionTriggered
                switch (action) {
                    case QAbstractSlider::SliderSingleStepAdd:
                        m_scrollBar->setSliderPosition(static_cast<int>(m_layout.rowPosition(top, 1))); // line down
                        break;
                    case QAbstractSlider::SliderSingleStepSub:
                        m_scrollBar->setSliderPosition(static_cast<int>(m_layout.rowPosition(top, -1))); // line up
                        break;
                    case QAbstractSlider::SliderPageStepAdd:
                        m_scrollBar->setSliderPosition(static_cast<int>(m_layout.rowPosition(top, 0))); // page down
                        break;
                    case QAbstractSlider::SliderPageStepSub:
                        m_scrollBar->setSliderPosition(static_cast<int>(m_layout.rowPosition(std::max(0.0f, top - height), 0))); // page up
                        break;
                    default:
                        break;
                }
                requestUpdate();
            }


            class DndHelper {
            private:
                CellView& m_cellView;
            public:
                DndHelper(CellView& cellView) :
                m_cellView(cellView) {
                    m_cellView.dndWillStart();
                }
                
                ~DndHelper() {
                    m_cellView.dndDidEnd();
                }
            };

            void mousePressEvent(QMouseEvent* event) override {
                if (event->button() == Qt::LeftButton) {
                    m_potentialDrag = true;
                } else if (event->button() == Qt::RightButton) {
                    if (event->modifiers() & Qt::AltModifier) {
                        m_lastMousePos = event->pos();
                    }
                }
            }

            void mouseReleaseEvent(QMouseEvent* event) override {
                if (event->button() == Qt::LeftButton) {
                    int top = m_scrollBar != nullptr ? m_scrollBar->value() : 0;
                    float x = static_cast<float>(event->localPos().x());
                    float y = static_cast<float>(event->localPos().y());
                    doLeftClick(m_layout, x, y);
                }
            }

            void mouseMoveEvent(QMouseEvent* event) override {
                if (event->buttons() & Qt::LeftButton) {
                    if (m_potentialDrag) {
                        startDrag(event);
                        m_potentialDrag = false;
                    }
                } else if ((event->buttons() & Qt::RightButton) && (event->modifiers() & Qt::AltModifier)) {
                    scroll(event);
                } else {
                    updateTooltip(event);
                }

                m_lastMousePos = event->pos();
            }

            void wheelEvent(QWheelEvent* event) override {
                if (m_scrollBar != nullptr) {
                    QPoint pixelDelta = event->pixelDelta();
                    if (pixelDelta.isNull()) {
                        QPoint degreeDelta = event->angleDelta() / 8;
                        pixelDelta = degreeDelta;
                    }

                    const int top = m_scrollBar->value();
                    const int height = static_cast<int>(m_layout.height());
                    const int newTop = std::min(std::max(0, top - pixelDelta.y()), height);
                    m_scrollBar->setValue(newTop);
                    requestUpdate();
                }
            }

            void startDrag(const QMouseEvent* event) {
#if 0 // FIXME: DND
                if (dndEnabled()) {
                    int top = m_scrollBar != nullptr ? m_scrollBar->value() : 0;
                    float x = static_cast<float>(event.GetX());
                    float y = static_cast<float>(event.GetY() + top);
                    const Cell* cell = nullptr;
                    if (m_layout.cellAt(x, y, &cell)) {
                        /*
                         wxImage* feedbackImage = dndImage(*cell);
                         int xOffset = event.GetX() - static_cast<int>(cell->itemBounds().left());
                         int yOffset = event.GetY() - static_cast<int>(cell->itemBounds().top()) + top;
                         */
                        
                        const DndHelper dndHelper(*this);
                        wxTextDataObject dropData(dndData(*cell));
                        DropSource dropSource(dropData, this);
                        dropSource.DoDragDrop();
                    }
                }
#endif
            }
            
            void scroll(const QMouseEvent* event) {
                if (m_scrollBar != nullptr) {
                    const QPoint mousePosition = event->pos();
                    const int delta = mousePosition.y() - m_lastMousePos.y();
                    const int newThumbPosition = m_scrollBar->value() - delta;
                    m_scrollBar->setValue(newThumbPosition);
                    requestUpdate();
                }
            }
            
            void updateTooltip(const QMouseEvent* event) {
#if 0 // FIXME: TOOLTIPS
                int top = m_scrollBar != nullptr ? m_scrollBar->GetThumbPosition() : 0;
                float x = static_cast<float>(event.GetX());
                float y = static_cast<float>(event.GetY() + top);
                const LayoutCell* cell = nullptr;
                if (m_layout.cellAt(x, y, &cell))
                    SetToolTip(tooltip(*cell));
                else
                    SetToolTip("");
#endif
            }
        private:
            void doRender() override {
                if (!m_valid)
                    validate();
                if (!m_layoutInitialized)
                    initLayout();

                // FIXME: check DPI awareness
                const int top = m_scrollBar != nullptr ? m_scrollBar->value() : 0;
                const QRect visibleRect = QRect(QPoint(0, top), size());
                
                const float y = static_cast<float>(visibleRect.y());
                const float h = static_cast<float>(visibleRect.height());
                
                const GLint viewLeft      = 0; //static_cast<GLint>(GetClientRect().GetLeft());
                const GLint viewTop       = 0; //static_cast<GLint>(GetClientRect().GetBottom());
                const GLint viewRight     = width(); //static_cast<GLint>(GetClientRect().GetRight());
                const GLint viewBottom    = height(); //static_cast<GLint>(GetClientRect().GetTop());
                glViewport(viewLeft, viewBottom, viewRight - viewLeft, viewTop - viewBottom);

                setupGL();
                doRender(m_layout, y, h);
            }
            
            void setupGL() {
                glAssert(glEnable(GL_MULTISAMPLE));
                glAssert(glEnable(GL_BLEND));
                glAssert(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
                glAssert(glEnable(GL_CULL_FACE));
                glAssert(glEnable(GL_DEPTH_TEST));
                glAssert(glDepthFunc(GL_LEQUAL));
                glAssert(glShadeModel(GL_SMOOTH));
            }
            
            virtual void doInitLayout(Layout& layout) = 0;
            virtual void doReloadLayout(Layout& layout) = 0;
            virtual void doClear() {}
            virtual void doRender(Layout& layout, float y, float height) = 0;
            virtual void doLeftClick(Layout& layout, float x, float y) {}
            
            virtual bool dndEnabled() { return false; }
            virtual void dndWillStart() {}
            virtual void dndDidEnd() {}
            virtual QPixmap dndImage(const Cell& cell) { assert(false); return QPixmap(); }
            virtual QString dndData(const Cell& cell) { assert(false); return ""; }
            virtual QString tooltip(const Cell& cell) { return ""; }
        public: // implement InputEventProcessor interface
            void processEvent(const KeyEvent& event) override {}
            void processEvent(const MouseEvent& event) override {}
            void processEvent(const CancelEvent& event) override {}
        };
    }
}

#endif
