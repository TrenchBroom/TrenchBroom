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

#include <memory>
#include <string>
#include <vector>

#include <QWidget>

class QTableView;
class QCheckBox;
class QAbstractButton;
class QShortcut;
class QSortFilterProxyModel;

namespace TrenchBroom {
    namespace Model {
        class Node;
    }

    namespace View {
        class EntityAttributeModel;
        class EntityAttributeTable;
        class MapDocument;
        class Selection;

        struct AttributeGridSelection {
            std::string attributeName;
            int column;
        };

        /**
         * Panel with the entity attribute table, and the toolbar below it (add/remove icons,
         * "show default properties" checkbox, etc.)
         */
        class EntityAttributeGrid : public QWidget {
            Q_OBJECT
        private:
            std::weak_ptr<MapDocument> m_document;

            EntityAttributeModel* m_model;
            QSortFilterProxyModel* m_proxyModel;
            EntityAttributeTable* m_table;
            QAbstractButton* m_addAttributeButton;
            QAbstractButton* m_removePropertiesButton;
            QCheckBox* m_showDefaultPropertiesCheckBox;
            std::vector<AttributeGridSelection> m_selectionBackup;
        public:
            explicit EntityAttributeGrid(std::weak_ptr<MapDocument> document, QWidget* parent = nullptr);
            ~EntityAttributeGrid() override;
        private:
            void backupSelection();
            void restoreSelection();

            void addAttribute();
            void removeSelectedAttributes();

            bool canRemoveSelectedAttributes() const;
            std::vector<int> selectedRowsAndCursorRow() const;
        private:
            void createGui(std::weak_ptr<MapDocument> document);

            void bindObservers();
            void unbindObservers();

            void documentWasNewed(MapDocument* document);
            void documentWasLoaded(MapDocument* document);
            void nodesDidChange(const std::vector<Model::Node*>& nodes);
            void selectionWillChange();
            void selectionDidChange(const Selection& selection);
            void entityDefinitionsOrModsDidChange();
        private:
            void ensureSelectionVisible();
            void updateControls();
            void updateControlsEnabled();
        public:
            std::string selectedRowName() const;
        signals:
            void currentRowChanged();
        };
    }
}

#endif /* defined(TrenchBroom_EntityAttributeGrid) */
