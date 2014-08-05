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
#include "Model/EntityProperties.h"
#include "Model/Issue.h"
#include "Model/QuickFix.h"
#include "Model/SharedQuickFixes.h"
#include "View/ControllerFacade.h"
#include "View/ViewTypes.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        class EntityLinkSourceIssue : public EntityIssue {
        public:
            static const IssueType Type;
        public:
            EntityLinkSourceIssue(Entity* entity) :
            EntityIssue(Type, entity) {
                addSharedQuickFix(DeleteEntityPropertyQuickFix::instance());
            }
            
            String description() const {
                return entity()->classname() + " entity has unused targetname key";
            }
            
            void applyQuickFix(const QuickFix* quickFix, View::ControllerSPtr controller) {
                if (quickFix->type() == DeleteEntityPropertyQuickFix::Type)
                    static_cast<const DeleteEntityPropertyQuickFix*>(quickFix)->apply(entity(), PropertyKeys::Targetname, controller);
            }
        };
        
        const IssueType EntityLinkSourceIssue::Type = Issue::freeType();

        IssueType EntityLinkSourceIssueGenerator::type() const {
            return EntityLinkSourceIssue::Type;
        }
        
        const String& EntityLinkSourceIssueGenerator::description() const {
            static const String description("Missing entity link source");
            return description;
        }
        
        void EntityLinkSourceIssueGenerator::generate(Entity* entity, IssueList& issues) const {
            assert(entity != NULL);
            if (entity->hasMissingSources())
                issues.push_back(new EntityLinkSourceIssue(entity));
        }
    }
}
