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

#ifndef __TrenchBroom__EntityPropertyIndex__
#define __TrenchBroom__EntityPropertyIndex__

#include "StringUtils.h"
#include "Model/ModelTypes.h"
#include "StringIndex.h"

#include <map>

namespace TrenchBroom {
    namespace Model {
        class AttributableIndexQuery {
        public:
            typedef enum {
                Type_Exact,
                Type_Prefix,
                Type_Numbered,
                Type_Any
            } Type;
        private:
            Type m_type;
            String m_pattern;
        public:
            static AttributableIndexQuery exact(const String& pattern);
            static AttributableIndexQuery prefix(const String& pattern);
            static AttributableIndexQuery numbered(const String& pattern);
            static AttributableIndexQuery any();

            AttributableSet execute(const StringIndex<Attributable*>& index) const;
        private:
            AttributableIndexQuery(Type type, const String& pattern = "");
        };
        
        class AttributableIndex {
        private:
            StringIndex<Attributable*> m_nameIndex;
            StringIndex<Attributable*> m_valueIndex;
        public:
            void addAttributable(Attributable* attributable);
            void removeAttributable(Attributable* attributable);
            
            void addAttribute(Attributable* attributable, const AttributeName& name, const AttributeValue& value);
            void removeAttribute(Attributable* attributable, const AttributeName& name, const AttributeValue& value);
            
            AttributableList findAttributables(const AttributableIndexQuery& keyQuery, const AttributableIndexQuery& valueQuery) const;
        };
    }
}

#endif /* defined(__TrenchBroom__EntityPropertyIndex__) */
