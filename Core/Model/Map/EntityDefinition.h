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
#include "VecMath.h"

using namespace std;

namespace TrenchBroom {
    namespace Model {
        typedef enum {
            EDT_POINT,
            EDT_BRUSH,
            EDT_BASE
        } EEntityDefinitionType;
        
        typedef enum {
            EDP_CHOICE,
            EDP_MODEL,
            EDP_DEFAULT,
            EDP_BASE
        } EPropertyType;
        
        class Property {
        public:
            EPropertyType type;
            Property(EPropertyType type) : type(type) {};
        };
        
        class BaseProperty : public Property {
        public:
            string baseName;
            BaseProperty(string& baseName) : Property(EDP_BASE), baseName(baseName) {};
        };
        
        class DefaultProperty : public Property {
        public:
            string name;
            string value;
            DefaultProperty(string& name, string& value) : Property(EDP_DEFAULT), name(name), value(value) {};
        };
        
        class ModelProperty : public Property {
        public:
            string flagName;
            string modelPath;
            int skinIndex;
            ModelProperty(string& flagName, string& modelPath, int skinIndex) : Property(EDP_MODEL), flagName(flagName), modelPath(modelPath), skinIndex(skinIndex) {};
            ModelProperty(string& modelPath, int skinIndex) : Property(EDP_MODEL), flagName(""), modelPath(modelPath), skinIndex(skinIndex) {};
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
            ChoiceProperty(string& name, vector<ChoiceArgument>& arguments) : Property(EDP_CHOICE), name(name), arguments(arguments) {};
        };
        
        class SpawnFlag {
        public:
            string name;
            int flag;
            SpawnFlag() {};
            SpawnFlag(string& name, int flag) : name(name), flag(flag) {};
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
        };
        
        typedef enum {
            ES_NAME,
            ES_USAGE
        } EEntityDefinitionSortCriterion;
        
        class EntityDefinitionManager {
        private:
            map<const string, EntityDefinition*> m_definitions;
            vector<EntityDefinition*> m_definitionsByName;
        public:
            EntityDefinitionManager(const string& path);
            ~EntityDefinitionManager();
            static EntityDefinitionManager* sharedManager(const string& path);
            
            EntityDefinition* definition(const string& name) const;
            const vector<EntityDefinition*> definitions() const;
            const vector<EntityDefinition*> definitions(EEntityDefinitionType type) const;
            const vector<EntityDefinition*>definitions(EEntityDefinitionType type, EEntityDefinitionSortCriterion criterion) const;
        };
    }
}
#endif
