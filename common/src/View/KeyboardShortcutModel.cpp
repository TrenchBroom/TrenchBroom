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

#include "Preference.h"
#include "PreferenceManager.h"
#include "View/ActionContext.h"
#include "View/Actions.h"
#include "View/MapDocument.h"
#include "IO/PathQt.h"

#include <kdl/vector_set.h>

#include <functional>
#include <set>

#include <QBrush>

namespace TrenchBroom {
    namespace View {
        KeyboardShortcutModel::ActionInfo::ActionInfo(const IO::Path& i_displayPath, const Action& i_action) :
        displayPath(i_displayPath),
        action(i_action) {}

        KeyboardShortcutModel::KeyboardShortcutModel(MapDocument* document, QObject* parent) :
        QAbstractTableModel(parent),
        m_document(document) {
            initializeActions();
            updateConflicts();
        }

        void KeyboardShortcutModel::reset() {
            m_actions.clear();
            initializeActions();
            updateConflicts();
            if (totalActionCount() > 0) {
                emit dataChanged(createIndex(0, 0), createIndex(totalActionCount() - 1, 2));
            }
        }

        int KeyboardShortcutModel::rowCount(const QModelIndex& /* parent */) const {
            return totalActionCount();
        }

        int KeyboardShortcutModel::columnCount(const QModelIndex& /* parent */) const {
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
                const auto& actionInfo = this->actionInfo(index.row());
                if (index.column() == 0) {
                    return actionInfo.action.keySequence();
                } else if (index.column() == 1) {
                    return QString::fromStdString(actionContextName(actionInfo.action.actionContext()));
                } else {
                    return QString::fromStdString(actionInfo.displayPath.asString(" > "));
                }
            } else if (role == Qt::ForegroundRole) {
                if (hasConflicts(index)) {
                    return QBrush(Qt::red);
                }
                return QVariant();
            } else {
                return QVariant();
            }
        }

        bool KeyboardShortcutModel::setData(const QModelIndex& index, const QVariant& value, const int role) {
            if (!checkIndex(index) || role != Qt::EditRole) {
                return false;
            }

            // We take a copy here on purpose in order to set the key further below.
            auto& actionInfo = this->actionInfo(index.row());
            actionInfo.action.setKeySequence(value.value<QKeySequence>());

            updateConflicts();

            emit dataChanged(index, index, { Qt::DisplayRole, role });
            return true;
        }

        Qt::ItemFlags KeyboardShortcutModel::flags(const QModelIndex& index) const {
            if (!checkIndex(index)) {
                return Qt::ItemIsEnabled;
            }
            if (index.column() == 0) {
                return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
            } else {
                return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
            }
        }

        bool KeyboardShortcutModel::hasConflicts() const {
            return !m_conflicts.empty();
        }

        bool KeyboardShortcutModel::hasConflicts(const QModelIndex& index) const {
            if (!checkIndex(index)) {
                return false;
            }

            return kdl::wrap_set(m_conflicts).count(index.row()) > 0u;
        }

        void KeyboardShortcutModel::initializeActions() {
            initializeMenuActions();
            initializeViewActions();
            if (m_document != nullptr) {
                initializeTagActions();
                initializeEntityDefinitionActions();
            }
        }

        class KeyboardShortcutModel::MenuActionVisitor : public MenuVisitor {
        private:
            std::vector<ActionInfo>& m_actions;
            IO::Path m_currentPath;
        public:
            explicit MenuActionVisitor(std::vector<ActionInfo>& actions) :
            m_actions(actions) {}

            void visit(const Menu& menu) override {
                m_currentPath = m_currentPath + IO::Path(menu.name());
                menu.visitEntries(*this);
                m_currentPath = m_currentPath.deleteLastComponent();
            }

            void visit(const MenuSeparatorItem&) override {}

            void visit(const MenuActionItem& item) override {
                m_actions.emplace_back(m_currentPath + IO::pathFromQString(item.label()), item.action());
            }

        };

        void KeyboardShortcutModel::initializeMenuActions() {
            MenuActionVisitor visitor(m_actions);
            const auto& actionManager = ActionManager::instance();
            actionManager.visitMainMenu(visitor);
        }

        void KeyboardShortcutModel::initializeViewActions() {
            const auto& actionManager = ActionManager::instance();
            actionManager.visitMapViewActions([this](const Action& action) {
                m_actions.emplace_back(IO::Path("Map View") + IO::pathFromQString(action.label()), action);
            });
        }

        void KeyboardShortcutModel::initializeTagActions() {
            assert(m_document != nullptr);
            m_document->visitTagActions([this](const Action& action) {
                m_actions.emplace_back(IO::Path("Tags") + IO::pathFromQString(action.label()), action);
            });
        }

        void KeyboardShortcutModel::initializeEntityDefinitionActions() {
            assert(m_document != nullptr);
            m_document->visitEntityDefinitionActions([this](const Action& action) {
                m_actions.emplace_back(IO::Path("Entity Definitions") + IO::pathFromQString(action.label()), action);
            });
        }

        void KeyboardShortcutModel::updateConflicts() {
            using ConflictEntry = std::pair<std::reference_wrapper<const ActionInfo>, int>;
            const auto cmp = [](const ConflictEntry& lhs, const ConflictEntry& rhs) {

                const auto& lhsAction = lhs.first.get().action;
                const auto& rhsAction = rhs.first.get().action;
                if (actionContextMatches(lhsAction.actionContext(), rhsAction.actionContext())) {
                    // if the two have the same sequence, they would be in conflict, so we compare the sequences
                    const auto lhsKeySequence = lhsAction.keySequence();
                    const auto rhsKeySequence = rhsAction.keySequence();
                    return lhsKeySequence < rhsKeySequence;
                } else {
                    // otherwise, we just compare by the action context
                    return lhsAction.actionContext() < rhsAction.actionContext();
                }
            };
            std::set<ConflictEntry, decltype(cmp)> entrySet(cmp);

            m_conflicts.clear();
            auto conflictSet = kdl::wrap_set(m_conflicts);

            for (int row = 0; row < totalActionCount(); ++row) {
                const auto& actionInfo = this->actionInfo(row);
                const auto keySequence = actionInfo.action.keySequence();
                if (keySequence.count() > 0) {
                    auto [it, noConflict] = entrySet.insert(std::make_pair(std::cref(actionInfo), row));
                    if (!noConflict) {
                        // found a duplicate, so there are conflicts
                        const auto otherRow = it->second;
                        conflictSet.insert(row);
                        conflictSet.insert(otherRow);

                        const auto index = createIndex(row, 0);
                        const auto otherIndex = createIndex(otherRow, 0);
                        emit dataChanged(index, index, { Qt::DisplayRole });
                        emit dataChanged(otherIndex, otherIndex, { Qt::DisplayRole });
                    }
                }
            }
        }

        const KeyboardShortcutModel::ActionInfo& KeyboardShortcutModel::actionInfo(const int index) const {
            assert(index < totalActionCount());
            return m_actions[static_cast<size_t>(index)];
        }

        int KeyboardShortcutModel::totalActionCount() const {
            return static_cast<int>(m_actions.size());
        }

        bool KeyboardShortcutModel::checkIndex(const QModelIndex& index) const {
            return index.isValid() && index.column() < 3 && index.row() < totalActionCount();
        }
    }
}
