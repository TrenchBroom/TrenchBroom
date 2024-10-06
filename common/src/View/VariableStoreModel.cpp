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

#include "VariableStoreModel.h"

#include "el/VariableStore.h"

namespace tb::View
{

VariableStoreModel::VariableStoreModel(const el::VariableStore& variables)
  : m_variables{variables.clone()}
{
  for (const auto& name : m_variables->names())
  {
    m_variableNames.push_back(name);
  }
}

VariableStoreModel::~VariableStoreModel() = default;

int VariableStoreModel::rowCount(const QModelIndex& /* parent */) const
{
  return int(m_variables->size());
}

QVariant VariableStoreModel::data(const QModelIndex& index, const int role) const
{
  if (
    index.column() < 0 || index.column() > 1 || index.row() < 0
    || index.row() >= int(m_variables->size()))
  {
    return {};
  }

  const auto& name = m_variableNames[size_t(index.row())];
  return index.column() == 0
           ? role == Qt::EditRole      ? QString::fromStdString("${" + name + "}")
             : role == Qt::DisplayRole ? QString::fromStdString(name)
                                       : QVariant{}
           : QString::fromStdString(m_variables->value(name).stringValue());
}

} // namespace tb::View
