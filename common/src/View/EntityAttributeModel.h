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

#ifndef TrenchBroom_EntityAttributeGridTable
#define TrenchBroom_EntityAttributeGridTable

#include "StringType.h"
#include "StringList.h"
#include "Model/ModelTypes.h"
#include "View/ViewTypes.h"

#include <QAbstractTableModel>

#include <vector>
#include <tuple>
#include <map>

namespace TrenchBroom {
    namespace Assets {
        class AttributeDefinition;
    }

    namespace View {
        using AttribRow = std::tuple<QString, QString>;
        using RowList = std::vector<AttribRow>;

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
            String m_name;
            String m_value;
            ValueType m_valueType;

            bool m_nameMutable;
            bool m_valueMutable;
            String m_tooltip;
        public:
            AttributeRow();
            AttributeRow(const String& name, const Model::AttributableNode* node);
            bool operator==(const AttributeRow& other) const;
            void merge(const Model::AttributableNode* other);

            const String& name() const;
            String value() const;
            bool nameMutable() const;
            bool valueMutable() const;
            const String& tooltip() const;
            bool isDefault() const;
            bool multi() const;
            bool subset() const;

            static AttributeRow rowForAttributableNodes(const String& key, const Model::AttributableNodeList& attributables);
            static std::set<String> allKeys(const Model::AttributableNodeList& attributables, bool showDefaultRows);
            static std::map<String, AttributeRow> rowsForAttributableNodes(const Model::AttributableNodeList& attributables, bool showDefaultRows);
            /**
             * Suggests a new, unused attribute name of the form "property X".
             */
            static String newAttributeNameForAttributableNodes(const Model::AttributableNodeList& attributables);
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
         */
        class EntityAttributeModel : public QAbstractTableModel {
            Q_OBJECT
        private:
            std::vector<AttributeRow> m_rows;
            bool m_showDefaultRows;
            MapDocumentWPtr m_document;
        public:
            explicit EntityAttributeModel(MapDocumentWPtr document, QObject* parent);

            bool showDefaultRows() const;
            void setShowDefaultRows(bool showDefaultRows);

            void setRows(const std::map<String, AttributeRow>& newRows);

            const AttributeRow* dataForModelIndex(const QModelIndex& index) const;
            int rowForAttributeName(const String& name) const;

        public: // for autocompletion
            QStringList getCompletions(const QModelIndex& index) const;
        private: // autocompletion helpers
            Model::AttributeName attributeName(int row) const;
            Model::AttributeNameList attributeNames(int row, int count) const;
            StringList getAllAttributeNames() const;
            StringList getAllValuesForAttributeNames(const StringList& names) const;
            StringList getAllClassnames() const;
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
            void SetValue(const int row, const int col, const QString& value);
            bool InsertRow(const size_t pos);
            bool AppendRow();
            bool DeleteRows(const size_t pos, size_t numRows);
            int rowForName(const Model::AttributeName& name) const;
            bool canRemove(int rowIndexInt);
            bool hasRowWithAttributeName(const Model::AttributeName& name) const;
            void renameAttribute(const size_t rowIndex, const String& newName, const Model::AttributableNodeList& attributables);
            void updateAttribute(const size_t rowIndex, const String& newValue, const Model::AttributableNodeList& attributables);
        };
    }
}

#endif /* defined(TrenchBroom_EntityAttributeGridTable) */
