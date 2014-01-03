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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "EntityLinkIssueGenerator.h"

#include "CollectionUtils.h"
#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/EntityProperties.h"
#include "Model/Issue.h"
#include "Model/Object.h"
#include "Model/QuickFix.h"
#include "Model/SharedQuickFixes.h"
#include "View/ControllerFacade.h"
#include "View/ViewTypes.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        class EntityLinkIssue : public EntityIssue {
        public:
            static const IssueType Type;
        private:
            PropertyKey m_key;
        public:
            EntityLinkIssue(Entity* entity, const PropertyKey& key) :
            EntityIssue(Type, entity),
            m_key(key) {
                addSharedQuickFix(DeleteEntityPropertyQuickFix::instance());
            }
            
            String description() const {
                return entity()->classname() + " entity has missing target for key '" + m_key + "'";
            }
            
            void applyQuickFix(const QuickFix* quickFix, View::ControllerSPtr controller) {
                if (quickFix->type() == DeleteEntityPropertyQuickFix::Type)
                    static_cast<const DeleteEntityPropertyQuickFix*>(quickFix)->apply(entity(), m_key, controller);
            }
        };
        
        const IssueType EntityLinkIssue::Type = Issue::freeType();
        
        IssueType EntityLinkIssueGenerator::type() const {
            return EntityLinkIssue::Type;
        }
        
        const String& EntityLinkIssueGenerator::description() const {
            static const String description("Missing entity link target");
            return description;
        }

        Issue* EntityLinkIssueGenerator::generate(Entity* entity) const {
            assert(entity != NULL);
            const Model::PropertyKeyList missingLinkTargets = entity->findMissingLinkTargets();
            const Model::PropertyKeyList missingKillTargets = entity->findMissingKillTargets();
            
            Issue* issue = NULL;
            processKeys(entity, missingLinkTargets, issue);
            processKeys(entity, missingKillTargets, issue);
            
            return issue;
        }

        void EntityLinkIssueGenerator::processKeys(Entity* entity, const Model::PropertyKeyList& keys, Issue*& issue) const {
            Model::PropertyKeyList::const_iterator it, end;
            for (it = keys.begin(), end = keys.end(); it != end; ++it) {
                const Model::PropertyKey& key = *it;
                Issue* newIssue = new EntityLinkIssue(entity, key);
                if (issue == NULL)
                    issue = newIssue;
                else
                    newIssue->insertAfter(issue);
            }
        }
    }
}
