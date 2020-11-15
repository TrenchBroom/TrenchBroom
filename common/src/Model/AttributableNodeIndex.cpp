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

#include "AttributableNodeIndex.h"

#include "Macros.h"
#include "Model/AttributableNode.h"
#include "Model/Entity.h"
#include "Model/EntityAttributes.h"

#include <kdl/compact_trie.h>
#include <kdl/vector_utils.h>

#include <list>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        AttributableNodeIndexQuery AttributableNodeIndexQuery::exact(const std::string& pattern) {
            return AttributableNodeIndexQuery(Type_Exact, pattern);
        }

        AttributableNodeIndexQuery AttributableNodeIndexQuery::prefix(const std::string& pattern) {
            return AttributableNodeIndexQuery(Type_Prefix, pattern);
        }

        AttributableNodeIndexQuery AttributableNodeIndexQuery::numbered(const std::string& pattern) {
            return AttributableNodeIndexQuery(Type_Numbered, pattern);
        }

        AttributableNodeIndexQuery AttributableNodeIndexQuery::any() {
            return AttributableNodeIndexQuery(Type_Any);
        }

        std::set<AttributableNode*> AttributableNodeIndexQuery::execute(const AttributableNodeStringIndex& index) const {
            std::set<AttributableNode*> result;
            switch (m_type) {
                case Type_Exact:
                    index.find_matches(m_pattern, std::inserter(result, std::end(result)));
                    break;
                case Type_Prefix:
                    index.find_matches(m_pattern + "*", std::inserter(result, std::end(result)));
                    break;
                case Type_Numbered:
                    index.find_matches(m_pattern + "%*", std::inserter(result, std::end(result)));
                    break;
                case Type_Any:
                    break;
                switchDefault()
            }
            return result;
        }

        bool AttributableNodeIndexQuery::execute(const AttributableNode* node, const std::string& value) const {
            switch (m_type) {
                case Type_Exact:
                    return node->entity().hasAttribute(m_pattern, value);
                case Type_Prefix:
                    return node->entity().hasAttributeWithPrefix(m_pattern, value);
                case Type_Numbered:
                    return node->entity().hasNumberedAttribute(m_pattern, value);
                case Type_Any:
                    return true;
                switchDefault()
            }
        }

        std::vector<Model::EntityAttribute> AttributableNodeIndexQuery::execute(const AttributableNode* node) const {
            const auto& entity = node->entity();
            switch (m_type) {
                case Type_Exact:
                    return entity.attributeWithName(m_pattern);
                case Type_Prefix:
                    return entity.attributesWithPrefix(m_pattern);
                case Type_Numbered:
                    return entity.numberedAttributes(m_pattern);
                case Type_Any:
                    return entity.attributes();
                switchDefault()
            }
        }

        AttributableNodeIndexQuery::AttributableNodeIndexQuery(const Type type, const std::string& pattern) :
        m_type(type),
        m_pattern(pattern) {}

        AttributableNodeIndex::AttributableNodeIndex() :
        m_nameIndex(std::make_unique<AttributableNodeStringIndex>()),
        m_valueIndex(std::make_unique<AttributableNodeStringIndex>()) {}

        AttributableNodeIndex::~AttributableNodeIndex() = default;

        void AttributableNodeIndex::addAttributableNode(AttributableNode* attributable) {
            for (const EntityAttribute& attribute : attributable->entity().attributes())
                addAttribute(attributable, attribute.name(), attribute.value());
        }

        void AttributableNodeIndex::removeAttributableNode(AttributableNode* attributable) {
            for (const EntityAttribute& attribute : attributable->entity().attributes())
                removeAttribute(attributable, attribute.name(), attribute.value());
        }

        void AttributableNodeIndex::addAttribute(AttributableNode* attributable, const std::string& name, const std::string& value) {
            m_nameIndex->insert(name, attributable);
            m_valueIndex->insert(value, attributable);
        }

        void AttributableNodeIndex::removeAttribute(AttributableNode* attributable, const std::string& name, const std::string& value) {
            m_nameIndex->remove(name, attributable);
            m_valueIndex->remove(value, attributable);
        }

        std::vector<AttributableNode*> AttributableNodeIndex::findAttributableNodes(const AttributableNodeIndexQuery& nameQuery, const std::string& value) const {
            const std::set<AttributableNode*> nameResult = nameQuery.execute(*m_nameIndex);

            std::set<AttributableNode*> valueResult;
            m_valueIndex->find_matches(value, std::inserter(valueResult, std::end(valueResult)));
            if (nameResult.empty() || valueResult.empty()) {
                return {};
            }

            std::vector<AttributableNode*> result = kdl::set_intersection(nameResult, valueResult);

            auto it = std::begin(result);
            while (it != std::end(result)) {
                const AttributableNode* node = *it;
                if (!nameQuery.execute(node, value))
                    it = result.erase(it);
                else
                    ++it;
            }

            return result;
        }

        std::vector<std::string> AttributableNodeIndex::allNames() const {
            std::vector<std::string> result;
            m_nameIndex->get_keys(std::back_inserter(result));
            return result;
        }

        std::vector<std::string> AttributableNodeIndex::allValuesForNames(const AttributableNodeIndexQuery& keyQuery) const {
            std::vector<std::string> result;

            const std::set<AttributableNode*> nameResult = keyQuery.execute(*m_nameIndex);
            for (const auto node : nameResult) {
                const auto matchingAttributes = keyQuery.execute(node);
                for (const auto& attribute : matchingAttributes) {
                    result.push_back(attribute.value());
                }
            }

            return result;
        }
    }
}
