/*
 Copyright (C) 2010-2012 Kristian Duske

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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TrenchBroom_Tool_h
#define TrenchBroom_Tool_h

#include <cstdio>

namespace TrenchBroom {
    namespace Model {
        class HitList;
    }

    namespace Controller {

        typedef enum {
            TB_MK_NONE = 0,
            TB_MK_SHIFT = 1 << 0,
            TB_MK_CTRL = 1 << 1,
            TB_MK_ALT = 1 << 2,
            TB_MK_CMD = 1 << 3
        } EModifierKeys;

        typedef enum {
            TB_MB_NONE = 0,
            TB_MB_LEFT = 1,
            TB_MB_RIGHT = 2,
            TB_MB_MIDDLE = 3
        } EMouseButton;

        class Editor;

        class ToolEvent {
        public:
            int modifierKeys;
            EMouseButton mouseButton;
            float mouseX;
            float mouseY;
            float deltaX;
            float deltaY;
            float scrollX;
            float scrollY;
            Model::HitList* hits;
            ToolEvent() : modifierKeys(TB_MK_NONE), mouseButton(TB_MB_NONE), hits(NULL) {}
        };

        class Tool {
        protected:
            Editor& m_editor;
        public:
            Tool(Editor& editor) : m_editor(editor) {}
            virtual ~Tool() {}

            virtual void activated(ToolEvent& event) {}
            virtual void deactivated(ToolEvent& event) {}
            virtual bool leftMouseDown(ToolEvent& event) { return false; }
            virtual bool leftMouseUp(ToolEvent& event) { return false; }
            virtual bool rightMouseDown(ToolEvent& event) { return false; }
            virtual bool rightMouseUp(ToolEvent& event) { return false; }
            virtual bool mouseMoved(ToolEvent& event) { return false; }
            virtual bool scrolled(ToolEvent& event) { return false; }

            virtual bool beginLeftDrag(ToolEvent& event) { return false; }
            virtual void leftDrag(ToolEvent& event) {}
            virtual void endLeftDrag(ToolEvent& event) {}

            virtual bool beginRightDrag(ToolEvent& event) { return false; }
            virtual void rightDrag(ToolEvent& event) {}
            virtual void endRightDrag(ToolEvent& event) {}

            virtual bool beginLeftScroll(ToolEvent& event) { return false; }
            virtual void leftScroll(ToolEvent& event) {}
            virtual void endLeftScroll(ToolEvent& event) {}

            virtual bool beginRightScroll(ToolEvent& event) { return false; }
            virtual void rightScroll(ToolEvent& event) {}
            virtual void endRightScroll(ToolEvent& event) {}

            static bool noModifierPressed(ToolEvent& event) { return event.modifierKeys == TB_MK_NONE; }
        };
    }
}


#endif
