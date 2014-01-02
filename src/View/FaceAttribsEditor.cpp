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
#include "View/FlagChangedCommand.h"
#include "View/FlagsPopupEditor.h"
#include "View/Grid.h"
#include "View/ViewConstants.h"
#include "View/MapDocument.h"
#include "View/SpinControl.h"
#include "View/TextureView.h"
#include "View/ViewUtils.h"

#include <wx/gbsizer.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

namespace TrenchBroom {
    namespace View {
        FaceAttribsEditor::FaceAttribsEditor(wxWindow* parent, Renderer::RenderResources& resources, MapDocumentWPtr document, ControllerWPtr controller) :
        wxPanel(parent),
        m_document(document),
        m_controller(controller),
        m_textureView(NULL),
        m_textureNameLabel(NULL),
        m_xOffsetEditor(NULL),
        m_yOffsetEditor(NULL),
        m_xScaleEditor(NULL),
        m_yScaleEditor(NULL),
        m_rotationEditor(NULL),
        m_surfaceValueLabel(NULL),
        m_surfaceValueEditor(NULL),
        m_faceAttribsSizer(NULL),
        m_surfaceFlagsLabel(NULL),
        m_surfaceFlagsEditor(NULL),
        m_contentFlagsLabel(NULL),
        m_contentFlagsEditor(NULL) {
            createGui(resources);
            bindEvents();
            bindObservers();
        }

        FaceAttribsEditor::~FaceAttribsEditor() {
            unbindObservers();
        }

        void FaceAttribsEditor::OnXOffsetChanged(SpinControlEvent& event) {
            ControllerSPtr controller = lock(m_controller);
            if (!controller->setFaceXOffset(m_faces, static_cast<float>(event.GetValue()), event.IsSpin()))
                event.Veto();
        }
        
        void FaceAttribsEditor::OnYOffsetChanged(SpinControlEvent& event) {
            ControllerSPtr controller = lock(m_controller);
            if (!controller->setFaceYOffset(m_faces, static_cast<float>(event.GetValue()), event.IsSpin()))
                event.Veto();
        }
        
        void FaceAttribsEditor::OnRotationChanged(SpinControlEvent& event) {
            ControllerSPtr controller = lock(m_controller);
            if (!controller->setFaceRotation(m_faces, static_cast<float>(event.GetValue()), event.IsSpin()))
                event.Veto();
        }
        
        void FaceAttribsEditor::OnXScaleChanged(SpinControlEvent& event) {
            ControllerSPtr controller = lock(m_controller);
            if (!controller->setFaceXScale(m_faces, static_cast<float>(event.GetValue()), event.IsSpin()))
                event.Veto();
        }
        
        void FaceAttribsEditor::OnYScaleChanged(SpinControlEvent& event) {
            ControllerSPtr controller = lock(m_controller);
            if (!controller->setFaceYScale(m_faces, static_cast<float>(event.GetValue()), event.IsSpin()))
                event.Veto();
        }
        
        void FaceAttribsEditor::OnSurfaceFlagChanged(FlagChangedCommand& command) {
            ControllerSPtr controller = lock(m_controller);
            if (!controller->setSurfaceFlag(m_faces, command.index(), command.flagSet()))
                command.Veto();
        }
        
        void FaceAttribsEditor::OnContentFlagChanged(FlagChangedCommand& command) {
            ControllerSPtr controller = lock(m_controller);
            if (!controller->setContentFlag(m_faces, command.index(), command.flagSet()))
                command.Veto();
        }

        void FaceAttribsEditor::OnSurfaceValueChanged(SpinControlEvent& event) {
            ControllerSPtr controller = lock(m_controller);
            if (!controller->setSurfaceValue(m_faces, static_cast<float>(event.GetValue()), event.IsSpin()))
                event.Veto();
        }

        void FaceAttribsEditor::OnIdle(wxIdleEvent& event) {
            MapDocumentSPtr document = lock(m_document);
            Grid& grid = document->grid();
            
            m_xOffsetEditor->SetIncrements(grid.actualSize(), 2.0 * grid.actualSize(), 1.0);
            m_yOffsetEditor->SetIncrements(grid.actualSize(), 2.0 * grid.actualSize(), 1.0);
            m_rotationEditor->SetIncrements(grid.angle(), 90.0, 1.0);
        }

        void FaceAttribsEditor::createGui(Renderer::RenderResources& resources) {
            wxPanel* textureViewPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER);
            m_textureView = new TextureView(textureViewPanel, wxID_ANY, resources);
            m_textureNameLabel = new wxStaticText(textureViewPanel, wxID_ANY, _("n/a"));
            
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
            
            m_surfaceValueLabel = new wxStaticText(this, wxID_ANY, _("Value:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
            m_surfaceValueEditor = new SpinControl(this);
            m_surfaceValueEditor->SetRange(min, max);
            m_surfaceValueEditor->SetIncrements(1.0, 10.0, 100.0);
            
            m_surfaceFlagsLabel = new wxStaticText(this, wxID_ANY, _("Surface:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
            m_surfaceFlagsEditor = new FlagsPopupEditor(this, 2);
            
            m_contentFlagsLabel = new wxStaticText(this, wxID_ANY, _("Content:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
            m_contentFlagsEditor = new FlagsPopupEditor(this, 2);
            
            wxSizer* textureLabelSizer = new wxBoxSizer(wxHORIZONTAL);
            textureLabelSizer->AddStretchSpacer();
            textureLabelSizer->Add(m_textureNameLabel);
            textureLabelSizer->AddStretchSpacer();
            
            wxSizer* texturePanelSizer = new wxBoxSizer(wxVERTICAL);
            texturePanelSizer->Add(m_textureView, 1, wxEXPAND);
            texturePanelSizer->AddSpacer(LayoutConstants::ControlVerticalMargin / 2);
            texturePanelSizer->Add(textureLabelSizer, 0, wxEXPAND);
            texturePanelSizer->SetItemMinSize(m_textureView, 128, 128);
            textureViewPanel->SetSizer(texturePanelSizer);
            
            m_faceAttribsSizer = new wxGridBagSizer(LayoutConstants::FaceAttribsControlMargin, LayoutConstants::FaceAttribsControlMargin);
            m_faceAttribsSizer->Add(textureViewPanel, wxGBPosition(0, 0), wxGBSpan(7, 1), wxEXPAND);
            
            m_faceAttribsSizer->Add(new wxStaticText(this, wxID_ANY, _("")), wxGBPosition(0, 1), wxDefaultSpan, wxALIGN_CENTER); // fake
            m_faceAttribsSizer->Add(new wxStaticText(this, wxID_ANY, _("X"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER), wxGBPosition(0, 2), wxDefaultSpan, wxEXPAND | wxALIGN_CENTER);
            m_faceAttribsSizer->Add(new wxStaticText(this, wxID_ANY, _("Y"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER), wxGBPosition(0, 3), wxDefaultSpan, wxEXPAND | wxALIGN_CENTER);
            
            m_faceAttribsSizer->Add(new wxStaticText(this, wxID_ANY, _("Offset:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT), wxGBPosition(1, 1), wxDefaultSpan, wxALIGN_RIGHT);
            m_faceAttribsSizer->Add(m_xOffsetEditor, wxGBPosition(1, 2), wxDefaultSpan, wxEXPAND);
            m_faceAttribsSizer->Add(m_yOffsetEditor, wxGBPosition(1, 3), wxDefaultSpan, wxEXPAND);
            
            m_faceAttribsSizer->Add(new wxStaticText(this, wxID_ANY, _("Scale:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT), wxGBPosition(2, 1), wxDefaultSpan, wxALIGN_RIGHT);
            m_faceAttribsSizer->Add(m_xScaleEditor, wxGBPosition(2, 2), wxDefaultSpan, wxEXPAND);
            m_faceAttribsSizer->Add(m_yScaleEditor, wxGBPosition(2, 3), wxDefaultSpan, wxEXPAND);
            
            m_faceAttribsSizer->Add(new wxStaticText(this, wxID_ANY, _("Angle:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT), wxGBPosition(3, 1), wxDefaultSpan, wxEXPAND | wxALIGN_RIGHT);
            m_faceAttribsSizer->Add(m_rotationEditor, wxGBPosition(3, 2), wxDefaultSpan, wxEXPAND);
            
            m_faceAttribsSizer->Add(m_surfaceValueLabel, wxGBPosition(4, 1), wxDefaultSpan, wxEXPAND | wxALIGN_RIGHT);
            m_faceAttribsSizer->Add(m_surfaceValueEditor, wxGBPosition(4, 2), wxDefaultSpan, wxEXPAND);
            
            m_faceAttribsSizer->Add(m_surfaceFlagsLabel, wxGBPosition(5, 1), wxDefaultSpan, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
            m_faceAttribsSizer->Add(m_surfaceFlagsEditor, wxGBPosition(5, 2), wxGBSpan(1, 3), wxEXPAND);
            
            m_faceAttribsSizer->Add(m_contentFlagsLabel, wxGBPosition(6, 1), wxDefaultSpan, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
            m_faceAttribsSizer->Add(m_contentFlagsEditor, wxGBPosition(6, 2), wxGBSpan(1, 3), wxEXPAND);
            
            m_faceAttribsSizer->AddGrowableCol(2);
            m_faceAttribsSizer->AddGrowableCol(3);
            m_faceAttribsSizer->SetItemMinSize(m_xOffsetEditor, 50, m_xOffsetEditor->GetSize().y);
            m_faceAttribsSizer->SetItemMinSize(m_yOffsetEditor, 50, m_yOffsetEditor->GetSize().y);
            m_faceAttribsSizer->SetItemMinSize(m_xScaleEditor, 50, m_xScaleEditor->GetSize().y);
            m_faceAttribsSizer->SetItemMinSize(m_yScaleEditor, 50, m_yScaleEditor->GetSize().y);
            m_faceAttribsSizer->SetItemMinSize(m_rotationEditor, 50, m_rotationEditor->GetSize().y);
            m_faceAttribsSizer->SetItemMinSize(m_surfaceValueEditor, 50, m_rotationEditor->GetSize().y);
            
            SetSizer(m_faceAttribsSizer);
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
            m_surfaceValueEditor->Bind(EVT_SPINCONTROL_EVENT,
                                       EVT_SPINCONTROL_HANDLER(FaceAttribsEditor::OnSurfaceValueChanged),
                                       this);
            m_surfaceFlagsEditor->Bind(EVT_FLAG_CHANGED_EVENT,
                                       EVT_FLAG_CHANGED_HANDLER(FaceAttribsEditor::OnSurfaceFlagChanged),
                                       this);
            m_contentFlagsEditor->Bind(EVT_FLAG_CHANGED_EVENT,
                                       EVT_FLAG_CHANGED_HANDLER(FaceAttribsEditor::OnContentFlagChanged),
                                       this);
            Bind(wxEVT_IDLE, &FaceAttribsEditor::OnIdle, this);
        }
        
        void FaceAttribsEditor::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->documentWasNewedNotifier.addObserver(this, &FaceAttribsEditor::documentWasNewed);
            document->documentWasLoadedNotifier.addObserver(this, &FaceAttribsEditor::documentWasLoaded);
            document->faceDidChangeNotifier.addObserver(this, &FaceAttribsEditor::faceDidChange);
            document->selectionDidChangeNotifier.addObserver(this, &FaceAttribsEditor::selectionDidChange);
            document->textureCollectionsDidChangeNotifier.addObserver(this, &FaceAttribsEditor::textureCollectionsDidChange);
        }
        
        void FaceAttribsEditor::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->documentWasNewedNotifier.removeObserver(this, &FaceAttribsEditor::documentWasNewed);
                document->documentWasLoadedNotifier.removeObserver(this, &FaceAttribsEditor::documentWasLoaded);
                document->faceDidChangeNotifier.removeObserver(this, &FaceAttribsEditor::faceDidChange);
                document->selectionDidChangeNotifier.removeObserver(this, &FaceAttribsEditor::selectionDidChange);
                document->textureCollectionsDidChangeNotifier.removeObserver(this, &FaceAttribsEditor::textureCollectionsDidChange);
            }
        }
        
        void FaceAttribsEditor::documentWasNewed() {
            MapDocumentSPtr document = lock(m_document);
            m_faces = document->allSelectedFaces();
            updateControls();
        }
        
        void FaceAttribsEditor::documentWasLoaded() {
            MapDocumentSPtr document = lock(m_document);
            m_faces = document->allSelectedFaces();
            updateControls();
        }
        
        void FaceAttribsEditor::faceDidChange(Model::BrushFace* face) {
            updateControls();
        }
        
        void FaceAttribsEditor::selectionDidChange(const Model::SelectionResult& result) {
            MapDocumentSPtr document = lock(m_document);
            m_faces = document->allSelectedFaces();
            updateControls();
        }
        
        void FaceAttribsEditor::textureCollectionsDidChange() {
            updateControls();
        }
        
        void FaceAttribsEditor::updateControls() {
            if (hasSurfaceAttribs()) {
                showSurfaceAttribEditors();
                wxArrayString surfaceFlagLabels, surfaceFlagTooltips, contentFlagLabels, contentFlagTooltips;
                getSurfaceFlags(surfaceFlagLabels, surfaceFlagTooltips);
                getContentFlags(contentFlagLabels, contentFlagTooltips);
                m_surfaceFlagsEditor->setFlags(surfaceFlagLabels, surfaceFlagTooltips);
                m_contentFlagsEditor->setFlags(contentFlagLabels, contentFlagTooltips);
            } else {
                hideSurfaceAttribEditors();
            }
            
            if (!m_faces.empty()) {
                bool textureMulti = false;
                bool xOffsetMulti = false;
                bool yOffsetMulti = false;
                bool rotationMulti = false;
                bool xScaleMulti = false;
                bool yScaleMulti = false;
                bool surfaceValueMulti = false;
                
                Assets::Texture* texture = m_faces[0]->texture();
                const String& textureName = m_faces[0]->textureName();
                const float xOffset = m_faces[0]->xOffset();
                const float yOffset = m_faces[0]->yOffset();
                const float rotation = m_faces[0]->rotation();
                const float xScale = m_faces[0]->xScale();
                const float yScale = m_faces[0]->yScale();
                int setSurfaceFlags = m_faces[0]->surfaceFlags();
                int setSurfaceContents = m_faces[0]->surfaceContents();
                int mixedSurfaceFlags = 0;
                int mixedSurfaceContents = 0;
                const float surfaceValue = m_faces[0]->surfaceValue();
                
                for (size_t i = 1; i < m_faces.size(); i++) {
                    Model::BrushFace* face = m_faces[i];
                    textureMulti            |= (texture         != face->texture());
                    xOffsetMulti            |= (xOffset         != face->xOffset());
                    yOffsetMulti            |= (yOffset         != face->yOffset());
                    rotationMulti           |= (rotation        != face->rotation());
                    xScaleMulti             |= (xScale          != face->xScale());
                    yScaleMulti             |= (yScale          != face->yScale());
                    surfaceValueMulti       |= (surfaceValue    != face->surfaceValue());
                    
                    combineFlags(sizeof(int)*8, face->surfaceFlags(), setSurfaceFlags, mixedSurfaceFlags);
                    combineFlags(sizeof(int)*8, face->surfaceContents(), setSurfaceContents, mixedSurfaceContents);
                }
                
                m_xOffsetEditor->Enable();
                m_yOffsetEditor->Enable();
                m_rotationEditor->Enable();
                m_xScaleEditor->Enable();
                m_yScaleEditor->Enable();
                m_surfaceValueEditor->Enable();
                m_surfaceFlagsEditor->Enable();
                m_contentFlagsEditor->Enable();
                
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
                if (surfaceValueMulti) {
                    m_surfaceValueEditor->SetHint(_("multi"));
                    m_surfaceValueEditor->SetValue(_(""));
                } else {
                    m_surfaceValueEditor->SetHint(_(""));
                    m_surfaceValueEditor->SetValue(surfaceValue);
                }
                m_surfaceFlagsEditor->setFlagValue(setSurfaceFlags, mixedSurfaceFlags);
                m_contentFlagsEditor->setFlagValue(setSurfaceContents, mixedSurfaceContents);
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
                m_surfaceValueEditor->SetValue(_("n/a"));
                m_surfaceValueEditor->Disable();
                m_textureView->setTexture(NULL);
                m_textureNameLabel->SetLabel("n/a");
                m_surfaceFlagsEditor->Disable();
                m_contentFlagsEditor->Disable();
            }
            Layout();
        }

        
        bool FaceAttribsEditor::hasSurfaceAttribs() const {
            MapDocumentSPtr document = lock(m_document);
            const Model::GamePtr game = document->game();
            const Model::GameConfig::FlagsConfig& surfaceFlags = game->surfaceFlags();
            const Model::GameConfig::FlagsConfig& contentFlags = game->contentFlags();
            
            return !surfaceFlags.flags.empty() && !contentFlags.flags.empty();
        }
        
        void FaceAttribsEditor::showSurfaceAttribEditors() {
            m_faceAttribsSizer->Show(m_surfaceValueLabel);
            m_faceAttribsSizer->Show(m_surfaceValueEditor);
            m_faceAttribsSizer->Show(m_surfaceFlagsLabel);
            m_faceAttribsSizer->Show(m_surfaceFlagsEditor);
            m_faceAttribsSizer->Show(m_contentFlagsLabel);
            m_faceAttribsSizer->Show(m_contentFlagsEditor);
            GetParent()->Layout();
        }
        
        void FaceAttribsEditor::hideSurfaceAttribEditors() {
            m_faceAttribsSizer->Hide(m_surfaceValueLabel);
            m_faceAttribsSizer->Hide(m_surfaceValueEditor);
            m_faceAttribsSizer->Hide(m_surfaceFlagsLabel);
            m_faceAttribsSizer->Hide(m_surfaceFlagsEditor);
            m_faceAttribsSizer->Hide(m_contentFlagsLabel);
            m_faceAttribsSizer->Hide(m_contentFlagsEditor);
            GetParent()->Layout();
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
            MapDocumentSPtr document = lock(m_document);
            const Model::GamePtr game = document->game();
            const Model::GameConfig::FlagsConfig& surfaceFlags = game->surfaceFlags();
            getFlags(surfaceFlags.flags, names, descriptions);
        }
        
        void FaceAttribsEditor::getContentFlags(wxArrayString& names, wxArrayString& descriptions) const {
            MapDocumentSPtr document = lock(m_document);
            const Model::GamePtr game = document->game();
            const Model::GameConfig::FlagsConfig& contentFlags = game->contentFlags();
            getFlags(contentFlags.flags, names, descriptions);
        }
    }
}
