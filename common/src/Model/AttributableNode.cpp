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

#include "AttributableNode.h"

#include "Assets/AttributeDefinition.h"

namespace TrenchBroom {
    namespace Model {
        Assets::EntityDefinition* AttributableNode::selectEntityDefinition(const AttributableNodeList& attributables) {
            Assets::EntityDefinition* definition = NULL;
            
            AttributableNodeList::const_iterator it, end;
            for (it = attributables.begin(), end = attributables.end(); it != end; ++it) {
                AttributableNode* attributable = *it;
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
            AttributableNodeList::const_iterator it = attributables.begin();
            AttributableNodeList::const_iterator end = attributables.end();
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
            AttributableNodeList::const_iterator it = attributables.begin();
            AttributableNodeList::const_iterator end = attributables.end();
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
            const NotifyAttributeChange notifyChange(this);
            updateAttributeIndex(attributes);
            m_attributes.setAttributes(attributes);
            m_attributes.updateDefinitions(m_definition);
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
        
        void AttributableNode::addOrUpdateAttribute(const AttributeName& name, const AttributeValue& value) {
            const NotifyAttributeChange notifyChange(this);

            const Assets::AttributeDefinition* definition = Assets::EntityDefinition::safeGetAttributeDefinition(m_definition, name);
            const AttributeValue* oldValue = m_attributes.attribute(name);
            if (oldValue != NULL) {
                removeAttributeFromIndex(name, *oldValue);
                removeLinks(name, *oldValue);
            }
            
            m_attributes.addOrUpdateAttribute(name, value, definition);
            addAttributeToIndex(name, value);
            addLinks(name, value);
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
            
            const NotifyAttributeChange notifyChange(this);

            const Assets::AttributeDefinition* newDefinition = Assets::EntityDefinition::safeGetAttributeDefinition(m_definition, newName);
            m_attributes.renameAttribute(name, newName, newDefinition);
            
            const AttributeValue value = *valuePtr;
            updateAttributeIndex(name, value, newName, value);
            updateLinks(name, value, newName, value);
        }
        
        bool AttributableNode::canRemoveAttribute(const AttributeName& name) const {
            return isAttributeNameMutable(name) && isAttributeValueMutable(name);
        }
        
        void AttributableNode::removeAttribute(const AttributeName& name) {
            const AttributeValue* valuePtr = m_attributes.attribute(name);
            if (valuePtr == NULL)
                return;

            const NotifyAttributeChange notifyChange(this);

            const AttributeValue value = *valuePtr;
            m_attributes.removeAttribute(name);
            
            removeAttributeFromIndex(name, value);
            removeLinks(name, value);
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
            assert(m_node != NULL);
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
            const EntityAttribute::List& attributes = m_attributes.attributes();
            EntityAttribute::List::const_iterator it, end;
            for (it = attributes.begin(), end = attributes.end(); it != end; ++it) {
                const EntityAttribute& attribute = *it;
                addAttributeToIndex(attribute.name(), attribute.value());
            }
        }
        
        void AttributableNode::removeAttributesFromIndex() {
            const EntityAttribute::List& attributes = m_attributes.attributes();
            EntityAttribute::List::const_iterator it, end;
            for (it = attributes.begin(), end = attributes.end(); it != end; ++it) {
                const EntityAttribute& attribute = *it;
                removeAttributeFromIndex(attribute.name(), attribute.value());
            }
        }

        void AttributableNode::updateAttributeIndex(const EntityAttribute::List& newAttributes) {
            EntityAttribute::List oldSorted = m_attributes.attributes();
            EntityAttribute::List newSorted = newAttributes;
            
            oldSorted.sort();
            newSorted.sort();
            
            EntityAttribute::List::const_iterator oldIt = oldSorted.begin();
            EntityAttribute::List::const_iterator oldEnd = oldSorted.end();
            EntityAttribute::List::const_iterator newIt = newSorted.begin();
            EntityAttribute::List::const_iterator newEnd = newSorted.end();
            
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
            const EntityAttribute::List attributes = m_attributes.numberedAttributes(prefix);
            EntityAttribute::List::const_iterator aIt, aEnd;
            for (aIt = attributes.begin(), aEnd = attributes.end(); aIt != aEnd; ++aIt) {
                const EntityAttribute& attribute = *aIt;
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
                AttributableNodeList::iterator rem = m_linkTargets.end();
                AttributableNodeList::iterator it = m_linkTargets.begin();
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
                m_linkTargets.erase(rem, m_linkTargets.end());
            }
        }
        
        void AttributableNode::removeKillTargets(const AttributeValue& targetname) {
            if (!targetname.empty()) {
                AttributableNodeList::iterator rem = m_killTargets.end();
                AttributableNodeList::iterator it = m_killTargets.begin();
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
                m_killTargets.erase(rem, m_killTargets.end());
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
            const EntityAttribute::List attributes = m_attributes.numberedAttributes(AttributeNames::Target);
            EntityAttribute::List::const_iterator aIt, aEnd;
            for (aIt = attributes.begin(), aEnd = attributes.end(); aIt != aEnd; ++aIt) {
                const EntityAttribute& attribute = *aIt;
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
            const EntityAttribute::List attributes = m_attributes.numberedAttributes(AttributeNames::Killtarget);
            EntityAttribute::List::const_iterator aIt, aEnd;
            for (aIt = attributes.begin(), aEnd = attributes.end(); aIt != aEnd; ++aIt) {
                const EntityAttribute& attribute = *aIt;
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
            
            AttributableNodeList::const_iterator it, end;
            for (it = targets.begin(), end = targets.end(); it != end; ++it) {
                AttributableNode* target = *it;
                target->addLinkSource(this);
                m_linkTargets.push_back(target);
            }
            invalidateIssues();
        }
        
        void AttributableNode::addKillTargets(const AttributableNodeList& targets) {
            m_killTargets.reserve(m_killTargets.size() + targets.size());
            
            AttributableNodeList::const_iterator it, end;
            for (it = targets.begin(), end = targets.end(); it != end; ++it) {
                AttributableNode* target = *it;
                target->addKillSource(this);
                m_killTargets.push_back(target);
            }
            invalidateIssues();
        }

        void AttributableNode::addLinkSources(const AttributableNodeList& sources) {
            m_linkSources.reserve(m_linkSources.size() + sources.size());
            
            AttributableNodeList::const_iterator it, end;
            for (it = sources.begin(), end = sources.end(); it != end; ++it) {
                AttributableNode* linkSource = *it;
                linkSource->addLinkTarget(this);
                m_linkSources.push_back(linkSource);
            }
            invalidateIssues();
        }
        
        void AttributableNode::addKillSources(const AttributableNodeList& sources) {
            m_killSources.reserve(m_killSources.size() + sources.size());
            
            AttributableNodeList::const_iterator it, end;
            for (it = sources.begin(), end = sources.end(); it != end; ++it) {
                AttributableNode* killSource = *it;
                killSource->addKillTarget(this);
                m_killSources.push_back(killSource);
            }
            invalidateIssues();
        }

        void AttributableNode::removeAllLinkSources() {
            AttributableNodeList::const_iterator it, end;
            for (it = m_linkSources.begin(), end = m_linkSources.end(); it != end; ++it) {
                AttributableNode* linkSource = *it;
                linkSource->removeLinkTarget(this);
            }
            m_linkSources.clear();
            invalidateIssues();
        }
        
        void AttributableNode::removeAllLinkTargets() {
            AttributableNodeList::const_iterator it, end;
            for (it = m_linkTargets.begin(), end = m_linkTargets.end(); it != end; ++it) {
                AttributableNode* linkTarget = *it;
                linkTarget->removeLinkSource(this);
            }
            m_linkTargets.clear();
            invalidateIssues();
        }
        
        void AttributableNode::removeAllKillSources() {
            AttributableNodeList::const_iterator it, end;
            for (it = m_killSources.begin(), end = m_killSources.end(); it != end; ++it) {
                AttributableNode* killSource = *it;
                killSource->removeKillTarget(this);
            }
            m_killSources.clear();
            invalidateIssues();
        }
        
        void AttributableNode::removeAllKillTargets() {
            AttributableNodeList::const_iterator it, end;
            for (it = m_killTargets.begin(), end = m_killTargets.end(); it != end; ++it) {
                AttributableNode* killTarget = *it;
                killTarget->removeKillSource(this);
            }
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
            assert(attributable != NULL);
            m_linkSources.push_back(attributable);
            invalidateIssues();
        }
        
        void AttributableNode::addLinkTarget(AttributableNode* attributable) {
            assert(attributable != NULL);
            m_linkTargets.push_back(attributable);
            invalidateIssues();
        }
        
        void AttributableNode::addKillSource(AttributableNode* attributable) {
            assert(attributable != NULL);
            m_killSources.push_back(attributable);
            invalidateIssues();
        }
        
        void AttributableNode::addKillTarget(AttributableNode* attributable) {
            assert(attributable != NULL);
            m_killTargets.push_back(attributable);
            invalidateIssues();
        }
        
        void AttributableNode::removeLinkSource(AttributableNode* attributable) {
            assert(attributable != NULL);
            VectorUtils::erase(m_linkSources, attributable);
            invalidateIssues();
        }
        
        void AttributableNode::removeLinkTarget(AttributableNode* attributable) {
            assert(attributable != NULL);
            VectorUtils::erase(m_linkTargets, attributable);
            invalidateIssues();
        }
        
        void AttributableNode::removeKillSource(AttributableNode* attributable) {
            assert(attributable != NULL);
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
            assert(attributable != NULL);
            VectorUtils::erase(m_killTargets, attributable);
        }
    }
}
