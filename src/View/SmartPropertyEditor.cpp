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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "SmartPropertyEditor.h"

#include "Notifier.h"
#include "Model/Object.h"
#include "View/ControllerFacade.h"
#include "View/MapDocument.h"

#include <wx/window.h>

namespace TrenchBroom {
    namespace View {
        SmartPropertyEditor::SmartPropertyEditor(View::MapDocumentPtr document, View::ControllerPtr controller) :
        m_document(document),
        m_controller(controller),
        m_visual(NULL) {}
        
        SmartPropertyEditor::~SmartPropertyEditor() {
            if (m_visual != NULL)
                destroyVisual();
        }

        wxWindow* SmartPropertyEditor::activate(wxWindow* parent, const Model::PropertyKey& key) {
            m_key = key;
            createVisual(parent);
            bindObservers();
            return m_visual;
        }
        
        void SmartPropertyEditor::deactivate() {
            unbindObservers();
            destroyVisual();
            m_key = "";
        }

        void SmartPropertyEditor::addOrUpdateProperty(const Model::PropertyValue& value) {
            m_controller->setEntityProperty(m_document->allSelectedEntities(), m_key, value);
        }

        void SmartPropertyEditor::bindObservers() {
            m_document->objectDidChangeNotifier.addObserver(this, &SmartPropertyEditor::objectDidChange);
            m_document->selectionDidChangeNotifier.addObserver(this, &SmartPropertyEditor::selectionDidChange);
        }
        
        void SmartPropertyEditor::unbindObservers() {
            m_document->objectDidChangeNotifier.removeObserver(this, &SmartPropertyEditor::objectDidChange);
            m_document->selectionDidChangeNotifier.removeObserver(this, &SmartPropertyEditor::selectionDidChange);
        }

        void SmartPropertyEditor::selectionDidChange(const Model::SelectionResult& result) {
            updateVisual();
        }
        
        void SmartPropertyEditor::objectDidChange(Model::Object* object) {
            if (object->type() == Model::Object::OTEntity)
                updateVisual();
        }

        void SmartPropertyEditor::createVisual(wxWindow* parent) {
            assert(m_visual == NULL);
            m_visual = doCreateVisual(parent);
        }

        void SmartPropertyEditor::destroyVisual() {
            assert(m_visual != NULL);
            m_visual->Destroy();
            m_visual = NULL;
        }

        void SmartPropertyEditor::updateVisual() {
            assert(m_visual != NULL);
            doUpdateVisual(m_key, m_document->allSelectedEntities());
        }
    }
}
