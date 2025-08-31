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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "SmartFlagsEditor.h"

#include <QScrollArea>
#include <QVBoxLayout>

#include "mdl/Entity.h"
#include "mdl/EntityDefinition.h"
#include "mdl/EntityNodeBase.h"
#include "mdl/Map_Entities.h"
#include "mdl/PropertyDefinition.h"
#include "ui/FlagsEditor.h"
#include "ui/ViewUtils.h"

#include "kdl/set_temp.h"
#include "kdl/string_utils.h"

#include <cassert>
#include <vector>

namespace tb::ui
{

SmartFlagsEditor::SmartFlagsEditor(MapDocument& document, QWidget* parent)
  : SmartPropertyEditor{document, parent}
{
  createGui();
}

void SmartFlagsEditor::createGui()
{
  assert(m_scrolledWindow == nullptr);

  m_scrolledWindow = new QScrollArea{};

  m_flagsEditor = new FlagsEditor{NumCols};
  connect(m_flagsEditor, &FlagsEditor::flagChanged, this, &SmartFlagsEditor::flagChanged);

  m_scrolledWindow->setWidget(m_flagsEditor);

  auto* layout = new QVBoxLayout{};
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_scrolledWindow, 1);
  setLayout(layout);
}

void SmartFlagsEditor::doUpdateVisual(const std::vector<mdl::EntityNodeBase*>& nodes)
{
  assert(!nodes.empty());
  if (!m_ignoreUpdates)
  {
    auto labels = QStringList{};
    auto tooltips = QStringList{};
    getFlags(nodes, labels, tooltips);
    m_flagsEditor->setFlags(labels, tooltips);

    int set, mixed;
    getFlagValues(nodes, set, mixed);
    m_flagsEditor->setFlagValue(set, mixed);
  }
}

void SmartFlagsEditor::getFlags(
  const std::vector<mdl::EntityNodeBase*>& nodes,
  QStringList& labels,
  QStringList& tooltips) const
{
  auto defaultLabels = QStringList{};

  // Initialize the labels and tooltips.
  for (size_t i = 0; i < NumFlags; ++i)
  {
    auto defaultLabel = QString::number(1 << i);

    defaultLabels.push_back(defaultLabel);
    labels.push_back(defaultLabel);
    tooltips.push_back("");
  }

  for (size_t i = 0; i < NumFlags; ++i)
  {
    auto firstPass = true;
    for (const auto* node : nodes)
    {
      const auto indexI = int(i);
      auto label = defaultLabels[indexI];
      auto tooltip = QString{""};

      if (
        const auto* propDef =
          getPropertyDefinition(node->entity().definition(), propertyKey()))
      {
        if (
          const auto* flagType =
            std::get_if<mdl::PropertyValueTypes::Flags>(&propDef->valueType))
        {
          const auto flagValue = int(1 << i);
          if (const auto* flag = flagType->flag(flagValue))
          {
            label = QString::fromStdString(flag->shortDescription);
            tooltip = QString::fromStdString(flag->longDescription);
          }
        }
      }

      if (firstPass)
      {
        labels[indexI] = label;
        tooltips[indexI] = tooltip;
        firstPass = false;
      }
      else if (labels[indexI] != label)
      {
        labels[indexI] = defaultLabels[indexI];
        tooltips[indexI].clear();
      }
    }
  }
}

void SmartFlagsEditor::getFlagValues(
  const std::vector<mdl::EntityNodeBase*>& nodes, int& setFlags, int& mixedFlags) const
{
  if (nodes.empty())
  {
    setFlags = 0;
    mixedFlags = 0;
    return;
  }

  auto it = std::begin(nodes);
  auto end = std::end(nodes);
  setFlags = getFlagValue(*it);
  mixedFlags = 0;

  while (++it != end)
  {
    combineFlags(NumFlags, getFlagValue(*it), setFlags, mixedFlags);
  }
}

int SmartFlagsEditor::getFlagValue(const mdl::EntityNodeBase* node) const
{
  if (const auto* value = node->entity().property(propertyKey()))
  {
    return kdl::str_to_int(*value).value_or(0);
  }
  return 0;
}

void SmartFlagsEditor::flagChanged(
  const size_t index,
  const int /* value */,
  const int /* setFlag */,
  const int /* mixedFlag */)
{
  if (const auto& toUpdate = nodes(); !toUpdate.empty())
  {
    const auto ignoreUpdates = kdl::set_temp{m_ignoreUpdates};
    const auto set = m_flagsEditor->isFlagSet(index);
    updateEntitySpawnflag(map(), propertyKey(), index, set);
  }
}

} // namespace tb::ui
