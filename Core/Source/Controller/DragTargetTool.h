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
#include "Utilities/Console.h"

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
        public:
        private:
            bool m_figureDataValid;
            bool m_active;
        protected:
            Editor& m_editor;

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
            
            virtual bool handleActivate(const DragInfo& info) { return true; }
            virtual void handleDeactivate(const DragInfo& info) {}
            virtual bool handleMove(const DragInfo& info) { return true; }
            virtual bool handleDrop(const DragInfo& info) { return false; }
        public:
            DragTargetTool(Editor& editor) : m_editor(editor), m_figureDataValid(false), m_active(false) {}
            virtual ~DragTargetTool() {}
            
            Editor& editor() {
                return m_editor;
            }
            
            bool checkFigureDataValid() {
                bool result = m_figureDataValid;
                m_figureDataValid = true;
                return result;
            }

            bool active() {
                return m_active;
            }
            
            virtual bool accepts(const DragInfo& info) = 0;
            
            bool activate(const DragInfo& info) {
                assert(!m_active);
                
                m_active = true;
                return handleActivate(info);
            }
            
            void deactivate(const DragInfo& info) {
                handleDeactivate(info);
                m_active = false;
            }
            
            bool move(const DragInfo& info) {
                assert(m_active);
                
                return handleMove(info);
            }
            
            bool drop(const DragInfo& info) {
                assert(m_active);
                
                m_active = false;
                return handleDrop(info);
            }
        };
    }
}

#endif
