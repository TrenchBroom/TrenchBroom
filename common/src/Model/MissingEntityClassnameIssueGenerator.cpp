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

#include "MissingEntityClassnameIssueGenerator.h"

#include "StringUtils.h"
#include "Assets/EntityDefinition.h"
#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/Issue.h"
#include "Model/IssueQuickFix.h"
#include "Model/MapFacade.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        class MissingEntityClassnameIssueGenerator::MissingEntityClassnameIssue : public EntityIssue {
        public:
            static const IssueType Type;
        public:
            MissingEntityClassnameIssue(Entity* entity) :
            EntityIssue(entity) {}
        private:
            IssueType doGetType() const {
                return Type;
            }
            
            const String doGetDescription() const {
                return "Entity has no classname property";
            }
        };
        
        const IssueType MissingEntityClassnameIssueGenerator::MissingEntityClassnameIssue::Type = Issue::freeType();
        
        class MissingEntityClassnameIssueGenerator::MissingEntityClassnameIssueQuickFix : public IssueQuickFix {
        public:
            MissingEntityClassnameIssueQuickFix() :
            IssueQuickFix("Delete entities") {}
        private:
            void doApply(MapFacade* facade, const IssueList& issues) const {
                facade->deleteObjects();
            }
        };
        
        MissingEntityClassnameIssueGenerator::MissingEntityClassnameIssueGenerator() :
        IssueGenerator(MissingEntityClassnameIssue::Type, "Missing entity classname") {
            addQuickFix(new MissingEntityClassnameIssueQuickFix());
        }
        
        void MissingEntityClassnameIssueGenerator::doGenerate(Entity* entity, IssueList& issues) const {
            if (!entity->hasAttribute(AttributeNames::Classname))
                issues.push_back(new MissingEntityClassnameIssue(entity));
        }
    }
}
