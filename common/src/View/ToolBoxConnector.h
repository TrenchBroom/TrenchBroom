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

#ifndef TrenchBroom_ToolBoxConnector
#define TrenchBroom_ToolBoxConnector

#include "StringUtils.h"
#include "View/InputState.h"
#include "View/PickRequest.h"

#include <QObject>
#include <QPoint>

class QWidget;
class QMouseEvent;
class QWheelEvent;
class QKeyEvent;
class QFocusEvent;

namespace TrenchBroom {
    namespace Model {
        class PickResult;
    }
    
    namespace Renderer {
        class Camera;
        class RenderBatch;
        class RenderContext;
    }

    namespace View {
        class ToolController;
        class ToolBox;
        class ToolChain;

        class ToolBoxConnector {
        private:
            class EventFilter : public QObject {
            private:
                ToolBoxConnector* m_owner;
            public:
                explicit EventFilter(ToolBoxConnector* owner);
            protected: // QObject
                bool eventFilter(QObject *obj, QEvent *ev) override;
            };
            friend class EventFilter;
        private:
            QWidget* m_window;
            ToolBox* m_toolBox;
            ToolChain* m_toolChain;
            
            InputState m_inputState;
            
            ulong m_clickTime;
            QPoint m_clickPos;
            QPoint m_lastMousePos;
            bool m_ignoreNextDrag;

            EventFilter* m_eventFilter;
        public:
            explicit ToolBoxConnector(QWidget* window);
            virtual ~ToolBoxConnector();

        public:
            const vm::ray3& pickRay() const;
            const Model::PickResult& pickResult() const;

            void updatePickResult();
            void updateLastActivation();
        protected:
            void setToolBox(ToolBox& toolBox);
            void addTool(ToolController* tool);
        public: // drag and drop
            bool dragEnter(int x, int y, const String& text);
            bool dragMove(int x, int y, const String& text);
            void dragLeave();
            bool dragDrop(int x, int y, const String& text);
        public: // cancel
            bool cancel();
        protected: // rendering
            void setRenderOptions(Renderer::RenderContext& renderContext);
            void renderTools(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
        private:
            void OnKey(QKeyEvent* event);
            void OnMouseButton(QMouseEvent* event);
            void OnMouseDoubleClick(QMouseEvent* event);
            void OnMouseMotion(QMouseEvent* event);
            void OnMouseWheel(QWheelEvent* event);
            //void OnMouseCaptureLost(wxMouseCaptureLostEvent& event);
            void OnSetFocus(QFocusEvent* event);
            void OnKillFocus(QFocusEvent* event);
        private:
            bool isWithinClickDistance(const QPoint& pos) const;
            
            void startDrag(QMouseEvent* event);
            void drag(QMouseEvent* event);
            void endDrag(QMouseEvent* event);
        public:
            bool cancelDrag();
        private:
            void captureMouse();
            void releaseMouse();

            
            ModifierKeyState modifierKeys();
            bool setModifierKeys();
            bool clearModifierKeys();
            void updateModifierKeys();
            
            MouseButtonState mouseButton(QMouseEvent* event);
            void mouseMoved(const QPoint& position);

            void showPopupMenu();
        private:
            virtual PickRequest doGetPickRequest(int x, int y) const = 0;
            virtual Model::PickResult doPick(const vm::ray3& pickRay) const = 0;
            virtual void doShowPopupMenu();
        };
    }
}

#endif /* defined(TrenchBroom_ToolBoxConnector) */
