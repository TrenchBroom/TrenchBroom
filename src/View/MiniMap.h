/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#ifndef __TrenchBroom__MiniMap__
#define __TrenchBroom__MiniMap__

#include "TrenchBroom.h"
#include "VecMath.h"

#include "Renderer/MiniMapRenderer.h"
#include "View/ViewTypes.h"

#include <wx/panel.h>

class wxSlider;
class wxWindow;

namespace TrenchBroom {
    namespace Renderer {
        class Camera;
        class RenderResources;
    }

    namespace View {
        class MiniMapXYView;
        class MiniMapZView;
        
        class MiniMap : public wxPanel {
        private:
            Renderer::MiniMapRenderer m_renderer;
            MiniMapZView* m_miniMapZView;
            MiniMapXYView* m_miniMapXYView;
        public:
            MiniMap(wxWindow* parent, View::MapDocumentWPtr document, Renderer::RenderResources& renderResources, Renderer::Camera& camera);
            
            void OnXYMiniMapChanged(wxCommandEvent& event);
            void OnZMiniMapChanged(wxCommandEvent& event);
        private:
            void createGui(View::MapDocumentWPtr document, Renderer::RenderResources& renderResources, Renderer::Camera& camera);
            void bindEvents();
        };
    }
}

#endif /* defined(__TrenchBroom__MiniMap__) */
