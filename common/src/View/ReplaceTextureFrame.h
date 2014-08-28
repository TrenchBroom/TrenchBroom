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

#ifndef __TrenchBroom__ReplaceTextureFrame__
#define __TrenchBroom__ReplaceTextureFrame__

#include "Model/ModelTypes.h"
#include "View/GLContextHolder.h"
#include "View/ViewTypes.h"

#include <wx/frame.h>

namespace TrenchBroom {
    namespace View {
        class TextureBrowser;
        
        class ReplaceTextureFrame : public wxFrame {
        private:
            MapDocumentWPtr m_document;
            ControllerWPtr m_controller;
            
            TextureBrowser* m_subjectBrowser;
            TextureBrowser* m_replacementBrowser;
        public:
            ReplaceTextureFrame(wxWindow* parent, GLContextHolder::Ptr sharedContext, MapDocumentWPtr document, ControllerWPtr controller);
            
            void OnReplace(wxCommandEvent& event);
        private:
            Model::BrushFaceList getApplicableFaces(const Model::BrushFaceList& faces) const;
        public:
            void OnClose(wxCommandEvent& event);
            
            void OnUpdateReplaceButton(wxUpdateUIEvent& event);
        private:
            void createGui(GLContextHolder::Ptr sharedContext);
        };
    }
}

#endif /* defined(__TrenchBroom__ReplaceTextureFrame__) */
