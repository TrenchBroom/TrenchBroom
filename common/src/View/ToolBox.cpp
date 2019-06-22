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

#include "CollectionUtils.h"
#include "ToolBox.h"
#include "TemporarilySetAny.h"
#include "View/InputState.h"
#include "View/Tool.h"
#include "View/ToolController.h"
#include "View/ToolChain.h"

#include <QApplication>
#include <QEvent>
#include <QFocusEvent>
#include <QEnterEvent>
#include <QWidget>
#include <QDateTime>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        ToolBox::ToolBox() :
        m_dragReceiver(nullptr),
        m_dropReceiver(nullptr),
        m_savedDropReceiver(nullptr),
        m_modalTool(nullptr),
        m_clickToActivate(true),
        m_ignoreNextClick(false),
        m_lastActivation(QDateTime::currentMSecsSinceEpoch()),
        m_enabled(true) {}

        bool ToolBox::eventFilter(QObject *obj, QEvent *ev) {
            QWidget* observedWidget = dynamic_cast<QWidget*>(obj);
            ensure(observedWidget != nullptr, "expected a QWidget");

            switch (ev->type()) {
                case QEvent::FocusIn:
                    OnSetFocus(static_cast<QFocusEvent *>(ev));
                    break;
                case QEvent::FocusOut:
                    OnKillFocus(static_cast<QFocusEvent *>(ev));
                    break;
                case QEvent::Enter:
                    OnEnterWindow(static_cast<QEnterEvent *>(ev), observedWidget);
                    break;
                case QEvent::Leave:
                    OnLeaveWindow();
                    break;
                // FIXME: handle ToolBoxConnector::OnMouseCaptureLost?
                default:
                    break;
            }
            return QObject::eventFilter(obj, ev);
        }

        void ToolBox::addWindow(QWidget* window) {
            ensure(window != nullptr, "window is null");

            window->installEventFilter(this);
            m_focusGroup.push_back(window);
        }

        void ToolBox::removeWindow(QWidget* window) {
            window->removeEventFilter(this);
            VectorUtils::erase(m_focusGroup, window);
        }

        void ToolBox::OnSetFocus(QFocusEvent* /*event*/) {
            if ((QDateTime::currentMSecsSinceEpoch() - m_lastActivation) < 100)
                m_ignoreNextClick = false;
            clearFocusCursor();
        }

        void ToolBox::OnKillFocus(QFocusEvent* /*event*/) {
            if (m_clickToActivate) {
                // FIXME: Check that this returns the right thing, since we're in a state when we're being notified of a widget losing focus.
                const QWidget* focusedWindow = QApplication::focusWidget();
                if (!VectorUtils::contains(m_focusGroup, focusedWindow)) {
                    m_ignoreNextClick = true;
                    setFocusCursor();
                }
            }
        }

        void ToolBox::OnEnterWindow(QEnterEvent* /*event*/, QWidget* enteredWidget) {
            QWidget* newFocus = enteredWidget;
            QWidget* currentFocus = QApplication::focusWidget();
            // FIXME: not sure about Qt equivalent for this
            if (VectorUtils::contains(m_focusGroup, currentFocus)/* && !currentFocus->HasCapture()*/)
                newFocus->setFocus();
        }

        void ToolBox::OnLeaveWindow() {
        }

        void ToolBox::setFocusCursor() {
            for (size_t i = 0; i < m_focusGroup.size(); ++i)
                m_focusGroup[i]->setCursor(Qt::OpenHandCursor);
        }

        void ToolBox::clearFocusCursor() {
            for (size_t i = 0; i < m_focusGroup.size(); ++i)
                m_focusGroup[i]->setCursor(Qt::ArrowCursor);
        }

        void ToolBox::addTool(Tool* tool) {
            ensure(tool != nullptr, "tool is null");
            tool->refreshViewsNotifier.addObserver(refreshViewsNotifier);
        }

        void ToolBox::pick(ToolChain* chain, const InputState& inputState, Model::PickResult& pickResult) {
            chain->pick(inputState, pickResult);
        }

        bool ToolBox::clickToActivate() const {
            return m_clickToActivate;
        }

        void ToolBox::setClickToActivate(const bool clickToActivate) {
            m_clickToActivate = clickToActivate;
            if (!m_clickToActivate)
                m_ignoreNextClick = false;
        }

        void ToolBox::updateLastActivation() {
            m_lastActivation = QDateTime::currentMSecsSinceEpoch();
        }

        bool ToolBox::ignoreNextClick() const {
            return m_ignoreNextClick;
        }

        void ToolBox::clearIgnoreNextClick() {
            m_ignoreNextClick = false;
        }

        bool ToolBox::dragEnter(ToolChain* chain, const InputState& inputState, const String& text) {
            // On XFCE, this crashes when dragging an entity across a 2D view into the 3D view
            // I assume that the drag leave event is swallowed somehow.
            // assert(m_dropReceiver == nullptr);

            if (!m_enabled)
                return false;

            if (m_dropReceiver != nullptr)
                dragLeave(chain, inputState);

            deactivateAllTools();
            m_dropReceiver = chain->dragEnter(inputState, text);
            return m_dropReceiver != nullptr;
        }

        bool ToolBox::dragMove(ToolChain* chain, const InputState& inputState, const String& text) {
            if (m_dropReceiver == nullptr)
                return false;

            if (!m_enabled)
                return false;

            m_dropReceiver->dragMove(inputState);
            return true;
        }

        void ToolBox::dragLeave(ToolChain* chain, const InputState& inputState) {
            if (m_dropReceiver == nullptr)
                return;
            if (!m_enabled)
                return;

            // This is a workaround for a bug in wxWidgets 3.0.0 on GTK2, where a drag leave event
            // is sent right before the drop event. So we save the drag receiver in an instance variable
            // and if dragDrop() is called, it can use that variable to find out who the drop receiver is.
            m_savedDropReceiver = m_dropReceiver;

            m_dropReceiver->dragLeave(inputState);
            m_dropReceiver = nullptr;
        }

        bool ToolBox::dragDrop(ToolChain* chain, const InputState& inputState, const String& text) {
            if (m_dropReceiver == nullptr && m_savedDropReceiver == nullptr)
                return false;

            if (!m_enabled)
                return false;

            if (m_dropReceiver == nullptr) {
                m_dropReceiver = m_savedDropReceiver;
                Tool* tool = m_dropReceiver->tool();
                if (!tool->active()) // GTK2 fix: has been deactivated by dragLeave()
                    tool->activate();
                m_dropReceiver->dragEnter(inputState, text);
            }

            const bool result = m_dropReceiver->dragDrop(inputState);
            m_dropReceiver = nullptr;
            m_savedDropReceiver = nullptr;
            return result;
        }

        void ToolBox::modifierKeyChange(ToolChain* chain, const InputState& inputState) {
            if (m_enabled)
                chain->modifierKeyChange(inputState);
        }

        void ToolBox::mouseDown(ToolChain* chain, const InputState& inputState) {
            if (m_enabled)
                chain->mouseDown(inputState);
        }

        void ToolBox::mouseUp(ToolChain* chain, const InputState& inputState) {
            if (m_enabled)
                chain->mouseUp(inputState);
        }

        bool ToolBox::mouseClick(ToolChain* chain, const InputState& inputState) {
            if (m_enabled)
                return chain->mouseClick(inputState);
            return false;
        }

        void ToolBox::mouseDoubleClick(ToolChain* chain, const InputState& inputState) {
            if (m_enabled)
                chain->mouseDoubleClick(inputState);
        }

        void ToolBox::mouseMove(ToolChain* chain, const InputState& inputState) {
            if (m_enabled)
                chain->mouseMove(inputState);
        }

        bool ToolBox::dragging() const {
            return m_dragReceiver != nullptr;
        }

        bool ToolBox::startMouseDrag(ToolChain* chain, const InputState& inputState) {
            if (!m_enabled)
                return false;
            m_dragReceiver = chain->startMouseDrag(inputState);
            return m_dragReceiver != nullptr;
        }

        bool ToolBox::mouseDrag(const InputState& inputState) {
            assert(enabled() && dragging());
            return m_dragReceiver->mouseDrag(inputState);
        }

        void ToolBox::endMouseDrag(const InputState& inputState) {
            assert(enabled() && dragging());
            m_dragReceiver->endMouseDrag(inputState);
            m_dragReceiver = nullptr;
        }

        void ToolBox::cancelMouseDrag() {
            assert(dragging());
            m_dragReceiver->cancelMouseDrag();
            m_dragReceiver = nullptr;
        }

        void ToolBox::mouseScroll(ToolChain* chain, const InputState& inputState) {
            if (m_enabled)
                chain->mouseScroll(inputState);
        }

        bool ToolBox::cancel(ToolChain* chain) {
            if (dragging()) {
                cancelMouseDrag();
                return true;
            }

            if (chain->cancel())
                return true;

            if (anyToolActive()) {
                deactivateAllTools();
                return true;
            }

            return false;
        }

        void ToolBox::deactivateWhen(Tool* master, Tool* slave) {
            ensure(master != nullptr, "master is null");
            ensure(slave != nullptr, "slave is null");
            assert(master != slave);
            m_deactivateWhen[master].push_back(slave);
        }

        bool ToolBox::anyToolActive() const {
            return m_modalTool != nullptr;
        }

        bool ToolBox::toolActive(const Tool* tool) const {
            if (tool == nullptr)
                return false;
            return tool->active();
        }

        void ToolBox::toggleTool(Tool* tool) {
            if (tool == nullptr) {
                if (m_modalTool != nullptr) {
                    deactivateTool(m_modalTool);
                    m_modalTool = nullptr;
                }
            } else {
                if (m_modalTool == tool) {
                    deactivateTool(m_modalTool);
                    m_modalTool = nullptr;
                } else {
                    if (m_modalTool != nullptr) {
                        deactivateTool(m_modalTool);
                        m_modalTool = nullptr;
                    }
                    if (activateTool(tool))
                        m_modalTool = tool;
                }
            }
        }

        void ToolBox::deactivateAllTools() {
            toggleTool(nullptr);
        }

        bool ToolBox::enabled() const {
            return m_enabled;
        }

        void ToolBox::enable() {
            m_enabled = true;
        }

        void ToolBox::disable() {
            assert(!dragging());
            m_enabled = false;
        }

        void ToolBox::setRenderOptions(ToolChain* chain, const InputState& inputState, Renderer::RenderContext& renderContext) {
            chain->setRenderOptions(inputState, renderContext);
        }

        void ToolBox::renderTools(ToolChain* chain, const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            /* if (m_modalTool != nullptr)
                m_modalTool->renderOnly(m_inputState, renderContext);
            else */
            chain->render(inputState, renderContext, renderBatch);
        }

        bool ToolBox::activateTool(Tool* tool) {
            if (!tool->activate())
                return false;

            ToolMap::iterator it = m_deactivateWhen.find(tool);
            if (it != std::end(m_deactivateWhen)) {
                const ToolList& slaves = it->second;
                for (Tool* slave : slaves) {
                    slave->deactivate();
                    toolDeactivatedNotifier(slave);
                }
            }

            toolActivatedNotifier(tool);
            return true;
        }

        void ToolBox::deactivateTool(Tool* tool) {
            if (dragging())
                cancelMouseDrag();

            ToolMap::iterator it = m_deactivateWhen.find(tool);
            if (it != std::end(m_deactivateWhen)) {
                const ToolList& slaves = it->second;
                for (Tool* slave : slaves) {
                    slave->activate();
                    toolActivatedNotifier(slave);
                }
            }

            tool->deactivate();
            toolDeactivatedNotifier(tool);
        }
    }
}
