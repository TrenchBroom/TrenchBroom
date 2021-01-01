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

#include <initializer_list>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class EntityNodeBase;
    }

    namespace View {
        class SmartPropertyEditorMatcher {
        public:
            virtual ~SmartPropertyEditorMatcher();
            bool matches(const std::string& propertyKey, const std::vector<Model::EntityNodeBase*>& nodes) const;
        private:
            virtual bool doMatches(const std::string& propertyKey, const std::vector<Model::EntityNodeBase*>& nodes) const = 0;
        };

        class SmartPropertyEditorKeyMatcher : public SmartPropertyEditorMatcher {
        private:
            std::vector<std::string> m_patterns;
        public:
            SmartPropertyEditorKeyMatcher(const std::string& pattern);
            SmartPropertyEditorKeyMatcher(std::initializer_list<std::string> patterns);
        private:
            bool doMatches(const std::string& propertyKey, const std::vector<Model::EntityNodeBase*>& nodes) const override;
        };

        class SmartPropertyEditorDefaultMatcher : public SmartPropertyEditorMatcher {
        private:
            bool doMatches(const std::string& propertyKey, const std::vector<Model::EntityNodeBase*>& nodes) const override;
        };
    }
}

