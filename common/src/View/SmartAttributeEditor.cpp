/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "SmartAttributeEditor.h"

#include "Notifier.h"
#include "Model/Object.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace View {
        SmartAttributeEditor::SmartAttributeEditor(View::MapDocumentWPtr document, QWidget* parent) :
        QWidget(parent),
        m_document(document),
        m_active(false) {}

        SmartAttributeEditor::~SmartAttributeEditor() {}

        void SmartAttributeEditor::activate(const Model::AttributeName& name) {
            assert(!m_active);
            m_name = name;
            m_active = true;
        }

        void SmartAttributeEditor::update(const Model::AttributableNodeList& attributables) {
            m_attributables = attributables;
            doUpdateVisual(m_attributables);
        }

        void SmartAttributeEditor::deactivate() {
            m_active = false;
            m_name = "";
        }

        bool SmartAttributeEditor::usesName(const Model::AttributeName& name) const {
            return m_name == name;
        }

        View::MapDocumentSPtr SmartAttributeEditor::document() const {
            return lock(m_document);
        }

        const Model::AttributeName& SmartAttributeEditor::name() const {
            return m_name;
        }

        const Model::AttributableNodeList SmartAttributeEditor::attributables() const {
            return m_attributables;
        }

        void SmartAttributeEditor::addOrUpdateAttribute(const Model::AttributeValue& value) {
            assert(m_active);
            document()->setAttribute(m_name, value);
        }
    }
}
