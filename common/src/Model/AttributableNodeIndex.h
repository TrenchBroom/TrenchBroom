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

#include <kdl/compact_trie_forward.h>

#include <memory>
#include <set>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class AttributableNode;
        class EntityAttribute;

        using AttributableNodeStringIndex = kdl::compact_trie<AttributableNode*>;

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
            std::vector<Model::EntityAttribute> execute(const AttributableNode* node) const;
        private:
            explicit AttributableNodeIndexQuery(Type type, const std::string& pattern = "");
        };

        class AttributableNodeIndex {
        private:
            std::unique_ptr<AttributableNodeStringIndex> m_nameIndex;
            std::unique_ptr<AttributableNodeStringIndex> m_valueIndex;
        public:
            AttributableNodeIndex();
            ~AttributableNodeIndex();

            void addAttributableNode(AttributableNode* attributable);
            void removeAttributableNode(AttributableNode* attributable);

            void addAttribute(AttributableNode* attributable, const std::string& name, const std::string& value);
            void removeAttribute(AttributableNode* attributable, const std::string& name, const std::string& value);

            std::vector<AttributableNode*> findAttributableNodes(const AttributableNodeIndexQuery& keyQuery, const std::string& value) const;
            std::vector<std::string> allNames() const;
            std::vector<std::string> allValuesForNames(const AttributableNodeIndexQuery& keyQuery) const;
        };
    }
}

#endif /* defined(TrenchBroom_EntityAttributeIndex) */
