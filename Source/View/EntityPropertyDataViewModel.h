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

#ifndef __TrenchBroom__EntityPropertyDataViewModel__
#define __TrenchBroom__EntityPropertyDataViewModel__

#include "Model/EntityTypes.h"
#include "Utility/String.h"

#include <wx/dataview.h>

#include <vector>

namespace TrenchBroom {
    namespace Model {
        class MapDocument;
    }
    
    namespace View {
        class EntityPropertyDataViewModel : public wxDataViewVirtualListModel {
        protected:
            class EntityProperty {
            public:
                String key;
                String value;
                bool multi;
            public:
                EntityProperty() {}
                EntityProperty(const String& key, const String& value) :
                key(key),
                value(value),
                multi(false) {}

                EntityProperty(const String& key) :
                key(key),
                value(""),
                multi(true) {}
            };
            
            typedef std::vector<EntityProperty> EntityPropertyList;
            
            Model::MapDocument& m_document;
            EntityPropertyList m_properties;
            
            void clear();
        public:
            EntityPropertyDataViewModel(Model::MapDocument& document);
            
            virtual unsigned int GetColumnCount() const;
            virtual wxString GetColumnType(unsigned int col) const;
            virtual void GetValueByRow(wxVariant &variant, unsigned int row, unsigned int col) const;
            virtual bool SetValueByRow(const wxVariant &variant, unsigned int row, unsigned int col);
            virtual bool GetAttrByRow(unsigned int row, unsigned int col, wxDataViewItemAttr &attr) const;

            void update();
        };
    }
}

#endif /* defined(__TrenchBroom__EntityPropertyDataViewModel__) */
