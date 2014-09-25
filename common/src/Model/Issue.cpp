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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Model/Issue.h"

#include "Model/Node.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        Issue::~Issue() {}

        size_t Issue::lineNumber() const {
            return m_node->lineNumber();
        }
        
        const String& Issue::description() const {
            return doGetDescription();
        }

        bool Issue::hidden() const {
            return m_node->issueHidden(type());
        }
        
        void Issue::setHidden(const bool hidden) {
            m_node->setIssueHidden(type(), hidden);
        }

        Issue::Issue(Node* node) :
        m_node(node) {
            assert(m_node != NULL);
        }

        IssueType Issue::type() const {
            return doGetType();
        }

        IssueType Issue::freeType() {
            static IssueType type = 1;
            const IssueType result = type;
            type = (type << 1);
            return result;
        }
    }
}
