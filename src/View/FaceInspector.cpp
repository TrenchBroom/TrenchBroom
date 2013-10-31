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

#include "Controller/EntityPropertyCommand.h"
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
        FaceInspector::FaceInspector(wxWindow* parent, MapDocumentPtr document, ControllerPtr controller, Renderer::RenderResources& resources) :
        wxPanel(parent),
        m_document(document),
        m_controller(controller) {
            m_faceAttribsEditor = new FaceAttribsEditor(this, resources, m_document, m_controller);
            m_textureBrowser = new TextureBrowser(this, resources, m_document);
            
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(m_faceAttribsEditor, 0, wxEXPAND | wxLEFT | wxTOP | wxRIGHT, LayoutConstants::NotebookPageInnerMargin);
            outerSizer->AddSpacer(LayoutConstants::ControlHorizontalMargin);
            outerSizer->Add(m_textureBrowser, 1, wxEXPAND | wxLEFT | wxBOTTOM | wxRIGHT, LayoutConstants::NotebookPageInnerMargin);
            
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
            if (!m_controller->setTexture(m_document->allSelectedFaces(), event.texture()))
                event.Veto();
        }

        void FaceInspector::bindObservers() {
            m_controller->commandDoneNotifier.addObserver(this, &FaceInspector::commandDoneOrUndone);
            m_controller->commandUndoneNotifier.addObserver(this, &FaceInspector::commandDoneOrUndone);
            m_document->documentWasNewedNotifier.addObserver(this, &FaceInspector::documentWasNewedOrLoaded);
            m_document->documentWasLoadedNotifier.addObserver(this, &FaceInspector::documentWasNewedOrLoaded);
            m_document->faceDidChangeNotifier.addObserver(this, &FaceInspector::faceDidChange);
            m_document->selectionDidChangeNotifier.addObserver(this, &FaceInspector::selectionDidChange);
        }
        
        void FaceInspector::unbindObservers() {
            m_controller->commandDoneNotifier.removeObserver(this, &FaceInspector::commandDoneOrUndone);
            m_controller->commandUndoneNotifier.removeObserver(this, &FaceInspector::commandDoneOrUndone);
            m_document->documentWasNewedNotifier.removeObserver(this, &FaceInspector::documentWasNewedOrLoaded);
            m_document->documentWasLoadedNotifier.removeObserver(this, &FaceInspector::documentWasNewedOrLoaded);
            m_document->faceDidChangeNotifier.removeObserver(this, &FaceInspector::faceDidChange);
            m_document->selectionDidChangeNotifier.removeObserver(this, &FaceInspector::selectionDidChange);
        }

        void FaceInspector::commandDoneOrUndone(Controller::Command::Ptr command) {
            using namespace Controller;
            if (command->type() == EntityPropertyCommand::Type) {
                const EntityPropertyCommand::Ptr propCmd = command->cast<EntityPropertyCommand>(command);
                if (propCmd->entityAffected(m_document->worldspawn()) &&
                    (propCmd->propertyAffected(Model::PropertyKeys::Wad) ||
                     propCmd->propertyAffected(Model::PropertyKeys::Wal)))
                    m_textureBrowser->reload();
            }
        }

        void FaceInspector::documentWasNewedOrLoaded() {
            m_faceAttribsEditor->updateFaces(m_document->allSelectedFaces());
            m_textureBrowser->reload();
        }

        void FaceInspector::faceDidChange(Model::BrushFace* face) {
            m_faceAttribsEditor->updateFaces(m_document->allSelectedFaces());
        }
        
        void FaceInspector::selectionDidChange(const Model::SelectionResult& result) {
            m_faceAttribsEditor->updateFaces(m_document->allSelectedFaces());
        }
    }
}
