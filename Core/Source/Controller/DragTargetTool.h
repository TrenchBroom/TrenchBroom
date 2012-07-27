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

#ifndef TrenchBroom_DragTargetTool_h
#define TrenchBroom_DragTargetTool_h

#include <string>
#include "Controller/Editor.h"
#include "Controller/Tool.h"
#include "Renderer/MapRenderer.h"

namespace TrenchBroom {
    namespace Controller {
        
        class DragInfo {
        public:
            Tool::InputEvent& event;
            std::string name;
            void* payload;

            DragInfo(Tool::InputEvent& event) : event(event) {}
        };
        
        class DragTargetTool {
        protected:
            Editor& m_editor;

            void addFigure(Renderer::Figure& figure) {
                m_editor.renderer()->addFigure(figure);
            }
            
            void removeFigure(Renderer::Figure& figure) {
                m_editor.renderer()->removeFigure(figure);
            }
            
        public:
            DragTargetTool(Editor& editor) : m_editor(editor) {}
            virtual ~DragTargetTool() {}
            
            virtual bool accepts(const DragInfo& info) = 0;
            virtual bool activate(const DragInfo& info) { return true; };
            virtual void deactivate(const DragInfo& info) {};
            virtual bool move(const DragInfo& info) { return true; };
            virtual bool drop(const DragInfo& info) { return false; };
        };
    }
}

#endif
