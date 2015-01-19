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

#include "EntityLinkTargetIssueGenerator.h"

#include "Model/Entity.h"
#include "Model/EntityAttributes.h"
#include "Model/Issue.h"
#include "Model/IssueQuickFix.h"
#include "Model/MapFacade.h"

#include <cassert>
#include <map>

namespace TrenchBroom {
    namespace Model {
        class EntityLinkTargetIssueGenerator::EntityLinkTargetIssue : public EntityIssue {
        public:
            friend class EntityLinkTargetIssueQuickFix;
        private:
            const AttributeName m_name;
        public:
            static const IssueType Type;
        public:
            EntityLinkTargetIssue(Entity* entity, const AttributeName& name) :
            EntityIssue(entity),
            m_name(name) {}
            
            IssueType doGetType() const {
                return Type;
            }
            
            const String doGetDescription() const {
                return entity()->classname() + " has missing target for key '" + m_name + "'";
            }
        };
        
        const IssueType EntityLinkTargetIssueGenerator::EntityLinkTargetIssue::Type = Issue::freeType();

        class EntityLinkTargetIssueGenerator::EntityLinkTargetIssueQuickFix : public IssueQuickFix {
        private:
            typedef std::map<AttributeName, NodeList> AttributeNameMap;
        public:
            EntityLinkTargetIssueQuickFix() :
            IssueQuickFix("Delete property") {}
        private:
            void doApply(MapFacade* facade, const IssueList& issues) const {
                const PushSelection selection(facade);
                removeAttributes(facade, collectEntities(issues));
            }
            
            AttributeNameMap collectEntities(const IssueList& issues) const {
                AttributeNameMap result;
                
                IssueList::const_iterator it, end;
                for (it = issues.begin(), end = issues.end(); it != end; ++it) {
                    const Issue* issue = *it;
                    assert(issue->type() == EntityLinkTargetIssue::Type);
                    const EntityLinkTargetIssue* targetIssue = static_cast<const EntityLinkTargetIssue*>(issue);
                    
                    const AttributeName& attributeName = targetIssue->m_name;
                    result[attributeName].push_back(targetIssue->m_node);
                }
                
                return result;
            }
            
            void removeAttributes(MapFacade* facade, const AttributeNameMap& namesToEntities) const {
                AttributeNameMap::const_iterator it, end;
                for (it = namesToEntities.begin(), end = namesToEntities.end(); it != end; ++it) {
                    const AttributeName& name = it->first;
                    const NodeList& nodes = it->second;
                    
                    facade->deselectAll();
                    facade->select(nodes);
                    facade->removeAttribute(name);
                }
            }
        };
        
        EntityLinkTargetIssueGenerator::EntityLinkTargetIssueGenerator() :
        IssueGenerator(EntityLinkTargetIssue::Type, "Missing entity link source") {
            addQuickFix(new EntityLinkTargetIssueQuickFix());
        }

        void EntityLinkTargetIssueGenerator::doGenerate(Entity* entity, IssueList& issues) const {
            processKeys(entity, entity->findMissingLinkTargets(), issues);
            processKeys(entity, entity->findMissingKillTargets(), issues);
        }

        void EntityLinkTargetIssueGenerator::processKeys(Entity* entity, const Model::AttributeNameList& names, IssueList& issues) const {
            Model::AttributeNameList::const_iterator it, end;
            for (it = names.begin(), end = names.end(); it != end; ++it) {
                const Model::AttributeName& name = *it;
                issues.push_back(new EntityLinkTargetIssue(entity, name));
            }
        }
    }
}
