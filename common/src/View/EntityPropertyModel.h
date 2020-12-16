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

#include <QAbstractTableModel>

#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class EntityNodeBase;
    }

    namespace View {
        class MapDocument;

        enum class ValueType {
            /**
             * No entities have this key set; the provided value is the default from the entity definition
             */
            Unset,
            /**
             * All entities have the same value set for this key
             */
            SingleValue,
            /**
             * 1+ entities have this key unset, the rest have the same value set
             */
            SingleValueAndUnset,
            /**
             * Two or more entities have different values for this key
             */
            MultipleValues
        };

        /**
         * Viewmodel (as in MVVM) for a single row in the table
         */
        class AttributeRow {
        private:
            std::string m_name;
            std::string m_value;
            ValueType m_valueType;

            bool m_nameMutable;
            bool m_valueMutable;
            std::string m_tooltip;
        public:
            AttributeRow();
            AttributeRow(const std::string& name, const Model::EntityNodeBase* node);
            bool operator==(const AttributeRow& other) const;
            bool operator<(const AttributeRow& other) const;
            void merge(const Model::EntityNodeBase* other);

            const std::string& name() const;
            std::string value() const;
            bool nameMutable() const;
            bool valueMutable() const;
            const std::string& tooltip() const;
            bool isDefault() const;
            bool multi() const;
            bool subset() const;

            static AttributeRow rowForAttributableNodes(const std::string& key, const std::vector<Model::EntityNodeBase*>& attributables);
            static std::vector<std::string> allKeys(const std::vector<Model::EntityNodeBase*>& attributables, bool showDefaultRows);
            static std::map<std::string, AttributeRow> rowsForAttributableNodes(const std::vector<Model::EntityNodeBase*>& attributables, bool showDefaultRows);
            /**
             * Suggests a new, unused attribute name of the form "property X".
             */
            static std::string newAttributeNameForAttributableNodes(const std::vector<Model::EntityNodeBase*>& attributables);
        };

        /**
         * Model for the QTableView.
         *
         * Data flow:
         *
         * 1. MapDocument is modified, or entities are added/removed from the list that EntityAttributeGridTable is observing
         * 2. EntityAttributeGridTable observes the change, and builds a list of AttributeRow for the new state
         * 3. The new state and old state are diffed, and the necessary QAbstractTableModel methods called
         *    to update the view correctly (preserving selection, etc.)
         *
         * All edits to the table flow this way; the EntityAttributeGridTable is never modified in response to
         * a UI action.
         *
         * The order of m_rows is not significant; it's expected that there is a sort proxy model
         * used on top of this model.
         */
        class EntityAttributeModel : public QAbstractTableModel {
            Q_OBJECT
        private:
            std::vector<AttributeRow> m_rows;
            bool m_showDefaultRows;
            std::weak_ptr<MapDocument> m_document;
        public:
            explicit EntityAttributeModel(std::weak_ptr<MapDocument> document, QObject* parent);

            bool showDefaultRows() const;
            void setShowDefaultRows(bool showDefaultRows);

            void setRows(const std::map<std::string, AttributeRow>& newRows);

            const AttributeRow* dataForModelIndex(const QModelIndex& index) const;
            int rowForAttributeName(const std::string& name) const;

        public: // for autocompletion
            QStringList getCompletions(const QModelIndex& index) const;
        private: // autocompletion helpers
            std::vector<std::string> attributeNames(int row, int count) const;
            std::vector<std::string> getAllAttributeNames() const;
            std::vector<std::string> getAllValuesForAttributeNames(const std::vector<std::string>& names) const;
            std::vector<std::string> getAllClassnames() const;
        public slots:
            void updateFromMapDocument();

        public: // QAbstractTableModel overrides
            int rowCount(const QModelIndex& parent) const override;
            int columnCount(const QModelIndex& parent) const override;
            Qt::ItemFlags flags(const QModelIndex &index) const override;
            QVariant data(const QModelIndex& index, int role) const override;
            bool setData(const QModelIndex &index, const QVariant &value, int role) override;
            QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

        private: // helpers
            int rowForName(const std::string& name) const;
            bool hasRowWithAttributeName(const std::string& name) const;
            bool renameAttribute(const size_t rowIndex, const std::string& newName, const std::vector<Model::EntityNodeBase*>& attributables);
            bool updateAttribute(const size_t rowIndex, const std::string& newValue, const std::vector<Model::EntityNodeBase*>& attributables);

        public: // EntityAttributeGrid helpers
            std::string attributeName(int row) const;
            bool canRemove(int rowIndexInt);
            /**
             * Return the desired sort order for these two rows.
             * Used by EntitySortFilterProxyModel to sort the rows.
             */
            bool lessThan(size_t rowIndexA, size_t rowIndexB) const;
        };
    }
}

