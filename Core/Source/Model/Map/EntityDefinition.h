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
        
        bool compareByName(const EntityDefinition* def1, const EntityDefinition* def2);
        bool compareByUsage(const EntityDefinition* def1, const EntityDefinition* def2);
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
            ChoiceArgument(int key, string& value) : key(key), value(value) {};
        };
        
        class ChoiceProperty : public Property {
        public:
            string name;
            vector<ChoiceArgument> arguments;
            ChoiceProperty(const string& name, vector<ChoiceArgument>& arguments) : Property(TB_EDP_CHOICE), name(name), arguments(arguments) {};
        };
        
        class SpawnFlag {
        public:
            string name;
            int flag;
            SpawnFlag() {};
            SpawnFlag(const string& name, int flag) : name(name), flag(flag) {};
        };
        
        class EntityDefinition {
        public:
            static EntityDefinition* baseDefinition(const string& name, const map<string, SpawnFlag>& flags, const vector<Property*>& properties);
            static EntityDefinition* pointDefinition(const string& name, const Vec4f& color, const BBox& bounds, const map<string, SpawnFlag>& flags, const vector<Property*>& properties, const string& description);
            static EntityDefinition* brushDefinition(const string& name, const Vec4f& color, const map<string, SpawnFlag>& flags, vector<Property*> properties, const string& description);
            ~EntityDefinition();
            EEntityDefinitionType type;
            string name;
            Vec4f color;
            Vec3f center;
            BBox bounds;
            BBox maxBounds;
            map<string, SpawnFlag> flags;
            vector<Property*> properties;
            string description;
            int usageCount;
            
            vector<SpawnFlag> flagsForMask(int mask) const;
            bool flagSetOnEntity(const string& name, const Entity& entity) const;
            ModelProperty* modelPropertyForEntity(const Entity& entity) const;
            ModelProperty* defaultModelProperty() const;
        };
        
        typedef enum {
            ES_NAME,
            ES_USAGE
        } EEntityDefinitionSortCriterion;
        
        class EntityDefinitionMap;
        class EntityDefinitionManager {
        private:
            map<const string, EntityDefinition*> m_definitions;
            vector<EntityDefinition*> m_definitionsByName;
        public:
            static EntityDefinitionMap* sharedManagers;
            
            EntityDefinitionManager(const string& path);
            ~EntityDefinitionManager();
            static EntityDefinitionManager* sharedManager(const string& path);
            
            EntityDefinition* definition(const string& name) const;
            const vector<EntityDefinition*> definitions() const;
            const vector<EntityDefinition*> definitions(EEntityDefinitionType type) const;
            const vector<EntityDefinition*>definitions(EEntityDefinitionType type, EEntityDefinitionSortCriterion criterion) const;
        };

        class EntityDefinitionMap {
        public:
            map<string, EntityDefinitionManager*> managers;
            ~EntityDefinitionMap() {
                map<string, EntityDefinitionManager*>::iterator it;
                for (it = managers.begin(); it != managers.end(); ++it)
                    delete it->second;
            }
        };
        
    }
}
#endif
