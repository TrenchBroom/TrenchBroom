/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "SmartPropertyEditorMatcher.h"

namespace TrenchBroom {
    namespace View {
        SmartPropertyEditorMatcher::~SmartPropertyEditorMatcher() {}
        
        bool SmartPropertyEditorMatcher::matches(const Model::PropertyKey& key, const Model::EntityList& entities) const {
            return doMatches(key, entities);
        }

        SmartPropertyEditorKeyMatcher::SmartPropertyEditorKeyMatcher(const Model::PropertyKey& key) :
        m_key(key) {}
        
        bool SmartPropertyEditorKeyMatcher::doMatches(const Model::PropertyKey& key, const Model::EntityList& entities) const {
            return !entities.empty() && m_key == key;
        }

        bool SmartPropertyEditorDefaultMatcher::doMatches(const Model::PropertyKey& key, const Model::EntityList& entities) const {
            return true;
        }
    }
}
