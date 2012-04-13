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

#include "EditorGui.h"
#include "FontManager.h"

namespace TrenchBroom {
    namespace Gui {

        EditorGui::EditorGui(Controller::Editor& editor, Renderer::FontManager& fontManager, const string& skinPath) : m_editor(editor) {
            m_renderer = new Gwen::Renderer::OpenGL_DebugFont();
            m_skin = new Gwen::Skin::TexturedBase();
            m_skin->SetRender(m_renderer);
            m_skin->Init(skinPath);
            m_canvas = new Gwen::Controls::Canvas(m_skin);
            m_splitter = new Splitter(m_canvas, true);
            m_mapRenderer = new MapRendererControl(m_splitter, m_editor, fontManager);
            m_splitter->SetPanel(0, m_mapRenderer);
            m_inspectorTab = new Gwen::Controls::TabControl(m_splitter);
            m_splitter->SetPanel(1, m_inspectorTab);
        }
        
        EditorGui::~EditorGui() {
            delete m_mapRenderer;
            delete m_inspectorTab;
            delete m_splitter;
            delete m_canvas;
            delete m_skin;
            delete m_renderer;
        }
        
        void EditorGui::resizeTo(int width, int height) {
            m_canvas->SetSize(width, height);
            m_splitter->SetBounds(m_canvas->GetBounds());
        }
        
        void EditorGui::render() {
            m_canvas->RenderCanvas();
        }
        
        Gwen::Controls::Canvas* EditorGui::canvas() {
            return m_canvas;
        }
    }
}