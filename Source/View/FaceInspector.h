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


#ifndef __TrenchBroom__FaceInspector__
#define __TrenchBroom__FaceInspector__

#include <wx/panel.h>

#include "Model/BrushTypes.h"
#include "Model/FaceTypes.h"
#include "View/SpinControl.h"

class SpinControl;
class wxButton;
class wxListBox;
class wxGLContext;
class wxSpinDoubleEvent;
class wxStaticText;

namespace TrenchBroom {
    namespace Controller {
        class Command;
    }
    
    namespace Model {
        class Texture;
    }
    
    namespace Renderer {
        class Camera;
    }
    
    namespace View {
        class DocumentViewHolder;
        class SingleTextureViewer;
        class TextureBrowser;
        class TextureSelectedCommand;
        
        class FaceInspector : public wxPanel {
        protected:
            DocumentViewHolder& m_documentViewHolder;
            
            SingleTextureViewer* m_textureViewer;
            wxStaticText* m_textureNameLabel;
            
            SpinControl* m_xOffsetEditor;
            SpinControl* m_yOffsetEditor;
            SpinControl* m_xScaleEditor;
            SpinControl* m_yScaleEditor;
            SpinControl* m_rotationEditor;
            
            TextureBrowser* m_textureBrowser;

            wxWindow* createFaceEditor();
            wxWindow* createTextureBrowser();

            void updateFaceAttributes();
            void updateSelectedTexture();
            void updateTextureBrowser(bool reloadTextures);
        public:
            FaceInspector(wxWindow* parent, DocumentViewHolder& documentViewHolder);
            
            void update(const Controller::Command& command);
            void cameraChanged(const Renderer::Camera& camera) {}

            void OnXOffsetChanged(SpinControlEvent& event);
            void OnYOffsetChanged(SpinControlEvent& event);
            void OnXScaleChanged(SpinControlEvent& event);
            void OnYScaleChanged(SpinControlEvent& event);
            void OnRotationChanged(SpinControlEvent& event);
            
            void OnResetFaceAttribsPressed(wxCommandEvent& event);
            void OnAlignTexturePressed(wxCommandEvent& event);
            void OnFitTexturePressed(wxCommandEvent& event);
            void OnFlipTextureHorizontallyPressed(wxCommandEvent& event);
            void OnFlipTextureVerticallyPressed(wxCommandEvent& event);
            void OnUpdateFaceButtons(wxUpdateUIEvent& event);
            void OnTextureSelected(TextureSelectedCommand& event);
            
            void OnIdle(wxIdleEvent& event);
            
            DECLARE_EVENT_TABLE()
        };
    }
}

#endif /* defined(__TrenchBroom__FaceInspector__) */
