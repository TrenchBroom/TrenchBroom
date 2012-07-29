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

#include "Controller/Editor.h"
#include "Renderer/MapRenderer.h"
#include "Utilities/Event.h"
#include "Utilities/VecMath.h"
#include <cstdio>

namespace TrenchBroom {
    namespace Model {
        class HitList;
    }

    namespace Renderer {
        class Figure;
    }
    
    namespace Controller {
        class Editor;

        class Tool {
        public:
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
            
            typedef enum {
                TB_TS_DEFAULT = 0,
                TB_TS_MOUSE_DOWN = 1,
                TB_TS_DRAG = 2,
                TB_TS_SCROLL = 3
            } EToolState;

            class InputEvent {
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
                Ray ray;
                InputEvent() : modifierKeys(TB_MK_NONE), mouseButton(TB_MB_NONE), hits(NULL) {}
            };
            
        private:
            EToolState m_state;
            bool m_active;
            Editor& m_editor;
            bool m_figureDataValid;
        protected:
            void addFigure(Renderer::Figure& figure) {
				Renderer::MapRenderer* renderer = m_editor.renderer();
				if (renderer != NULL)
					renderer->addFigure(figure);
            }
            
            void removeFigure(Renderer::Figure& figure) {
                Renderer::MapRenderer* renderer = m_editor.renderer();
				if (renderer != NULL)
					renderer->removeFigure(figure);
            }
            
            void refreshFigure(bool invalidateFigureData) {
                if (invalidateFigureData)
                    m_figureDataValid = false;
                    
                Renderer::MapRenderer* renderer = m_editor.renderer();
				if (renderer != NULL)
					renderer->rendererChanged(*m_editor.renderer());
            }
        public:
            Tool(Editor& editor) : m_editor(editor), m_state(TB_TS_DEFAULT), m_active(false), m_figureDataValid(false) {}
            virtual ~Tool() {}

            EToolState state() {
                return m_state;
            }
            
            bool active() {
                return m_active;
            }
            
            Editor& editor() {
                return m_editor;
            }
            
            bool checkFigureDataValid() {
                bool result = m_figureDataValid;
                m_figureDataValid = true;
                return result;
            }
            
            virtual bool handleActivated(InputEvent& event) { return false; }
            virtual bool handleDeactivated(InputEvent& event) { return false; }
            virtual bool handleMouseDown(InputEvent& event) { return false; }
            virtual bool handleMouseUp(InputEvent& event) { return false; }
            virtual bool handleMouseMoved(InputEvent& event) { return false; }
            virtual bool handleScrolled(InputEvent& event) { return false; }
            virtual bool handleBeginDrag(InputEvent& event) { return false; }
            virtual bool handleDrag(InputEvent& event) { return false; }
            virtual void handleEndDrag(InputEvent& event) {}
            
            bool activated(InputEvent& event) {
                if (handleActivated(event)) {
                    m_active = true;
                    toolActivated(event);
                    return true;
                }
                
                return false;
            }
            
            bool deactivated(InputEvent& event) {
                if (handleDeactivated(event)) {
                    m_active = false;
                    m_state = TB_TS_DEFAULT;
                    toolDeactivated(event);
                    return true;
                }
                
                return false;
            }
            
            bool mouseDown(InputEvent& event) {
                if (handleMouseDown(event)) {
                    m_state = TB_TS_MOUSE_DOWN;
                    toolMouseDown(event);
                    return true;
                }
                
                return false;
            }
            
            bool mouseUp(InputEvent& event) {
                if (handleMouseUp(event)) {
                    m_state = TB_TS_DEFAULT;
                    toolMouseUp(event);
                    return true;
                }
                
                return false;
            }
            
            bool mouseMoved(InputEvent& event) {
                if (handleMouseMoved(event)) {
                    toolMouseMoved(event);
                    return true;
                }
                
                return false;
            }
            
            bool scrolled(InputEvent& event) {
                if (handleScrolled(event)) {
                    toolScrolled(event);
                    return true;
                }
                
                return false;
            }

            bool beginDrag(InputEvent& event) {
                if (handleBeginDrag(event)) {
                    m_state = TB_TS_DRAG;
                    toolDragBegun(event);
                    return true;
                }
                
                return false;
            }
            
            void drag(InputEvent& event) {
                if (handleDrag(event)) {
                    toolDragged(event);
                } else {
                    endDrag(event);
                }
            }
            
            void endDrag(InputEvent& event) {
                handleEndDrag(event);
                m_state = TB_TS_DEFAULT;
                toolDragEnded(event);
            }

            typedef Event<InputEvent&> ToolEvent;
            
            ToolEvent toolActivated;
            ToolEvent toolDeactivated;
            ToolEvent toolMouseDown;
            ToolEvent toolMouseMoved;
            ToolEvent toolMouseUp;
            ToolEvent toolScrolled;
            ToolEvent toolDragBegun;
            ToolEvent toolDragged;
            ToolEvent toolDragEnded;
            
            static bool noModifierPressed(InputEvent& event) { return event.modifierKeys == TB_MK_NONE; }
        };
    }
}


#endif
