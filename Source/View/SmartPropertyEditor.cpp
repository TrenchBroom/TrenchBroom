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

#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

namespace TrenchBroom {
    namespace View {
        void SmartPropertyEditor::updateValue(const Model::PropertyValue& value) {
            m_manager.updateValue(m_property, value);
        }

        SmartPropertyEditor::SmartPropertyEditor(SmartPropertyEditorManager& manager) :
        m_manager(manager),
        m_active(false) {}

        void SmartPropertyEditor::activate(wxWindow* parent) {
            assert(!m_active);
            
            wxWindow* visual = createVisual(parent);
            
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(visual, 1, wxEXPAND);
            parent->SetSizer(outerSizer);
            
            m_active = true;
        }
        
        void SmartPropertyEditor::deactivate() {
            assert(m_active);
            destroyVisual();
            m_active = false;
        }

        void SmartPropertyEditor::setValues(const Model::PropertyValueList& values) {
            assert(m_active);
            updateVisual(values);
        }

        wxWindow* DefaultPropertyEditor::createVisual(wxWindow* parent) {
            assert(m_visual == NULL);
            m_visual = new wxPanel(parent);
            wxStaticText* text = new wxStaticText(m_visual, wxID_ANY, wxT("No Smart Editor Available"), wxDefaultPosition, wxDefaultSize);
            
            wxFont defaultFont = text->GetFont();
            wxFont boldFont(defaultFont.GetPointSize() + 2, defaultFont.GetFamily(), defaultFont.GetStyle(), wxFONTWEIGHT_BOLD);
            text->SetFont(boldFont);
            text->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
            
            wxSizer* innerSizer = new wxBoxSizer(wxVERTICAL);
            innerSizer->AddStretchSpacer();
            innerSizer->Add(text);
            innerSizer->AddStretchSpacer();

            wxSizer* outerSizer = new wxBoxSizer(wxHORIZONTAL);
            outerSizer->AddStretchSpacer();
            outerSizer->Add(innerSizer, 1, wxEXPAND);
            outerSizer->AddStretchSpacer();
            m_visual->SetSizerAndFit(outerSizer);
            
            return m_visual;
        }
        
        void DefaultPropertyEditor::destroyVisual() {
            assert(m_visual != NULL);
            m_visual->Destroy();
            m_visual = NULL;
        }

        void DefaultPropertyEditor::updateVisual(const Model::PropertyValueList& values) {
        }

        DefaultPropertyEditor::DefaultPropertyEditor(SmartPropertyEditorManager& manager) :
        SmartPropertyEditor(manager),
        m_visual(NULL) {}

        void SmartPropertyEditorManager::activateEditor(SmartPropertyEditor* editor) {
            if (m_activeEditor == editor)
                return;
            deactivateEditor();
            m_activeEditor = editor;
            m_activeEditor->activate(m_panel);
        }
        
        void SmartPropertyEditorManager::deactivateEditor() {
            if (m_activeEditor != NULL) {
                m_activeEditor->deactivate();
                m_activeEditor = NULL;
            }
        }

        SmartPropertyEditorManager::SmartPropertyEditorManager(wxWindow* parent) :
        m_panel(new wxPanel(parent)),
        m_defaultEditor(new DefaultPropertyEditor(*this)),
        m_activeEditor(NULL) {
            m_panel->SetMinSize(wxSize(wxDefaultSize.x, 120));
            
            wxSizer* outerSizer = new wxBoxSizer(wxHORIZONTAL);
            outerSizer->Add(m_panel, 1, wxEXPAND);
            parent->SetSizerAndFit(outerSizer);
            
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

        void SmartPropertyEditorManager::updateValue(const Model::PropertyKey& key, const Model::PropertyValue& value) {
        }
    }
}
