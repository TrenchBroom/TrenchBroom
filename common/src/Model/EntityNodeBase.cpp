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

#include "EntityNodeBase.h"

#include "Assets/PropertyDefinition.h"
#include "Assets/EntityDefinition.h"

#include <kdl/collection_utils.h>
#include <kdl/vector_utils.h>

#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        const Assets::EntityDefinition* selectEntityDefinition(const std::vector<EntityNodeBase*>& attributables) {
            const Assets::EntityDefinition* definition = nullptr;

            for (EntityNodeBase* attributable : attributables) {
                if (definition == nullptr) {
                    definition = attributable->entity().definition();
                } else if (definition != attributable->entity().definition()) {
                    definition = nullptr;
                    break;
                }
            }

            return definition;
        }

        const Assets::PropertyDefinition* attributeDefinition(const EntityNodeBase* node, const std::string& name) {
            const auto* definition = node->entity().definition();
            return definition ? definition->propertyDefinition(name) : nullptr;
        }

        const Assets::PropertyDefinition* selectAttributeDefinition(const std::string& name, const std::vector<EntityNodeBase*>& attributables) {
            std::vector<EntityNodeBase*>::const_iterator it = std::begin(attributables);
            std::vector<EntityNodeBase*>::const_iterator end = std::end(attributables);
            if (it == end)
                return nullptr;

            const EntityNodeBase* attributable = *it;
            const Assets::PropertyDefinition* definition = attributeDefinition(attributable, name);
            if (definition == nullptr)
                return nullptr;

            while (++it != end) {
                attributable = *it;
                const Assets::PropertyDefinition* currentDefinition = attributeDefinition(attributable, name);
                if (currentDefinition == nullptr)
                    return nullptr;

                if (!definition->equals(currentDefinition))
                    return nullptr;
            }

            return definition;
        }

        std::string selectAttributeValue(const std::string& name, const std::vector<EntityNodeBase*>& attributables) {
            std::vector<EntityNodeBase*>::const_iterator it = std::begin(attributables);
            std::vector<EntityNodeBase*>::const_iterator end = std::end(attributables);
            if (it == end)
                return "";

            const EntityNodeBase* attributable = *it;
            const auto* value = attributable->entity().attribute(name);
            if (!value)
                return "";

            while (++it != end) {
                attributable = *it;
                const auto* itValue = attributable->entity().attribute(name);
                if (!itValue) {
                    return "";
                }
                if (*value != *itValue)
                    return "";
            }
            return *value;
        }

        EntityNodeBase::EntityNodeBase(Entity entity) :
        m_entity(std::move(entity)) {}

        EntityNodeBase::~EntityNodeBase() = default;

        const Entity& EntityNodeBase::entity() const {
            return m_entity;
        }

        Entity EntityNodeBase::setEntity(Entity entity) {
            const NotifyAttributeChange notifyChange(this);
            updateIndexAndLinks(entity.attributes());

            using std::swap;
            swap(m_entity, entity);
            return entity;
        }

        void EntityNodeBase::setDefinition(Assets::EntityDefinition* definition) {
            if (m_entity.definition() == definition) {
                return;
            }

            const NotifyAttributeChange notifyChange(this);
            m_entity.setDefinition(definition);
        }

        EntityNodeBase::NotifyAttributeChange::NotifyAttributeChange(EntityNodeBase* node) :
        m_nodeChange(node),
        m_node(node),
        m_oldPhysicalBounds(node->physicalBounds()) {
            ensure(m_node != nullptr, "node is null");
            m_node->attributesWillChange();
        }

        EntityNodeBase::NotifyAttributeChange::~NotifyAttributeChange() {
            m_node->attributesDidChange(m_oldPhysicalBounds);
        }

        void EntityNodeBase::attributesWillChange() {}

        void EntityNodeBase::attributesDidChange(const vm::bbox3& oldPhysicalBounds) {
            doAttributesDidChange(oldPhysicalBounds);
        }

        void EntityNodeBase::updateIndexAndLinks(const std::vector<EntityProperty>& newAttributes) {
            const auto oldSorted = kdl::vec_sort(m_entity.attributes());
            const auto newSorted = kdl::vec_sort(newAttributes);

            updateAttributeIndex(oldSorted, newSorted);
            updateLinks(oldSorted, newSorted);
        }

        void EntityNodeBase::updateAttributeIndex(const std::vector<EntityProperty>& oldAttributes, const std::vector<EntityProperty>& newAttributes) {
            auto oldIt = std::begin(oldAttributes);
            auto oldEnd = std::end(oldAttributes);
            auto newIt = std::begin(newAttributes);
            auto newEnd = std::end(newAttributes);

            while (oldIt != oldEnd && newIt != newEnd) {
                const EntityProperty& oldAttr = *oldIt;
                const EntityProperty& newAttr = *newIt;

                const int cmp = oldAttr.compare(newAttr);
                if (cmp < 0) {
                    removeAttributeFromIndex(oldAttr.key(), oldAttr.value());
                    ++oldIt;
                } else if (cmp > 0) {
                    addAttributeToIndex(newAttr.key(), newAttr.value());
                    ++newIt;
                } else {
                    updateAttributeIndex(oldAttr.key(), oldAttr.value(), newAttr.key(), newAttr.value());
                    ++oldIt; ++newIt;
                }
            }

            while (oldIt != oldEnd) {
                const EntityProperty& oldAttr = *oldIt;
                removeAttributeFromIndex(oldAttr.key(), oldAttr.value());
                ++oldIt;
            }

            while (newIt != newEnd) {
                const EntityProperty& newAttr = *newIt;
                addAttributeToIndex(newAttr.key(), newAttr.value());
                ++newIt;
            }
        }

        void EntityNodeBase::updateLinks(const std::vector<EntityProperty>& oldAttributes, const std::vector<EntityProperty>& newAttributes) {
            auto oldIt = std::begin(oldAttributes);
            auto oldEnd = std::end(oldAttributes);
            auto newIt = std::begin(newAttributes);
            auto newEnd = std::end(newAttributes);

            while (oldIt != oldEnd && newIt != newEnd) {
                const EntityProperty& oldAttr = *oldIt;
                const EntityProperty& newAttr = *newIt;

                const int cmp = oldAttr.compare(newAttr);
                if (cmp < 0) {
                    removeLinks(oldAttr.key(), oldAttr.value());
                    ++oldIt;
                } else if (cmp > 0) {
                    addLinks(newAttr.key(), newAttr.value());
                    ++newIt;
                } else {
                    updateLinks(oldAttr.key(), oldAttr.value(), newAttr.key(), newAttr.value());
                    ++oldIt; ++newIt;
                }
            }

            while (oldIt != oldEnd) {
                const EntityProperty& oldAttr = *oldIt;
                removeLinks(oldAttr.key(), oldAttr.value());
                ++oldIt;
            }

            while (newIt != newEnd) {
                const EntityProperty& newAttr = *newIt;
                addLinks(newAttr.key(), newAttr.value());
                ++newIt;
            }
        }
        
        void EntityNodeBase::addAttributesToIndex() {
            for (const EntityProperty& attribute : m_entity.attributes())
                addAttributeToIndex(attribute.key(), attribute.value());
        }

        void EntityNodeBase::removeAttributesFromIndex() {
            for (const EntityProperty& attribute : m_entity.attributes())
                removeAttributeFromIndex(attribute.key(), attribute.value());
        }

        void EntityNodeBase::addAttributeToIndex(const std::string& name, const std::string& value) {
            addToIndex(this, name, value);
        }

        void EntityNodeBase::removeAttributeFromIndex(const std::string& name, const std::string& value) {
            removeFromIndex(this, name, value);
        }

        void EntityNodeBase::updateAttributeIndex(const std::string& oldName, const std::string& oldValue, const std::string& newName, const std::string& newValue) {
            if (oldName == newName && oldValue == newValue) {
                return;
            }
            removeFromIndex(this, oldName, oldValue);
            addToIndex(this, newName, newValue);
        }

        const std::vector<EntityNodeBase*>& EntityNodeBase::linkSources() const {
            return m_linkSources;
        }

        const std::vector<EntityNodeBase*>& EntityNodeBase::linkTargets() const {
            return m_linkTargets;
        }

        const std::vector<EntityNodeBase*>& EntityNodeBase::killSources() const {
            return m_killSources;
        }

        const std::vector<EntityNodeBase*>& EntityNodeBase::killTargets() const {
            return m_killTargets;
        }

        vm::vec3 EntityNodeBase::linkSourceAnchor() const {
            return doGetLinkSourceAnchor();
        }

        vm::vec3 EntityNodeBase::linkTargetAnchor() const {
            return doGetLinkTargetAnchor();
        }

        bool EntityNodeBase::hasMissingSources() const {
            return (m_linkSources.empty() &&
                    m_killSources.empty() &&
                    m_entity.hasAttribute(PropertyKeys::Targetname));
        }

        std::vector<std::string> EntityNodeBase::findMissingLinkTargets() const {
            std::vector<std::string> result;
            findMissingTargets(PropertyKeys::Target, result);
            return result;
        }

        std::vector<std::string> EntityNodeBase::findMissingKillTargets() const {
            std::vector<std::string> result;
            findMissingTargets(PropertyKeys::Killtarget, result);
            return result;
        }

        void EntityNodeBase::findMissingTargets(const std::string& prefix, std::vector<std::string>& result) const {
            for (const EntityProperty& attribute : m_entity.numberedAttributes(prefix)) {
                const std::string& targetname = attribute.value();
                if (targetname.empty()) {
                    result.push_back(attribute.key());
                } else {
                    std::vector<EntityNodeBase*> linkTargets;
                    findAttributableNodesWithAttribute(PropertyKeys::Targetname, targetname, linkTargets);
                    if (linkTargets.empty())
                        result.push_back(attribute.key());
                }
            }
        }

        void EntityNodeBase::addLinks(const std::string& name, const std::string& value) {
            if (isNumberedProperty(PropertyKeys::Target, name)) {
                addLinkTargets(value);
            } else if (isNumberedProperty(PropertyKeys::Killtarget, name)) {
                addKillTargets(value);
            } else if (name == PropertyKeys::Targetname) {
                addAllLinkSources(value);
                addAllKillSources(value);
            }
        }

        void EntityNodeBase::removeLinks(const std::string& name, const std::string& value) {
            if (isNumberedProperty(PropertyKeys::Target, name)) {
                removeLinkTargets(value);
            } else if (isNumberedProperty(PropertyKeys::Killtarget, name)) {
                removeKillTargets(value);
            } else if (name == PropertyKeys::Targetname) {
                removeAllLinkSources();
                removeAllKillSources();
            }
        }

        void EntityNodeBase::updateLinks(const std::string& oldName, const std::string& oldValue, const std::string& newName, const std::string& newValue) {
            if (oldName == newName && oldValue == newValue) {
                return;
            }
            removeLinks(oldName, oldValue);
            addLinks(newName, newValue);
        }

        void EntityNodeBase::addLinkTargets(const std::string& targetname) {
            if (!targetname.empty()) {
                std::vector<EntityNodeBase*> targets;
                findAttributableNodesWithAttribute(PropertyKeys::Targetname, targetname, targets);
                addLinkTargets(targets);
            }
        }

        void EntityNodeBase::addKillTargets(const std::string& targetname) {
            if (!targetname.empty()) {
                std::vector<EntityNodeBase*> targets;
                findAttributableNodesWithAttribute(PropertyKeys::Targetname, targetname, targets);
                addKillTargets(targets);
            }
        }

        void EntityNodeBase::removeLinkTargets(const std::string& targetname) {
            if (!targetname.empty()) {
                std::vector<EntityNodeBase*>::iterator rem = std::end(m_linkTargets);
                std::vector<EntityNodeBase*>::iterator it = std::begin(m_linkTargets);
                while (it != rem) {
                    EntityNodeBase* target = *it;
                    const auto* targetTargetname = target->entity().attribute(PropertyKeys::Targetname);
                    if (targetTargetname && *targetTargetname == targetname) {
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

        void EntityNodeBase::removeKillTargets(const std::string& targetname) {
            if (!targetname.empty()) {
                std::vector<EntityNodeBase*>::iterator rem = std::end(m_killTargets);
                std::vector<EntityNodeBase*>::iterator it = std::begin(m_killTargets);
                while (it != rem) {
                    EntityNodeBase* target = *it;
                    const auto* targetTargetname = target->entity().attribute(PropertyKeys::Targetname);
                    if (targetTargetname && *targetTargetname == targetname) {
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

        void EntityNodeBase::addAllLinkSources(const std::string& targetname) {
            if (!targetname.empty()) {
                std::vector<EntityNodeBase*> linkSources;
                findAttributableNodesWithNumberedAttribute(PropertyKeys::Target, targetname, linkSources);
                addLinkSources(linkSources);
            }
        }

        void EntityNodeBase::addAllLinkTargets() {
            for (const EntityProperty& attribute : m_entity.numberedAttributes(PropertyKeys::Target)) {
                const std::string& targetname = attribute.value();
                if (!targetname.empty()) {
                    std::vector<EntityNodeBase*> linkTargets;
                    findAttributableNodesWithAttribute(PropertyKeys::Targetname, targetname, linkTargets);
                    addLinkTargets(linkTargets);
                }
            }
        }

        void EntityNodeBase::addAllKillSources(const std::string& targetname) {
            if (!targetname.empty()) {
                std::vector<EntityNodeBase*> killSources;
                findAttributableNodesWithNumberedAttribute(PropertyKeys::Killtarget, targetname, killSources);
                addKillSources(killSources);
            }
        }

        void EntityNodeBase::addAllKillTargets() {
            for (const EntityProperty& attribute : m_entity.numberedAttributes(PropertyKeys::Killtarget)) {
                const std::string& targetname = attribute.value();
                if (!targetname.empty()) {
                    std::vector<EntityNodeBase*> killTargets;
                    findAttributableNodesWithAttribute(PropertyKeys::Targetname, targetname, killTargets);
                    addKillTargets(killTargets);
                }
            }
        }

        void EntityNodeBase::addLinkTargets(const std::vector<EntityNodeBase*>& targets) {
            m_linkTargets.reserve(m_linkTargets.size() + targets.size());
            for (EntityNodeBase* target : targets) {
                target->addLinkSource(this);
                m_linkTargets.push_back(target);
            }
            invalidateIssues();
        }

        void EntityNodeBase::addKillTargets(const std::vector<EntityNodeBase*>& targets) {
            m_killTargets.reserve(m_killTargets.size() + targets.size());
            for (EntityNodeBase* target : targets) {
                target->addKillSource(this);
                m_killTargets.push_back(target);
            }
            invalidateIssues();
        }

        void EntityNodeBase::addLinkSources(const std::vector<EntityNodeBase*>& sources) {
            m_linkSources.reserve(m_linkSources.size() + sources.size());
            for (EntityNodeBase* linkSource : sources) {
                linkSource->addLinkTarget(this);
                m_linkSources.push_back(linkSource);
            }
            invalidateIssues();
        }

        void EntityNodeBase::addKillSources(const std::vector<EntityNodeBase*>& sources) {
            m_killSources.reserve(m_killSources.size() + sources.size());
            for (EntityNodeBase* killSource : sources) {
                killSource->addKillTarget(this);
                m_killSources.push_back(killSource);
            }
            invalidateIssues();
        }

        void EntityNodeBase::removeAllLinkSources() {
            for (EntityNodeBase* linkSource : m_linkSources)
                linkSource->removeLinkTarget(this);
            m_linkSources.clear();
            invalidateIssues();
        }

        void EntityNodeBase::removeAllLinkTargets() {
            for (EntityNodeBase* linkTarget : m_linkTargets)
                linkTarget->removeLinkSource(this);
            m_linkTargets.clear();
            invalidateIssues();
        }

        void EntityNodeBase::removeAllKillSources() {
            for (EntityNodeBase* killSource : m_killSources)
                killSource->removeKillTarget(this);
            m_killSources.clear();
            invalidateIssues();
        }

        void EntityNodeBase::removeAllKillTargets() {
            for (EntityNodeBase* killTarget : m_killTargets)
                killTarget->removeKillSource(this);
            m_killTargets.clear();
            invalidateIssues();
        }

        void EntityNodeBase::removeAllLinks() {
            removeAllLinkSources();
            removeAllLinkTargets();
            removeAllKillSources();
            removeAllKillTargets();
        }

        void EntityNodeBase::addAllLinks() {
            addAllLinkTargets();
            addAllKillTargets();

            const std::string* targetname = m_entity.attribute(PropertyKeys::Targetname);
            if (targetname != nullptr && !targetname->empty()) {
                addAllLinkSources(*targetname);
                addAllKillSources(*targetname);
            }
        }

        void EntityNodeBase::doAncestorWillChange() {
            removeAllLinks();
            removeAttributesFromIndex();
        }

        void EntityNodeBase::doAncestorDidChange() {
            addAttributesToIndex();
            addAllLinks();
        }

        void EntityNodeBase::addLinkSource(EntityNodeBase* attributable) {
            ensure(attributable != nullptr, "attributable is null");
            m_linkSources.push_back(attributable);
            invalidateIssues();
        }

        void EntityNodeBase::addLinkTarget(EntityNodeBase* attributable) {
            ensure(attributable != nullptr, "attributable is null");
            m_linkTargets.push_back(attributable);
            invalidateIssues();
        }

        void EntityNodeBase::addKillSource(EntityNodeBase* attributable) {
            ensure(attributable != nullptr, "attributable is null");
            m_killSources.push_back(attributable);
            invalidateIssues();
        }

        void EntityNodeBase::addKillTarget(EntityNodeBase* attributable) {
            ensure(attributable != nullptr, "attributable is null");
            m_killTargets.push_back(attributable);
            invalidateIssues();
        }

        void EntityNodeBase::removeLinkSource(EntityNodeBase* attributable) {
            ensure(attributable != nullptr, "attributable is null");
            m_linkSources = kdl::vec_erase(std::move(m_linkSources), attributable);
            invalidateIssues();
        }

        void EntityNodeBase::removeLinkTarget(EntityNodeBase* attributable) {
            ensure(attributable != nullptr, "attributable is null");
            m_linkTargets = kdl::vec_erase(std::move(m_linkTargets), attributable);
            invalidateIssues();
        }

        void EntityNodeBase::removeKillSource(EntityNodeBase* attributable) {
            ensure(attributable != nullptr, "attributable is null");
            m_killSources = kdl::vec_erase(std::move(m_killSources), attributable);
            invalidateIssues();
        }

        EntityNodeBase::EntityNodeBase() :
        Node() {}

        const std::string& EntityNodeBase::doGetName() const {
            return m_entity.classname();
        }

        void EntityNodeBase::removeKillTarget(EntityNodeBase* attributable) {
            ensure(attributable != nullptr, "attributable is null");
            m_killTargets = kdl::vec_erase(std::move(m_killTargets), attributable);
        }

        bool operator==(const EntityNodeBase& lhs, const EntityNodeBase& rhs) {
            return lhs.entity() == rhs.entity();
        }

        bool operator!=(const EntityNodeBase& lhs, const EntityNodeBase& rhs) {
            return !(lhs == rhs);
        }
    }
}
