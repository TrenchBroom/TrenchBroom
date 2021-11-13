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

#include "SmartPropertyEditor.h"

#include "Model/Object.h"
#include "Notifier.h"
#include "View/MapDocument.h"

#include <kdl/memory_utils.h>

#include <memory>
#include <vector>

namespace TrenchBroom {
namespace View {
SmartPropertyEditor::SmartPropertyEditor(std::weak_ptr<MapDocument> document, QWidget* parent)
  : QWidget(parent)
  , m_document(document)
  , m_active(false) {}

SmartPropertyEditor::~SmartPropertyEditor() {}

void SmartPropertyEditor::activate(const std::string& propertyKey) {
  assert(!m_active);
  m_propertyKey = propertyKey;
  m_active = true;
}

void SmartPropertyEditor::update(const std::vector<Model::EntityNodeBase*>& nodes) {
  m_nodes = nodes;
  doUpdateVisual(m_nodes);
}

void SmartPropertyEditor::deactivate() {
  m_active = false;
  m_propertyKey = "";
}

bool SmartPropertyEditor::usesPropertyKey(const std::string& propertyKey) const {
  return m_propertyKey == propertyKey;
}

std::shared_ptr<MapDocument> SmartPropertyEditor::document() const {
  return kdl::mem_lock(m_document);
}

const std::string& SmartPropertyEditor::propertyKey() const {
  return m_propertyKey;
}

const std::vector<Model::EntityNodeBase*> SmartPropertyEditor::nodes() const {
  return m_nodes;
}

void SmartPropertyEditor::addOrUpdateProperty(const std::string& value) {
  assert(m_active);
  document()->setProperty(m_propertyKey, value);
}
} // namespace View
} // namespace TrenchBroom
