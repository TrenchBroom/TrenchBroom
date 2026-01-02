/*
 Copyright (C) 2010 Kristian Duske

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

#include "ui/SmartPropertyEditor.h"

#include "mdl/Map_Entities.h"
#include "ui/MapDocument.h"

#include "kd/contracts.h"

#include <vector>

namespace tb::ui
{

SmartPropertyEditor::SmartPropertyEditor(MapDocument& document, QWidget* parent)
  : QWidget{parent}
  , m_document{document}
{
}

SmartPropertyEditor::~SmartPropertyEditor() = default;

void SmartPropertyEditor::activate(const std::string& propertyKey)
{
  contract_pre(!m_active);

  m_propertyKey = propertyKey;
  m_active = true;
}

void SmartPropertyEditor::update(const std::vector<mdl::EntityNodeBase*>& nodes)
{
  m_nodes = nodes;
  doUpdateVisual(m_nodes);
}

void SmartPropertyEditor::deactivate()
{
  m_active = false;
  m_propertyKey = "";
}

bool SmartPropertyEditor::usesPropertyKey(const std::string& propertyKey) const
{
  return m_propertyKey == propertyKey;
}

MapDocument& SmartPropertyEditor::document()
{
  return m_document;
}

const std::string& SmartPropertyEditor::propertyKey() const
{
  return m_propertyKey;
}

const std::vector<mdl::EntityNodeBase*> SmartPropertyEditor::nodes() const
{
  return m_nodes;
}

void SmartPropertyEditor::addOrUpdateProperty(const std::string& value)
{
  contract_pre(m_active);

  setEntityProperty(document().map(), m_propertyKey, value);
}

} // namespace tb::ui
