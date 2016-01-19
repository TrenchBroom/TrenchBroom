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

#include "FaceAttribsEditor.h"

#include "Assets/AssetTypes.h"
#include "Assets/Texture.h"
#include "IO/Path.h"
#include "IO/ResourceUtils.h"
#include "Model/BrushFace.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "Model/Game.h"
#include "Model/GameConfig.h"
#include "View/BorderLine.h"
#include "View/FlagChangedCommand.h"
#include "View/FlagsPopupEditor.h"
#include "View/Grid.h"
#include "View/ViewConstants.h"
#include "View/MapDocument.h"
#include "View/SpinControl.h"
#include "View/UVEditor.h"
#include "View/ViewUtils.h"

#include <wx/bitmap.h>
#include <wx/button.h>
#include <wx/gbsizer.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

namespace TrenchBroom {
    namespace View {
        FaceAttribsEditor::FaceAttribsEditor(wxWindow* parent, MapDocumentWPtr document, GLContextManager& contextManager) :
        wxPanel(parent),
        m_document(document),
        m_uvEditor(NULL),
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
            createGui(contextManager);
            bindEvents();
            bindObservers();
        }

        FaceAttribsEditor::~FaceAttribsEditor() {
            unbindObservers();
        }

        void FaceAttribsEditor::OnXOffsetChanged(SpinControlEvent& event) {
            if (IsBeingDeleted()) return;

            Model::ChangeBrushFaceAttributesRequest request;
            if (event.IsSpin())
                request.addXOffset(static_cast<float>(event.GetValue()));
            else
                request.setXOffset(static_cast<float>(event.GetValue()));
            
            MapDocumentSPtr document = lock(m_document);
            if (!document->setFaceAttributes(request))
                event.Veto();
        }
        
        void FaceAttribsEditor::OnYOffsetChanged(SpinControlEvent& event) {
            if (IsBeingDeleted()) return;

            Model::ChangeBrushFaceAttributesRequest request;
            if (event.IsSpin())
                request.addYOffset(static_cast<float>(event.GetValue()));
            else
                request.setYOffset(static_cast<float>(event.GetValue()));
            
            MapDocumentSPtr document = lock(m_document);
            if (!document->setFaceAttributes(request))
                event.Veto();
        }
        
        void FaceAttribsEditor::OnRotationChanged(SpinControlEvent& event) {
            if (IsBeingDeleted()) return;

            Model::ChangeBrushFaceAttributesRequest request;
            if (event.IsSpin())
                request.addRotation(static_cast<float>(event.GetValue()));
            else
                request.setRotation(static_cast<float>(event.GetValue()));
            
            MapDocumentSPtr document = lock(m_document);
            if (!document->setFaceAttributes(request))
                event.Veto();
        }
        
        void FaceAttribsEditor::OnXScaleChanged(SpinControlEvent& event) {
            if (IsBeingDeleted()) return;

            Model::ChangeBrushFaceAttributesRequest request;
            if (event.IsSpin())
                request.addXScale(static_cast<float>(event.GetValue()));
            else
                request.setXScale(static_cast<float>(event.GetValue()));
            
            MapDocumentSPtr document = lock(m_document);
            if (!document->setFaceAttributes(request))
                event.Veto();
        }

        void FaceAttribsEditor::OnYScaleChanged(SpinControlEvent& event) {
            if (IsBeingDeleted()) return;

            Model::ChangeBrushFaceAttributesRequest request;
            if (event.IsSpin())
                request.addYScale(static_cast<float>(event.GetValue()));
            else
                request.setYScale(static_cast<float>(event.GetValue()));
            
            MapDocumentSPtr document = lock(m_document);
            if (!document->setFaceAttributes(request))
                event.Veto();
        }
        
        void FaceAttribsEditor::OnSurfaceFlagChanged(FlagChangedCommand& command) {
            if (IsBeingDeleted()) return;

            Model::ChangeBrushFaceAttributesRequest request;
            if (command.flagSet())
                request.setSurfaceFlag(command.index());
            else
                request.unsetSurfaceFlag(command.index());
            
            MapDocumentSPtr document = lock(m_document);
            if (!document->setFaceAttributes(request))
                command.Veto();
        }
        
        void FaceAttribsEditor::OnContentFlagChanged(FlagChangedCommand& command) {
            if (IsBeingDeleted()) return;

            Model::ChangeBrushFaceAttributesRequest request;
            if (command.flagSet())
                request.setContentFlag(command.index());
            else
                request.unsetContentFlag(command.index());
            
            MapDocumentSPtr document = lock(m_document);
            if (!document->setFaceAttributes(request))
                command.Veto();
        }

        void FaceAttribsEditor::OnSurfaceValueChanged(SpinControlEvent& event) {
            if (IsBeingDeleted()) return;

            Model::ChangeBrushFaceAttributesRequest request;
            if (event.IsSpin())
                request.addSurfaceValue(static_cast<float>(event.GetValue()));
            else
                request.setSurfaceValue(static_cast<float>(event.GetValue()));
            
            MapDocumentSPtr document = lock(m_document);
            if (!document->setFaceAttributes(request))
                event.Veto();
        }

        void FaceAttribsEditor::OnIdle(wxIdleEvent& event) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);
            Grid& grid = document->grid();
            
            m_xOffsetEditor->SetIncrements(grid.actualSize(), 2.0 * grid.actualSize(), 1.0);
            m_yOffsetEditor->SetIncrements(grid.actualSize(), 2.0 * grid.actualSize(), 1.0);
            m_rotationEditor->SetIncrements(Math::degrees(grid.angle()), 90.0, 1.0);
        }

        void FaceAttribsEditor::createGui(GLContextManager& contextManager) {
            m_uvEditor = new UVEditor(this, m_document, contextManager);
            
            wxStaticText* textureNameLabel = new wxStaticText(this, wxID_ANY, "Texture");
            textureNameLabel->SetFont(textureNameLabel->GetFont().Bold());
            m_textureName = new wxStaticText(this, wxID_ANY, "none");
            
            wxStaticText* textureSizeLabel = new wxStaticText(this, wxID_ANY, "Size");
            textureSizeLabel->SetFont(textureSizeLabel->GetFont().Bold());
            m_textureSize = new wxStaticText(this, wxID_ANY, "");
            
            const double max = std::numeric_limits<double>::max();
            const double min = -max;
            
            wxStaticText* xOffsetLabel = new wxStaticText(this, wxID_ANY, "X Offset");
            xOffsetLabel->SetFont(xOffsetLabel->GetFont().Bold());
            m_xOffsetEditor = new SpinControl(this);
            m_xOffsetEditor->SetRange(min, max);
            m_xOffsetEditor->SetDigits(0, 6);
            
            wxStaticText* yOffsetLabel = new wxStaticText(this, wxID_ANY, "Y Offset");
            yOffsetLabel->SetFont(yOffsetLabel->GetFont().Bold());
            m_yOffsetEditor = new SpinControl(this);
            m_yOffsetEditor->SetRange(min, max);
            m_yOffsetEditor->SetDigits(0, 6);
            
            wxStaticText* xScaleLabel = new wxStaticText(this, wxID_ANY, "X Scale");
            xScaleLabel->SetFont(xScaleLabel->GetFont().Bold());
            m_xScaleEditor = new SpinControl(this);
            m_xScaleEditor->SetRange(min, max);
            m_xScaleEditor->SetIncrements(0.1, 0.25, 0.01);
            m_xScaleEditor->SetDigits(0, 6);
            
            wxStaticText* yScaleLabel = new wxStaticText(this, wxID_ANY, "Y Scale");
            yScaleLabel->SetFont(yScaleLabel->GetFont().Bold());
            m_yScaleEditor = new SpinControl(this);
            m_yScaleEditor->SetRange(min, max);
            m_yScaleEditor->SetIncrements(0.1, 0.25, 0.01);
            m_yScaleEditor->SetDigits(0, 6);
            
            wxStaticText* rotationLabel = new wxStaticText(this, wxID_ANY, "Angle");
            rotationLabel->SetFont(rotationLabel->GetFont().Bold());
            m_rotationEditor = new SpinControl(this);
            m_rotationEditor->SetRange(min, max);
            m_rotationEditor->SetDigits(0, 6);
            
            m_surfaceValueLabel = new wxStaticText(this, wxID_ANY, "Value", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
            m_surfaceValueLabel->SetFont(m_surfaceValueLabel->GetFont().Bold());
            m_surfaceValueEditor = new SpinControl(this);
            m_surfaceValueEditor->SetRange(min, max);
            m_surfaceValueEditor->SetIncrements(1.0, 10.0, 100.0);
            m_surfaceValueEditor->SetDigits(0, 6);
            
            m_surfaceFlagsLabel = new wxStaticText(this, wxID_ANY, "Surface", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
            m_surfaceFlagsLabel->SetFont(m_surfaceFlagsLabel->GetFont().Bold());
            m_surfaceFlagsEditor = new FlagsPopupEditor(this, 2);
            
            m_contentFlagsLabel = new wxStaticText(this, wxID_ANY, "Content", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
            m_contentFlagsLabel->SetFont(m_contentFlagsLabel->GetFont().Bold());
            m_contentFlagsEditor = new FlagsPopupEditor(this, 2);
            
            const int LabelMargin  = LayoutConstants::NarrowHMargin;
            const int EditorMargin = LayoutConstants::WideHMargin;
            const int RowMargin    = LayoutConstants::NarrowVMargin;
            
            const int LabelFlags   = wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxRIGHT;
            const int ValueFlags   = wxALIGN_CENTER_VERTICAL | wxRIGHT;
            const int Editor1Flags = wxEXPAND | wxRIGHT;
            const int Editor2Flags = wxEXPAND;
            
            int r = 0;
            int c = 0;
            
            m_faceAttribsSizer = new wxGridBagSizer(RowMargin);
            m_faceAttribsSizer->Add(textureNameLabel,     wxGBPosition(r,c++), wxDefaultSpan, LabelFlags,   LabelMargin);
            m_faceAttribsSizer->Add(m_textureName,        wxGBPosition(r,c++), wxDefaultSpan, ValueFlags,   EditorMargin);
            m_faceAttribsSizer->Add(textureSizeLabel,     wxGBPosition(r,c++), wxDefaultSpan, LabelFlags,   LabelMargin);
            m_faceAttribsSizer->Add(m_textureSize,        wxGBPosition(r,c++), wxDefaultSpan, ValueFlags,   EditorMargin);
            ++r, c = 0;

            m_faceAttribsSizer->Add(xOffsetLabel,         wxGBPosition(r,c++), wxDefaultSpan, LabelFlags,   LabelMargin);
            m_faceAttribsSizer->Add(m_xOffsetEditor,      wxGBPosition(r,c++), wxDefaultSpan, Editor1Flags, EditorMargin);
            m_faceAttribsSizer->Add(yOffsetLabel,         wxGBPosition(r,c++), wxDefaultSpan, LabelFlags,   LabelMargin);
            m_faceAttribsSizer->Add(m_yOffsetEditor,      wxGBPosition(r,c++), wxDefaultSpan, Editor2Flags, EditorMargin);
            ++r; c = 0;
            
            m_faceAttribsSizer->Add(xScaleLabel,          wxGBPosition(r,c++), wxDefaultSpan, LabelFlags,   LabelMargin);
            m_faceAttribsSizer->Add(m_xScaleEditor,       wxGBPosition(r,c++), wxDefaultSpan, Editor1Flags, EditorMargin);
            m_faceAttribsSizer->Add(yScaleLabel,          wxGBPosition(r,c++), wxDefaultSpan, LabelFlags,   LabelMargin);
            m_faceAttribsSizer->Add(m_yScaleEditor,       wxGBPosition(r,c++), wxDefaultSpan, Editor2Flags, EditorMargin);
            ++r; c = 0;
            
            m_faceAttribsSizer->Add(rotationLabel,        wxGBPosition(r,c++), wxDefaultSpan, LabelFlags,   LabelMargin);
            m_faceAttribsSizer->Add(m_rotationEditor,     wxGBPosition(r,c++), wxDefaultSpan, Editor1Flags, EditorMargin);
            m_faceAttribsSizer->Add(m_surfaceValueLabel,  wxGBPosition(r,c++), wxDefaultSpan, LabelFlags,   LabelMargin);
            m_faceAttribsSizer->Add(m_surfaceValueEditor, wxGBPosition(r,c++), wxDefaultSpan, Editor2Flags, EditorMargin);
            ++r; c = 0;
            
            m_faceAttribsSizer->Add(m_surfaceFlagsLabel,  wxGBPosition(r,c++), wxDefaultSpan, LabelFlags,   LabelMargin);
            m_faceAttribsSizer->Add(m_surfaceFlagsEditor, wxGBPosition(r,c++), wxGBSpan(1,3), Editor2Flags, EditorMargin);
            ++r; c = 0;
            
            m_faceAttribsSizer->Add(m_contentFlagsLabel,  wxGBPosition(r,c++), wxDefaultSpan, LabelFlags,   LabelMargin);
            m_faceAttribsSizer->Add(m_contentFlagsEditor, wxGBPosition(r,c++), wxGBSpan(1,3), Editor2Flags, EditorMargin);
            ++r; c = 0;
            
            m_faceAttribsSizer->AddGrowableCol(1);
            m_faceAttribsSizer->AddGrowableCol(3);
            m_faceAttribsSizer->SetItemMinSize(m_uvEditor, 100, 100);
            m_faceAttribsSizer->SetItemMinSize(m_xOffsetEditor, 50, m_xOffsetEditor->GetSize().y);
            m_faceAttribsSizer->SetItemMinSize(m_yOffsetEditor, 50, m_yOffsetEditor->GetSize().y);
            m_faceAttribsSizer->SetItemMinSize(m_xScaleEditor, 50, m_xScaleEditor->GetSize().y);
            m_faceAttribsSizer->SetItemMinSize(m_yScaleEditor, 50, m_yScaleEditor->GetSize().y);
            m_faceAttribsSizer->SetItemMinSize(m_rotationEditor, 50, m_rotationEditor->GetSize().y);
            m_faceAttribsSizer->SetItemMinSize(m_surfaceValueEditor, 50, m_rotationEditor->GetSize().y);
            
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(m_uvEditor, 1, wxEXPAND);
            outerSizer->Add(new BorderLine(this, BorderLine::Direction_Horizontal), 0, wxEXPAND);
            outerSizer->AddSpacer(LayoutConstants::WideVMargin);
            outerSizer->Add(m_faceAttribsSizer, 0, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::MediumHMargin);
            outerSizer->AddSpacer(LayoutConstants::WideVMargin);
            
            SetSizer(outerSizer);
        }
        
        void FaceAttribsEditor::bindEvents() {
            m_xOffsetEditor->Bind(SPIN_CONTROL_EVENT, &FaceAttribsEditor::OnXOffsetChanged, this);
            m_yOffsetEditor->Bind(SPIN_CONTROL_EVENT, &FaceAttribsEditor::OnYOffsetChanged, this);
            m_xScaleEditor->Bind(SPIN_CONTROL_EVENT, &FaceAttribsEditor::OnXScaleChanged, this);
            m_yScaleEditor->Bind(SPIN_CONTROL_EVENT, &FaceAttribsEditor::OnYScaleChanged, this);
            m_rotationEditor->Bind(SPIN_CONTROL_EVENT, &FaceAttribsEditor::OnRotationChanged, this);
            m_surfaceValueEditor->Bind(SPIN_CONTROL_EVENT, &FaceAttribsEditor::OnSurfaceValueChanged, this);
            m_surfaceFlagsEditor->Bind(FLAG_CHANGED_EVENT, &FaceAttribsEditor::OnSurfaceFlagChanged, this);
            m_contentFlagsEditor->Bind(FLAG_CHANGED_EVENT, &FaceAttribsEditor::OnContentFlagChanged, this);
            Bind(wxEVT_IDLE, &FaceAttribsEditor::OnIdle, this);
        }
        
        void FaceAttribsEditor::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->documentWasNewedNotifier.addObserver(this, &FaceAttribsEditor::documentWasNewed);
            document->documentWasLoadedNotifier.addObserver(this, &FaceAttribsEditor::documentWasLoaded);
            document->brushFacesDidChangeNotifier.addObserver(this, &FaceAttribsEditor::brushFacesDidChange);
            document->selectionDidChangeNotifier.addObserver(this, &FaceAttribsEditor::selectionDidChange);
            document->textureCollectionsDidChangeNotifier.addObserver(this, &FaceAttribsEditor::textureCollectionsDidChange);
        }
        
        void FaceAttribsEditor::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->documentWasNewedNotifier.removeObserver(this, &FaceAttribsEditor::documentWasNewed);
                document->documentWasLoadedNotifier.removeObserver(this, &FaceAttribsEditor::documentWasLoaded);
                document->brushFacesDidChangeNotifier.removeObserver(this, &FaceAttribsEditor::brushFacesDidChange);
                document->selectionDidChangeNotifier.removeObserver(this, &FaceAttribsEditor::selectionDidChange);
                document->textureCollectionsDidChangeNotifier.removeObserver(this, &FaceAttribsEditor::textureCollectionsDidChange);
            }
        }
        
        void FaceAttribsEditor::documentWasNewed(MapDocument* document) {
            m_faces = document->allSelectedBrushFaces();
            updateControls();
        }
        
        void FaceAttribsEditor::documentWasLoaded(MapDocument* document) {
            m_faces = document->allSelectedBrushFaces();
            updateControls();
        }
        
        void FaceAttribsEditor::brushFacesDidChange(const Model::BrushFaceList& faces) {
            MapDocumentSPtr document = lock(m_document);
            m_faces = document->allSelectedBrushFaces();
            updateControls();
        }
        
        void FaceAttribsEditor::selectionDidChange(const Selection& selection) {
            MapDocumentSPtr document = lock(m_document);
            m_faces = document->allSelectedBrushFaces();
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
                    m_textureName->SetLabel("multi");
                    m_textureName->SetForegroundColour(*wxLIGHT_GREY);
                    m_textureSize->SetLabel("multi");
                    m_textureSize->SetForegroundColour(*wxLIGHT_GREY);
                } else {
                    const String& textureName = m_faces[0]->textureName();
                    if (textureName == Model::BrushFace::NoTextureName) {
                        m_textureName->SetLabel("none");
                        m_textureName->SetForegroundColour(*wxLIGHT_GREY);
                        m_textureSize->SetLabel("");
                        m_textureSize->SetForegroundColour(*wxLIGHT_GREY);
                    } else {
                        if (texture != NULL) {
                            wxString sizeLabel;
                            sizeLabel << texture->width() << "*" << texture->height();

                            m_textureName->SetLabel(textureName);
                            m_textureSize->SetLabel(sizeLabel);
                            m_textureName->SetForegroundColour(GetForegroundColour());
                            m_textureSize->SetForegroundColour(GetForegroundColour());
                        } else {
                            m_textureName->SetLabel(textureName + " (not found)");
                            m_textureName->SetForegroundColour(*wxLIGHT_GREY);
                            m_textureSize->SetForegroundColour(*wxLIGHT_GREY);
                        }
                    }
                }
                if (xOffsetMulti) {
                    m_xOffsetEditor->SetHint("multi");
                    m_xOffsetEditor->SetValue("");
                } else {
                    m_xOffsetEditor->SetHint("");
                    m_xOffsetEditor->SetValue(xOffset);
                }
                if (yOffsetMulti) {
                    m_yOffsetEditor->SetHint("multi");
                    m_yOffsetEditor->SetValue("");
                } else {
                    m_yOffsetEditor->SetHint("");
                    m_yOffsetEditor->SetValue(yOffset);
                }
                if (rotationMulti) {
                    m_rotationEditor->SetHint("multi");
                    m_rotationEditor->SetValue("");
                } else {
                    m_rotationEditor->SetHint("");
                    m_rotationEditor->SetValue(rotation);
                }
                if (xScaleMulti){
                    m_xScaleEditor->SetHint("multi");
                    m_xScaleEditor->SetValue("");
                } else {
                    m_xScaleEditor->SetHint("");
                    m_xScaleEditor->SetValue(xScale);
                }
                if (yScaleMulti) {
                    m_yScaleEditor->SetHint("multi");
                    m_yScaleEditor->SetValue("");
                } else {
                    m_yScaleEditor->SetHint("");
                    m_yScaleEditor->SetValue(yScale);
                }
                if (surfaceValueMulti) {
                    m_surfaceValueEditor->SetHint("multi");
                    m_surfaceValueEditor->SetValue("");
                } else {
                    m_surfaceValueEditor->SetHint("");
                    m_surfaceValueEditor->SetValue(surfaceValue);
                }
                m_surfaceFlagsEditor->setFlagValue(setSurfaceFlags, mixedSurfaceFlags);
                m_contentFlagsEditor->setFlagValue(setSurfaceContents, mixedSurfaceContents);
            } else {
                m_xOffsetEditor->SetValue("n/a");
                m_xOffsetEditor->Disable();
                m_yOffsetEditor->SetValue("n/a");
                m_yOffsetEditor->Disable();
                m_xScaleEditor->SetValue("n/a");
                m_xScaleEditor->Disable();
                m_yScaleEditor->SetValue("n/a");
                m_yScaleEditor->Disable();
                m_rotationEditor->SetValue("n/a");
                m_rotationEditor->Disable();
                m_surfaceValueEditor->SetValue("n/a");
                m_surfaceValueEditor->Disable();
                // m_textureView->setTexture(NULL);
                m_surfaceFlagsEditor->Disable();
                m_contentFlagsEditor->Disable();
            }
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

        void getFlags(const Model::GameConfig::FlagConfigList& flags, wxArrayString& names, wxArrayString& descriptions);
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
