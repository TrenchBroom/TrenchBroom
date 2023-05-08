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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <QAbstractListModel>

#include <memory>
#include <string>
#include <vector>

class QModelIndex;
class QVariant;

namespace TrenchBroom
{
namespace EL
{
class VariableStore;
}
namespace View
{
class VariableStoreModel : public QAbstractListModel
{
  Q_OBJECT
private:
  std::unique_ptr<EL::VariableStore> m_variables;
  std::vector<std::string> m_variableNames;

public:
  explicit VariableStoreModel(const EL::VariableStore& variables);
  ~VariableStoreModel() override;

  int rowCount(const QModelIndex& parent) const override;
  QVariant data(const QModelIndex& index, int role) const override;
};
} // namespace View
} // namespace TrenchBroom
