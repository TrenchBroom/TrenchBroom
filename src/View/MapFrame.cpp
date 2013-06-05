/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "MapFrame.h"

#include "TrenchBroomApp.h"
#include "View/DocumentManager.h"

namespace TrenchBroom {
    namespace View {
        IMPLEMENT_DYNAMIC_CLASS(MapFrame, wxFrame)

        MapFrame::MapFrame() :
        wxFrame(NULL, wxID_ANY, wxT("unnamed.map")) {}

        MapFrame::MapFrame(MapDocument::Ptr document) :
        wxFrame(NULL, wxID_ANY, wxT("unnamed.map")) {
            Create(document);
            createGui();
        }

        void MapFrame::Create(MapDocument::Ptr document) {
            m_document = document;
            Bind(wxEVT_CLOSE_WINDOW, &MapFrame::OnClose, this);
        }

        MapFrame::~MapFrame() {}

        void MapFrame::OnClose(wxCloseEvent& event) {
            DocumentManager& documentManager = ::documentManager();
            if (!documentManager.closeDocument(m_document))
                event.Veto();
        }

        void MapFrame::createGui() {
        }
    }
}
