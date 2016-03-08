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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TrenchBroom_SmartAttributeEditorMatcher
#define TrenchBroom_SmartAttributeEditorMatcher

#include "Model/ModelTypes.h"

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
            Model::AttributeNameSet m_names;
        public:
            SmartAttributeEditorKeyMatcher(const Model::AttributeName& name1, const Model::AttributeName& name2 = "", const Model::AttributeName& name3 = "", const Model::AttributeName& name4 = "", const Model::AttributeName& name5 = "");
        private:
            bool doMatches(const Model::AttributeName& name, const Model::AttributableNodeList& attributables) const;
        };
        
        class SmartAttributeEditorDefaultMatcher : public SmartAttributeEditorMatcher {
        private:
            bool doMatches(const Model::AttributeName& name, const Model::AttributableNodeList& attributables) const;
        };
    }
}

#endif /* defined(TrenchBroom_SmartAttributeEditorMatcher) */
