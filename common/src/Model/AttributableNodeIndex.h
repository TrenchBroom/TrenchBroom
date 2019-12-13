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

#ifndef TrenchBroom_EntityAttributeIndex
#define TrenchBroom_EntityAttributeIndex

#include "StringMap.h"
#include "Model/Model_Forward.h"

#include <list>
#include <set>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        using AttributableNodeIndexValueContainer = StringMultiMapValueContainer<AttributableNode*>;
        using AttributableNodeStringIndex = StringMap<AttributableNode*, AttributableNodeIndexValueContainer>;

        class AttributableNodeIndexQuery {
        public:
            typedef enum {
                Type_Exact,
                Type_Prefix,
                Type_Numbered,
                Type_Any
            } Type;
        private:
            Type m_type;
            std::string m_pattern;
        public:
            static AttributableNodeIndexQuery exact(const std::string& pattern);
            static AttributableNodeIndexQuery prefix(const std::string& pattern);
            static AttributableNodeIndexQuery numbered(const std::string& pattern);
            static AttributableNodeIndexQuery any();

            std::set<AttributableNode*> execute(const AttributableNodeStringIndex& index) const;
            bool execute(const AttributableNode* node, const std::string& value) const;
            std::list<Model::EntityAttribute> execute(const AttributableNode* node) const;
        private:
            AttributableNodeIndexQuery(Type type, const std::string& pattern = "");
        };

        class AttributableNodeIndex {
        private:
            AttributableNodeStringIndex m_nameIndex;
            AttributableNodeStringIndex m_valueIndex;
        public:
            void addAttributableNode(AttributableNode* attributable);
            void removeAttributableNode(AttributableNode* attributable);

            void addAttribute(AttributableNode* attributable, const AttributeName& name, const AttributeValue& value);
            void removeAttribute(AttributableNode* attributable, const AttributeName& name, const AttributeValue& value);

            std::vector<AttributableNode*> findAttributableNodes(const AttributableNodeIndexQuery& keyQuery, const AttributeValue& value) const;
            std::vector<std::string> allNames() const;
            std::vector<std::string> allValuesForNames(const AttributableNodeIndexQuery& keyQuery) const;
        };
    }
}

#endif /* defined(TrenchBroom_EntityAttributeIndex) */
