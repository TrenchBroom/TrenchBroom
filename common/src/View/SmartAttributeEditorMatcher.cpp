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

#include "SmartAttributeEditorMatcher.h"

#include <kdl/string_compare.h>
#include <kdl/vector_utils.h>

#include <vector>

namespace TrenchBroom {
    namespace View {
        SmartAttributeEditorMatcher::~SmartAttributeEditorMatcher() {}

        bool SmartAttributeEditorMatcher::matches(const std::string& name, const std::vector<Model::AttributableNode*>& attributables) const {
            return doMatches(name, attributables);
        }

        SmartAttributeEditorKeyMatcher::SmartAttributeEditorKeyMatcher(const std::string& pattern) :
        SmartAttributeEditorKeyMatcher({ pattern }) {}

        SmartAttributeEditorKeyMatcher::SmartAttributeEditorKeyMatcher(const std::initializer_list<std::string> patterns) :
        m_patterns(patterns) {
            m_patterns = kdl::vec_sort_and_remove_duplicates(std::move(m_patterns));
        }

        bool SmartAttributeEditorKeyMatcher::doMatches(const std::string& name, const std::vector<Model::AttributableNode*>& attributables) const {
            if (attributables.empty()) {
                return false;
            }

            for (const std::string& pattern : m_patterns) {
                if (kdl::cs::str_matches_glob(name, pattern)) {
                    return true;
                }
            }

            return false;
        }

        bool SmartAttributeEditorDefaultMatcher::doMatches(const std::string& /* name */, const std::vector<Model::AttributableNode*>& /* attributables */) const {
            return true;
        }
    }
}
