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

#ifndef TrenchBroom_VertexToolPage
#define TrenchBroom_VertexToolPage

#include "TrenchBroom.h"
#include "View/ViewTypes.h"
#include "IO/Path.h"

#include <wx/panel.h>

class wxCheckBox;

namespace TrenchBroom {
    namespace View {
        class VertexToolPage : public wxPanel {
        private:
            MapDocumentWPtr m_document;

            wxCheckBox* m_uvLockCheckBox;
        public:
            VertexToolPage(wxWindow* parent, MapDocumentWPtr document);
            virtual ~VertexToolPage();
        private:

            void createGui();
            void bindObservers();
            void unbindObservers();
            void preferenceDidChange(const IO::Path &path);
            void updateControls();
        };
    }
}

#endif /* defined(TrenchBroom_VertexToolPage) */
