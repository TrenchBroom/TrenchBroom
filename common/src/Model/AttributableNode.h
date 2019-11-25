/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "Notifier.h"
#include "Assets/AssetTypes.h"
#include "Assets/EntityDefinition.h"
#include "Model/EntityAttributes.h"
#include "Model/ModelTypes.h"
#include "Model/Node.h"

#include <vector>

namespace TrenchBroom {
    namespace Model {
        class AttributableNode : public Node {
        public: // some helper methods
            static Assets::EntityDefinition* selectEntityDefinition(const std::vector<AttributableNode*>& attributables);
            static const Assets::AttributeDefinition* selectAttributeDefinition(const AttributeName& name, const std::vector<AttributableNode*>& attributables);
            static AttributeValue selectAttributeValue(const AttributeName& name, const std::vector<AttributableNode*>& attributables);
        protected:
            static const String DefaultAttributeValue;

            Assets::EntityDefinition* m_definition;
            EntityAttributes m_attributes;

            std::vector<AttributableNode*> m_linkSources;
            std::vector<AttributableNode*> m_linkTargets;
            std::vector<AttributableNode*> m_killSources;
            std::vector<AttributableNode*> m_killTargets;

            // cache the classname for faster access
            AttributeValue m_classname;
        public:
            virtual ~AttributableNode() override;
        public: // definition
            Assets::EntityDefinition* definition() const;
            void setDefinition(Assets::EntityDefinition* definition);
        public: // notification
            using AttributeNotifier = Notifier<AttributableNode*, const AttributeName&>;

            AttributeNotifier attributeWasAddedNotifier;
            AttributeNotifier attributeWillBeRemovedNotifier;
            AttributeNotifier attributeWillChangeNotifier;
            AttributeNotifier attributeDidChangeNotifier;
        public: // attribute management
            const Assets::AttributeDefinition* attributeDefinition(const AttributeName& name) const;

            const EntityAttribute::List& attributes() const;
            void setAttributes(const EntityAttribute::List& attributes);

            AttributeNameSet attributeNames() const;

            bool hasAttribute(const AttributeName& name) const;
            bool hasAttribute(const AttributeName& name, const AttributeValue& value) const;
            bool hasAttributeWithPrefix(const AttributeName& prefix, const AttributeValue& value) const;
            bool hasNumberedAttribute(const AttributeName& prefix, const AttributeValue& value) const;

            EntityAttribute::List attributeWithName(const AttributeName& name) const;
            EntityAttribute::List attributesWithPrefix(const AttributeName& prefix) const;
            EntityAttribute::List numberedAttributes(const String& prefix) const;

            const AttributeValue& attribute(const AttributeName& name, const AttributeValue& defaultValue = DefaultAttributeValue) const;
            const AttributeValue& classname(const AttributeValue& defaultClassname = AttributeValues::NoClassname) const;

            EntityAttributeSnapshot attributeSnapshot(const AttributeName& name) const;

            bool canAddOrUpdateAttribute(const AttributeName& name, const AttributeValue& value) const;
            bool addOrUpdateAttribute(const AttributeName& name, const AttributeValue& value);

            bool canRenameAttribute(const AttributeName& name, const AttributeName& newName) const;
            void renameAttribute(const AttributeName& name, const AttributeName& newName);

            bool canRemoveAttribute(const AttributeName& name) const;
            void removeAttribute(const AttributeName& name);
            void removeNumberedAttribute(const AttributeName& prefix);

            bool isAttributeNameMutable(const AttributeName& name) const;
            bool isAttributeValueMutable(const AttributeName& name) const;
        private: // attribute management internals
            class NotifyAttributeChange {
            private:
                NotifyNodeChange m_nodeChange;
                AttributableNode* m_node;
                vm::bbox3 m_oldPhysicalBounds;
            public:
                NotifyAttributeChange(AttributableNode* node);
                ~NotifyAttributeChange();
            };

            void attributesWillChange();
            void attributesDidChange(const vm::bbox3& oldPhysicalBounds);

            void updateClassname();
        private: // search index management
            void addAttributesToIndex();
            void removeAttributesFromIndex();
            void updateAttributeIndex(const EntityAttribute::List& newAttributes);

            void addAttributeToIndex(const AttributeName& name, const AttributeValue& value);
            void removeAttributeFromIndex(const AttributeName& name, const AttributeValue& value);
            void updateAttributeIndex(const AttributeName& oldName, const AttributeValue& oldValue, const AttributeName& newName, const AttributeValue& newValue);
        public: // link management
            const std::vector<AttributableNode*>& linkSources() const;
            const std::vector<AttributableNode*>& linkTargets() const;
            const std::vector<AttributableNode*>& killSources() const;
            const std::vector<AttributableNode*>& killTargets() const;

            vm::vec3 linkSourceAnchor() const;
            vm::vec3 linkTargetAnchor() const;

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

            void addLinkTargets(const std::vector<AttributableNode*>& targets);
            void addKillTargets(const std::vector<AttributableNode*>& targets);
            void addLinkSources(const std::vector<AttributableNode*>& sources);
            void addKillSources(const std::vector<AttributableNode*>& sources);

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
            const String& doGetName() const override;
            virtual void doAncestorWillChange() override;
            virtual void doAncestorDidChange() override;
        private: // subclassing interface
            virtual void doAttributesDidChange(const vm::bbox3& oldBounds) = 0;
            virtual bool doIsAttributeNameMutable(const AttributeName& name) const = 0;
            virtual bool doIsAttributeValueMutable(const AttributeName& name) const = 0;
            virtual vm::vec3 doGetLinkSourceAnchor() const = 0;
            virtual vm::vec3 doGetLinkTargetAnchor() const = 0;
        private: // hide copy constructor and assignment operator
            AttributableNode(const AttributableNode&);
            AttributableNode& operator=(const AttributableNode&);
        };
    }
}

#endif /* defined(TrenchBroom_AttributableNode) */
