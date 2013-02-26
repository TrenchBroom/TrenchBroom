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

#ifndef TrenchBroom_ClassInfo_h
#define TrenchBroom_ClassInfo_h

#include "Model/EntityDefinition.h"
#include "Model/PropertyDefinition.h"
#include "Utility/Color.h"
#include "Utility/String.h"

namespace TrenchBroom {
    namespace IO {
        struct ClassInfo {
            typedef std::map<String, ClassInfo> Map;
            
            size_t line;
            size_t column;
            String name;
            String description;
            bool hasDescription;
            Color color;
            bool hasColor;
            BBox size;
            bool hasSize;
            Model::PropertyDefinition::Map properties;
            Model::ModelDefinition::List models;
            
            ClassInfo();
            ClassInfo(size_t i_line, size_t i_column, const Color& defaultColor);
            
            inline void setDescription(const String& i_description) {
                description = i_description;
                hasDescription = true;
            }
            
            inline void setColor(const Color& i_color) {
                color = i_color;
                hasColor = true;
            }
            
            inline void setSize(const BBox& i_size) {
                size = i_size;
                hasSize = true;
            }
            
            inline Model::PropertyDefinition::List propertyList() const {
                Model::PropertyDefinition::List list;
                Model::PropertyDefinition::Map::const_iterator propertyIt, propertyEnd;
                for (propertyIt = properties.begin(), propertyEnd = properties.end(); propertyIt != propertyEnd; ++propertyIt)
                    list.push_back(propertyIt->second);
                return list;
            }

            static void resolveBaseClasses(const Map& baseClasses, const StringList& classnames, ClassInfo& classInfo);
        };
    }
}

#endif
