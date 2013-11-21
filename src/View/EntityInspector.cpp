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

#include "EntityInspector.h"

#include "StringUtils.h"
#include "Controller/EntityPropertyCommand.h"
#include "Model/Entity.h"
#include "Model/EntityProperties.h"
#include "Model/Object.h"
#include "View/CommandIds.h"
#include "View/ControllerFacade.h"
#include "View/EntityBrowser.h"
#include "View/EntityDefinitionFileChooser.h"
#include "View/EntityPropertyEditor.h"
#include "View/LayoutConstants.h"
#include "View/MapDocument.h"

#include <wx/collpane.h>
#include <wx/event.h>
#include <wx/notebook.h>
#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        EntityInspector::EntityInspector(wxWindow* parent, MapDocumentPtr document, ControllerPtr controller, Renderer::RenderResources& resources) :
        wxPanel(parent),
        m_document(document),
        m_controller(controller) {
            createGui(parent, m_document, m_controller, resources);
            bindObservers();
        }
        
        EntityInspector::~EntityInspector() {
            unbindObservers();
        }

        void EntityInspector::OnEntityDefinitionFileChooserPaneChanged(wxCollapsiblePaneEvent& event) {
            Layout();
        }

        void EntityInspector::createGui(wxWindow* parent, MapDocumentPtr document, ControllerPtr controller, Renderer::RenderResources& resources) {
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(createPropertyEditor(this), 0, wxEXPAND | wxLEFT | wxTOP | wxRIGHT, LayoutConstants::NotebookPageInnerMargin);
            outerSizer->AddSpacer(LayoutConstants::ControlVerticalMargin);
            outerSizer->Add(createEntityBrowser(this, resources), 1, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::NotebookPageInnerMargin);
            outerSizer->AddSpacer(LayoutConstants::ControlVerticalMargin);
            outerSizer->Add(createEntityDefinitionFileChooser(this, document, controller), 0, wxEXPAND | wxLEFT | wxBOTTOM | wxRIGHT, LayoutConstants::NotebookPageInnerMargin);
            SetSizerAndFit(outerSizer);
        }
        
        wxWindow* EntityInspector::createPropertyEditor(wxWindow* parent) {
            m_propertyEditor = new EntityPropertyEditor(parent, m_document, m_controller);
            return m_propertyEditor;
        }
        
        wxWindow* EntityInspector::createEntityBrowser(wxWindow* parent, Renderer::RenderResources& resources) {
            m_entityBrowser = new EntityBrowser(parent, resources, m_document);
            return m_entityBrowser;
        }
        
        wxWindow* EntityInspector::createEntityDefinitionFileChooser(wxWindow* parent, MapDocumentPtr document, ControllerPtr controller) {
            wxCollapsiblePane* collPane = new wxCollapsiblePane(parent, wxID_ANY, _("Entity Definitions"), wxDefaultPosition, wxDefaultSize, wxCP_NO_TLW_RESIZE | wxTAB_TRAVERSAL | wxBORDER_NONE);

#if defined _WIN32
            // this is a hack to prevent the pane having the wrong background color on Windows 7
            wxNotebook* book = static_cast<wxNotebook*>(GetParent());
            wxColour col = book->GetThemeBackgroundColour();
            if (col.IsOk()) {
                collPane->SetBackgroundColour(col);
                collPane->GetPane()->SetBackgroundColour(col);
            }
#endif

            m_entityDefinitionFileChooser = new EntityDefinitionFileChooser(collPane->GetPane(), document, controller);

            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_entityDefinitionFileChooser, 1, wxEXPAND);
            collPane->GetPane()->SetSizerAndFit(sizer);
            
            collPane->Bind(wxEVT_COLLAPSIBLEPANE_CHANGED, &EntityInspector::OnEntityDefinitionFileChooserPaneChanged, this);
            return collPane;
        }

        void EntityInspector::bindObservers() {
            m_controller->commandDoneNotifier.addObserver(this, &EntityInspector::commandDoneOrUndone);
            m_controller->commandUndoneNotifier.addObserver(this, &EntityInspector::commandDoneOrUndone);
            m_document->documentWasNewedNotifier.addObserver(this, &EntityInspector::documentWasNewedOrLoaded);
            m_document->documentWasLoadedNotifier.addObserver(this, &EntityInspector::documentWasNewedOrLoaded);
            m_document->objectDidChangeNotifier.addObserver(this, &EntityInspector::objectDidChange);
            m_document->selectionDidChangeNotifier.addObserver(this, &EntityInspector::selectionDidChange);
        }
        
        void EntityInspector::unbindObservers() {
            m_controller->commandDoneNotifier.removeObserver(this, &EntityInspector::commandDoneOrUndone);
            m_controller->commandUndoneNotifier.removeObserver(this, &EntityInspector::commandDoneOrUndone);
            m_document->documentWasNewedNotifier.removeObserver(this, &EntityInspector::documentWasNewedOrLoaded);
            m_document->documentWasLoadedNotifier.removeObserver(this, &EntityInspector::documentWasNewedOrLoaded);
            m_document->objectDidChangeNotifier.removeObserver(this, &EntityInspector::objectDidChange);
            m_document->selectionDidChangeNotifier.removeObserver(this, &EntityInspector::selectionDidChange);
        }

        void EntityInspector::commandDoneOrUndone(Controller::Command::Ptr command) {
            using namespace Controller;
            if (command->type() == EntityPropertyCommand::Type) {
                const EntityPropertyCommand::Ptr propCmd = command->cast<EntityPropertyCommand>(command);
                if (propCmd->entityAffected(m_document->worldspawn()) &&
                    propCmd->propertyAffected(Model::PropertyKeys::EntityDefinitions))
                    updateEntityBrowser();
            }
        }

        void EntityInspector::documentWasNewedOrLoaded() {
            updatePropertyEditor();
            updateEntityBrowser();
        }
        
        void EntityInspector::objectDidChange(Model::Object* object) {
            if (object->type() == Model::Object::OTEntity)
                updatePropertyEditor();
        }
        
        void EntityInspector::selectionDidChange(const Model::SelectionResult& result) {
            updatePropertyEditor();
        }

        void EntityInspector::updatePropertyEditor() {
            m_propertyEditor->update();
        }
        
        void EntityInspector::updateEntityBrowser() {
            m_entityBrowser->reload();
        }
    }
}
