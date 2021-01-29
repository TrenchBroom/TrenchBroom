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

        enum class PropertyProtection {
            NotProtectable,
            Protected,
            NotProtected,
            Mixed
        };

        /**
         * Viewmodel (as in MVVM) for a single row in the table
         */
        class PropertyRow {
        private:
            std::string m_key;
            std::string m_value;
            ValueType m_valueType;

            bool m_keyMutable;
            bool m_valueMutable;
            PropertyProtection m_protected;
            std::string m_tooltip;
        public:
            PropertyRow();
            PropertyRow(const std::string& key, const Model::EntityNodeBase* node);
            bool operator==(const PropertyRow& other) const;
            bool operator<(const PropertyRow& other) const;
            void merge(const Model::EntityNodeBase* other);

            const std::string& key() const;
            std::string value() const;
            bool keyMutable() const;
            bool valueMutable() const;
            PropertyProtection isProtected() const;
            const std::string& tooltip() const;
            bool isDefault() const;
            bool multi() const;
            bool subset() const;

            static PropertyRow rowForEntityNodes(const std::string& key, const std::vector<Model::EntityNodeBase*>& nodes);
            static std::vector<std::string> allKeys(const std::vector<Model::EntityNodeBase*>& nodes, bool showDefaultRows, const bool showPreservedProperties);
            static std::map<std::string, PropertyRow> rowsForEntityNodes(const std::vector<Model::EntityNodeBase*>& nodes, bool showDefaultRows, const bool showPreservedProperties);
            /**
             * Suggests a new, unused property name of the form "property X".
             */
            static std::string newPropertyKeyForEntityNodes(const std::vector<Model::EntityNodeBase*>& nodes);
        };

        /**
         * Model for the QTableView.
         *
         * Data flow:
         *
         * 1. MapDocument is modified, or entities are added/removed from the list that EntityPropertyGridTable is observing
         * 2. EntityPropertyGridTable observes the change, and builds a list of PropertyRow for the new state
         * 3. The new state and old state are diffed, and the necessary QAbstractTableModel methods called
         *    to update the view correctly (preserving selection, etc.)
         *
         * All edits to the table flow this way; the EntityPropertyGridTable is never modified in response to
         * a UI action.
         *
         * The order of m_rows is not significant; it's expected that there is a sort proxy model
         * used on top of this model.
         */
        class EntityPropertyModel : public QAbstractTableModel {
            Q_OBJECT
        private:
            std::vector<PropertyRow> m_rows;
            bool m_showDefaultRows;
            bool m_shouldShowProtectedProperties;
            std::weak_ptr<MapDocument> m_document;
        public:
            explicit EntityPropertyModel(std::weak_ptr<MapDocument> document, QObject* parent);

            bool showDefaultRows() const;
            void setShowDefaultRows(bool showDefaultRows);

            bool shouldShowProtectedProperties() const;

            void setRows(const std::map<std::string, PropertyRow>& newRows);

            const PropertyRow* dataForModelIndex(const QModelIndex& index) const;
            int rowForPropertyKey(const std::string& propertyKey) const;

        public: // for autocompletion
            QStringList getCompletions(const QModelIndex& index) const;
        private: // autocompletion helpers
            std::vector<std::string> propertyKeys(int row, int count) const;
            std::vector<std::string> getAllPropertyKeys() const;
            std::vector<std::string> getAllValuesForPropertyKeys(const std::vector<std::string>& propertyKeys) const;
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
            bool hasRowWithPropertyKey(const std::string& propertyKey) const;
            bool renameProperty(size_t rowIndex, const std::string& newKey, const std::vector<Model::EntityNodeBase*>& nodes);
            bool updateProperty(size_t rowIndex, const std::string& newValue, const std::vector<Model::EntityNodeBase*>& nodes);
            bool setProtectedProperty(size_t rowIndex, bool newValue);

        public: // EntityPropertyGrid helpers
            std::string propertyKey(int row) const;
            bool canRemove(int rowIndexInt);
            /**
             * Return the desired sort order for these two rows.
             * Used by EntitySortFilterProxyModel to sort the rows.
             */
            bool lessThan(size_t rowIndexA, size_t rowIndexB) const;
        };
    }
}

