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

#ifndef __TrenchBroom__PropertyDefinition__
#define __TrenchBroom__PropertyDefinition__

#include "Utility/String.h"

#include <vector>

namespace TrenchBroom {
    namespace Model {
        class PropertyDefinition {
        public:
            typedef std::vector<PropertyDefinition*> List;
            
            enum class Type {
                Enum
            };
        private:
            String m_name;
            Type m_type;
        public:
            PropertyDefinition(const String& name, Type type) : m_name(name), m_type(type) {}
            virtual ~PropertyDefinition();
            
            inline const String& name() const {
                return m_name;
            }
            
            inline Type type() const {
                return m_type;
            }
        };
        
        class EnumPropertyDefinition : public PropertyDefinition {
        private:
            StringList m_values;
        public:
            EnumPropertyDefinition(const String& name, const StringList& values) :
            PropertyDefinition(name, Type::Enum), m_values(values) {}
            
            inline const StringList& values() const {
                return m_values;
            }
        };
    }
}

#endif /* defined(__TrenchBroom__PropertyDefinition__) */
