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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TrenchBroom_LinkTargetIssueGenerator
#define TrenchBroom_LinkTargetIssueGenerator

#include "Model/IssueGenerator.h"
#include "Model/ModelTypes.h"

namespace TrenchBroom {
    namespace Model {
        class LinkTargetIssueGenerator : public IssueGenerator {
        private:
            class LinkTargetIssue;
            class LinkTargetIssueQuickFix;
        public:
            LinkTargetIssueGenerator();
        private:
            void doGenerate(AttributableNode* node, IssueArray& issues) const;
            void processKeys(AttributableNode* node, const Model::AttributeNameArray& names, IssueArray& issues) const;
        };
    }
}

#endif /* defined(TrenchBroom_LinkTargetIssueGenerator) */
