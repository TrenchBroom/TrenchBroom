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

#include "Notifier.h"
#include "NotifierConnection.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <QObject>

class QWindow;
class QFocusEvent;
class QMouseEvent;

namespace TrenchBroom {
    namespace Model {
        class PickResult;
    }

    namespace Renderer {
        class RenderBatch;
        class RenderContext;
    }

    namespace View {
        class DragTracker;
        class DropTracker;
        class InputState;
        class Tool;
        class ToolController;
        class ToolChain;

        class ToolBox : public QObject {
            Q_OBJECT
        private:
            std::unique_ptr<DragTracker> m_dragTracker;
            std::unique_ptr<DropTracker> m_dropTracker;
            Tool* m_modalTool;

            using ToolList = std::vector<Tool*>;
            using ToolMap = std::map<Tool*, ToolList>;
            ToolMap m_suppressedTools;

            bool m_enabled;

            NotifierConnection m_notifierConnection;
        public:
            Notifier<Tool&> toolActivatedNotifier;
            Notifier<Tool&> toolDeactivatedNotifier;
            Notifier<Tool&> refreshViewsNotifier;
            Notifier<Tool&> toolHandleSelectionChangedNotifier;
        public:
            ToolBox();
            ~ToolBox();
        protected:
            void addTool(Tool& tool);
        public: // picking
            void pick(ToolChain* chain, const InputState& inputState, Model::PickResult& pickResult);
        public: // event handling
            bool dragEnter(ToolChain* chain, const InputState& inputState, const std::string& text);
            bool dragMove(ToolChain* chain, const InputState& inputState, const std::string& text);
            void dragLeave(ToolChain* chain, const InputState& inputState);
            bool dragDrop(ToolChain* chain, const InputState& inputState, const std::string& text);

            void modifierKeyChange(ToolChain* chain, const InputState& inputState);
            void mouseDown(ToolChain* chain, const InputState& inputState);
            void mouseUp(ToolChain* chain, const InputState& inputState);
            bool mouseClick(ToolChain* chain, const InputState& inputState);
            void mouseDoubleClick(ToolChain* chain, const InputState& inputState);
            void mouseMove(ToolChain* chain, const InputState& inputState);

            bool dragging() const;
            bool startMouseDrag(ToolChain* chain, const InputState& inputState);
            bool mouseDrag(const InputState& inputState);
            void endMouseDrag(const InputState& inputState);
            void cancelMouseDrag();

            void mouseScroll(ToolChain* chain, const InputState& inputState);

            bool cancel(ToolChain* chain);
        public: // tool management
            /**
             * Suppress a tool when another becomes active. The suppressed tool becomes temporarily deactivated.
             *
             * @param suppressedTool the tool that becomes supressed while the other is active
             * @param primaryTool the tool that controls when the suppressed tool is deactivated
             */
            void suppressWhileActive(Tool& suppressedTool, Tool& primaryTool);

            bool anyToolActive() const;
            void toggleTool(Tool& tool);
            void deactivateAllTools();

            bool enabled() const;
            void enable();
            void disable();
        public: // rendering
            void setRenderOptions(ToolChain* chain, const InputState& inputState, Renderer::RenderContext& renderContext);
            void renderTools(ToolChain* chain, const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
        private:
            bool activateTool(Tool& tool);
            void deactivateTool(Tool& tool);
        };
    }
}

