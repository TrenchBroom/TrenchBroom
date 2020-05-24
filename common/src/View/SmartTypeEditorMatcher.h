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

#ifndef TrenchBroom_SmartTypeEditorMatcher
#define TrenchBroom_SmartTypeEditorMatcher

#include "View/SmartAttributeEditorMatcher.h"

#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Assets {
        enum class AttributeDefinitionType;
    }
    
    namespace Model {
        class AttributableNode;
    }

    namespace View {
        class SmartTypeEditorMatcher : public SmartAttributeEditorMatcher {
        private:
            Assets::AttributeDefinitionType m_type;
        public:
            SmartTypeEditorMatcher(Assets::AttributeDefinitionType type);
        private:
            bool doMatches(const std::string& name, const std::vector<Model::AttributableNode*>& attributables) const override;
        };
    }
}

#endif /* defined(TrenchBroom_SmartTypeEditorMatcher) */
