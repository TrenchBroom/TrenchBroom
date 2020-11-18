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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "SmartTypeEditorMatcher.h"

#include "Assets/AttributeDefinition.h"
#include "Model/AttributableNode.h"

namespace TrenchBroom {
    namespace View {
        // SmartTypeEditorMatcher

        SmartTypeEditorMatcher::SmartTypeEditorMatcher(const Assets::AttributeDefinitionType type) :
        m_type(type) {}

        bool SmartTypeEditorMatcher::doMatches(const std::string& name, const std::vector<Model::AttributableNode*>& attributables) const {
            if (attributables.empty()) {
                return false;
            }
            for (const auto* node : attributables) {
                const auto* attrDef = Model::attributeDefinition(node, name);
                if (attrDef == nullptr) {
                    return false;
                }
                if (attrDef->type() != m_type) {
                    return false;
                }
            }
            return true;
        }

        // SmartTypeWithSameDefinitionEditorMatcher

        SmartTypeWithSameDefinitionEditorMatcher::SmartTypeWithSameDefinitionEditorMatcher(const Assets::AttributeDefinitionType type) :
        m_type(type) {}

        bool SmartTypeWithSameDefinitionEditorMatcher::doMatches(const std::string& name, const std::vector<Model::AttributableNode*>& attributables) const {
            const Assets::AttributeDefinition* attrDef = Model::selectAttributeDefinition(name, attributables);
            return attrDef != nullptr && attrDef->type() == m_type;
        }
    }
}
