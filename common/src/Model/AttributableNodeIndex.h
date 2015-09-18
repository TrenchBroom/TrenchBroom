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

#ifndef TrenchBroom_EntityAttributeIndex
#define TrenchBroom_EntityAttributeIndex

#include "StringUtils.h"
#include "Model/ModelTypes.h"
#include "StringMultiMap.h"

#include <map>

namespace TrenchBroom {
    namespace Model {
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
            String m_pattern;
        public:
            static AttributableNodeIndexQuery exact(const String& pattern);
            static AttributableNodeIndexQuery prefix(const String& pattern);
            static AttributableNodeIndexQuery numbered(const String& pattern);
            static AttributableNodeIndexQuery any();

            AttributableNodeSet execute(const StringMultiMap<AttributableNode*>& index) const;
            bool execute(const AttributableNode* node, const String& value) const;
        private:
            AttributableNodeIndexQuery(Type type, const String& pattern = "");
        };
        
        class AttributableNodeIndex {
        private:
            StringMultiMap<AttributableNode*> m_nameIndex;
            StringMultiMap<AttributableNode*> m_valueIndex;
        public:
            void addAttributableNode(AttributableNode* attributable);
            void removeAttributableNode(AttributableNode* attributable);
            
            void addAttribute(AttributableNode* attributable, const AttributeName& name, const AttributeValue& value);
            void removeAttribute(AttributableNode* attributable, const AttributeName& name, const AttributeValue& value);
            
            AttributableNodeList findAttributableNodes(const AttributableNodeIndexQuery& keyQuery, const AttributeValue& value) const;
        };
    }
}

#endif /* defined(TrenchBroom_EntityAttributeIndex) */
