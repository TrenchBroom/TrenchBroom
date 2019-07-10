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

#include "StringUtils.h"
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

        /**
         * Viewmodel (as in MVVM) for a single row in the table
         */
        class AttributeRow {
        private:
            String m_name;
            String m_value;
            bool m_nameMutable;
            bool m_valueMutable;
            String m_tooltip;
            /**
             * If this is a default value from the FGD that the user hasn't explicitly set
             */
            bool m_default;

            /**
             * How many entities have this key set?
             */
            size_t m_numEntitiesWithValueSet;
            /**
             * Whether
             */
            bool m_multi;
        public:
            AttributeRow();
            AttributeRow(const String& name, const String& value, bool nameMutable, bool valueMutable, const String& tooltip, bool isDefault);

            const String& name() const;
            const String& value() const;
            bool nameMutable() const;
            bool valueMutable() const;
            const String& tooltip() const;
            bool isDefault() const;
            bool multi() const;

        private:
            void merge(const String& i_valuec, bool nameMutable, bool valueMutable);
            static void mergeRowInToMap(std::map<String, AttributeRow>* rows,
                                        const Model::AttributeName& name, const Model::AttributeValue& value,
                                        const Assets::AttributeDefinition* definition,
                                        bool nameMutable, bool valueMutable, bool isDefault);

        public:
            static std::map<String, AttributeRow> rowsForAttributableNodes(const Model::AttributableNodeList& attributables);
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
        class EntityAttributeGridTable : public QAbstractTableModel {
            Q_OBJECT
        private:
            std::vector<AttributeRow> m_rows;
            MapDocumentWPtr m_document;
        public:
            explicit EntityAttributeGridTable(MapDocumentWPtr document, QObject* parent);

            void setRows(const std::map<String, AttributeRow>& newRows);

            const AttributeRow* dataForModelIndex(const QModelIndex& index) const;
            int rowForAttributeName(const String& name) const;

        public slots:
            void updateFromMapDocument();

        public: // QAbstractTableModel overrides
            int rowCount(const QModelIndex& parent) const override;
            int columnCount(const QModelIndex& parent) const override;
            Qt::ItemFlags flags(const QModelIndex &index) const override;
            QVariant data(const QModelIndex& index, int role) const override;
            bool setData(const QModelIndex &index, const QVariant &value, int role) override;
            QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
        };


        // Begin old code
#if 0
        class EntityAttributeGridTable : public QAbstractItemModel {
        private:
            class AttributeRow {
            public:
                using List = std::vector<AttributeRow>;
            private:
                String m_name;
                String m_value;
                bool m_nameMutable;
                bool m_valueMutable;
                String m_tooltip;
                bool m_default;

                size_t m_maxCount;
                size_t m_count;
                bool m_multi;
            public:
                AttributeRow();
                AttributeRow(const String& name, const String& value, bool nameMutable, bool valueMutable, const String& tooltip, bool i_default, size_t maxCount);

                const String& name() const;
                const String& value() const;
                bool nameMutable() const;
                bool valueMutable() const;
                const String& tooltip() const;
                bool isDefault() const;

                void merge(const String& i_valuec, bool nameMutable, bool valueMutable);
                bool multi() const;
                bool subset() const;
                void reset();
            };

            class RowManager {
            private:
                AttributeRow::List m_rows;
                size_t m_defaultRowCount;
            public:
                size_t totalRowCount() const;
                size_t defaultRowCount() const;
                size_t attributeRowCount() const;

                bool isAttributeRow(size_t rowIndex) const;
                bool isDefaultRow(size_t rowIndex) const;

                size_t indexOf(const String& name) const;

                const String& name(size_t rowIndex) const;
                const String& value(size_t rowIndex) const;
                bool nameMutable(size_t rowIndex) const;
                bool valueMutable(size_t rowIndex) const;
                const String& tooltip(size_t rowIndex) const;
                bool multi(size_t rowIndex) const;
                bool subset(size_t rowIndex) const;
                StringList names(size_t rowIndex, size_t count) const;

                bool hasRowWithName(const String& name) const;

                void updateRows(const Model::AttributableNodeList& attributables, bool showDefaultProperties);
                void addAttribute(const Model::AttributeName& name, const Model::AttributeValue& value, const Assets::AttributeDefinition* definition, bool nameMutable, bool valueMutable, bool isDefault, size_t index);

                StringList insertRows(size_t rowIndex, size_t count, const Model::AttributableNodeList& attributables);
                void deleteRows(size_t rowIndex, size_t count);
            private:
                static AttributeRow::List::iterator findRow(AttributeRow::List& rows, const String& name);
                static AttributeRow::List::const_iterator findRow(const AttributeRow::List& rows, const String& name);

                StringList newAttributeNames(size_t count, const Model::AttributableNodeList& attributables) const;
            };

            MapDocumentWPtr m_document;
            RowManager m_rows;
            bool m_ignoreUpdates;
            bool m_showDefaultRows;
        public:
            EntityAttributeGridTable(MapDocumentWPtr document);

            int GetNumberRows() override;
            int GetNumberAttributeRows() const;
            int GetNumberCols() override;

            QString GetValue(int row, int col) override;
            void SetValue(int row, int col, const QString& value) override;

            void Clear() override;
            bool InsertRows(size_t pos = 0, size_t numRows = 1) override;
            bool AppendRows(size_t numRows = 1) override;
            bool DeleteRows(size_t pos = 0, size_t numRows = 1) override;

            QString GetColLabelValue(int col) override;
            wxGridCellAttr* GetAttr(int row, int col, wxGridCellAttr::wxAttrKind kind) override;

            void update();
            String tooltip(const wxGridCellCoords& cellCoords) const;
            Model::AttributeName attributeName(int row) const;
            int rowForName(const Model::AttributeName& name) const;
            bool canRemove(int row);

            bool showDefaultRows() const;
            void setShowDefaultRows(bool showDefaultRows);

            QStringList getCompletions(int row, int col) const;
        private:
            static StringSet allSortedAttributeNames(MapDocumentSPtr document);
            static StringSet allSortedValuesForAttributeNames(MapDocumentSPtr document, const StringList& names);
            /**
             * Returns classnames in use in the map, as well as all classnames in loaded entity definitions.
             */
            static StringSet allSortedClassnames(MapDocumentSPtr document);
            static QStringList arrayString(const StringSet& set);
        private:
            void renameAttribute(size_t rowIndex, const String& newName, const Model::AttributableNodeList& attributables);
            void updateAttribute(size_t rowIndex, const String& newValue, const Model::AttributableNodeList& attributables);

            void notifyRowsUpdated(size_t pos, size_t numRows = 1);
            void notifyRowsInserted(size_t pos = 0, size_t numRows = 1);
            void notifyRowsAppended(size_t numRows = 1);
            void notifyRowsDeleted(size_t pos = 0, size_t numRows = 1);
        };
#endif
    }
}

#endif /* defined(TrenchBroom_EntityAttributeGridTable) */
