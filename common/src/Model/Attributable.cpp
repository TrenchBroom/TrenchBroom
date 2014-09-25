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

#include "Attributable.h"

namespace TrenchBroom {
    namespace Model {
        const String Attributable::DefaultAttributeValue("");

        Attributable::~Attributable() {
            m_definition = NULL;
        }
        
        Assets::EntityDefinition* Attributable::definition() const {
            return m_definition;
        }
        
        void Attributable::setDefinition(Assets::EntityDefinition* definition) {
            if (m_definition == definition)
                return;
            if (m_definition != NULL)
                m_definition->decUsageCount();
            m_definition = definition;
            m_attributes.updateDefinitions(m_definition);
            if (m_definition != NULL)
                m_definition->incUsageCount();
            attributesDidChange();
        }

        const EntityAttribute::List& Attributable::attributes() const {
            return m_attributes.attributes();
        }
        
        void Attributable::setAttributes(const EntityAttribute::List& attributes) {
            updateAttributeIndex(attributes);
            m_attributes.setAttributes(attributes);
            m_attributes.updateDefinitions(m_definition);
            attributesDidChange();
        }
        
        bool Attributable::hasAttribute(const AttributeName& name) const {
            return m_attributes.hasAttribute(name);
        }
        
        const AttributeValue& Attributable::attribute(const AttributeName& name, const AttributeValue& defaultValue) const {
            const AttributeValue* value = m_attributes.attribute(name);
            if (value == NULL)
                return defaultValue;
            return *value;
        }

        const AttributeValue& Attributable::classname(const AttributeValue& defaultClassname) const {
            return attribute(AttributeNames::Classname, defaultClassname);
        }

        bool Attributable::canAddOrUpdateAttribute(const AttributeName& name, const AttributeValue& value) const {
            return doCanAddOrUpdateAttribute(name, value);
        }
        
        void Attributable::addOrUpdateAttribute(const AttributeName& name, const AttributeValue& value) {
            const Assets::AttributeDefinition* definition = Assets::EntityDefinition::safeGetAttributeDefinition(m_definition, name);
            const AttributeValue* oldValue = m_attributes.attribute(name);
            if (oldValue != NULL) {
                removeAttributeFromIndex(name, *oldValue);
                removeLinks(name, *oldValue);
            }
            
            m_attributes.addOrUpdateAttribute(name, value, definition);
            addAttributeToIndex(name, value);
            addLinks(name, value);
            attributesDidChange();
        }
        
        bool Attributable::canRenameAttribute(const AttributeName& name, const AttributeName& newName) const {
            return doCanRenameAttribute(name, newName);
        }
        
        void Attributable::renameAttribute(const AttributeName& name, const AttributeName& newName) {
            if (name == newName)
                return;
            
            const AttributeValue* valuePtr = m_attributes.attribute(name);
            if (valuePtr == NULL)
                return;
            
            const Assets::AttributeDefinition* newDefinition = Assets::EntityDefinition::safeGetAttributeDefinition(m_definition, newName);
            m_attributes.renameAttribute(name, newName, newDefinition);
            
            const AttributeValue value = *valuePtr;
            updateAttributeIndex(name, value, newName, value);
            updateLinks(name, value, newName, value);
            attributesDidChange();
        }
        
        bool Attributable::canRemoveAttribute(const AttributeName& name) const {
            return doCanRemoveAttribute(name);
        }
        
        void Attributable::removeAttribute(const AttributeName& name) {
            const AttributeValue* valuePtr = m_attributes.attribute(name);
            if (valuePtr == NULL)
                return;
            const AttributeValue value = *valuePtr;
            m_attributes.removeAttribute(name);
            
            removeAttributeFromIndex(name, value);
            removeLinks(name, value);
            attributesDidChange();
        }

        void Attributable::attributesDidChange() {
            nodeDidChange();
            doAttributesDidChange();
        }

        void Attributable::addAttributesToIndex() {
            const EntityAttribute::List& attributes = m_attributes.attributes();
            EntityAttribute::List::const_iterator it, end;
            for (it = attributes.begin(), end = attributes.end(); it != end; ++it) {
                const EntityAttribute& attribute = *it;
                addAttributeToIndex(attribute.name(), attribute.value());
            }
        }
        
        void Attributable::removeAttributesFromIndex() {
            const EntityAttribute::List& attributes = m_attributes.attributes();
            EntityAttribute::List::const_iterator it, end;
            for (it = attributes.begin(), end = attributes.end(); it != end; ++it) {
                const EntityAttribute& attribute = *it;
                removeAttributeFromIndex(attribute.name(), attribute.value());
            }
        }

        void Attributable::updateAttributeIndex(const EntityAttribute::List& newAttributes) {
            EntityAttribute::List oldSorted = m_attributes.attributes();
            EntityAttribute::List newSorted = newAttributes;
            
            VectorUtils::sort(oldSorted);
            VectorUtils::sort(newSorted);
            
            size_t i = 0, j = 0;
            while (i < oldSorted.size() && j < newSorted.size()) {
                const EntityAttribute& oldAttr = oldSorted[i];
                const EntityAttribute& newAttr = newSorted[j];
                
                const int cmp = oldAttr.compare(newAttr);
                if (cmp < 0) {
                    removeAttributeFromIndex(oldAttr.name(), oldAttr.value());
                    ++i;
                } else if (cmp > 0) {
                    addAttributeToIndex(newAttr.name(), newAttr.value());
                    ++j;
                } else {
                    updateAttributeIndex(oldAttr.name(), oldAttr.value(), newAttr.name(), newAttr.value());
                    ++i; ++j;
                }
            }
            
            while (i < oldSorted.size()) {
                const EntityAttribute& oldAttr = oldSorted[i];
                removeAttributeFromIndex(oldAttr.name(), oldAttr.value());
                ++i;
            }
            
            while (j < newSorted.size()) {
                const EntityAttribute& newAttr = newSorted[j];
                addAttributeToIndex(newAttr.name(), newAttr.value());
                ++j;
            }
        }
        
        void Attributable::addAttributeToIndex(const AttributeName& name, const AttributeValue& value) {
            addToIndex(this, name, value);
        }
        
        void Attributable::removeAttributeFromIndex(const AttributeName& name, const AttributeValue& value) {
            removeFromIndex(this, name, value);
        }
        
        void Attributable::updateAttributeIndex(const AttributeName& oldName, const AttributeValue& oldValue, const AttributeName& newName, const AttributeValue& newValue) {
            removeFromIndex(this, oldName, oldValue);
            addToIndex(this, newName, newValue);
        }
        
        const AttributableList& Attributable::linkSources() const {
            return m_linkSources;
        }
        
        const AttributableList& Attributable::linkTargets() const {
            return m_linkTargets;
        }
        
        const AttributableList& Attributable::killSources() const {
            return m_killSources;
        }
        
        const AttributableList& Attributable::killTargets() const {
            return m_killTargets;
        }
        
        bool Attributable::hasMissingSources() const {
            return (m_linkSources.empty() &&
                    m_killSources.empty() &&
                    hasAttribute(AttributeNames::Targetname));
        }
        
        AttributeNameList Attributable::findMissingLinkTargets() const {
            AttributeNameList result;
            findMissingTargets(AttributeNames::Target, result);
            return result;
        }
        
        AttributeNameList Attributable::findMissingKillTargets() const {
            AttributeNameList result;
            findMissingTargets(AttributeNames::Killtarget, result);
            return result;
        }

        void Attributable::findMissingTargets(const AttributeName& prefix, AttributeNameList& result) const {
            const EntityAttribute::List attributes = m_attributes.numberedAttributes(prefix);
            EntityAttribute::List::const_iterator aIt, aEnd;
            for (aIt = attributes.begin(), aEnd = attributes.end(); aIt != aEnd; ++aIt) {
                const EntityAttribute& attribute = *aIt;
                const AttributeValue& targetname = attribute.value();
                if (targetname.empty()) {
                    result.push_back(attribute.name());
                } else {
                    AttributableList linkTargets;
                    findAttributablesWithAttribute(AttributeNames::Targetname, targetname, linkTargets);
                    if (linkTargets.empty())
                        result.push_back(attribute.name());
                }
            }
        }

        void Attributable::addLinks(const AttributeName& name, const AttributeValue& value) {
            if (isNumberedAttribute(AttributeNames::Target, name)) {
                addLinkTargets(value);
            } else if (isNumberedAttribute(AttributeNames::Killtarget, name)) {
                addKillTargets(value);
            } else if (name == AttributeNames::Targetname) {
                addAllLinkSources(value);
                addAllKillSources(value);
            }
        }
        
        void Attributable::removeLinks(const AttributeName& name, const AttributeValue& value) {
            if (isNumberedAttribute(AttributeNames::Target, name)) {
                removeLinkTargets(value);
            } else if (isNumberedAttribute(AttributeNames::Killtarget, name)) {
                removeKillTargets(value);
            } else if (name == AttributeNames::Targetname) {
                removeAllLinkSources();
                removeAllKillSources();
            }
        }
        
        void Attributable::updateLinks(const AttributeName& oldName, const AttributeName& oldValue, const AttributeName& newName, const AttributeValue& newValue) {
            removeLinks(oldName, oldValue);
            addLinks(newName, newValue);
        }

        void Attributable::addLinkTargets(const AttributeValue& targetname) {
            if (!targetname.empty()) {
                AttributableList targets;
                findAttributablesWithAttribute(AttributeNames::Targetname, targetname, targets);
                addLinkTargets(targets);
            }
        }
        
        void Attributable::addKillTargets(const AttributeValue& targetname) {
            if (!targetname.empty()) {
                AttributableList targets;
                findAttributablesWithAttribute(AttributeNames::Targetname, targetname, targets);
                addKillTargets(targets);
            }
        }

        void Attributable::removeLinkTargets(const AttributeValue& targetname) {
            if (!targetname.empty()) {
                AttributableList::iterator rem = m_linkTargets.end();
                AttributableList::iterator it = m_linkTargets.begin();
                while (it != rem) {
                    Attributable* target = *it;
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
        
        void Attributable::removeKillTargets(const AttributeValue& targetname) {
            if (!targetname.empty()) {
                AttributableList::iterator rem = m_killTargets.end();
                AttributableList::iterator it = m_killTargets.begin();
                while (it != rem) {
                    Attributable* target = *it;
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

        void Attributable::addAllLinkSources(const AttributeValue& targetname) {
            if (!targetname.empty()) {
                AttributableList linkSources;
                findAttributablesWithNumberedAttribute(AttributeNames::Target, targetname, linkSources);
                addLinkSources(linkSources);
            }
        }
        
        void Attributable::addAllLinkTargets() {
            const EntityAttribute::List attributes = m_attributes.numberedAttributes(AttributeNames::Target);
            EntityAttribute::List::const_iterator aIt, aEnd;
            for (aIt = attributes.begin(), aEnd = attributes.end(); aIt != aEnd; ++aIt) {
                const EntityAttribute& attribute = *aIt;
                const String& targetname = attribute.value();
                if (!targetname.empty()) {
                    AttributableList linkTargets;
                    findAttributablesWithAttribute(AttributeNames::Targetname, targetname, linkTargets);
                    addLinkTargets(linkTargets);
                }
            }
        }
        
        void Attributable::addAllKillSources(const AttributeValue& targetname) {
            if (!targetname.empty()) {
                AttributableList killSources;
                findAttributablesWithNumberedAttribute(AttributeNames::Killtarget, targetname, killSources);
                addKillSources(killSources);
            }
        }
        
        void Attributable::addAllKillTargets() {
            const EntityAttribute::List attributes = m_attributes.numberedAttributes(AttributeNames::Killtarget);
            EntityAttribute::List::const_iterator aIt, aEnd;
            for (aIt = attributes.begin(), aEnd = attributes.end(); aIt != aEnd; ++aIt) {
                const EntityAttribute& attribute = *aIt;
                const String& targetname = attribute.value();
                if (!targetname.empty()) {
                    AttributableList killTargets;
                    findAttributablesWithAttribute(AttributeNames::Targetname, targetname, killTargets);
                    addKillTargets(killTargets);
                }
            }
        }

        void Attributable::addLinkTargets(const AttributableList& targets) {
            m_linkTargets.reserve(m_linkTargets.size() + targets.size());
            
            AttributableList::const_iterator it, end;
            for (it = targets.begin(), end = targets.end(); it != end; ++it) {
                Attributable* target = *it;
                target->addLinkSource(this);
                m_linkTargets.push_back(target);
            }
        }
        
        void Attributable::addKillTargets(const AttributableList& targets) {
            m_killTargets.reserve(m_killTargets.size() + targets.size());
            
            AttributableList::const_iterator it, end;
            for (it = targets.begin(), end = targets.end(); it != end; ++it) {
                Attributable* target = *it;
                target->addKillSource(this);
                m_killTargets.push_back(target);
            }
        }

        void Attributable::addLinkSources(const AttributableList& sources) {
            m_linkSources.reserve(m_linkSources.size() + sources.size());
            
            AttributableList::const_iterator it, end;
            for (it = sources.begin(), end = sources.end(); it != end; ++it) {
                Attributable* linkSource = *it;
                linkSource->addLinkTarget(this);
                m_linkSources.push_back(linkSource);
            }
        }
        
        void Attributable::addKillSources(const AttributableList& sources) {
            m_killSources.reserve(m_killSources.size() + sources.size());
            
            AttributableList::const_iterator it, end;
            for (it = sources.begin(), end = sources.end(); it != end; ++it) {
                Attributable* killSource = *it;
                killSource->addKillTarget(this);
                m_killSources.push_back(killSource);
            }
        }

        void Attributable::removeAllLinkSources() {
            AttributableList::const_iterator it, end;
            for (it = m_linkSources.begin(), end = m_linkSources.end(); it != end; ++it) {
                Attributable* linkSource = *it;
                linkSource->removeLinkTarget(this);
            }
            m_linkSources.clear();
        }
        
        void Attributable::removeAllLinkTargets() {
            AttributableList::const_iterator it, end;
            for (it = m_linkTargets.begin(), end = m_linkTargets.end(); it != end; ++it) {
                Attributable* linkTarget = *it;
                linkTarget->removeLinkSource(this);
            }
            m_linkTargets.clear();
        }
        
        void Attributable::removeAllKillSources() {
            AttributableList::const_iterator it, end;
            for (it = m_killSources.begin(), end = m_killSources.end(); it != end; ++it) {
                Attributable* killSource = *it;
                killSource->removeKillTarget(this);
            }
            m_killSources.clear();
        }
        
        void Attributable::removeAllKillTargets() {
            AttributableList::const_iterator it, end;
            for (it = m_killTargets.begin(), end = m_killTargets.end(); it != end; ++it) {
                Attributable* killTarget = *it;
                killTarget->removeKillSource(this);
            }
            m_killTargets.clear();
        }

        void Attributable::refreshAllLinks() {
            removeAllLinkSources();
            removeAllLinkTargets();
            removeAllKillSources();
            removeAllKillTargets();

            addAllLinkTargets();
            addAllKillTargets();
            
            const AttributeValue* targetname = m_attributes.attribute(AttributeNames::Targetname);
            if (targetname != NULL && !targetname->empty()) {
                addAllLinkSources(*targetname);
                addAllKillSources(*targetname);
            }
        }
        
        void Attributable::doAncestorWillChange() {
            removeAttributesFromIndex();
        }

        void Attributable::doAncestorDidChange() {
            addAttributesToIndex();
            refreshAllLinks();
        }

        void Attributable::addLinkSource(Attributable* attributable) {
            assert(attributable != NULL);
            m_linkSources.push_back(attributable);
        }
        
        void Attributable::addLinkTarget(Attributable* attributable) {
            assert(attributable != NULL);
            m_linkTargets.push_back(attributable);
        }
        
        void Attributable::addKillSource(Attributable* attributable) {
            assert(attributable != NULL);
            m_killSources.push_back(attributable);
        }
        
        void Attributable::addKillTarget(Attributable* attributable) {
            assert(attributable != NULL);
            m_killTargets.push_back(attributable);
        }
        
        void Attributable::removeLinkSource(Attributable* attributable) {
            assert(attributable != NULL);
            VectorUtils::erase(m_linkSources, attributable);
        }
        
        void Attributable::removeLinkTarget(Attributable* attributable) {
            assert(attributable != NULL);
            VectorUtils::erase(m_linkTargets, attributable);
        }
        
        void Attributable::removeKillSource(Attributable* attributable) {
            assert(attributable != NULL);
            VectorUtils::erase(m_killSources, attributable);
        }
        
        Attributable::Attributable() :
        m_definition(NULL) {}

        void Attributable::removeKillTarget(Attributable* attributable) {
            assert(attributable != NULL);
            VectorUtils::erase(m_killTargets, attributable);
        }
    }
}
