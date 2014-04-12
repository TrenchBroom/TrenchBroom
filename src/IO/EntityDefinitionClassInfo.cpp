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

#include "EntityDefinitionClassInfo.h"

#include "Assets/PropertyDefinition.h"
#include "Model/EntityProperties.h"

namespace TrenchBroom {
    namespace IO {
        EntityDefinitionClassInfo::EntityDefinitionClassInfo() :
        m_line(0),
        m_column(0),
        m_hasDescription(false),
        m_hasColor(false),
        m_size(BBox3(-8.0, 8.0)),
        m_hasSize(false) {}
        
        EntityDefinitionClassInfo::EntityDefinitionClassInfo(const size_t line, const size_t column, const Color& defaultColor) :
        m_line(line),
        m_column(column),
        m_hasDescription(false),
        m_color(defaultColor),
        m_hasColor(false),
        m_size(BBox3(-8.0, 8.0)),
        m_hasSize(false) {}
        
        
        size_t EntityDefinitionClassInfo::line() const {
            return m_line;
        }
        
        size_t EntityDefinitionClassInfo::column() const {
            return m_column;
        }

        const String& EntityDefinitionClassInfo::name() const {
            return m_name;
        }
        
        const String& EntityDefinitionClassInfo::description() const {
            return m_description;
        }
        
        bool EntityDefinitionClassInfo::hasDescription() const {
            return m_hasDescription;
        }
        
        const Color& EntityDefinitionClassInfo::color() const {
            return m_color;
        }
        
        bool EntityDefinitionClassInfo::hasColor() const {
            return m_hasColor;
        }
        
        const BBox3& EntityDefinitionClassInfo::size() const {
            return m_size;
        }
        
        bool EntityDefinitionClassInfo::hasSize() const {
            return m_hasSize;
        }

        Assets::PropertyDefinitionList EntityDefinitionClassInfo::propertyList() const {
            Assets::PropertyDefinitionList list;
            Assets::PropertyDefinitionMap::const_iterator propertyIt, propertyEnd;
            for (propertyIt = m_properties.begin(), propertyEnd = m_properties.end(); propertyIt != propertyEnd; ++propertyIt)
                list.push_back(propertyIt->second);
            return list;
        }
        
        const Assets::PropertyDefinitionMap& EntityDefinitionClassInfo::propertyMap() const {
            return m_properties;
        }
        
        const Assets::ModelDefinitionList& EntityDefinitionClassInfo::models() const {
            return m_models;
        }

        void EntityDefinitionClassInfo::setName(const String& name) {
            m_name = name;
        }

        void EntityDefinitionClassInfo::setDescription(const String& description) {
            m_description = description;
            m_hasDescription = true;
        }
        
        void EntityDefinitionClassInfo::setColor(const Color& color) {
            m_color = color;
            m_hasColor = true;
        }
        
        void EntityDefinitionClassInfo::setSize(const BBox3& size) {
            m_size = size;
            m_hasSize = true;
        }
        
        void EntityDefinitionClassInfo::addPropertyDefinition(Assets::PropertyDefinitionPtr propertyDefinition) {
            m_properties[propertyDefinition->name()] = propertyDefinition;
        }
        
        void EntityDefinitionClassInfo::addPropertyDefinitions(const Assets::PropertyDefinitionList& propertyDefinitions) {
            Assets::PropertyDefinitionList::const_iterator it, end;
            for (it = propertyDefinitions.begin(), end = propertyDefinitions.end(); it != end; ++it)
                addPropertyDefinition(*it);
        }

        void EntityDefinitionClassInfo::addPropertyDefinitions(const Assets::PropertyDefinitionMap& propertyDefinitions) {
            m_properties.insert(propertyDefinitions.begin(), propertyDefinitions.end());
        }

        void EntityDefinitionClassInfo::addModelDefinition(Assets::ModelDefinitionPtr modelDefinition) {
            m_models.push_back(modelDefinition);
        }

        void EntityDefinitionClassInfo::addModelDefinitions(const Assets::ModelDefinitionList& modelDefinitions) {
            m_models.insert(m_models.end(), modelDefinitions.begin(), modelDefinitions.end());
        }

        void EntityDefinitionClassInfo::resolveBaseClasses(const EntityDefinitionClassInfoMap& baseClasses, const StringList& classnames) {
            StringList::const_reverse_iterator classnameIt, classnameEnd;
            for (classnameIt = classnames.rbegin(), classnameEnd = classnames.rend(); classnameIt != classnameEnd; ++classnameIt) {
                const String& classname = *classnameIt;
                EntityDefinitionClassInfoMap::const_iterator baseClassIt = baseClasses.find(classname);
                if (baseClassIt != baseClasses.end()) {
                    const EntityDefinitionClassInfo& baseClass = baseClassIt->second;
                    if (!hasDescription() && baseClass.hasDescription())
                        setDescription(baseClass.description());
                    if (!hasColor() && baseClass.hasColor())
                        setColor(baseClass.color());
                    if (!hasSize() && baseClass.hasSize())
                        setSize(baseClass.size());
                    
                    const Assets::PropertyDefinitionMap& baseProperties = baseClass.propertyMap();
                    Assets::PropertyDefinitionMap::const_iterator propertyIt, propertyEnd;
                    for (propertyIt = baseProperties.begin(), propertyEnd = baseProperties.end(); propertyIt != propertyEnd; ++propertyIt) {
                        const Assets::PropertyDefinitionPtr baseProperty = propertyIt->second;
                        
                        Assets::PropertyDefinitionMap::iterator classPropertyIt = m_properties.find(baseProperty->name());
                        if (classPropertyIt != m_properties.end()) {
                            // the class already has a definition for this property, attempt merging them
                            mergeProperties(baseProperty.get(), classPropertyIt->second.get());
                        } else {
                            // the class doesn't have a definition for this property, add the base class property
                            addPropertyDefinition(baseProperty);
                        }
                    }
                    
                    const Assets::ModelDefinitionList& baseModels = baseClass.models();
                    Assets::ModelDefinitionList::const_iterator modelIt, modelEnd;
                    for (modelIt = baseModels.begin(), modelEnd = baseModels.end(); modelIt != modelEnd; ++modelIt) {
                        const Assets::ModelDefinitionPtr model = *modelIt;
                        addModelDefinition(model);
                    }
                }
            }
        }

        void EntityDefinitionClassInfo::mergeProperties(Assets::PropertyDefinition* classProperty, const Assets::PropertyDefinition* baseclassProperty) {
            // for now, only merge spawnflags
            if (baseclassProperty->type() == Assets::PropertyDefinition::Type_FlagsProperty &&
                classProperty->type() == Assets::PropertyDefinition::Type_FlagsProperty &&
                baseclassProperty->name() == Model::PropertyKeys::Spawnflags &&
                classProperty->name() == Model::PropertyKeys::Spawnflags) {
                
                const Assets::FlagsPropertyDefinition* baseclassFlags = static_cast<const Assets::FlagsPropertyDefinition*>(baseclassProperty);
                Assets::FlagsPropertyDefinition* classFlags = static_cast<Assets::FlagsPropertyDefinition*>(classProperty);
                
                for (int i = 0; i < 24; i++) {
                    const Assets::FlagsPropertyOption* baseclassFlag = baseclassFlags->option(static_cast<int>(1 << i));
                    const Assets::FlagsPropertyOption* classFlag = classFlags->option(static_cast<int>(1 << i));
                    
                    if (baseclassFlag != NULL && classFlag == NULL)
                        classFlags->addOption(baseclassFlag->value(), baseclassFlag->description(), baseclassFlag->isDefault());
                }
            }
        }
    }
}
