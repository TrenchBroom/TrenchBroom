/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#ifndef __TrenchBroom__EntityPropertyGridTable__
#define __TrenchBroom__EntityPropertyGridTable__

#include "StringUtils.h"
#include "Model/ModelTypes.h"
#include "View/ViewTypes.h"

#include <wx/grid.h>
#include <vector>

namespace TrenchBroom {
    namespace View {
        class EntityPropertyGridTable : public wxGridTableBase {
        private:
            class PropertyRow {
            public:
                typedef std::vector<PropertyRow> List;
            private:
                String m_key;
                String m_value;
                String m_tooltip;
                
                size_t m_maxCount;
                size_t m_count;
                bool m_multi;
            public:
                PropertyRow();
                PropertyRow(const String& key, const String& value, const String& tooltip, size_t maxCount);
                
                const String& key() const;
                const String& value() const;
                const String& tooltip() const;
                
                void compareValue(const String& i_value);
                bool multi() const;
                bool subset() const;
                void reset();
            };
            
            class DefaultRow {
            public:
                typedef std::vector<DefaultRow> List;
            private:
                String m_key;
                String m_value;
                String m_tooltip;
            public:
                DefaultRow();
                DefaultRow(const String& key, const String& value, const String& tooltip);
                
                const String& key() const;
                const String& value() const;
                const String& tooltip() const;
            };
            
            class RowManager {
            private:
                PropertyRow::List m_propertyRows;
                DefaultRow::List m_defaultRows;
            public:
                size_t propertyCount() const;
                size_t rowCount() const;
                
                bool isPropertyRow(size_t rowIndex) const;
                bool isDefaultRow(size_t rowIndex) const;

                const String& key(size_t rowIndex) const;
                const String& value(size_t rowIndex) const;
                const String& tooltip(size_t rowIndex) const;
                bool multi(size_t rowIndex) const;
                bool subset(size_t rowIndex) const;
                const StringList keys(size_t rowIndex, size_t count) const;
                
                void updateRows(const Model::EntityList& entities);
                StringList insertRows(size_t rowIndex, size_t count, const Model::EntityList& entities);
                void deleteRows(size_t rowIndex, size_t count);
            private:
                const PropertyRow& propertyRow(size_t rowIndex) const;
                PropertyRow& propertyRow(size_t rowIndex);
                const DefaultRow& defaultRow(size_t rowIndex) const;
                DefaultRow& defaultRow(size_t rowIndex);
                
                PropertyRow::List collectPropertyRows(const Model::EntityList& entities) const;
                DefaultRow::List collectDefaultRows(const Model::EntityList& entities, const PropertyRow::List& propertyRows) const;

                PropertyRow::List::iterator findPropertyRow(PropertyRow::List& rows, const String& key) const;
                PropertyRow::List::const_iterator findPropertyRow(const PropertyRow::List& rows, const String& key) const;
                DefaultRow::List::iterator findDefaultRow(DefaultRow::List& rows, const String& key) const;
                
                StringList newKeyNames(size_t count, const Model::EntityList& entities) const;
            };
            
            MapDocumentPtr m_document;
            ControllerPtr m_controller;
            RowManager m_rows;
            bool m_ignoreUpdates;
            wxColor m_readonlyCellColor;
            wxColor m_specialCellColor;
        public:
            EntityPropertyGridTable(MapDocumentPtr document, ControllerPtr controller);
            
            int GetNumberRows();
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
        private:
            void renameProperty(size_t rowIndex, const String& newKey, const Model::EntityList& entities);
            void updateProperty(size_t rowIndex, const String& newValue, const Model::EntityList& entities);
            
            void notifyRowsUpdated(size_t pos, size_t numRows = 1);
            void notifyRowsInserted(size_t pos = 0, size_t numRows = 1);
            void notifyRowsAppended(size_t numRows = 1);
            void notifyRowsDeleted(size_t pos = 0, size_t numRows = 1);
        };
    }
}

#endif /* defined(__TrenchBroom__EntityPropertyGridTable__) */
