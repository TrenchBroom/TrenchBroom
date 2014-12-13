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
        
        AttributableNodeSet AttributableNodeIndexQuery::execute(const StringIndex<AttributableNode*>& index) const {
            switch (m_type) {
                case Type_Exact:
                    return index.queryExactMatches(m_pattern);
                case Type_Prefix:
                    return index.queryPrefixMatches(m_pattern);
                case Type_Numbered:
                    return index.queryNumberedMatches(m_pattern);
                case Type_Any:
                    return EmptyAttributableNodeSet;
                DEFAULT_SWITCH()
            }
        }
        
        AttributableNodeIndexQuery::AttributableNodeIndexQuery(const Type type, const String& pattern) :
        m_type(type),
        m_pattern(pattern) {}

        void AttributableNodeIndex::addAttributableNode(AttributableNode* attributable) {
            const EntityAttribute::List& attributes = attributable->attributes();
            EntityAttribute::List::const_iterator it, end;
            for (it = attributes.begin(), end = attributes.end(); it != end; ++it) {
                const EntityAttribute& attribute = *it;
                addAttribute(attributable, attribute.name(), attribute.value());
            }
        }
        
        void AttributableNodeIndex::removeAttributableNode(AttributableNode* attributable) {
            const EntityAttribute::List& attributes = attributable->attributes();
            EntityAttribute::List::const_iterator it, end;
            for (it = attributes.begin(), end = attributes.end(); it != end; ++it) {
                const EntityAttribute& attribute = *it;
                removeAttribute(attributable, attribute.name(), attribute.value());
            }
        }

        void AttributableNodeIndex::addAttribute(AttributableNode* attributable, const AttributeName& name, const AttributeValue& value) {
            m_nameIndex.insert(name, attributable);
            m_valueIndex.insert(value, attributable);
        }
        
        void AttributableNodeIndex::removeAttribute(AttributableNode* attributable, const AttributeName& name, const AttributeValue& value) {
            m_nameIndex.remove(name, attributable);
            m_valueIndex.remove(value, attributable);
        }

        AttributableNodeList AttributableNodeIndex::findAttributableNodes(const AttributableNodeIndexQuery& nameQuery, const AttributableNodeIndexQuery& valueQuery) const {
            const AttributableNodeSet nameResult = nameQuery.execute(m_nameIndex);
            const AttributableNodeSet valueResult = valueQuery.execute(m_valueIndex);
            
            if (nameResult.empty() || valueResult.empty())
                return EmptyAttributableNodeList;

            AttributableNodeList result;
            SetUtils::intersection(nameResult, valueResult, result);
            return result;
        }
    }
}
