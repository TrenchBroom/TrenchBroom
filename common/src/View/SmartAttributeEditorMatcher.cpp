/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "SmartAttributeEditorMatcher.h"

namespace TrenchBroom {
    namespace View {
        SmartAttributeEditorMatcher::~SmartAttributeEditorMatcher() {}
        
        bool SmartAttributeEditorMatcher::matches(const Model::AttributeName& name, const Model::AttributableNodeList& attributables) const {
            return doMatches(name, attributables);
        }

        SmartAttributeEditorKeyMatcher::SmartAttributeEditorKeyMatcher(const Model::AttributeName& name1, const Model::AttributeName& name2, const Model::AttributeName& name3, const Model::AttributeName& name4, const Model::AttributeName& name5) {
            if (!name1.empty())
                m_names.insert(name1);
            if (!name2.empty())
                m_names.insert(name2);
            if (!name3.empty())
                m_names.insert(name3);
            if (!name4.empty())
                m_names.insert(name4);
            if (!name5.empty())
                m_names.insert(name5);
        }
        
        bool SmartAttributeEditorKeyMatcher::doMatches(const Model::AttributeName& name, const Model::AttributableNodeList& attributables) const {
            return !attributables.empty() && m_names.count(name) > 0;
        }

        bool SmartAttributeEditorDefaultMatcher::doMatches(const Model::AttributeName& name, const Model::AttributableNodeList& attributables) const {
            return true;
        }
    }
}
