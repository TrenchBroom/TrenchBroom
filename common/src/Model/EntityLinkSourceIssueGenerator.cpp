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

#include "EntityLinkSourceIssueGenerator.h"

#include "Model/Entity.h"
#include "Model/EntityAttributes.h"
#include "Model/Issue.h"
#include "Model/IssueQuickFix.h"
#include "Model/MapFacade.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        class EntityLinkSourceIssueGenerator::EntityLinkSourceIssue : public EntityIssue {
        public:
            static const IssueType Type;
        public:
            EntityLinkSourceIssue(Entity* entity) :
            EntityIssue(entity) {}
            
            IssueType doGetType() const {
                return Type;
            }
            
            const String doGetDescription() const {
                return entity()->classname() + " has unused targetname key";
            }
        };
        
        const IssueType EntityLinkSourceIssueGenerator::EntityLinkSourceIssue::Type = Issue::freeType();

        class EntityLinkSourceIssueGenerator::EntityLinkSourceIssueQuickFix : public IssueQuickFix {
        public:
            EntityLinkSourceIssueQuickFix() :
            IssueQuickFix("Delete property") {}
        private:
            void doApply(MapFacade* facade, const IssueList& issues) const {
                facade->removeAttribute(AttributeNames::Targetname);
            }
        };
        
        EntityLinkSourceIssueGenerator::EntityLinkSourceIssueGenerator() :
        IssueGenerator(EntityLinkSourceIssue::Type, "Missing entity link source") {
            addQuickFix(new EntityLinkSourceIssueQuickFix());
        }

        void EntityLinkSourceIssueGenerator::doGenerate(Entity* entity, IssueList& issues) const {
            if (entity->hasMissingSources())
                issues.push_back(new EntityLinkSourceIssue(entity));
        }
    }
}
