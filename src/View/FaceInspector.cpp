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

#include "FaceInspector.h"

#include "Model/Entity.h"
#include "Model/Object.h"
#include "View/ControllerFacade.h"
#include "View/FaceAttribsEditor.h"
#include "View/LayoutConstants.h"
#include "View/MapDocument.h"
#include "View/TextureBrowser.h"
#include "View/TextureSelectedCommand.h"

#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        FaceInspector::FaceInspector(wxWindow* parent, MapDocumentPtr document, ControllerFacade& controller, Renderer::RenderResources& resources) :
        wxPanel(parent),
        m_document(document),
        m_controller(controller) {
            m_faceAttribsEditor = new FaceAttribsEditor(this, resources, m_document, m_controller);
            m_textureBrowser = new TextureBrowser(this, resources, m_document);
            
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(m_faceAttribsEditor, 0, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::NotebookPageInnerMargin);
            outerSizer->AddSpacer(LayoutConstants::ControlHorizontalMargin);
            outerSizer->Add(m_textureBrowser, 1, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::NotebookPageInnerMargin);
            
            SetSizer(outerSizer);
            m_textureBrowser->Bind(EVT_TEXTURE_SELECTED_EVENT,
                                   EVT_TEXTURE_SELECTED_HANDLER(FaceInspector::OnTextureSelected),
                                   this);
            
            bindObservers();
        }
        
        FaceInspector::~FaceInspector() {
            unbindObservers();
        }

        void FaceInspector::OnTextureSelected(TextureSelectedCommand& event) {
            if (!m_controller.setFaceTexture(m_document->allSelectedFaces(), event.texture()))
                event.Veto();
        }

        void FaceInspector::bindObservers() {
            m_document->documentWasNewedNotifier.addObserver(this, &FaceInspector::documentWasNewed);
            m_document->documentWasLoadedNotifier.addObserver(this, &FaceInspector::documentWasLoaded);
            m_document->objectDidChangeNotifier.addObserver(this, &FaceInspector::objectDidChange);
            m_document->faceDidChangeNotifier.addObserver(this, &FaceInspector::faceDidChange);
            m_document->selectionDidChangeNotifier.addObserver(this, &FaceInspector::selectionDidChange);
        }
        
        void FaceInspector::unbindObservers() {
            m_document->documentWasNewedNotifier.removeObserver(this, &FaceInspector::documentWasNewed);
            m_document->documentWasLoadedNotifier.removeObserver(this, &FaceInspector::documentWasLoaded);
            m_document->objectDidChangeNotifier.removeObserver(this, &FaceInspector::objectDidChange);
            m_document->faceDidChangeNotifier.removeObserver(this, &FaceInspector::faceDidChange);
            m_document->selectionDidChangeNotifier.removeObserver(this, &FaceInspector::selectionDidChange);
        }

        void FaceInspector::documentWasNewed() {
            m_faceAttribsEditor->updateFaces(m_document->allSelectedFaces());
            m_textureBrowser->reload();
        }
        
        void FaceInspector::documentWasLoaded() {
            m_faceAttribsEditor->updateFaces(m_document->allSelectedFaces());
            m_textureBrowser->reload();
        }
        
        void FaceInspector::objectDidChange(Model::Object* object) {
            if (object->type() == Model::Object::OTEntity) {
                const Model::Entity* entity = static_cast<Model::Entity*>(object);
                if (entity->worldspawn())
                    m_textureBrowser->reload();
            }
        }

        void FaceInspector::faceDidChange(Model::BrushFace* face) {
            m_faceAttribsEditor->updateFaces(m_document->allSelectedFaces());
        }
        
        void FaceInspector::selectionDidChange(const Model::SelectionResult& result) {
            m_faceAttribsEditor->updateFaces(m_document->allSelectedFaces());
        }
    }
}
