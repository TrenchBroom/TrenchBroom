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

#include "DocumentCanvas.h"

void DocumentCanvas::render() {
    SetCurrent();
    wxPaintDC(this);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, (GLint)GetSize().x, (GLint)GetSize().y);

    glFlush();
    SwapBuffers();
}

BEGIN_EVENT_TABLE(DocumentCanvas, wxGLCanvas)
    EVT_PAINT    (DocumentCanvas::Paintit)
END_EVENT_TABLE()

DocumentCanvas::DocumentCanvas(wxWindow *parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name, int* attribList) : wxGLCanvas(parent, id, pos, size, style, name, attribList) {
    //ctor
}

DocumentCanvas::~DocumentCanvas() {
    //dtor
}

void DocumentCanvas::Paintit(wxPaintEvent& event) {
    render();
}
