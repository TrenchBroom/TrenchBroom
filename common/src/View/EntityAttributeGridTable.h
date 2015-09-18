/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include <wx/grid.h>

#include <vector>

namespace TrenchBroom {
    namespace View {
        class EntityAttributeGridTable : public wxGridTableBase {
        private:
            class AttributeRow {
            public:
                typedef std::vector<AttributeRow> List;
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
                const StringList names(size_t rowIndex, size_t count) const;
                
                void updateRows(const Model::AttributableNodeList& attributables, bool showDefaultProperties);
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
            wxColor m_readonlyCellColor;
            wxColor m_specialCellColor;
        public:
            EntityAttributeGridTable(MapDocumentWPtr document);
            
            int GetNumberRows();
            int GetNumberAttributeRows() const;
            int GetNumberCols();
            
            wxString GetValue(int row, int col);
            void SetValue(int row, int col, const wxString& value);
            
            void Clear();
            bool InsertRows(size_t pos = 0, size_t numRows = 1);
            bool AppendRows(size_t numRows = 1);
            bool DeleteRows(size_t pos = 0, size_t numRows = 1);
            
            wxString GetColLabelValue(int col);
            wxGridCellAttr* GetAttr(int row, int col, wxGridCellAttr::wxAttrKind kind);
            
            void update();
            String tooltip(wxGridCellCoords cellCoords) const;
            Model::AttributeName attributeName(int row) const;
            int rowForName(const Model::AttributeName& name) const;
            bool canRemove(int row);
            
            bool showDefaultRows() const;
            void setShowDefaultRows(bool showDefaultRows);
        private:
            void renameAttribute(size_t rowIndex, const String& newName, const Model::AttributableNodeList& attributables);
            void updateAttribute(size_t rowIndex, const String& newValue, const Model::AttributableNodeList& attributables);
            
            void notifyRowsUpdated(size_t pos, size_t numRows = 1);
            void notifyRowsInserted(size_t pos = 0, size_t numRows = 1);
            void notifyRowsAppended(size_t numRows = 1);
            void notifyRowsDeleted(size_t pos = 0, size_t numRows = 1);
        };
    }
}

#endif /* defined(TrenchBroom_EntityAttributeGridTable) */
