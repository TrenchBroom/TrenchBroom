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
#include "Assets/EntityDefinition.h"

#include <kdl/collection_utils.h>
#include <kdl/vector_utils.h>

#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        const Assets::EntityDefinition* selectEntityDefinition(const std::vector<AttributableNode*>& attributables) {
            const Assets::EntityDefinition* definition = nullptr;

            for (AttributableNode* attributable : attributables) {
                if (definition == nullptr) {
                    definition = attributable->entity().definition();
                } else if (definition != attributable->entity().definition()) {
                    definition = nullptr;
                    break;
                }
            }

            return definition;
        }

        const Assets::AttributeDefinition* attributeDefinition(const AttributableNode* node, const std::string& name) {
            const auto* definition = node->entity().definition();
            return definition ? definition->attributeDefinition(name) : nullptr;
        }

        const Assets::AttributeDefinition* selectAttributeDefinition(const std::string& name, const std::vector<AttributableNode*>& attributables) {
            std::vector<AttributableNode*>::const_iterator it = std::begin(attributables);
            std::vector<AttributableNode*>::const_iterator end = std::end(attributables);
            if (it == end)
                return nullptr;

            const AttributableNode* attributable = *it;
            const Assets::AttributeDefinition* definition = attributeDefinition(attributable, name);
            if (definition == nullptr)
                return nullptr;

            while (++it != end) {
                attributable = *it;
                const Assets::AttributeDefinition* currentDefinition = attributeDefinition(attributable, name);
                if (currentDefinition == nullptr)
                    return nullptr;

                if (!definition->equals(currentDefinition))
                    return nullptr;
            }

            return definition;
        }

        std::string selectAttributeValue(const std::string& name, const std::vector<AttributableNode*>& attributables) {
            std::vector<AttributableNode*>::const_iterator it = std::begin(attributables);
            std::vector<AttributableNode*>::const_iterator end = std::end(attributables);
            if (it == end)
                return "";

            const AttributableNode* attributable = *it;
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

        AttributableNode::AttributableNode(Entity entity) :
        m_entity(std::move(entity)) {}

        AttributableNode::~AttributableNode() = default;

        const Entity& AttributableNode::entity() const {
            return m_entity;
        }

        Entity AttributableNode::setEntity(Entity entity) {
            const NotifyAttributeChange notifyChange(this);
            updateIndexAndLinks(entity.attributes());

            using std::swap;
            swap(m_entity, entity);
            return entity;
        }

        void AttributableNode::setDefinition(Assets::EntityDefinition* definition) {
            if (m_entity.definition() == definition) {
                return;
            }

            const NotifyAttributeChange notifyChange(this);
            m_entity.setDefinition(definition);
        }

        AttributableNode::NotifyAttributeChange::NotifyAttributeChange(AttributableNode* node) :
        m_nodeChange(node),
        m_node(node),
        m_oldPhysicalBounds(node->physicalBounds()) {
            ensure(m_node != nullptr, "node is null");
            m_node->attributesWillChange();
        }

        AttributableNode::NotifyAttributeChange::~NotifyAttributeChange() {
            m_node->attributesDidChange(m_oldPhysicalBounds);
        }

        void AttributableNode::attributesWillChange() {}

        void AttributableNode::attributesDidChange(const vm::bbox3& oldPhysicalBounds) {
            doAttributesDidChange(oldPhysicalBounds);
        }

        void AttributableNode::updateIndexAndLinks(const std::vector<EntityAttribute>& newAttributes) {
            const auto oldSorted = kdl::vec_sort(m_entity.attributes());
            const auto newSorted = kdl::vec_sort(newAttributes);

            updateAttributeIndex(oldSorted, newSorted);
            updateLinks(oldSorted, newSorted);
        }

        void AttributableNode::updateAttributeIndex(const std::vector<EntityAttribute>& oldAttributes, const std::vector<EntityAttribute>& newAttributes) {
            auto oldIt = std::begin(oldAttributes);
            auto oldEnd = std::end(oldAttributes);
            auto newIt = std::begin(newAttributes);
            auto newEnd = std::end(newAttributes);

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

        void AttributableNode::updateLinks(const std::vector<EntityAttribute>& oldAttributes, const std::vector<EntityAttribute>& newAttributes) {
            auto oldIt = std::begin(oldAttributes);
            auto oldEnd = std::end(oldAttributes);
            auto newIt = std::begin(newAttributes);
            auto newEnd = std::end(newAttributes);

            while (oldIt != oldEnd && newIt != newEnd) {
                const EntityAttribute& oldAttr = *oldIt;
                const EntityAttribute& newAttr = *newIt;

                const int cmp = oldAttr.compare(newAttr);
                if (cmp < 0) {
                    removeLinks(oldAttr.name(), oldAttr.value());
                    ++oldIt;
                } else if (cmp > 0) {
                    addLinks(newAttr.name(), newAttr.value());
                    ++newIt;
                } else {
                    updateLinks(oldAttr.name(), oldAttr.value(), newAttr.name(), newAttr.value());
                    ++oldIt; ++newIt;
                }
            }

            while (oldIt != oldEnd) {
                const EntityAttribute& oldAttr = *oldIt;
                removeLinks(oldAttr.name(), oldAttr.value());
                ++oldIt;
            }

            while (newIt != newEnd) {
                const EntityAttribute& newAttr = *newIt;
                addLinks(newAttr.name(), newAttr.value());
                ++newIt;
            }
        }
        
        void AttributableNode::addAttributesToIndex() {
            for (const EntityAttribute& attribute : m_entity.attributes())
                addAttributeToIndex(attribute.name(), attribute.value());
        }

        void AttributableNode::removeAttributesFromIndex() {
            for (const EntityAttribute& attribute : m_entity.attributes())
                removeAttributeFromIndex(attribute.name(), attribute.value());
        }

        void AttributableNode::addAttributeToIndex(const std::string& name, const std::string& value) {
            addToIndex(this, name, value);
        }

        void AttributableNode::removeAttributeFromIndex(const std::string& name, const std::string& value) {
            removeFromIndex(this, name, value);
        }

        void AttributableNode::updateAttributeIndex(const std::string& oldName, const std::string& oldValue, const std::string& newName, const std::string& newValue) {
            if (oldName == newName && oldValue == newValue) {
                return;
            }
            removeFromIndex(this, oldName, oldValue);
            addToIndex(this, newName, newValue);
        }

        const std::vector<AttributableNode*>& AttributableNode::linkSources() const {
            return m_linkSources;
        }

        const std::vector<AttributableNode*>& AttributableNode::linkTargets() const {
            return m_linkTargets;
        }

        const std::vector<AttributableNode*>& AttributableNode::killSources() const {
            return m_killSources;
        }

        const std::vector<AttributableNode*>& AttributableNode::killTargets() const {
            return m_killTargets;
        }

        vm::vec3 AttributableNode::linkSourceAnchor() const {
            return doGetLinkSourceAnchor();
        }

        vm::vec3 AttributableNode::linkTargetAnchor() const {
            return doGetLinkTargetAnchor();
        }

        bool AttributableNode::hasMissingSources() const {
            return (m_linkSources.empty() &&
                    m_killSources.empty() &&
                    m_entity.hasAttribute(AttributeNames::Targetname));
        }

        std::vector<std::string> AttributableNode::findMissingLinkTargets() const {
            std::vector<std::string> result;
            findMissingTargets(AttributeNames::Target, result);
            return result;
        }

        std::vector<std::string> AttributableNode::findMissingKillTargets() const {
            std::vector<std::string> result;
            findMissingTargets(AttributeNames::Killtarget, result);
            return result;
        }

        void AttributableNode::findMissingTargets(const std::string& prefix, std::vector<std::string>& result) const {
            for (const EntityAttribute& attribute : m_entity.numberedAttributes(prefix)) {
                const std::string& targetname = attribute.value();
                if (targetname.empty()) {
                    result.push_back(attribute.name());
                } else {
                    std::vector<AttributableNode*> linkTargets;
                    findAttributableNodesWithAttribute(AttributeNames::Targetname, targetname, linkTargets);
                    if (linkTargets.empty())
                        result.push_back(attribute.name());
                }
            }
        }

        void AttributableNode::addLinks(const std::string& name, const std::string& value) {
            if (isNumberedAttribute(AttributeNames::Target, name)) {
                addLinkTargets(value);
            } else if (isNumberedAttribute(AttributeNames::Killtarget, name)) {
                addKillTargets(value);
            } else if (name == AttributeNames::Targetname) {
                addAllLinkSources(value);
                addAllKillSources(value);
            }
        }

        void AttributableNode::removeLinks(const std::string& name, const std::string& value) {
            if (isNumberedAttribute(AttributeNames::Target, name)) {
                removeLinkTargets(value);
            } else if (isNumberedAttribute(AttributeNames::Killtarget, name)) {
                removeKillTargets(value);
            } else if (name == AttributeNames::Targetname) {
                removeAllLinkSources();
                removeAllKillSources();
            }
        }

        void AttributableNode::updateLinks(const std::string& oldName, const std::string& oldValue, const std::string& newName, const std::string& newValue) {
            if (oldName == newName && oldValue == newValue) {
                return;
            }
            removeLinks(oldName, oldValue);
            addLinks(newName, newValue);
        }

        void AttributableNode::addLinkTargets(const std::string& targetname) {
            if (!targetname.empty()) {
                std::vector<AttributableNode*> targets;
                findAttributableNodesWithAttribute(AttributeNames::Targetname, targetname, targets);
                addLinkTargets(targets);
            }
        }

        void AttributableNode::addKillTargets(const std::string& targetname) {
            if (!targetname.empty()) {
                std::vector<AttributableNode*> targets;
                findAttributableNodesWithAttribute(AttributeNames::Targetname, targetname, targets);
                addKillTargets(targets);
            }
        }

        void AttributableNode::removeLinkTargets(const std::string& targetname) {
            if (!targetname.empty()) {
                std::vector<AttributableNode*>::iterator rem = std::end(m_linkTargets);
                std::vector<AttributableNode*>::iterator it = std::begin(m_linkTargets);
                while (it != rem) {
                    AttributableNode* target = *it;
                    const auto* targetTargetname = target->entity().attribute(AttributeNames::Targetname);
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

        void AttributableNode::removeKillTargets(const std::string& targetname) {
            if (!targetname.empty()) {
                std::vector<AttributableNode*>::iterator rem = std::end(m_killTargets);
                std::vector<AttributableNode*>::iterator it = std::begin(m_killTargets);
                while (it != rem) {
                    AttributableNode* target = *it;
                    const auto* targetTargetname = target->entity().attribute(AttributeNames::Targetname);
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

        void AttributableNode::addAllLinkSources(const std::string& targetname) {
            if (!targetname.empty()) {
                std::vector<AttributableNode*> linkSources;
                findAttributableNodesWithNumberedAttribute(AttributeNames::Target, targetname, linkSources);
                addLinkSources(linkSources);
            }
        }

        void AttributableNode::addAllLinkTargets() {
            for (const EntityAttribute& attribute : m_entity.numberedAttributes(AttributeNames::Target)) {
                const std::string& targetname = attribute.value();
                if (!targetname.empty()) {
                    std::vector<AttributableNode*> linkTargets;
                    findAttributableNodesWithAttribute(AttributeNames::Targetname, targetname, linkTargets);
                    addLinkTargets(linkTargets);
                }
            }
        }

        void AttributableNode::addAllKillSources(const std::string& targetname) {
            if (!targetname.empty()) {
                std::vector<AttributableNode*> killSources;
                findAttributableNodesWithNumberedAttribute(AttributeNames::Killtarget, targetname, killSources);
                addKillSources(killSources);
            }
        }

        void AttributableNode::addAllKillTargets() {
            for (const EntityAttribute& attribute : m_entity.numberedAttributes(AttributeNames::Killtarget)) {
                const std::string& targetname = attribute.value();
                if (!targetname.empty()) {
                    std::vector<AttributableNode*> killTargets;
                    findAttributableNodesWithAttribute(AttributeNames::Targetname, targetname, killTargets);
                    addKillTargets(killTargets);
                }
            }
        }

        void AttributableNode::addLinkTargets(const std::vector<AttributableNode*>& targets) {
            m_linkTargets.reserve(m_linkTargets.size() + targets.size());
            for (AttributableNode* target : targets) {
                target->addLinkSource(this);
                m_linkTargets.push_back(target);
            }
            invalidateIssues();
        }

        void AttributableNode::addKillTargets(const std::vector<AttributableNode*>& targets) {
            m_killTargets.reserve(m_killTargets.size() + targets.size());
            for (AttributableNode* target : targets) {
                target->addKillSource(this);
                m_killTargets.push_back(target);
            }
            invalidateIssues();
        }

        void AttributableNode::addLinkSources(const std::vector<AttributableNode*>& sources) {
            m_linkSources.reserve(m_linkSources.size() + sources.size());
            for (AttributableNode* linkSource : sources) {
                linkSource->addLinkTarget(this);
                m_linkSources.push_back(linkSource);
            }
            invalidateIssues();
        }

        void AttributableNode::addKillSources(const std::vector<AttributableNode*>& sources) {
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

            const std::string* targetname = m_entity.attribute(AttributeNames::Targetname);
            if (targetname != nullptr && !targetname->empty()) {
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
            ensure(attributable != nullptr, "attributable is null");
            m_linkSources.push_back(attributable);
            invalidateIssues();
        }

        void AttributableNode::addLinkTarget(AttributableNode* attributable) {
            ensure(attributable != nullptr, "attributable is null");
            m_linkTargets.push_back(attributable);
            invalidateIssues();
        }

        void AttributableNode::addKillSource(AttributableNode* attributable) {
            ensure(attributable != nullptr, "attributable is null");
            m_killSources.push_back(attributable);
            invalidateIssues();
        }

        void AttributableNode::addKillTarget(AttributableNode* attributable) {
            ensure(attributable != nullptr, "attributable is null");
            m_killTargets.push_back(attributable);
            invalidateIssues();
        }

        void AttributableNode::removeLinkSource(AttributableNode* attributable) {
            ensure(attributable != nullptr, "attributable is null");
            m_linkSources = kdl::vec_erase(std::move(m_linkSources), attributable);
            invalidateIssues();
        }

        void AttributableNode::removeLinkTarget(AttributableNode* attributable) {
            ensure(attributable != nullptr, "attributable is null");
            m_linkTargets = kdl::vec_erase(std::move(m_linkTargets), attributable);
            invalidateIssues();
        }

        void AttributableNode::removeKillSource(AttributableNode* attributable) {
            ensure(attributable != nullptr, "attributable is null");
            m_killSources = kdl::vec_erase(std::move(m_killSources), attributable);
            invalidateIssues();
        }

        AttributableNode::AttributableNode() :
        Node() {}

        const std::string& AttributableNode::doGetName() const {
            return m_entity.classname();
        }

        void AttributableNode::removeKillTarget(AttributableNode* attributable) {
            ensure(attributable != nullptr, "attributable is null");
            m_killTargets = kdl::vec_erase(std::move(m_killTargets), attributable);
        }

        bool operator==(const AttributableNode& lhs, const AttributableNode& rhs) {
            return lhs.entity() == rhs.entity();
        }

        bool operator!=(const AttributableNode& lhs, const AttributableNode& rhs) {
            return !(lhs == rhs);
        }
    }
}
