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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "FaceAttribsEditor.h"

#include "Assets/AssetTypes.h"
#include "Model/BrushFace.h"
#include "Model/Game.h"
#include "Model/GameConfig.h"
#include "View/ControllerFacade.h"
#include "View/FlagsPopupEditor.h"
#include "View/Grid.h"
#include "View/LayoutConstants.h"
#include "View/MapDocument.h"
#include "View/SpinControl.h"
#include "View/TextureView.h"

#include <wx/gbsizer.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

namespace TrenchBroom {
    namespace View {
        FaceAttribsEditor::FaceAttribsEditor(wxWindow* parent, Renderer::RenderResources& resources, MapDocumentPtr document, ControllerPtr controller) :
        wxPanel(parent),
        m_document(document),
        m_controller(controller) {
            createGui(resources);
            bindEvents();
            bindObservers();
        }

        FaceAttribsEditor::~FaceAttribsEditor() {
            unbindObservers();
        }

        void FaceAttribsEditor::OnXOffsetChanged(SpinControlEvent& event) {
            if (!m_controller->setFaceXOffset(m_faces, static_cast<float>(event.GetValue())))
                event.Veto();
        }
        
        void FaceAttribsEditor::OnYOffsetChanged(SpinControlEvent& event) {
            if (!m_controller->setFaceYOffset(m_faces, static_cast<float>(event.GetValue())))
                event.Veto();
        }
        
        void FaceAttribsEditor::OnRotationChanged(SpinControlEvent& event) {
            if (!m_controller->setFaceRotation(m_faces, static_cast<float>(event.GetValue())))
                event.Veto();
        }
        
        void FaceAttribsEditor::OnXScaleChanged(SpinControlEvent& event) {
            if (!m_controller->setFaceXScale(m_faces, static_cast<float>(event.GetValue())))
                event.Veto();
        }
        
        void FaceAttribsEditor::OnYScaleChanged(SpinControlEvent& event) {
            if (!m_controller->setFaceYScale(m_faces, static_cast<float>(event.GetValue())))
                event.Veto();
        }
        
        void FaceAttribsEditor::OnIdle(wxIdleEvent& event) {
            Grid& grid = m_document->grid();
            m_xOffsetEditor->SetIncrements(grid.actualSize(), 2.0 * grid.actualSize(), 1.0);
            m_yOffsetEditor->SetIncrements(grid.actualSize(), 2.0 * grid.actualSize(), 1.0);
            m_rotationEditor->SetIncrements(grid.angle(), 90.0, 1.0);
        }

        void FaceAttribsEditor::createGui(Renderer::RenderResources& resources) {
            m_textureView = new TextureView(this, wxID_ANY, resources);
            m_textureNameLabel = new wxStaticText(this, wxID_ANY, _("n/a"));
            
            const double max = std::numeric_limits<double>::max();
            const double min = -max;
            
            m_xOffsetEditor = new SpinControl(this);
            m_xOffsetEditor->SetRange(min, max);
            
            m_yOffsetEditor = new SpinControl(this);
            m_yOffsetEditor->SetRange(min, max);
            
            m_xScaleEditor = new SpinControl(this);
            m_xScaleEditor->SetRange(min, max);
            m_xScaleEditor->SetIncrements(0.1, 0.25, 0.01);
            
            m_yScaleEditor = new SpinControl(this);
            m_yScaleEditor->SetRange(min, max);
            m_yScaleEditor->SetIncrements(0.1, 0.25, 0.01);
            
            m_rotationEditor = new SpinControl(this);
            m_rotationEditor->SetRange(min, max);
            
            wxSizer* textureLabelSizer = new wxBoxSizer(wxHORIZONTAL);
            textureLabelSizer->AddStretchSpacer();
            textureLabelSizer->Add(m_textureNameLabel);
            textureLabelSizer->AddStretchSpacer();
            
            wxSizer* textureViewSizer = new wxBoxSizer(wxVERTICAL);
            textureViewSizer->Add(m_textureView, 0, wxEXPAND);
            textureViewSizer->AddSpacer(LayoutConstants::ControlVerticalMargin);
            textureViewSizer->Add(textureLabelSizer, 1, wxEXPAND);
            textureViewSizer->SetItemMinSize(m_textureView, 128, 128);
            
            wxGridBagSizer* faceAttribsSizer = new wxGridBagSizer(LayoutConstants::FaceAttribsControlMargin, LayoutConstants::FaceAttribsControlMargin);
            faceAttribsSizer->Add(new wxStaticText(this, wxID_ANY, _("")), wxGBPosition(0, 0), wxDefaultSpan, wxALIGN_CENTER); // fake
            faceAttribsSizer->Add(new wxStaticText(this, wxID_ANY, _("X"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER), wxGBPosition(0, 1), wxDefaultSpan, wxEXPAND | wxALIGN_CENTER);
            faceAttribsSizer->Add(new wxStaticText(this, wxID_ANY, _("Y"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER), wxGBPosition(0, 2), wxDefaultSpan, wxEXPAND | wxALIGN_CENTER);
            
            faceAttribsSizer->Add(new wxStaticText(this, wxID_ANY, _("Offset:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT), wxGBPosition(1, 0), wxDefaultSpan, wxALIGN_RIGHT);
            faceAttribsSizer->Add(m_xOffsetEditor, wxGBPosition(1, 1), wxDefaultSpan, wxEXPAND);
            faceAttribsSizer->Add(m_yOffsetEditor, wxGBPosition(1, 2), wxDefaultSpan, wxEXPAND);
            faceAttribsSizer->Add(new wxStaticText(this, wxID_ANY, _("Scale:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT), wxGBPosition(2, 0), wxDefaultSpan, wxALIGN_RIGHT);
            
            faceAttribsSizer->Add(m_xScaleEditor, wxGBPosition(2, 1), wxDefaultSpan, wxEXPAND);
            faceAttribsSizer->Add(m_yScaleEditor, wxGBPosition(2, 2), wxDefaultSpan, wxEXPAND);
            
            faceAttribsSizer->Add(new wxStaticText(this, wxID_ANY, _("Rotation:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT), wxGBPosition(3, 0), wxGBSpan(1, 2), wxEXPAND |wxALIGN_RIGHT);
            faceAttribsSizer->Add(m_rotationEditor, wxGBPosition(3, 2), wxDefaultSpan, wxEXPAND);
            
            //            faceAttribsSizer->Add(buttonSizer, wxGBPosition(4, 0), wxGBSpan(1, 3), wxALIGN_RIGHT);
            
            faceAttribsSizer->AddGrowableCol(1);
            faceAttribsSizer->AddGrowableCol(2);
            faceAttribsSizer->SetItemMinSize(m_xOffsetEditor, 50, m_xOffsetEditor->GetSize().y);
            faceAttribsSizer->SetItemMinSize(m_yOffsetEditor, 50, m_yOffsetEditor->GetSize().y);
            faceAttribsSizer->SetItemMinSize(m_xScaleEditor, 50, m_xScaleEditor->GetSize().y);
            faceAttribsSizer->SetItemMinSize(m_yScaleEditor, 50, m_yScaleEditor->GetSize().y);
            faceAttribsSizer->SetItemMinSize(m_rotationEditor, 50, m_rotationEditor->GetSize().y);
            
            wxSizer* standardAttribsSizer = new wxBoxSizer(wxHORIZONTAL);
            standardAttribsSizer->Add(textureViewSizer);
            standardAttribsSizer->AddSpacer(LayoutConstants::ControlHorizontalMargin);
            standardAttribsSizer->Add(faceAttribsSizer, 1, wxEXPAND);
            
            m_surfaceFlagsEditor = new FlagsPopupEditor(this, _("Surface Flags:"), 2);
            m_contentFlagsEditor = new FlagsPopupEditor(this, _("Content Flags:"), 2);
            
            wxSizer* editorSizer = new wxBoxSizer(wxVERTICAL);
            editorSizer->Add(standardAttribsSizer, 0, wxEXPAND);
            editorSizer->AddSpacer(LayoutConstants::ControlVerticalMargin);
            editorSizer->Add(m_surfaceFlagsEditor, 0, wxEXPAND);
            editorSizer->AddSpacer(LayoutConstants::ControlVerticalMargin);
            editorSizer->Add(m_contentFlagsEditor, 0, wxEXPAND);
            
            SetSizer(editorSizer);
        }

        void FaceAttribsEditor::bindEvents() {
            m_xOffsetEditor->Bind(EVT_SPINCONTROL_EVENT,
                                  EVT_SPINCONTROL_HANDLER(FaceAttribsEditor::OnXOffsetChanged),
                                  this);
            m_yOffsetEditor->Bind(EVT_SPINCONTROL_EVENT,
                                  EVT_SPINCONTROL_HANDLER(FaceAttribsEditor::OnYOffsetChanged),
                                  this);
            m_xScaleEditor->Bind(EVT_SPINCONTROL_EVENT,
                                 EVT_SPINCONTROL_HANDLER(FaceAttribsEditor::OnXScaleChanged),
                                 this);
            m_yScaleEditor->Bind(EVT_SPINCONTROL_EVENT,
                                 EVT_SPINCONTROL_HANDLER(FaceAttribsEditor::OnYScaleChanged),
                                 this);
            m_rotationEditor->Bind(EVT_SPINCONTROL_EVENT,
                                   EVT_SPINCONTROL_HANDLER(FaceAttribsEditor::OnRotationChanged),
                                   this);
            Bind(wxEVT_IDLE, &FaceAttribsEditor::OnIdle, this);
        }
        
        void FaceAttribsEditor::bindObservers() {
            m_document->documentWasNewedNotifier.addObserver(this, &FaceAttribsEditor::documentWasNewed);
            m_document->documentWasLoadedNotifier.addObserver(this, &FaceAttribsEditor::documentWasLoaded);
            m_document->faceDidChangeNotifier.addObserver(this, &FaceAttribsEditor::faceDidChange);
            m_document->selectionDidChangeNotifier.addObserver(this, &FaceAttribsEditor::selectionDidChange);
            m_document->textureCollectionsDidChangeNotifier.addObserver(this, &FaceAttribsEditor::textureCollectionsDidChange);
        }
        
        void FaceAttribsEditor::unbindObservers() {
            m_document->documentWasNewedNotifier.removeObserver(this, &FaceAttribsEditor::documentWasNewed);
            m_document->documentWasLoadedNotifier.removeObserver(this, &FaceAttribsEditor::documentWasLoaded);
            m_document->faceDidChangeNotifier.removeObserver(this, &FaceAttribsEditor::faceDidChange);
            m_document->selectionDidChangeNotifier.removeObserver(this, &FaceAttribsEditor::selectionDidChange);
            m_document->textureCollectionsDidChangeNotifier.removeObserver(this, &FaceAttribsEditor::textureCollectionsDidChange);
        }
        
        void FaceAttribsEditor::documentWasNewed() {
            m_faces = m_document->allSelectedFaces();
            updateControls();
        }
        
        void FaceAttribsEditor::documentWasLoaded() {
            m_faces = m_document->allSelectedFaces();
            updateControls();
        }
        
        void FaceAttribsEditor::faceDidChange(Model::BrushFace* face) {
            updateControls();
        }
        
        void FaceAttribsEditor::selectionDidChange(const Model::SelectionResult& result) {
            m_faces = m_document->allSelectedFaces();
            updateControls();
        }
        
        void FaceAttribsEditor::textureCollectionsDidChange() {
            updateControls();
        }
        
        void FaceAttribsEditor::updateControls() {
            wxArrayString surfaceFlagLabels, surfaceFlagTooltips, contentFlagLabels, contentFlagTooltips;
            getSurfaceFlags(surfaceFlagLabels, surfaceFlagTooltips);
            getContentFlags(contentFlagLabels, contentFlagTooltips);
            m_surfaceFlagsEditor->setFlags(surfaceFlagLabels, surfaceFlagTooltips);
            m_contentFlagsEditor->setFlags(contentFlagLabels, contentFlagTooltips);
            
            if (!m_faces.empty()) {
                bool textureMulti = false;
                bool xOffsetMulti = false;
                bool yOffsetMulti = false;
                bool rotationMulti = false;
                bool xScaleMulti = false;
                bool yScaleMulti = false;
                
                Assets::Texture* texture = m_faces[0]->texture();
                const String& textureName = m_faces[0]->textureName();
                const float xOffset = m_faces[0]->xOffset();
                const float yOffset = m_faces[0]->yOffset();
                const float rotation = m_faces[0]->rotation();
                const float xScale = m_faces[0]->xScale();
                const float yScale = m_faces[0]->yScale();
                
                for (size_t i = 1; i < m_faces.size(); i++) {
                    Model::BrushFace* face = m_faces[i];
                    textureMulti  |= (texture  != face->texture());
                    xOffsetMulti  |= (xOffset  != face->xOffset());
                    yOffsetMulti  |= (yOffset  != face->yOffset());
                    rotationMulti |= (rotation != face->rotation());
                    xScaleMulti   |= (xScale   != face->xScale());
                    yScaleMulti   |= (yScale   != face->yScale());
                }
                
                m_xOffsetEditor->Enable();
                m_yOffsetEditor->Enable();
                m_rotationEditor->Enable();
                m_xScaleEditor->Enable();
                m_yScaleEditor->Enable();
                
                if (textureMulti) {
                    m_textureView->setTexture(NULL);
                    m_textureNameLabel->SetLabel(_("multi"));
                } else {
                    m_textureView->setTexture(texture);
                    m_textureNameLabel->SetLabel(textureName.empty() ? "n/a" : textureName);
                }
                if (xOffsetMulti) {
                    m_xOffsetEditor->SetHint(_("multi"));
                    m_xOffsetEditor->SetValue(_(""));
                } else {
                    m_xOffsetEditor->SetHint(_(""));
                    m_xOffsetEditor->SetValue(xOffset);
                }
                if (yOffsetMulti) {
                    m_yOffsetEditor->SetHint(_("multi"));
                    m_yOffsetEditor->SetValue(_(""));
                } else {
                    m_yOffsetEditor->SetHint(_(""));
                    m_yOffsetEditor->SetValue(yOffset);
                }
                if (rotationMulti) {
                    m_rotationEditor->SetHint(_("multi"));
                    m_rotationEditor->SetValue(_(""));
                } else {
                    m_rotationEditor->SetHint(_(""));
                    m_rotationEditor->SetValue(rotation);
                }
                if (xScaleMulti){
                    m_xScaleEditor->SetHint(_("multi"));
                    m_xScaleEditor->SetValue(_(""));
                } else {
                    m_xScaleEditor->SetHint(_(""));
                    m_xScaleEditor->SetValue(xScale);
                }
                if (yScaleMulti) {
                    m_yScaleEditor->SetHint(_("multi"));
                    m_yScaleEditor->SetValue(_(""));
                } else {
                    m_yScaleEditor->SetHint(_(""));
                    m_yScaleEditor->SetValue(yScale);
                }
            } else {
                m_xOffsetEditor->SetValue(_("n/a"));
                m_xOffsetEditor->Disable();
                m_yOffsetEditor->SetValue(_("n/a"));
                m_yOffsetEditor->Disable();
                m_xScaleEditor->SetValue(_("n/a"));
                m_xScaleEditor->Disable();
                m_yScaleEditor->SetValue(_("n/a"));
                m_yScaleEditor->Disable();
                m_rotationEditor->SetValue(_("n/a"));
                m_rotationEditor->Disable();
                m_textureView->setTexture(NULL);
                m_textureNameLabel->SetLabel("n/a");
            }
            Layout();
        }

        
        void getFlags(const Model::GameConfig::FlagConfigList& flags, wxArrayString& names, wxArrayString& descriptions) {
            Model::GameConfig::FlagConfigList::const_iterator it, end;
            for (it = flags.begin(), end = flags.end(); it != end; ++it) {
                const Model::GameConfig::FlagConfig& flag = *it;
                names.push_back(flag.name);
                descriptions.push_back(flag.description);
            }
        }
        
        void FaceAttribsEditor::getSurfaceFlags(wxArrayString& names, wxArrayString& descriptions) const {
            const Model::GamePtr game = m_document->game();
            const Model::GameConfig::FlagConfigList& surfaceFlags = game->surfaceFlags();
            getFlags(surfaceFlags, names, descriptions);
        }
        
        void FaceAttribsEditor::getContentFlags(wxArrayString& names, wxArrayString& descriptions) const {
            const Model::GamePtr game = m_document->game();
            const Model::GameConfig::FlagConfigList& contentFlags = game->contentFlags();
            getFlags(contentFlags, names, descriptions);
        }
    }
}
