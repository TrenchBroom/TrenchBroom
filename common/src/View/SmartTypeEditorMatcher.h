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

#pragma once

#include "View/SmartPropertyEditorMatcher.h"

#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Assets {
        enum class PropertyDefinitionType;
    }
    
    namespace Model {
        class EntityNodeBase;
    }

    namespace View {
        /**
         * Matches if all of the nodes have an attribute definition for the give attribute name that is of the
         * type passed to the constructor.
         */
        class SmartTypeEditorMatcher : public SmartAttributeEditorMatcher {
        private:
            Assets::PropertyDefinitionType m_type;
        public:
            SmartTypeEditorMatcher(Assets::PropertyDefinitionType type);
        private:
            bool doMatches(const std::string& name, const std::vector<Model::EntityNodeBase*>& attributables) const override;
        };

        /**
         * Matches if all of the nodes have an attribute definition for the give attribute name that is of the
         * type passed to the constructor, and these attribute definitions are all equal.
         */
        class SmartTypeWithSameDefinitionEditorMatcher : public SmartAttributeEditorMatcher {
        private:
            Assets::PropertyDefinitionType m_type;
        public:
            SmartTypeWithSameDefinitionEditorMatcher(Assets::PropertyDefinitionType type);
        private:
            bool doMatches(const std::string& name, const std::vector<Model::EntityNodeBase*>& attributables) const override;
        };
    }
}

