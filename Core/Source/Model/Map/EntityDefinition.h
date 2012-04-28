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

using namespace std;

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
        };
        
        class BaseProperty : public Property {
        public:
            string baseName;
            BaseProperty(const string& baseName) : Property(TB_EDP_BASE), baseName(baseName) {};
        };
        
        class DefaultProperty : public Property {
        public:
            string name;
            string value;
            DefaultProperty(const string& name, const string& value) : Property(TB_EDP_DEFAULT), name(name), value(value) {};
        };
        
        class ModelProperty : public Property {
        public:
            string flagName;
            string modelPath;
            int skinIndex;
            ModelProperty(const string& flagName, const string& modelPath, int skinIndex) : Property(TB_EDP_MODEL), flagName(flagName), modelPath(modelPath), skinIndex(skinIndex) {};
            ModelProperty(const string& modelPath, int skinIndex) : Property(TB_EDP_MODEL), flagName(""), modelPath(modelPath), skinIndex(skinIndex) {};
        };
        
        class ChoiceArgument {
        public:
            int key;
            string value;
            ChoiceArgument(int key, const string& value) : key(key), value(value) {};
        };
        
        class ChoiceProperty : public Property {
        public:
            string name;
            vector<ChoiceArgument> arguments;
            ChoiceProperty(const string& name, const vector<ChoiceArgument>& arguments) : Property(TB_EDP_CHOICE), name(name), arguments(arguments) {};
        };
        
        typedef tr1::shared_ptr<Property> PropertyPtr;
        typedef tr1::shared_ptr<ModelProperty> ModelPropertyPtr;
        
        class SpawnFlag {
        public:
            string name;
            int flag;
            SpawnFlag() {};
            SpawnFlag(const string& name, int flag) : name(name), flag(flag) {};
        };
        
        typedef tr1::shared_ptr<EntityDefinition> EntityDefinitionPtr;
        bool compareByName(const EntityDefinitionPtr def1, const EntityDefinitionPtr def2);
        bool compareByUsage(const EntityDefinitionPtr def1, const EntityDefinitionPtr def2);
        
        class EntityDefinition {
        public:
            static EntityDefinitionPtr baseDefinition(const string& name, const map<string, SpawnFlag>& flags, const vector<PropertyPtr>& properties);
            static EntityDefinitionPtr pointDefinition(const string& name, const Vec4f& color, const BBox& bounds, const map<string, SpawnFlag>& flags, const vector<PropertyPtr>& properties, const string& description);
            static EntityDefinitionPtr brushDefinition(const string& name, const Vec4f& color, const map<string, SpawnFlag>& flags, const vector<PropertyPtr>& properties, const string& description);
            EEntityDefinitionType type;
            string name;
            Vec4f color;
            Vec3f center;
            BBox bounds;
            BBox maxBounds;
            map<string, SpawnFlag> flags;
            vector<PropertyPtr> properties;
            string description;
            int usageCount;
            
            vector<SpawnFlag> flagsForMask(int mask) const;
            bool flagSetOnEntity(const string& name, const Entity& entity) const;
            ModelPropertyPtr modelPropertyForEntity(const Entity& entity) const;
            ModelPropertyPtr defaultModelProperty() const;
        };
        
        typedef enum {
            ES_NAME,
            ES_USAGE
        } EEntityDefinitionSortCriterion;

        class EntityDefinitionManager;
        typedef tr1::shared_ptr<EntityDefinitionManager> EntityDefinitionManagerPtr;
        typedef map<string, EntityDefinitionManagerPtr> EntityDefinitionManagerMap;
        
        class EntityDefinitionManager {
        private:
            map<const string, EntityDefinitionPtr> m_definitions;
            vector<EntityDefinitionPtr> m_definitionsByName;
        public:
            static EntityDefinitionManagerMap* sharedManagers;
            
            EntityDefinitionManager(const string& path);
            static EntityDefinitionManagerPtr sharedManager(const string& path);
            
            EntityDefinitionPtr definition(const string& name) const;
            const vector<EntityDefinitionPtr>& definitions() const;
            vector<EntityDefinitionPtr> definitions(EEntityDefinitionType type) const;
            vector<EntityDefinitionPtr> definitions(EEntityDefinitionType type, EEntityDefinitionSortCriterion criterion) const;
        };
    }
}
#endif
