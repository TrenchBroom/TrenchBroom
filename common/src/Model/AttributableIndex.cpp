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

#include "AttributableIndex.h"

#include "CollectionUtils.h"
#include "Macros.h"
#include "Model/Attributable.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        AttributableIndexQuery AttributableIndexQuery::exact(const String& pattern) {
            return AttributableIndexQuery(Type_Exact, pattern);
        }
        
        AttributableIndexQuery AttributableIndexQuery::prefix(const String& pattern) {
            return AttributableIndexQuery(Type_Prefix, pattern);
        }
        
        AttributableIndexQuery AttributableIndexQuery::numbered(const String& pattern) {
            return AttributableIndexQuery(Type_Numbered, pattern);
        }
        
        AttributableIndexQuery AttributableIndexQuery::any() {
            return AttributableIndexQuery(Type_Any);
        }
        
        AttributableSet AttributableIndexQuery::execute(const StringIndex<Attributable*>& index) const {
            switch (m_type) {
                case Type_Exact:
                    return index.queryExactMatches(m_pattern);
                case Type_Prefix:
                    return index.queryPrefixMatches(m_pattern);
                case Type_Numbered:
                    return index.queryNumberedMatches(m_pattern);
                case Type_Any:
                    return EmptyAttributableSet;
                DEFAULT_SWITCH()
            }
        }
        
        AttributableIndexQuery::AttributableIndexQuery(const Type type, const String& pattern) :
        m_type(type),
        m_pattern(pattern) {}

        void AttributableIndex::addAttributable(Attributable* attributable) {
            const EntityAttribute::List& attributes = attributable->attributes();
            EntityAttribute::List::const_iterator it, end;
            for (it = attributes.begin(), end = attributes.end(); it != end; ++it) {
                const EntityAttribute& attribute = *it;
                addAttribute(attributable, attribute.name(), attribute.value());
            }
        }
        
        void AttributableIndex::removeAttributable(Attributable* attributable) {
            const EntityAttribute::List& attributes = attributable->attributes();
            EntityAttribute::List::const_iterator it, end;
            for (it = attributes.begin(), end = attributes.end(); it != end; ++it) {
                const EntityAttribute& attribute = *it;
                removeAttribute(attributable, attribute.name(), attribute.value());
            }
        }

        void AttributableIndex::addAttribute(Attributable* attributable, const AttributeName& name, const AttributeValue& value) {
            m_nameIndex.insert(name, attributable);
            m_valueIndex.insert(value, attributable);
        }
        
        void AttributableIndex::removeAttribute(Attributable* attributable, const AttributeName& name, const AttributeValue& value) {
            m_nameIndex.remove(name, attributable);
            m_valueIndex.remove(value, attributable);
        }

        AttributableList AttributableIndex::findAttributables(const AttributableIndexQuery& nameQuery, const AttributableIndexQuery& valueQuery) const {
            const AttributableSet nameResult = nameQuery.execute(m_nameIndex);
            const AttributableSet valueResult = valueQuery.execute(m_valueIndex);
            
            if (nameResult.empty() || valueResult.empty())
                return EmptyAttributableList;

            AttributableList result;
            SetUtils::intersection(nameResult, valueResult, result);
            return result;
        }
    }
}
