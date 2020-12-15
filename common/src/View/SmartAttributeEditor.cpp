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

#include <kdl/memory_utils.h>

#include <memory>
#include <vector>

namespace TrenchBroom {
    namespace View {
        SmartAttributeEditor::SmartAttributeEditor(std::weak_ptr<MapDocument> document, QWidget* parent) :
        QWidget(parent),
        m_document(document),
        m_active(false) {}

        SmartAttributeEditor::~SmartAttributeEditor() {}

        void SmartAttributeEditor::activate(const std::string& name) {
            assert(!m_active);
            m_name = name;
            m_active = true;
        }

        void SmartAttributeEditor::update(const std::vector<Model::EntityNodeBase*>& attributables) {
            m_attributables = attributables;
            doUpdateVisual(m_attributables);
        }

        void SmartAttributeEditor::deactivate() {
            m_active = false;
            m_name = "";
        }

        bool SmartAttributeEditor::usesName(const std::string& name) const {
            return m_name == name;
        }

        std::shared_ptr<MapDocument> SmartAttributeEditor::document() const {
            return kdl::mem_lock(m_document);
        }

        const std::string& SmartAttributeEditor::name() const {
            return m_name;
        }

        const std::vector<Model::EntityNodeBase*> SmartAttributeEditor::attributables() const {
            return m_attributables;
        }

        void SmartAttributeEditor::addOrUpdateAttribute(const std::string& value) {
            assert(m_active);
            document()->setProperty(m_name, value);
        }
    }
}
