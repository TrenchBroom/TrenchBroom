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
        m_active(false) {}
        
        SmartPropertyEditor::~SmartPropertyEditor() {}

        wxWindow* SmartPropertyEditor::activate(wxWindow* parent, const Model::PropertyKey& key) {
            assert(!m_active);
            
            m_key = key;
            
            wxWindow* visual = createVisual(parent);
            m_active = true;
            return visual;
        }
        
        void SmartPropertyEditor::update(const Model::EntityList& entities) {
            m_entities = entities;
            updateVisual(m_entities);
        }

        void SmartPropertyEditor::deactivate() {
            m_active = false;
            destroyVisual();
            m_key = "";
        }

        bool SmartPropertyEditor::usesKey(const Model::PropertyKey& key) const {
            return m_key == key;
        }

        View::MapDocumentPtr SmartPropertyEditor::document() const {
            return m_document;
        }

        View::ControllerPtr SmartPropertyEditor::controller() const {
            return m_controller;
        }

        const Model::PropertyKey& SmartPropertyEditor::key() const {
            return m_key;
        }

        const Model::EntityList SmartPropertyEditor::entities() const {
            return m_entities;
        }

        void SmartPropertyEditor::addOrUpdateProperty(const Model::PropertyValue& value) {
            assert(m_active);
            m_controller->setEntityProperty(m_document->allSelectedEntities(), m_key, value);
        }
        wxWindow* SmartPropertyEditor::createVisual(wxWindow* parent) {
            return doCreateVisual(parent);
        }

        void SmartPropertyEditor::destroyVisual() {
            doDestroyVisual();
        }

        void SmartPropertyEditor::updateVisual(const Model::EntityList& entities) {
            doUpdateVisual(entities);
        }
    }
}
