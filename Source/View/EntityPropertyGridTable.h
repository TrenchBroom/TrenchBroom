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
            class Entry {
            private:
                size_t m_maxCount;
                size_t m_count;
                bool m_multi;
            public:
                String key;
                String value;
                String tooltip;

                Entry() {}
                Entry(const String& i_key, const String& i_value, const String& i_tooltip, size_t maxCount) :
                m_maxCount(maxCount),
                m_count(1),
                m_multi(false),
                key(i_key),
                value(i_value),
                tooltip(i_tooltip) {}

                inline void compareValue(const String& i_value) {
                    if (!m_multi && value != i_value)
                        m_multi = true;
                    m_count++;
                }

                inline bool multi() const {
                    return m_multi;
                }

                inline bool subset() const {
                    return m_count < m_maxCount;
                }

                inline void reset() {
                    m_count = m_maxCount;
                    m_multi = false;
                }
            };

            typedef std::vector<Entry> EntryList;

            Model::MapDocument& m_document;
            EntryList m_entries;
            bool m_ignoreUpdates;
            wxColor m_specialCellColor;

            EntryList::iterator findEntry(EntryList& entries, const String& key) const;

            Model::EntityList selectedEntities();

            void notifyRowsUpdated(size_t pos, size_t numRows = 1);
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
            String tooltip(wxGridCellCoords cellCoords) const;
        };
    }
}

#endif /* defined(__TrenchBroom__EntityPropertyGridTable__) */
