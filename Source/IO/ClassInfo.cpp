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
                        const Model::PropertyDefinition::Ptr baseclassProperty = propertyIt->second;

                        Model::PropertyDefinition::Map::iterator classPropertyIt = classInfo.properties.find(baseclassProperty->name());
                        if (classPropertyIt != classInfo.properties.end()) {
                            // the class already has a definition for this property, attempt merging them
                            mergeProperties(baseclassProperty.get(), classPropertyIt->second.get());
                        } else {
                            // the class doesn't have a definition for this property, add the base class property
                            classInfo.properties[baseclassProperty->name()] = baseclassProperty;
                        }
                    }
                    
                    Model::ModelDefinition::List::const_iterator modelIt, modelEnd;
                    for (modelIt = baseClass.models.begin(), modelEnd = baseClass.models.end(); modelIt != modelEnd; ++modelIt) {
                        const Model::ModelDefinition::Ptr model = *modelIt;
                        classInfo.models.push_back(model);
                    }
                }
            }
        }

        void ClassInfo::mergeProperties(const Model::PropertyDefinition* baseclassProperty, Model::PropertyDefinition* classProperty) {
            // for now, only merge spawnflags
            if (baseclassProperty->type() == Model::PropertyDefinition::FlagsProperty &&
                classProperty->type() == Model::PropertyDefinition::FlagsProperty &&
                baseclassProperty->name() == Model::Entity::SpawnFlagsKey &&
                classProperty->name() == Model::Entity::SpawnFlagsKey) {
                
                const Model::FlagsPropertyDefinition* baseclassFlags = static_cast<const Model::FlagsPropertyDefinition*>(baseclassProperty);
                Model::FlagsPropertyDefinition* classFlags = static_cast<Model::FlagsPropertyDefinition*>(classProperty);
                
                for (int i = 0; i < 24; i++) {
                    const Model::FlagsPropertyOption* baseclassFlag = baseclassFlags->option(static_cast<int>(1 << i));
                    Model::FlagsPropertyOption* classFlag = classFlags->option(static_cast<int>(1 << i));

                    if (baseclassFlag != NULL && classFlag == NULL)
                        classFlags->addOption(baseclassFlag->value(), baseclassFlag->description(), baseclassFlag->isDefault());
                }
            }
        }
    }
}
