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

#ifndef TrenchBroom_EntityDefinition_h
#define TrenchBroom_EntityDefinition_h

#include <string>
#include <vector>
#include <map>
#include "Utilities/VecMath.h"
#include "Utilities/SharedPointer.h"
#include <cstdio>

namespace TrenchBroom {
    namespace Model {
        typedef enum {
            TB_EDT_POINT,
            TB_EDT_BRUSH,
            TB_EDT_BASE
        } EEntityDefinitionType;
        
        typedef enum {
            TB_EDP_CHOICE,
            TB_EDP_MODEL,
            TB_EDP_DEFAULT,
            TB_EDP_BASE
        } EPropertyType;

        class EntityDefinition;
        class Entity;
        class SpawnFlag;
        
        bool compareByFlag(const SpawnFlag& left, const SpawnFlag& right);

        class Property {
        public:
            EPropertyType type;
            Property(EPropertyType type) : type(type) {};
            virtual ~Property() {};
        };
        
        class BaseProperty : public Property {
        public:
            std::string baseName;
            BaseProperty(const std::string& baseName) : Property(TB_EDP_BASE), baseName(baseName) {};
            virtual ~BaseProperty() {}
        };
        
        class DefaultProperty : public Property {
        public:
            std::string name;
            std::string value;
            DefaultProperty(const std::string& name, const std::string& value) : Property(TB_EDP_DEFAULT), name(name), value(value) {};
            virtual ~DefaultProperty() {}
        };
        
        class ModelProperty : public Property {
        public:
            std::string flagName;
            std::string modelPath;
            int skinIndex;
            ModelProperty(const std::string& flagName, const std::string& modelPath, int skinIndex) : Property(TB_EDP_MODEL), flagName(flagName), modelPath(modelPath), skinIndex(skinIndex) {};
            ModelProperty(const std::string& modelPath, int skinIndex) : Property(TB_EDP_MODEL), flagName(""), modelPath(modelPath), skinIndex(skinIndex) {};
            virtual ~ModelProperty() {}
        };
        
        class ChoiceArgument {
        public:
            int key;
            std::string value;
            ChoiceArgument(int key, const std::string& value) : key(key), value(value) {};
            virtual ~ChoiceArgument() {}
        };
        
        class ChoiceProperty : public Property {
        public:
            std::string name;
            std::vector<ChoiceArgument> arguments;
            ChoiceProperty(const std::string& name, const std::vector<ChoiceArgument>& arguments) : Property(TB_EDP_CHOICE), name(name), arguments(arguments) {};
            ~ChoiceProperty() {}
        };
        
        typedef std::tr1::shared_ptr<Property> PropertyPtr;
        typedef std::tr1::shared_ptr<ModelProperty> ModelPropertyPtr;
        
        class SpawnFlag {
        public:
            std::string name;
            int flag;
            SpawnFlag() {};
            SpawnFlag(const std::string& name, int flag) : name(name), flag(flag) {};
        };
        
        typedef std::tr1::shared_ptr<EntityDefinition> EntityDefinitionPtr;
        typedef std::vector<EntityDefinitionPtr> EntityDefinitionList;
        bool compareByName(const EntityDefinitionPtr def1, const EntityDefinitionPtr def2);
        bool compareByUsage(const EntityDefinitionPtr def1, const EntityDefinitionPtr def2);
        
        class EntityDefinition {
        public:
            static EntityDefinitionPtr baseDefinition(const std::string& name, const std::map<std::string, SpawnFlag>& flags, const std::vector<PropertyPtr>& properties);
            static EntityDefinitionPtr pointDefinition(const std::string& name, const Vec4f& color, const BBox& bounds, const std::map<std::string, SpawnFlag>& flags, const std::vector<PropertyPtr>& properties, const std::string& description);
            static EntityDefinitionPtr brushDefinition(const std::string& name, const Vec4f& color, const std::map<std::string, SpawnFlag>& flags, const std::vector<PropertyPtr>& properties, const std::string& description);
            EEntityDefinitionType type;
            std::string name;
            Vec4f color;
            Vec3f center;
            BBox bounds;
            BBox maxBounds;
            std::map<std::string, SpawnFlag> flags;
            std::vector<PropertyPtr> properties;
            std::string description;
            int usageCount;
            
            std::vector<SpawnFlag> flagsForMask(int mask) const;
            bool flagSetOnEntity(const std::string& name, const Entity& entity) const;
            ModelPropertyPtr modelPropertyForEntity(const Entity& entity) const;
            ModelPropertyPtr defaultModelProperty() const;
        };
        
        typedef enum {
            ES_NAME,
            ES_USAGE
        } EEntityDefinitionSortCriterion;

        class EntityDefinitionManager;
        typedef std::tr1::shared_ptr<EntityDefinitionManager> EntityDefinitionManagerPtr;
        typedef std::map<std::string, EntityDefinitionManagerPtr> EntityDefinitionManagerMap;
        
        class EntityDefinitionManager {
        private:
            std::map<const std::string, EntityDefinitionPtr> m_definitions;
            EntityDefinitionList m_definitionsByName;
        public:
            static EntityDefinitionManagerMap* sharedManagers;
            
            EntityDefinitionManager(const std::string& path);
            static EntityDefinitionManagerPtr sharedManager(const std::string& path);
            
            EntityDefinitionPtr definition(const std::string& name) const;
            const EntityDefinitionList& definitions() const;
            EntityDefinitionList definitions(EEntityDefinitionType type) const;
            EntityDefinitionList definitions(EEntityDefinitionType type, EEntityDefinitionSortCriterion criterion) const;
        };
    }
}
#endif
