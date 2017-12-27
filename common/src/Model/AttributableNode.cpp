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

#include "AttributableNode.h"

#include "Assets/AttributeDefinition.h"

namespace TrenchBroom {
    namespace Model {
        Assets::EntityDefinition* AttributableNode::selectEntityDefinition(const AttributableNodeList& attributables) {
            Assets::EntityDefinition* definition = NULL;
            
            for (AttributableNode* attributable : attributables) {
                if (definition == NULL) {
                    definition = attributable->definition();
                } else if (definition != attributable->definition()) {
                    definition = NULL;
                    break;
                }
            }
            
            return definition;
        }
        
        const Assets::AttributeDefinition* AttributableNode::selectAttributeDefinition(const AttributeName& name, const AttributableNodeList& attributables) {
            AttributableNodeList::const_iterator it = std::begin(attributables);
            AttributableNodeList::const_iterator end = std::end(attributables);
            if (it == end)
                return NULL;
            
            const AttributableNode* attributable = *it;
            const Assets::AttributeDefinition* definition = attributable->attributeDefinition(name);
            if (definition == NULL)
                return NULL;
            
            while (++it != end) {
                attributable = *it;
                const Assets::AttributeDefinition* currentDefinition = attributable->attributeDefinition(name);
                if (currentDefinition == NULL)
                    return NULL;
                
                if (!definition->equals(currentDefinition))
                    return NULL;
            }
            
            return definition;
        }
        
        AttributeValue AttributableNode::selectAttributeValue(const AttributeName& name, const AttributableNodeList& attributables) {
            AttributableNodeList::const_iterator it = std::begin(attributables);
            AttributableNodeList::const_iterator end = std::end(attributables);
            if (it == end)
                return "";
            
            const AttributableNode* attributable = *it;
            if (!attributable->hasAttribute(name))
                return "";
            
            const AttributeValue& value = attributable->attribute(name);
            while (++it != end) {
                attributable = *it;
                if (!attributable->hasAttribute(name))
                    return "";
                if (value != attributable->attribute(name))
                    return "";
            }
            return value;
        }

        const String AttributableNode::DefaultAttributeValue("");

        AttributableNode::~AttributableNode() {
            m_definition = NULL;
        }
        
        Assets::EntityDefinition* AttributableNode::definition() const {
            return m_definition;
        }
        
        void AttributableNode::setDefinition(Assets::EntityDefinition* definition) {
            if (m_definition == definition)
                return;
            
            const NotifyAttributeChange notifyChange(this);
            if (m_definition != NULL)
                m_definition->decUsageCount();
            m_definition = definition;
            m_attributes.updateDefinitions(m_definition);
            if (m_definition != NULL)
                m_definition->incUsageCount();
        }

        const Assets::AttributeDefinition* AttributableNode::attributeDefinition(const AttributeName& name) const {
            return m_definition == NULL ? NULL : m_definition->attributeDefinition(name);
        }

        const EntityAttribute::List& AttributableNode::attributes() const {
            return m_attributes.attributes();
        }
        
        void AttributableNode::setAttributes(const EntityAttribute::List& attributes) {
            for (const EntityAttribute& attribute : m_attributes.attributes())
                attributeWillBeRemovedNotifier(this, attribute.name());
            
            const NotifyAttributeChange notifyChange(this);
            updateAttributeIndex(attributes);
            m_attributes.setAttributes(attributes);
            m_attributes.updateDefinitions(m_definition);

            for (const EntityAttribute& attribute : m_attributes.attributes())
                attributeWasAddedNotifier(this, attribute.name());
        }
        
        AttributeNameSet AttributableNode::attributeNames() const {
            return m_attributes.names();
        }

        bool AttributableNode::hasAttribute(const AttributeName& name) const {
            return m_attributes.hasAttribute(name);
        }
        
        bool AttributableNode::hasAttribute(const AttributeName& name, const AttributeValue& value) const {
            return m_attributes.hasAttribute(name, value);
        }
        
        bool AttributableNode::hasAttributeWithPrefix(const AttributeName& prefix, const AttributeValue& value) const {
            return m_attributes.hasAttributeWithPrefix(prefix, value);
        }
        
        bool AttributableNode::hasNumberedAttribute(const AttributeName& prefix, const AttributeValue& value) const {
            return m_attributes.hasNumberedAttribute(prefix, value);
        }
        
        EntityAttribute::List AttributableNode::attributeWithName(const AttributeName& name) const {
            return m_attributes.attributeWithName(name);
        }
        
        EntityAttribute::List AttributableNode::attributesWithPrefix(const AttributeName& prefix) const {
            return m_attributes.attributesWithPrefix(prefix);
        }
        
        EntityAttribute::List AttributableNode::numberedAttributes(const String& prefix) const {
            return m_attributes.numberedAttributes(prefix);
        }
        
        const AttributeValue& AttributableNode::attribute(const AttributeName& name, const AttributeValue& defaultValue) const {
            const AttributeValue* value = m_attributes.attribute(name);
            if (value == NULL)
                return defaultValue;
            return *value;
        }

        const AttributeValue& AttributableNode::classname(const AttributeValue& defaultClassname) const {
            return m_classname.empty() ? defaultClassname : m_classname;
        }

        EntityAttributeSnapshot AttributableNode::attributeSnapshot(const AttributeName& name) const {
            return m_attributes.snapshot(name);
        }

        bool AttributableNode::canAddOrUpdateAttribute(const AttributeName& name, const AttributeValue& value) const {
            return isAttributeValueMutable(name);
        }
        
        bool AttributableNode::addOrUpdateAttribute(const AttributeName& name, const AttributeValue& value) {
            const NotifyAttributeChange notifyChange(this);

            const Assets::AttributeDefinition* definition = Assets::EntityDefinition::safeGetAttributeDefinition(m_definition, name);
            const AttributeValue* oldValue = m_attributes.attribute(name);
            if (oldValue != NULL) {
                attributeWillChangeNotifier(this, name);
                removeAttributeFromIndex(name, *oldValue);
                removeLinks(name, *oldValue);
            }
            
            m_attributes.addOrUpdateAttribute(name, value, definition);
            addAttributeToIndex(name, value);
            addLinks(name, value);
            
            if (oldValue == NULL)
                attributeWasAddedNotifier(this, name);
            return oldValue == NULL;
        }
        
        bool AttributableNode::canRenameAttribute(const AttributeName& name, const AttributeName& newName) const {
            return isAttributeNameMutable(name) && isAttributeNameMutable(newName);
        }
        
        void AttributableNode::renameAttribute(const AttributeName& name, const AttributeName& newName) {
            if (name == newName)
                return;
            
            const AttributeValue* valuePtr = m_attributes.attribute(name);
            if (valuePtr == NULL)
                return;
            
            const AttributeValue value = *valuePtr;
			const NotifyAttributeChange notifyChange(this);

            const Assets::AttributeDefinition* newDefinition = Assets::EntityDefinition::safeGetAttributeDefinition(m_definition, newName);
            
            attributeWillBeRemovedNotifier(this, name);
            m_attributes.renameAttribute(name, newName, newDefinition);
            
            updateAttributeIndex(name, value, newName, value);
            updateLinks(name, value, newName, value);
            attributeWasAddedNotifier(this, newName);
        }
        
        bool AttributableNode::canRemoveAttribute(const AttributeName& name) const {
            return isAttributeNameMutable(name) && isAttributeValueMutable(name);
        }
        
        void AttributableNode::removeAttribute(const AttributeName& name) {
            const AttributeValue* valuePtr = m_attributes.attribute(name);
            if (valuePtr == NULL)
                return;

            attributeWillBeRemovedNotifier(this, name);
            const NotifyAttributeChange notifyChange(this);

            const AttributeValue value = *valuePtr;
            m_attributes.removeAttribute(name);
            
            removeAttributeFromIndex(name, value);
            removeLinks(name, value);
        }
        
        void AttributableNode::removeNumberedAttribute(const AttributeName& prefix) {
            const EntityAttribute::List attributes = m_attributes.numberedAttributes(prefix);
            if (!attributes.empty()) {
                const NotifyAttributeChange notifyChange(this);

                for (const EntityAttribute& attribute : m_attributes.attributes()) {
                    const AttributeName& name = attribute.name();
                    const AttributeValue& value = attribute.value();
                    
                    attributeWillBeRemovedNotifier(this, name);
                    m_attributes.removeAttribute(name);
                    removeAttributeFromIndex(name, value);
                    removeLinks(name, value);
                }
            }
        }

        bool AttributableNode::isAttributeNameMutable(const AttributeName& name) const {
            return doIsAttributeNameMutable(name);
        }
        
        bool AttributableNode::isAttributeValueMutable(const AttributeName& name) const {
            return doIsAttributeValueMutable(name);
        }

        AttributableNode::NotifyAttributeChange::NotifyAttributeChange(AttributableNode* node) :
        m_nodeChange(node),
        m_node(node) {
            ensure(m_node != NULL, "node is null");
            m_node->attributesWillChange();
        }
        
        AttributableNode::NotifyAttributeChange::~NotifyAttributeChange() {
            m_node->attributesDidChange();
        }

        void AttributableNode::attributesWillChange() {}

        void AttributableNode::attributesDidChange() {
            updateClassname();
            doAttributesDidChange();
        }

        void AttributableNode::updateClassname() {
            m_classname = attribute(AttributeNames::Classname);
        }

        void AttributableNode::addAttributesToIndex() {
            for (const EntityAttribute& attribute : m_attributes.attributes())
                addAttributeToIndex(attribute.name(), attribute.value());
        }
        
        void AttributableNode::removeAttributesFromIndex() {
            for (const EntityAttribute& attribute : m_attributes.attributes())
                removeAttributeFromIndex(attribute.name(), attribute.value());
        }

        void AttributableNode::updateAttributeIndex(const EntityAttribute::List& newAttributes) {
            EntityAttribute::List oldSorted = m_attributes.attributes();
            EntityAttribute::List newSorted = newAttributes;
            
            oldSorted.sort();
            newSorted.sort();
            
            auto oldIt = std::begin(oldSorted);
            auto oldEnd = std::end(oldSorted);
            auto newIt = std::begin(newSorted);
            auto newEnd = std::end(newSorted);
            
            while (oldIt != oldEnd && newIt != newEnd) {
                const EntityAttribute& oldAttr = *oldIt;
                const EntityAttribute& newAttr = *newIt;
                
                const int cmp = oldAttr.compare(newAttr);
                if (cmp < 0) {
                    removeAttributeFromIndex(oldAttr.name(), oldAttr.value());
                    ++oldIt;
                } else if (cmp > 0) {
                    addAttributeToIndex(newAttr.name(), newAttr.value());
                    ++newIt;
                } else {
                    updateAttributeIndex(oldAttr.name(), oldAttr.value(), newAttr.name(), newAttr.value());
                    ++oldIt; ++newIt;
                }
            }
            
            while (oldIt != oldEnd) {
                const EntityAttribute& oldAttr = *oldIt;
                removeAttributeFromIndex(oldAttr.name(), oldAttr.value());
                ++oldIt;
            }
            
            while (newIt != newEnd) {
                const EntityAttribute& newAttr = *newIt;
                addAttributeToIndex(newAttr.name(), newAttr.value());
                ++newIt;
            }
        }
        
        void AttributableNode::addAttributeToIndex(const AttributeName& name, const AttributeValue& value) {
            addToIndex(this, name, value);
        }
        
        void AttributableNode::removeAttributeFromIndex(const AttributeName& name, const AttributeValue& value) {
            removeFromIndex(this, name, value);
        }
        
        void AttributableNode::updateAttributeIndex(const AttributeName& oldName, const AttributeValue& oldValue, const AttributeName& newName, const AttributeValue& newValue) {
            removeFromIndex(this, oldName, oldValue);
            addToIndex(this, newName, newValue);
        }
        
        const AttributableNodeList& AttributableNode::linkSources() const {
            return m_linkSources;
        }
        
        const AttributableNodeList& AttributableNode::linkTargets() const {
            return m_linkTargets;
        }
        
        const AttributableNodeList& AttributableNode::killSources() const {
            return m_killSources;
        }
        
        const AttributableNodeList& AttributableNode::killTargets() const {
            return m_killTargets;
        }
        
        Vec3 AttributableNode::linkSourceAnchor() const {
            return doGetLinkSourceAnchor();
        }
        
        Vec3 AttributableNode::linkTargetAnchor() const {
            return doGetLinkTargetAnchor();
        }

        bool AttributableNode::hasMissingSources() const {
            return (m_linkSources.empty() &&
                    m_killSources.empty() &&
                    hasAttribute(AttributeNames::Targetname));
        }
        
        AttributeNameList AttributableNode::findMissingLinkTargets() const {
            AttributeNameList result;
            findMissingTargets(AttributeNames::Target, result);
            return result;
        }
        
        AttributeNameList AttributableNode::findMissingKillTargets() const {
            AttributeNameList result;
            findMissingTargets(AttributeNames::Killtarget, result);
            return result;
        }

        void AttributableNode::findMissingTargets(const AttributeName& prefix, AttributeNameList& result) const {
            for (const EntityAttribute& attribute : m_attributes.numberedAttributes(prefix)) {
                const AttributeValue& targetname = attribute.value();
                if (targetname.empty()) {
                    result.push_back(attribute.name());
                } else {
                    AttributableNodeList linkTargets;
                    findAttributableNodesWithAttribute(AttributeNames::Targetname, targetname, linkTargets);
                    if (linkTargets.empty())
                        result.push_back(attribute.name());
                }
            }
        }

        void AttributableNode::addLinks(const AttributeName& name, const AttributeValue& value) {
            if (isNumberedAttribute(AttributeNames::Target, name)) {
                addLinkTargets(value);
            } else if (isNumberedAttribute(AttributeNames::Killtarget, name)) {
                addKillTargets(value);
            } else if (name == AttributeNames::Targetname) {
                addAllLinkSources(value);
                addAllKillSources(value);
            }
        }
        
        void AttributableNode::removeLinks(const AttributeName& name, const AttributeValue& value) {
            if (isNumberedAttribute(AttributeNames::Target, name)) {
                removeLinkTargets(value);
            } else if (isNumberedAttribute(AttributeNames::Killtarget, name)) {
                removeKillTargets(value);
            } else if (name == AttributeNames::Targetname) {
                removeAllLinkSources();
                removeAllKillSources();
            }
        }
        
        void AttributableNode::updateLinks(const AttributeName& oldName, const AttributeName& oldValue, const AttributeName& newName, const AttributeValue& newValue) {
            removeLinks(oldName, oldValue);
            addLinks(newName, newValue);
        }

        void AttributableNode::addLinkTargets(const AttributeValue& targetname) {
            if (!targetname.empty()) {
                AttributableNodeList targets;
                findAttributableNodesWithAttribute(AttributeNames::Targetname, targetname, targets);
                addLinkTargets(targets);
            }
        }
        
        void AttributableNode::addKillTargets(const AttributeValue& targetname) {
            if (!targetname.empty()) {
                AttributableNodeList targets;
                findAttributableNodesWithAttribute(AttributeNames::Targetname, targetname, targets);
                addKillTargets(targets);
            }
        }

        void AttributableNode::removeLinkTargets(const AttributeValue& targetname) {
            if (!targetname.empty()) {
                AttributableNodeList::iterator rem = std::end(m_linkTargets);
                AttributableNodeList::iterator it = std::begin(m_linkTargets);
                while (it != rem) {
                    AttributableNode* target = *it;
                    const AttributeValue& targetTargetname = target->attribute(AttributeNames::Targetname);
                    if (targetTargetname == targetname) {
                        target->removeLinkSource(this);
                        --rem;
                        std::iter_swap(it, rem);
                    } else {
                        ++it;
                    }
                }
                m_linkTargets.erase(rem, std::end(m_linkTargets));
            }
        }
        
        void AttributableNode::removeKillTargets(const AttributeValue& targetname) {
            if (!targetname.empty()) {
                AttributableNodeList::iterator rem = std::end(m_killTargets);
                AttributableNodeList::iterator it = std::begin(m_killTargets);
                while (it != rem) {
                    AttributableNode* target = *it;
                    const AttributeValue& targetTargetname = target->attribute(AttributeNames::Targetname);
                    if (targetTargetname == targetname) {
                        target->removeKillSource(this);
                        --rem;
                        std::iter_swap(it, rem);
                    } else {
                        ++it;
                    }
                }
                m_killTargets.erase(rem, std::end(m_killTargets));
            }
        }

        void AttributableNode::addAllLinkSources(const AttributeValue& targetname) {
            if (!targetname.empty()) {
                AttributableNodeList linkSources;
                findAttributableNodesWithNumberedAttribute(AttributeNames::Target, targetname, linkSources);
                addLinkSources(linkSources);
            }
        }
        
        void AttributableNode::addAllLinkTargets() {
            for (const EntityAttribute& attribute : m_attributes.numberedAttributes(AttributeNames::Target)) {
                const String& targetname = attribute.value();
                if (!targetname.empty()) {
                    AttributableNodeList linkTargets;
                    findAttributableNodesWithAttribute(AttributeNames::Targetname, targetname, linkTargets);
                    addLinkTargets(linkTargets);
                }
            }
        }
        
        void AttributableNode::addAllKillSources(const AttributeValue& targetname) {
            if (!targetname.empty()) {
                AttributableNodeList killSources;
                findAttributableNodesWithNumberedAttribute(AttributeNames::Killtarget, targetname, killSources);
                addKillSources(killSources);
            }
        }
        
        void AttributableNode::addAllKillTargets() {
            for (const EntityAttribute& attribute : m_attributes.numberedAttributes(AttributeNames::Killtarget)) {
                const String& targetname = attribute.value();
                if (!targetname.empty()) {
                    AttributableNodeList killTargets;
                    findAttributableNodesWithAttribute(AttributeNames::Targetname, targetname, killTargets);
                    addKillTargets(killTargets);
                }
            }
        }

        void AttributableNode::addLinkTargets(const AttributableNodeList& targets) {
            m_linkTargets.reserve(m_linkTargets.size() + targets.size());
            for (AttributableNode* target : targets) {
                target->addLinkSource(this);
                m_linkTargets.push_back(target);
            }
            invalidateIssues();
        }
        
        void AttributableNode::addKillTargets(const AttributableNodeList& targets) {
            m_killTargets.reserve(m_killTargets.size() + targets.size());
            for (AttributableNode* target : targets) {
                target->addKillSource(this);
                m_killTargets.push_back(target);
            }
            invalidateIssues();
        }

        void AttributableNode::addLinkSources(const AttributableNodeList& sources) {
            m_linkSources.reserve(m_linkSources.size() + sources.size());
            for (AttributableNode* linkSource : sources) {
                linkSource->addLinkTarget(this);
                m_linkSources.push_back(linkSource);
            }
            invalidateIssues();
        }
        
        void AttributableNode::addKillSources(const AttributableNodeList& sources) {
            m_killSources.reserve(m_killSources.size() + sources.size());
            for (AttributableNode* killSource : sources) {
                killSource->addKillTarget(this);
                m_killSources.push_back(killSource);
            }
            invalidateIssues();
        }

        void AttributableNode::removeAllLinkSources() {
            for (AttributableNode* linkSource : m_linkSources)
                linkSource->removeLinkTarget(this);
            m_linkSources.clear();
            invalidateIssues();
        }
        
        void AttributableNode::removeAllLinkTargets() {
            for (AttributableNode* linkTarget : m_linkTargets)
                linkTarget->removeLinkSource(this);
            m_linkTargets.clear();
            invalidateIssues();
        }
        
        void AttributableNode::removeAllKillSources() {
            for (AttributableNode* killSource : m_killSources)
                killSource->removeKillTarget(this);
            m_killSources.clear();
            invalidateIssues();
        }
        
        void AttributableNode::removeAllKillTargets() {
            for (AttributableNode* killTarget : m_killTargets)
                killTarget->removeKillSource(this);
            m_killTargets.clear();
            invalidateIssues();
        }

        void AttributableNode::removeAllLinks() {
            removeAllLinkSources();
            removeAllLinkTargets();
            removeAllKillSources();
            removeAllKillTargets();
        }
        
        void AttributableNode::addAllLinks() {
            addAllLinkTargets();
            addAllKillTargets();
            
            const AttributeValue* targetname = m_attributes.attribute(AttributeNames::Targetname);
            if (targetname != NULL && !targetname->empty()) {
                addAllLinkSources(*targetname);
                addAllKillSources(*targetname);
            }
        }
        
        void AttributableNode::doAncestorWillChange() {
            removeAllLinks();
            removeAttributesFromIndex();
        }

        void AttributableNode::doAncestorDidChange() {
            addAttributesToIndex();
            addAllLinks();
        }

        void AttributableNode::addLinkSource(AttributableNode* attributable) {
            ensure(attributable != NULL, "attributable is null");
            m_linkSources.push_back(attributable);
            invalidateIssues();
        }
        
        void AttributableNode::addLinkTarget(AttributableNode* attributable) {
            ensure(attributable != NULL, "attributable is null");
            m_linkTargets.push_back(attributable);
            invalidateIssues();
        }
        
        void AttributableNode::addKillSource(AttributableNode* attributable) {
            ensure(attributable != NULL, "attributable is null");
            m_killSources.push_back(attributable);
            invalidateIssues();
        }
        
        void AttributableNode::addKillTarget(AttributableNode* attributable) {
            ensure(attributable != NULL, "attributable is null");
            m_killTargets.push_back(attributable);
            invalidateIssues();
        }
        
        void AttributableNode::removeLinkSource(AttributableNode* attributable) {
            ensure(attributable != NULL, "attributable is null");
            VectorUtils::erase(m_linkSources, attributable);
            invalidateIssues();
        }
        
        void AttributableNode::removeLinkTarget(AttributableNode* attributable) {
            ensure(attributable != NULL, "attributable is null");
            VectorUtils::erase(m_linkTargets, attributable);
            invalidateIssues();
        }
        
        void AttributableNode::removeKillSource(AttributableNode* attributable) {
            ensure(attributable != NULL, "attributable is null");
            VectorUtils::erase(m_killSources, attributable);
            invalidateIssues();
        }
        
        AttributableNode::AttributableNode() :
        Node(),
        m_definition(NULL) {}

        const String& AttributableNode::doGetName() const {
            static const String defaultName("<missing classname>");
            return classname(defaultName);
        }

        void AttributableNode::removeKillTarget(AttributableNode* attributable) {
            ensure(attributable != NULL, "attributable is null");
            VectorUtils::erase(m_killTargets, attributable);
        }
    }
}
