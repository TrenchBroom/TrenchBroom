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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
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
            class Entry {
            private:
                size_t m_maxCount;
                size_t m_count;
                bool m_multi;
            public:
                String key;
                String value;
                String tooltip;
                
                Entry();
                Entry(const String& i_key, const String& i_value, const String& i_tooltip, size_t maxCount);
                
                void compareValue(const String& i_value);
                bool multi() const;
                bool subset() const;
                void reset();
            };
            
            typedef std::vector<Entry> EntryList;
            
            MapDocumentPtr m_document;
            ControllerFacade& m_controller;
            EntryList m_entries;
            bool m_ignoreUpdates;
            wxColor m_specialCellColor;
        public:
            EntityPropertyGridTable(MapDocumentPtr document, ControllerFacade& controller);
            
            int GetNumberRows();
            int GetNumberCols();
            
            wxString GetValue(int row, int col);
            void SetValue(int row, int col, const wxString& value);
            
            void Clear();
            bool InsertRows(size_t pos = 0, size_t numRows = 1);
            bool AppendRows(size_t numRows = 1);
            bool DeleteRows(size_t pos = 0, size_t numRows = 1);
            
            wxString GetColLabelValue(int col);
            wxGridCellAttr* getAttr(int row, int col, wxGridCellAttr::wxAttrKind kind);
            
            void update();
            String tooltip(const wxGridCellCoords cellCoords) const;
        private:
            EntryList::iterator findEntry(EntryList& entries, const String& key) const;
            
            void notifyRowsUpdated(size_t pos, size_t numRows = 1);
            void notifyRowsInserted(size_t pos = 0, size_t numRows = 1);
            void notifyRowsAppended(size_t numRows = 1);
            void notifyRowsDeleted(size_t pos = 0, size_t numRows = 1);
        };
    }
}

#endif /* defined(__TrenchBroom__EntityPropertyGridTable__) */
