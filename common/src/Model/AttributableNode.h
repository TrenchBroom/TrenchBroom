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

#include "Model/EntityAttributes.h"
#include "Model/Node.h"

#include <vecmath/bbox.h>

#include <set>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Assets {
        class AttributeDefinition;
        class EntityDefinition;
    }

    namespace Model {
        class AttributableNode : public Node {
        public: // some helper methods
            static Assets::EntityDefinition* selectEntityDefinition(const std::vector<AttributableNode*>& attributables);
            static const Assets::AttributeDefinition* selectAttributeDefinition(const std::string& name, const std::vector<AttributableNode*>& attributables);
            static std::string selectAttributeValue(const std::string& name, const std::vector<AttributableNode*>& attributables);
        protected:
            static const std::string DefaultAttributeValue;

            Assets::EntityDefinition* m_definition;
            EntityAttributes m_attributes;

            std::vector<AttributableNode*> m_linkSources;
            std::vector<AttributableNode*> m_linkTargets;
            std::vector<AttributableNode*> m_killSources;
            std::vector<AttributableNode*> m_killTargets;

            // cache the classname for faster access
            std::string m_classname;
        public:
            virtual ~AttributableNode() override;
        public: // definition
            Assets::EntityDefinition* definition() const;
            void setDefinition(Assets::EntityDefinition* definition);
        public: // attribute management
            const Assets::AttributeDefinition* attributeDefinition(const std::string& name) const;

            const std::vector<EntityAttribute>& attributes() const;
            void setAttributes(const std::vector<EntityAttribute>& attributes);

            std::vector<std::string> attributeNames() const;

            bool hasAttribute(const std::string& name) const;
            bool hasAttribute(const std::string& name, const std::string& value) const;
            bool hasAttributeWithPrefix(const std::string& prefix, const std::string& value) const;
            bool hasNumberedAttribute(const std::string& prefix, const std::string& value) const;

            std::vector<EntityAttribute> attributeWithName(const std::string& name) const;
            std::vector<EntityAttribute> attributesWithPrefix(const std::string& prefix) const;
            std::vector<EntityAttribute> numberedAttributes(const std::string& prefix) const;

            const std::string& attribute(const std::string& name, const std::string& defaultValue = DefaultAttributeValue) const;
            const std::string& classname(const std::string& defaultClassname = AttributeValues::NoClassname) const;

            EntityAttributeSnapshot attributeSnapshot(const std::string& name) const;

            bool canAddOrUpdateAttribute(const std::string& name, const std::string& value) const;
            bool addOrUpdateAttribute(const std::string& name, const std::string& value);

            bool canRenameAttribute(const std::string& name, const std::string& newName) const;
            void renameAttribute(const std::string& name, const std::string& newName);

            bool canRemoveAttribute(const std::string& name) const;
            void removeAttribute(const std::string& name);
            void removeNumberedAttribute(const std::string& prefix);

            bool isAttributeNameMutable(const std::string& name) const;
            bool isAttributeValueMutable(const std::string& name) const;
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
            void updateAttributeIndex(const std::vector<EntityAttribute>& newAttributes);

            void addAttributeToIndex(const std::string& name, const std::string& value);
            void removeAttributeFromIndex(const std::string& name, const std::string& value);
            void updateAttributeIndex(const std::string& oldName, const std::string& oldValue, const std::string& newName, const std::string& newValue);
        public: // link management
            const std::vector<AttributableNode*>& linkSources() const;
            const std::vector<AttributableNode*>& linkTargets() const;
            const std::vector<AttributableNode*>& killSources() const;
            const std::vector<AttributableNode*>& killTargets() const;

            vm::vec3 linkSourceAnchor() const;
            vm::vec3 linkTargetAnchor() const;

            bool hasMissingSources() const;
            std::vector<std::string> findMissingLinkTargets() const;
            std::vector<std::string> findMissingKillTargets() const;
        private: // link management internals
            void findMissingTargets(const std::string& prefix, std::vector<std::string>& result) const;

            void addLinks(const std::string& name, const std::string& value);
            void removeLinks(const std::string& name, const std::string& value);
            void updateLinks(const std::string& oldName, const std::string& oldValue, const std::string& newName, const std::string& newValue);

            void addLinkTargets(const std::string& targetname);
            void addKillTargets(const std::string& targetname);

            void removeLinkTargets(const std::string& targetname);
            void removeKillTargets(const std::string& targetname);

            void addAllLinkSources(const std::string& targetname);
            void addAllLinkTargets();
            void addAllKillSources(const std::string& targetname);
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
            const std::string& doGetName() const override;
            virtual void doAncestorWillChange() override;
            virtual void doAncestorDidChange() override;
        private: // subclassing interface
            virtual void doAttributesDidChange(const vm::bbox3& oldBounds) = 0;
            virtual bool doIsAttributeNameMutable(const std::string& name) const = 0;
            virtual bool doIsAttributeValueMutable(const std::string& name) const = 0;
            virtual vm::vec3 doGetLinkSourceAnchor() const = 0;
            virtual vm::vec3 doGetLinkTargetAnchor() const = 0;
        private: // hide copy constructor and assignment operator
            AttributableNode(const AttributableNode&);
            AttributableNode& operator=(const AttributableNode&);
        };

        bool operator==(const AttributableNode& lhs, const AttributableNode& rhs);
        bool operator!=(const AttributableNode& lhs, const AttributableNode& rhs);
    }
}

#endif /* defined(TrenchBroom_AttributableNode) */
