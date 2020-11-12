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
#include "Model/EntityAttributeSnapshot.h"

#include <kdl/collection_utils.h>
#include <kdl/vector_utils.h>

#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        Assets::EntityDefinition* AttributableNode::selectEntityDefinition(const std::vector<AttributableNode*>& attributables) {
            Assets::EntityDefinition* definition = nullptr;

            for (AttributableNode* attributable : attributables) {
                if (definition == nullptr) {
                    definition = attributable->definition();
                } else if (definition != attributable->definition()) {
                    definition = nullptr;
                    break;
                }
            }

            return definition;
        }

        const Assets::AttributeDefinition* AttributableNode::selectAttributeDefinition(const std::string& name, const std::vector<AttributableNode*>& attributables) {
            std::vector<AttributableNode*>::const_iterator it = std::begin(attributables);
            std::vector<AttributableNode*>::const_iterator end = std::end(attributables);
            if (it == end)
                return nullptr;

            const AttributableNode* attributable = *it;
            const Assets::AttributeDefinition* definition = attributable->attributeDefinition(name);
            if (definition == nullptr)
                return nullptr;

            while (++it != end) {
                attributable = *it;
                const Assets::AttributeDefinition* currentDefinition = attributable->attributeDefinition(name);
                if (currentDefinition == nullptr)
                    return nullptr;

                if (!definition->equals(currentDefinition))
                    return nullptr;
            }

            return definition;
        }

        std::string AttributableNode::selectAttributeValue(const std::string& name, const std::vector<AttributableNode*>& attributables) {
            std::vector<AttributableNode*>::const_iterator it = std::begin(attributables);
            std::vector<AttributableNode*>::const_iterator end = std::end(attributables);
            if (it == end)
                return "";

            const AttributableNode* attributable = *it;
            if (!attributable->hasAttribute(name))
                return "";

            const std::string& value = attributable->attribute(name);
            while (++it != end) {
                attributable = *it;
                if (!attributable->hasAttribute(name))
                    return "";
                if (value != attributable->attribute(name))
                    return "";
            }
            return value;
        }

        const std::string AttributableNode::DefaultAttributeValue("");

        AttributableNode::~AttributableNode() {
            m_definition = nullptr;
        }

        Assets::EntityDefinition* AttributableNode::definition() const {
            return m_definition;
        }

        void AttributableNode::setDefinition(Assets::EntityDefinition* definition) {
            if (m_definition == definition)
                return;

            const NotifyAttributeChange notifyChange(this);
            if (m_definition != nullptr)
                m_definition->decUsageCount();
            m_definition = definition;
            m_attributes.updateDefinitions(m_definition);
            if (m_definition != nullptr)
                m_definition->incUsageCount();
        }

        const Assets::AttributeDefinition* AttributableNode::attributeDefinition(const std::string& name) const {
            return m_definition == nullptr ? nullptr : m_definition->attributeDefinition(name);
        }

        const std::vector<EntityAttribute>& AttributableNode::attributes() const {
            return m_attributes.attributes();
        }

        void AttributableNode::setAttributes(const std::vector<EntityAttribute>& attributes) {
            const NotifyAttributeChange notifyChange(this);
            updateAttributeIndex(attributes);
            m_attributes.setAttributes(attributes);
            m_attributes.updateDefinitions(m_definition);
        }

        std::vector<std::string> AttributableNode::attributeNames() const {
            return m_attributes.names();
        }

        bool AttributableNode::hasAttribute(const std::string& name) const {
            return m_attributes.hasAttribute(name);
        }

        bool AttributableNode::hasAttribute(const std::string& name, const std::string& value) const {
            return m_attributes.hasAttribute(name, value);
        }

        bool AttributableNode::hasAttributeWithPrefix(const std::string& prefix, const std::string& value) const {
            return m_attributes.hasAttributeWithPrefix(prefix, value);
        }

        bool AttributableNode::hasNumberedAttribute(const std::string& prefix, const std::string& value) const {
            return m_attributes.hasNumberedAttribute(prefix, value);
        }

        std::vector<EntityAttribute> AttributableNode::attributeWithName(const std::string& name) const {
            return m_attributes.attributeWithName(name);
        }

        std::vector<EntityAttribute> AttributableNode::attributesWithPrefix(const std::string& prefix) const {
            return m_attributes.attributesWithPrefix(prefix);
        }

        std::vector<EntityAttribute> AttributableNode::numberedAttributes(const std::string& prefix) const {
            return m_attributes.numberedAttributes(prefix);
        }

        const std::string& AttributableNode::attribute(const std::string& name, const std::string& defaultValue) const {
            const std::string* value = m_attributes.attribute(name);
            if (value == nullptr)
                return defaultValue;
            return *value;
        }

        const std::string& AttributableNode::classname(const std::string& defaultClassname) const {
            return m_classname.empty() ? defaultClassname : m_classname;
        }

        EntityAttributeSnapshot AttributableNode::attributeSnapshot(const std::string& name) const {
            return m_attributes.snapshot(name);
        }

        bool AttributableNode::canAddOrUpdateAttribute(const std::string& name, const std::string& /* value */) const {
            return isAttributeValueMutable(name);
        }

        bool AttributableNode::addOrUpdateAttribute(const std::string& name, const std::string& value) {
            const NotifyAttributeChange notifyChange(this);

            const Assets::AttributeDefinition* definition = Assets::EntityDefinition::safeGetAttributeDefinition(m_definition, name);
            const std::string* oldValue = m_attributes.attribute(name);
            if (oldValue != nullptr) {
                removeAttributeFromIndex(name, *oldValue);
                removeLinks(name, *oldValue);
            }

            m_attributes.addOrUpdateAttribute(name, value, definition);
            addAttributeToIndex(name, value);
            addLinks(name, value);

            return oldValue == nullptr;
        }

        bool AttributableNode::canRenameAttribute(const std::string& name, const std::string& newName) const {
            return isAttributeNameMutable(name) && isAttributeNameMutable(newName);
        }

        void AttributableNode::renameAttribute(const std::string& name, const std::string& newName) {
            if (name == newName)
                return;

            const std::string* valuePtr = m_attributes.attribute(name);
            if (valuePtr == nullptr)
                return;

            const std::string value = *valuePtr;
			const NotifyAttributeChange notifyChange(this);

            const Assets::AttributeDefinition* newDefinition = Assets::EntityDefinition::safeGetAttributeDefinition(m_definition, newName);

            m_attributes.renameAttribute(name, newName, newDefinition);

            updateAttributeIndex(name, value, newName, value);
            updateLinks(name, value, newName, value);
        }

        bool AttributableNode::canRemoveAttribute(const std::string& name) const {
            return isAttributeNameMutable(name) && isAttributeValueMutable(name);
        }

        void AttributableNode::removeAttribute(const std::string& name) {
            const std::string* valuePtr = m_attributes.attribute(name);
            if (valuePtr == nullptr)
                return;

            const NotifyAttributeChange notifyChange(this);

            const std::string value = *valuePtr;
            m_attributes.removeAttribute(name);

            removeAttributeFromIndex(name, value);
            removeLinks(name, value);
        }

        void AttributableNode::removeNumberedAttribute(const std::string& prefix) {
            const auto attributes = m_attributes.numberedAttributes(prefix);
            if (!attributes.empty()) {
                const NotifyAttributeChange notifyChange(this);

                for (const EntityAttribute& attribute : m_attributes.attributes()) {
                    const std::string& name = attribute.name();
                    const std::string& value = attribute.value();

                    m_attributes.removeAttribute(name);
                    removeAttributeFromIndex(name, value);
                    removeLinks(name, value);
                }
            }
        }

        bool AttributableNode::isAttributeNameMutable(const std::string& name) const {
            return doIsAttributeNameMutable(name);
        }

        bool AttributableNode::isAttributeValueMutable(const std::string& name) const {
            return doIsAttributeValueMutable(name);
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
            updateClassname();
            doAttributesDidChange(oldPhysicalBounds);
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

        void AttributableNode::updateAttributeIndex(const std::vector<EntityAttribute>& newAttributes) {
            const auto oldSorted = kdl::col_sort(m_attributes.attributes());
            const auto newSorted = kdl::col_sort(newAttributes);

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

        void AttributableNode::addAttributeToIndex(const std::string& name, const std::string& value) {
            addToIndex(this, name, value);
        }

        void AttributableNode::removeAttributeFromIndex(const std::string& name, const std::string& value) {
            removeFromIndex(this, name, value);
        }

        void AttributableNode::updateAttributeIndex(const std::string& oldName, const std::string& oldValue, const std::string& newName, const std::string& newValue) {
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
                    hasAttribute(AttributeNames::Targetname));
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
            for (const EntityAttribute& attribute : m_attributes.numberedAttributes(prefix)) {
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
                    const std::string& targetTargetname = target->attribute(AttributeNames::Targetname);
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

        void AttributableNode::removeKillTargets(const std::string& targetname) {
            if (!targetname.empty()) {
                std::vector<AttributableNode*>::iterator rem = std::end(m_killTargets);
                std::vector<AttributableNode*>::iterator it = std::begin(m_killTargets);
                while (it != rem) {
                    AttributableNode* target = *it;
                    const std::string& targetTargetname = target->attribute(AttributeNames::Targetname);
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

        void AttributableNode::addAllLinkSources(const std::string& targetname) {
            if (!targetname.empty()) {
                std::vector<AttributableNode*> linkSources;
                findAttributableNodesWithNumberedAttribute(AttributeNames::Target, targetname, linkSources);
                addLinkSources(linkSources);
            }
        }

        void AttributableNode::addAllLinkTargets() {
            for (const EntityAttribute& attribute : m_attributes.numberedAttributes(AttributeNames::Target)) {
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
            for (const EntityAttribute& attribute : m_attributes.numberedAttributes(AttributeNames::Killtarget)) {
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

            const std::string* targetname = m_attributes.attribute(AttributeNames::Targetname);
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
        Node(),
        m_definition(nullptr) {}

        const std::string& AttributableNode::doGetName() const {
            static const std::string defaultName("<missing classname>");
            return classname(defaultName);
        }

        void AttributableNode::removeKillTarget(AttributableNode* attributable) {
            ensure(attributable != nullptr, "attributable is null");
            m_killTargets = kdl::vec_erase(std::move(m_killTargets), attributable);
        }

        bool operator==(const AttributableNode& lhs, const AttributableNode& rhs) {
            return lhs.attributes() == rhs.attributes();
        }

        bool operator!=(const AttributableNode& lhs, const AttributableNode& rhs) {
            return !(lhs == rhs);
        }
    }
}
