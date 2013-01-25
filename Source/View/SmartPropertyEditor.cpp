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

#include "SmartPropertyEditor.h"

#include "Model/EditStateManager.h"
#include "Model/MapDocument.h"
#include "View/SpawnFlagsEditor.h"

#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

namespace TrenchBroom {
    namespace View {
        Model::MapDocument& SmartPropertyEditor::document() const {
            return m_manager.document();
        }

        const Model::EntityList SmartPropertyEditor::selectedEntities() const {
            return document().editStateManager().allSelectedEntities();
        }

        SmartPropertyEditor::SmartPropertyEditor(SmartPropertyEditorManager& manager) :
        m_manager(manager),
        m_active(false) {}

        void SmartPropertyEditor::activate(wxWindow* parent) {
            assert(!m_active);
            
            createVisual(parent);
            parent->Layout();
            
            m_active = true;
        }
        
        void SmartPropertyEditor::deactivate() {
            assert(m_active);
            destroyVisual();
            m_active = false;
        }

        void SmartPropertyEditor::update() {
            assert(m_active);
            updateVisual();
        }
        
        wxWindow* DefaultPropertyEditor::createVisual(wxWindow* parent) {
            assert(m_text == NULL);
            m_text = new wxStaticText(parent, wxID_ANY, wxT("No Smart Editor Available"));
            
            wxFont defaultFont = m_text->GetFont();
            wxFont boldFont(defaultFont.GetPointSize() + 2, defaultFont.GetFamily(), defaultFont.GetStyle(), wxFONTWEIGHT_BOLD);
            m_text->SetFont(boldFont);
            m_text->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
            
            wxSizer* outerSizer = new wxBoxSizer(wxHORIZONTAL);
            outerSizer->AddStretchSpacer();
            outerSizer->Add(m_text, 0, wxALIGN_CENTER_VERTICAL);
            outerSizer->AddStretchSpacer();
            parent->SetSizer(outerSizer);

            return m_text;
        }
        
        void DefaultPropertyEditor::destroyVisual() {
            assert(m_text != NULL);
            m_text->Destroy();
            m_text = NULL;
        }

        void DefaultPropertyEditor::updateVisual() {}

        DefaultPropertyEditor::DefaultPropertyEditor(SmartPropertyEditorManager& manager) :
        SmartPropertyEditor(manager),
        m_text(NULL) {}

        void SmartPropertyEditorManager::activateEditor(SmartPropertyEditor* editor) {
            if (m_activeEditor != editor) {
                deactivateEditor();
                m_activeEditor = editor;
                m_activeEditor->activate(m_panel);
            }
            m_activeEditor->update();
        }
        
        void SmartPropertyEditorManager::deactivateEditor() {
            if (m_activeEditor != NULL) {
                m_activeEditor->deactivate();
                m_activeEditor = NULL;
            }
        }

        SmartPropertyEditorManager::SmartPropertyEditorManager(wxWindow* parent, Model::MapDocument& document) :
        m_document(document),
        m_panel(new wxPanel(parent)),
        m_defaultEditor(new DefaultPropertyEditor(*this)),
        m_activeEditor(NULL) {
            m_editors["spawnflags"] = new SpawnFlagsEditor(*this);
            
            m_panel->SetMinSize(wxSize(wxDefaultSize.x, 150));
            
            wxSizer* outerSizer = new wxBoxSizer(wxHORIZONTAL);
            outerSizer->Add(m_panel, 1, wxEXPAND);
            parent->SetSizer(outerSizer);
            
            activateEditor(m_defaultEditor);
        }

        SmartPropertyEditorManager::~SmartPropertyEditorManager() {
            deactivateEditor();
            
            EditorMap::const_iterator it, end;
            for (it = m_editors.begin(), end = m_editors.end(); it != end; ++it)
                delete it->second;
            m_editors.clear();
            
            delete m_defaultEditor;
            m_defaultEditor = NULL;
        }

        void SmartPropertyEditorManager::selectEditor(const Model::PropertyKey& key) {
            EditorMap::const_iterator it = m_editors.find(key);
            SmartPropertyEditor* editor = it == m_editors.end() ? m_defaultEditor : it->second;
            activateEditor(editor);
        }

        void SmartPropertyEditorManager::update() {
            m_activeEditor->update();
        }
    }
}
