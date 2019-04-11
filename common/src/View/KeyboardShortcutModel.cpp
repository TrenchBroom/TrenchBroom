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

#include "KeyboardShortcutModel.h"

#include "CollectionUtils.h"
#include "Preference.h"
#include "PreferenceManager.h"
#include "View/ActionContext.h"
#include "View/ActionList.h"
#include "View/KeyboardShortcut.h"

#include <functional>
#include <set>

namespace TrenchBroom {
    namespace View {
        KeyboardShortcutModel::KeyboardShortcutModel() {
            updateConflicts();
        }

        int KeyboardShortcutModel::rowCount(const QModelIndex& parent) const {
            return static_cast<int>(actions().size());
        }

        int KeyboardShortcutModel::columnCount(const QModelIndex& parent) const {
            // Shortcut, Context, Description
            return 3;
        }

        QVariant KeyboardShortcutModel::headerData(const int section, const Qt::Orientation orientation, const int role) const {
            if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
                if (section == 0) {
                    return QString("Shortcut");
                } else if (section == 1) {
                    return QString("Context");
                } else {
                    return QString("Description");
                }
            } else {
                return QVariant();
            }
        }

        QVariant KeyboardShortcutModel::data(const QModelIndex& index, const int role) const {
            if (!checkIndex(index)) {
                return QVariant();
            }
            if (role == Qt::DisplayRole || role == Qt::EditRole) {
                const auto& action = this->action(index.row());
                if (index.column() == 0) {
                    return action.key();
                } else if (index.column() == 1) {
                    return QString::fromStdString(actionContextName(action.actionContext));
                } else {
                    return QString::fromStdString(action.preferencePath.asString());
                }
            } else {
                return QVariant();
            }
        }

        bool KeyboardShortcutModel::setData(const QModelIndex& index, const QVariant& value, const int role) {
            if (!checkIndex(index) || role != Qt::EditRole) {
                return false;
            }

            // We take a copy here on purpose in order to set the key further below.
            auto action = this->action(index.row());
            if (!action.modifiable) {
                return false;
            }

            action.setKey(value.value<QKeySequence>());
            updateConflicts();

            emit dataChanged(index, index, { Qt::DisplayRole, role });
            return true;
        }

        Qt::ItemFlags KeyboardShortcutModel::flags(const QModelIndex& index) const {
            if (!checkIndex(index)) {
                return Qt::ItemIsEnabled;
            }
            if (index.column() == 0 && action(index.row()).modifiable) {
                return Qt::ItemIsEnabled | Qt::ItemIsEditable;
            } else {
                return Qt::ItemIsEnabled;
            }
        }

        bool KeyboardShortcutModel::hasConflicts() const {
            return !m_conflicts.empty();
        }

        bool KeyboardShortcutModel::hasConflicts(const QModelIndex& index) const {
            if (!checkIndex(index)) {
                return false;
            }

            return VectorUtils::setContains(m_conflicts, index.row());
        }

        void KeyboardShortcutModel::updateConflicts() {
            using ConflictEntry = std::pair<std::reference_wrapper<const ActionInfo>, int>;
            const auto cmp = [](const ConflictEntry& lhs, const ConflictEntry& rhs) {

                const auto& lhsAction = lhs.first.get();
                const auto& rhsAction = rhs.first.get();
                if ((lhsAction.actionContext & rhsAction.actionContext) == 0) {
                    if (lhsAction.actionContext < rhsAction.actionContext) {
                        return true;
                    } else if (lhsAction.actionContext > rhsAction.actionContext) {
                        return false;
                    }
                }

                const auto lhsKey = lhsAction.key();
                const auto rhsKey = rhsAction.key();
                return lhsKey < rhsKey;
            };
            std::set<ConflictEntry, decltype(cmp)> entrySet(cmp);

            m_conflicts.clear();

            const auto& actions = this->actions();
            for (int row = 0; row < static_cast<int>(actions.size()); ++row) {
                const auto& action = actions[static_cast<size_t>(row)];
                if (action.key().count() > 0) {
                    auto [it, noConflict] = entrySet.insert(std::make_pair(std::cref(action), row));
                    if (!noConflict) {
                        // found a duplicate, so there are conflicts
                        const auto otherRow = it->second;
                        VectorUtils::setInsert(m_conflicts, row);
                        VectorUtils::setInsert(m_conflicts, otherRow);

                        const auto index = createIndex(row, 0);
                        const auto otherIndex = createIndex(otherRow, 0);
                        emit dataChanged(index, index, { Qt::DisplayRole });
                        emit dataChanged(otherIndex, otherIndex, { Qt::DisplayRole });
                    }
                }
            }
        }

        const ActionInfo& KeyboardShortcutModel::action(const int index) const {
            return actions().at(static_cast<size_t>(index));
        }

        const std::vector<ActionInfo>& KeyboardShortcutModel::actions() const {
            return ActionList::instance().actions();
        }

        bool KeyboardShortcutModel::checkIndex(const QModelIndex& index) const {
            return index.isValid() && index.column() < 3 && index.row() < static_cast<int>(actions().size());
        }
    }
}
