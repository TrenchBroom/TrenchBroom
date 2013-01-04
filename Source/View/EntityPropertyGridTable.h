/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__EntityPropertyGridTable__
#define __TrenchBroom__EntityPropertyGridTable__

#include "Model/EntityTypes.h"
#include "Utility/String.h"

#include <wx/grid.h>

#include <vector>

namespace TrenchBroom {
    namespace Model {
        class MapDocument;
    }
    
    namespace View {
        class EntityPropertyGridTable : public wxGridTableBase {
        protected:
            class EntityProperty {
            public:
                String key;
                String value;
                bool multi;
            public:
                EntityProperty() {}
                EntityProperty(const String& i_key, const String& i_value) :
                key(i_key),
                value(i_value),
                multi(false) {}
                
                EntityProperty(const String& i_key) :
                key(i_key),
                value(""),
                multi(true) {}
            };
            
            typedef std::vector<EntityProperty> EntityPropertyList;
            
            Model::MapDocument& m_document;
            EntityPropertyList m_properties;
            
            Model::EntityList selectedEntities();

            void notifyRowsUpdated();
            void notifyRowsInserted(size_t pos = 0, size_t numRows = 1);
            void notifyRowsAppended(size_t numRows = 1);
            void notifyRowsDeleted(size_t pos = 0, size_t numRows = 1);
        public:
            EntityPropertyGridTable(Model::MapDocument& document);
            
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
        };
    }
}

#endif /* defined(__TrenchBroom__EntityPropertyGridTable__) */
