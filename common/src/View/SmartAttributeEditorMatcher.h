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

#ifndef TrenchBroom_SmartAttributeEditorMatcher
#define TrenchBroom_SmartAttributeEditorMatcher

#include "Model/ModelTypes.h"

#include <initializer_list>

namespace TrenchBroom {
    namespace View {
        class SmartAttributeEditorMatcher {
        public:
            virtual ~SmartAttributeEditorMatcher();
            bool matches(const Model::AttributeName& name, const Model::AttributableNodeList& attributables) const;
        private:
            virtual bool doMatches(const Model::AttributeName& name, const Model::AttributableNodeList& attributables) const = 0;
        };
        
        class SmartAttributeEditorKeyMatcher : public SmartAttributeEditorMatcher {
        private:
            Model::AttributeNameSet m_patterns;
        public:
            SmartAttributeEditorKeyMatcher(const String& pattern);
            SmartAttributeEditorKeyMatcher(std::initializer_list<String> patterns);
        private:
            bool doMatches(const Model::AttributeName& name, const Model::AttributableNodeList& attributables) const override;
        };
        
        class SmartAttributeEditorDefaultMatcher : public SmartAttributeEditorMatcher {
        private:
            bool doMatches(const Model::AttributeName& name, const Model::AttributableNodeList& attributables) const override;
        };
    }
}

#endif /* defined(TrenchBroom_SmartAttributeEditorMatcher) */
