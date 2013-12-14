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

#ifndef __TrenchBroom__SmartPropertyEditorMatcher__
#define __TrenchBroom__SmartPropertyEditorMatcher__

#include "Model/ModelTypes.h"

namespace TrenchBroom {
    namespace View {
        class SmartPropertyEditorMatcher {
        public:
            virtual ~SmartPropertyEditorMatcher();
            bool matches(const Model::PropertyKey& key, const Model::EntityList& entities) const;
        private:
            virtual bool doMatches(const Model::PropertyKey& key, const Model::EntityList& entities) const = 0;
        };
        
        class SmartPropertyEditorKeyMatcher : public SmartPropertyEditorMatcher {
        private:
            Model::PropertyKeySet m_keys;
        public:
            SmartPropertyEditorKeyMatcher(const Model::PropertyKey& key1, const Model::PropertyKey& key2 = "", const Model::PropertyKey& key3 = "", const Model::PropertyKey& key4 = "", const Model::PropertyKey& key5 = "");
        private:
            bool doMatches(const Model::PropertyKey& key, const Model::EntityList& entities) const;
        };
        
        class SmartPropertyEditorDefaultMatcher : public SmartPropertyEditorMatcher {
        private:
            bool doMatches(const Model::PropertyKey& key, const Model::EntityList& entities) const;
        };
    }
}

#endif /* defined(__TrenchBroom__SmartPropertyEditorMatcher__) */
