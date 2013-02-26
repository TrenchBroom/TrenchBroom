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

#include "ClassInfo.h"

namespace TrenchBroom {
    namespace IO {
        void ClassInfo::resolveBaseClasses(const Map& baseClasses, const StringList& classnames, ClassInfo& classInfo) {
            StringList::const_reverse_iterator classnameIt, classnameEnd;
            for (classnameIt = classnames.rbegin(), classnameEnd = classnames.rend(); classnameIt != classnameEnd; ++classnameIt) {
                const String& classname = *classnameIt;
                ClassInfo::Map::const_iterator baseClassIt = baseClasses.find(classname);
                if (baseClassIt != baseClasses.end()) {
                    const ClassInfo& baseClass = baseClassIt->second;
                    if (!classInfo.hasDescription && baseClass.hasDescription)
                        classInfo.setDescription(baseClass.description);
                    if (!classInfo.hasColor && baseClass.hasColor)
                        classInfo.setColor(baseClass.color);
                    if (!classInfo.hasSize && baseClass.hasSize)
                        classInfo.setSize(baseClass.size);
                    
                    Model::PropertyDefinition::Map::const_iterator propertyIt, propertyEnd;
                    for (propertyIt = baseClass.properties.begin(), propertyEnd = baseClass.properties.end(); propertyIt != propertyEnd; ++propertyIt) {
                        const Model::PropertyDefinition::Ptr property = propertyIt->second;
                        const bool hasProperty = classInfo.properties.find(property->name()) != classInfo.properties.end();
                        if (!hasProperty)
                            classInfo.properties[property->name()] = property;
                    }
                    
                    Model::ModelDefinition::List::const_iterator modelIt, modelEnd;
                    for (modelIt = baseClass.models.begin(), modelEnd = baseClass.models.end(); modelIt != modelEnd; ++modelIt) {
                        const Model::ModelDefinition::Ptr model = *modelIt;
                        classInfo.models.push_back(model);
                    }
                }
            }
        }
    }
}
