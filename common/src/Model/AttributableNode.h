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

#ifndef TrenchBroom_AttributableNode
#define TrenchBroom_AttributableNode

#include "Assets/AssetTypes.h"
#include "Assets/EntityDefinition.h"
#include "Model/EntityAttributes.h"
#include "Model/ModelTypes.h"
#include "Model/Node.h"

namespace TrenchBroom {
    namespace Model {
        class AttributableNode : public Node {
        public: // some helper methods
            static Assets::EntityDefinition* selectEntityDefinition(const AttributableNodeList& attributables);
            static const Assets::AttributeDefinition* selectAttributeDefinition(const AttributeName& name, const AttributableNodeList& attributables);
            static AttributeValue selectAttributeValue(const AttributeName& name, const AttributableNodeList& attributables);
        protected:
            static const String DefaultAttributeValue;

            Assets::EntityDefinition* m_definition;
            EntityAttributes m_attributes;

            AttributableNodeList m_linkSources;
            AttributableNodeList m_linkTargets;
            AttributableNodeList m_killSources;
            AttributableNodeList m_killTargets;

            // cache the classname for faster access
            AttributeValue m_classname;
        public:
            virtual ~AttributableNode();
            
            Assets::EntityDefinition* definition() const;
            void setDefinition(Assets::EntityDefinition* definition);
        public: // attribute management
            const Assets::AttributeDefinition* attributeDefinition(const AttributeName& name) const;
            
            const EntityAttribute::List& attributes() const;
            void setAttributes(const EntityAttribute::List& attributes);

            bool hasAttribute(const AttributeName& name) const;
            bool hasAttribute(const AttributeName& name, const AttributeValue& value) const;
            bool hasAttributeWithPrefix(const AttributeName& prefix, const AttributeValue& value) const;
            bool hasNumberedAttribute(const AttributeName& prefix, const AttributeValue& value) const;
            
            const AttributeValue& attribute(const AttributeName& name, const AttributeValue& defaultValue = DefaultAttributeValue) const;
            const AttributeValue& classname(const AttributeValue& defaultClassname = AttributeValues::NoClassname) const;
            
            EntityAttributeSnapshot attributeSnapshot(const AttributeName& name) const;
            
            template <typename T>
            void addOrUpdateAttribute(const AttributeName& name, const T& value) {
                addOrUpdateAttribute(name, convertValue(value));
            }
            
            template <typename T, size_t S>
            void addOrUpdateAttribute(const AttributeName& name, const Vec<T,S>& value) {
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
            
            class NotifyAttributeChange {
            private:
                NotifyNodeChange m_nodeChange;
                AttributableNode* m_node;
            public:
                NotifyAttributeChange(AttributableNode* node);
                ~NotifyAttributeChange();
            };
            
            void attributesWillChange();
            void attributesDidChange();
            
            void updateClassname();
        private: // search index management
            void addAttributesToIndex();
            void removeAttributesFromIndex();
            void updateAttributeIndex(const EntityAttribute::List& newAttributes);
            
            void addAttributeToIndex(const AttributeName& name, const AttributeValue& value);
            void removeAttributeFromIndex(const AttributeName& name, const AttributeValue& value);
            void updateAttributeIndex(const AttributeName& oldName, const AttributeValue& oldValue, const AttributeName& newName, const AttributeValue& newValue);
        public: // link management
            const AttributableNodeList& linkSources() const;
            const AttributableNodeList& linkTargets() const;
            const AttributableNodeList& killSources() const;
            const AttributableNodeList& killTargets() const;
            
            Vec3 linkSourceAnchor() const;
            Vec3 linkTargetAnchor() const;
            
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
            
            void addLinkTargets(const AttributableNodeList& targets);
            void addKillTargets(const AttributableNodeList& targets);
            void addLinkSources(const AttributableNodeList& sources);
            void addKillSources(const AttributableNodeList& sources);
            
            void removeAllLinkSources();
            void removeAllLinkTargets();
            void removeAllKillSources();
            void removeAllKillTargets();
            
            void removeAllLinks();
            void addAllLinks();
            
            void addLinkSource(AttributableNode* attributable);
            void addLinkTarget(AttributableNode* attributable);
            void addKillSource(AttributableNode* attributable);
            void addKillTarget(AttributableNode* attributable);
            
            void removeLinkSource(AttributableNode* attributable);
            void removeLinkTarget(AttributableNode* attributable);
            void removeKillSource(AttributableNode* attributable);
            void removeKillTarget(AttributableNode* attributable);
        protected:
            AttributableNode();
        private: // implemenation of node interface
            const String& doGetName() const;
            virtual void doAncestorWillChange();
            virtual void doAncestorDidChange();
        private: // subclassing interface
            virtual void doAttributesDidChange() = 0;
            virtual bool doIsAttributeNameMutable(const AttributeName& name) const = 0;
            virtual bool doIsAttributeValueMutable(const AttributeName& name) const = 0;
            virtual Vec3 doGetLinkSourceAnchor() const = 0;
            virtual Vec3 doGetLinkTargetAnchor() const = 0;
        private: // hide copy constructor and assignment operator
            AttributableNode(const AttributableNode&);
            AttributableNode& operator=(const AttributableNode&);
        };
    }
}

#endif /* defined(TrenchBroom_AttributableNode) */
