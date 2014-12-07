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

#ifndef __TrenchBroom__Attributable__
#define __TrenchBroom__Attributable__

#include "Assets/AssetTypes.h"
#include "Assets/EntityDefinition.h"
#include "Model/EntityAttributes.h"
#include "Model/ModelTypes.h"
#include "Model/Node.h"

namespace TrenchBroom {
    namespace Model {
        class Attributable : public Node {
        public: // some helper methods
            static Assets::EntityDefinition* selectEntityDefinition(const AttributableList& attributables);
            static const Assets::AttributeDefinition* selectAttributeDefinition(const AttributeName& name, const AttributableList& attributables);
            static AttributeValue selectAttributeValue(const AttributeName& name, const AttributableList& attributables);
        protected:
            static const String DefaultAttributeValue;

            Assets::EntityDefinition* m_definition;
            EntityAttributes m_attributes;

            AttributableList m_linkSources;
            AttributableList m_linkTargets;
            AttributableList m_killSources;
            AttributableList m_killTargets;
        public:
            virtual ~Attributable();
            
            Assets::EntityDefinition* definition() const;
            void setDefinition(Assets::EntityDefinition* definition);
        public: // attribute management
            const Assets::AttributeDefinition* attributeDefinition(const AttributeName& name) const;
            
            const EntityAttribute::List& attributes() const;
            void setAttributes(const EntityAttribute::List& attributes);

            bool hasAttribute(const AttributeName& name) const;
            const AttributeValue& attribute(const AttributeName& name, const AttributeValue& defaultValue = DefaultAttributeValue) const;
            const AttributeValue& classname(const AttributeValue& defaultClassname = AttributeValues::NoClassname) const;
            
            template <typename T>
            void addOrUpdateAttribute(const AttributeName& name, const T& value) {
                addOrUpdateAttribute(name, convertValue(value));
            }
            
            template <typename T, size_t S>
            void addOrUpdateAttribute(const AttributeName& name, const Vec<T,S> value) {
                addOrUpdateAttribute(name, value.asString());
            }
            
            
            bool canAddOrUpdateAttribute(const AttributeName& name, const AttributeValue& value) const;
            void addOrUpdateAttribute(const AttributeName& name, const AttributeValue& value);
            
            bool canRenameAttribute(const AttributeName& name, const AttributeName& newName) const;
            void renameAttribute(const AttributeName& name, const AttributeName& newName);
            
            bool canRemoveAttribute(const AttributeName& name) const;
            void removeAttribute(const AttributeName& name);
            
            bool isAttributeNameMutable(const AttributeName& name) const;
            bool isAttributeValueMutable(const AttributeName& name) const;
        private: // attribute management internals
            template <typename T>
            AttributeValue convertValue(const T& value) const {
                static StringStream str;
                str.str("");
                str << value;
                return str.str();
            }
            void attributesDidChange();
        private: // search index management
            void addAttributesToIndex();
            void removeAttributesFromIndex();
            void updateAttributeIndex(const EntityAttribute::List& newAttributes);
            
            void addAttributeToIndex(const AttributeName& name, const AttributeValue& value);
            void removeAttributeFromIndex(const AttributeName& name, const AttributeValue& value);
            void updateAttributeIndex(const AttributeName& oldName, const AttributeValue& oldValue, const AttributeName& newName, const AttributeValue& newValue);
        public: // link management
            const AttributableList& linkSources() const;
            const AttributableList& linkTargets() const;
            const AttributableList& killSources() const;
            const AttributableList& killTargets() const;
            
            bool hasMissingSources() const;
            AttributeNameList findMissingLinkTargets() const;
            AttributeNameList findMissingKillTargets() const;
        private: // link management internals
            void findMissingTargets(const AttributeName& prefix, AttributeNameList& result) const;
            
            void addLinks(const AttributeName& name, const AttributeValue& value);
            void removeLinks(const AttributeName& name, const AttributeValue& value);
            void updateLinks(const AttributeName& oldName, const AttributeName& oldValue, const AttributeName& newName, const AttributeValue& newValue);
            
            void addLinkTargets(const AttributeValue& targetname);
            void addKillTargets(const AttributeValue& targetname);
            
            void removeLinkTargets(const AttributeValue& targetname);
            void removeKillTargets(const AttributeValue& targetname);

            void addAllLinkSources(const AttributeValue& targetname);
            void addAllLinkTargets();
            void addAllKillSources(const AttributeValue& targetname);
            void addAllKillTargets();
            
            void addLinkTargets(const AttributableList& targets);
            void addKillTargets(const AttributableList& targets);
            void addLinkSources(const AttributableList& sources);
            void addKillSources(const AttributableList& sources);
            
            void removeAllLinkSources();
            void removeAllLinkTargets();
            void removeAllKillSources();
            void removeAllKillTargets();
            
            void refreshAllLinks();
            
            void addLinkSource(Attributable* attributable);
            void addLinkTarget(Attributable* attributable);
            void addKillSource(Attributable* attributable);
            void addKillTarget(Attributable* attributable);
            
            void removeLinkSource(Attributable* attributable);
            void removeLinkTarget(Attributable* attributable);
            void removeKillSource(Attributable* attributable);
            void removeKillTarget(Attributable* attributable);
        protected:
            Attributable();
        private: // implemenation of node interface
            virtual void doAncestorWillChange();
            virtual void doAncestorDidChange();
        private: // subclassing interface
            virtual void doAttributesDidChange() = 0;
            virtual bool doIsAttributeNameMutable(const AttributeName& name) const = 0;
            virtual bool doIsAttributeValueMutable(const AttributeName& name) const = 0;
        private: // hide copy constructor and assignment operator
            Attributable(const Attributable&);
            Attributable& operator=(const Attributable&);
        };
    }
}

#endif /* defined(__TrenchBroom__Attributable__) */
