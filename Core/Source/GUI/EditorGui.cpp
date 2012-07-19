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
#include "Gwen/Controls/Splitter.h"
#include "Controller/Editor.h"
#include "GUI/MapRendererControl.h"
#include "GUI/Inspector.h"
#include "Renderer/FontManager.h"
#include "Gwen/Platform.h"
#include "Gwen/Renderers/OpenGL_FTGL.h"
#include "Gwen/Skins/TexturedBase.h"

namespace TrenchBroom {
	namespace Gui {

		void EditorGui::onCanvasRedraw(Gwen::Controls::Base* control) {
			editorGuiRedraw(*this);
		}

		EditorGui::EditorGui(Controller::Editor& editor, Renderer::FontManager& fontManager, const std::string& skinPath) : m_editor(editor) {
			m_renderer = new Gwen::Renderer::OpenGL_FTGL();
			m_skin = new Gwen::Skin::TexturedBase();
			m_skin->SetRender(m_renderer);
			m_skin->Init(skinPath);
			m_skin->SetDefaultFont(Gwen::Platform::GetDefaultFontFace(), Gwen::Platform::GetDefaultFontSize());
			m_canvas = new Gwen::Controls::Canvas(m_skin);
			m_splitter = new Gwen::Controls::Splitter(m_canvas, true, -400);
			m_mapRenderer = new MapRendererControl(m_splitter, m_editor, fontManager);
			m_splitter->SetPanel(0, m_mapRenderer);
			m_inspector = new Inspector(m_splitter, editor);
			m_splitter->SetPanel(1, m_inspector);
			m_splitter->SetMinSize(0, 300);
			m_splitter->SetMinSize(1, 400);
			m_splitter->SetResize(0, true);
			m_splitter->SetResize(1, false);

			m_canvas->onRedraw.Add(this, &EditorGui::onCanvasRedraw);
		}

		EditorGui::~EditorGui() {
			m_canvas->Release();
			m_canvas = NULL;

			delete m_skin;
			m_skin = NULL;
			delete m_renderer;
			m_renderer = NULL;
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
 
		bool EditorGui::mapViewFocused() {
			return Gwen::KeyboardFocus == m_mapRenderer;
		}
	}
}
