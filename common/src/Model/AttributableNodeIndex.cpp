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

#include "CollectionUtils.h"
#include "Macros.h"
#include "Model/AttributableNode.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        AttributableNodeIndexQuery AttributableNodeIndexQuery::exact(const String& pattern) {
            return AttributableNodeIndexQuery(Type_Exact, pattern);
        }

        AttributableNodeIndexQuery AttributableNodeIndexQuery::prefix(const String& pattern) {
            return AttributableNodeIndexQuery(Type_Prefix, pattern);
        }

        AttributableNodeIndexQuery AttributableNodeIndexQuery::numbered(const String& pattern) {
            return AttributableNodeIndexQuery(Type_Numbered, pattern);
        }

        AttributableNodeIndexQuery AttributableNodeIndexQuery::any() {
            return AttributableNodeIndexQuery(Type_Any);
        }

        AttributableNodeSet AttributableNodeIndexQuery::execute(const AttributableNodeStringIndex& index) const {
            switch (m_type) {
                case Type_Exact:
                    return index.queryExactMatches(m_pattern);
                case Type_Prefix:
                    return index.queryPrefixMatches(m_pattern);
                case Type_Numbered:
                    return index.queryNumberedMatches(m_pattern);
                case Type_Any:
                    return EmptyAttributableNodeSet;
                switchDefault()
            }
        }

        bool AttributableNodeIndexQuery::execute(const AttributableNode* node, const String& value) const {
            switch (m_type) {
                case Type_Exact:
                    return node->hasAttribute(m_pattern, value);
                case Type_Prefix:
                    return node->hasAttributeWithPrefix(m_pattern, value);
                case Type_Numbered:
                    return node->hasNumberedAttribute(m_pattern, value);
                case Type_Any:
                    return true;
                switchDefault()
            }
        }

        Model::EntityAttribute::List AttributableNodeIndexQuery::execute(const AttributableNode* node) const {
            switch (m_type) {
                case Type_Exact:
                    return node->attributeWithName(m_pattern);
                case Type_Prefix:
                    return node->attributesWithPrefix(m_pattern);
                case Type_Numbered:
                    return node->numberedAttributes(m_pattern);
                case Type_Any:
                    return node->attributes();
                switchDefault()
            }
        }

        AttributableNodeIndexQuery::AttributableNodeIndexQuery(const Type type, const String& pattern) :
        m_type(type),
        m_pattern(pattern) {}

        void AttributableNodeIndex::addAttributableNode(AttributableNode* attributable) {
            for (const EntityAttribute& attribute : attributable->attributes())
                addAttribute(attributable, attribute.name(), attribute.value());
        }

        void AttributableNodeIndex::removeAttributableNode(AttributableNode* attributable) {
            for (const EntityAttribute& attribute : attributable->attributes())
                removeAttribute(attributable, attribute.name(), attribute.value());
        }

        void AttributableNodeIndex::addAttribute(AttributableNode* attributable, const AttributeName& name, const AttributeValue& value) {
            m_nameIndex.insert(name, attributable);
            m_valueIndex.insert(value, attributable);
        }

        void AttributableNodeIndex::removeAttribute(AttributableNode* attributable, const AttributeName& name, const AttributeValue& value) {
            m_nameIndex.remove(name, attributable);
            m_valueIndex.remove(value, attributable);
        }

        AttributableNodeList AttributableNodeIndex::findAttributableNodes(const AttributableNodeIndexQuery& nameQuery, const AttributeValue& value) const {
            const AttributableNodeSet nameResult = nameQuery.execute(m_nameIndex);
            const AttributableNodeSet valueResult = m_valueIndex.queryExactMatches(value);

            if (nameResult.empty() || valueResult.empty())
                return EmptyAttributableNodeList;

            AttributableNodeList result;
            SetUtils::intersection(nameResult, valueResult, result);

            AttributableNodeList::iterator it = std::begin(result);
            while (it != std::end(result)) {
                const AttributableNode* node = *it;
                if (!nameQuery.execute(node, value))
                    it = result.erase(it);
                else
                    ++it;
            }

            return result;
        }

        StringList AttributableNodeIndex::allNames() const {
            return m_nameIndex.getKeys();
        }

        StringList AttributableNodeIndex::allValuesForNames(const AttributableNodeIndexQuery& keyQuery) const {
            StringList result;

            const AttributableNodeSet nameResult = keyQuery.execute(m_nameIndex);
            for (const auto node : nameResult) {
                const Model::EntityAttribute::List matchingAttributes = keyQuery.execute(node);
                for (const auto& attribute : matchingAttributes) {
                    result.push_back(attribute.value());
                }
            }

            return result;
        }
    }
}
