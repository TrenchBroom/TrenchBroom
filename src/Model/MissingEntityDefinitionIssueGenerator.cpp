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

#include "MissingEntityDefinitionIssueGenerator.h"

#include "StringUtils.h"
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
        class MissingEntityDefinitionIssue : public EntityIssue {
        public:
            static const IssueType Type;
        public:
            MissingEntityDefinitionIssue(Entity* entity) :
            EntityIssue(Type, entity) {
                addSharedQuickFix(DeleteObjectQuickFix::instance());
            }
            
            String description() const {
                return "Entity definition for '" + entity()->classname() + "' not found";
            }
            
            void applyQuickFix(const QuickFix* quickFix, View::ControllerSPtr controller) {
                if (quickFix->type() == DeleteObjectQuickFix::Type)
                    static_cast<const DeleteObjectQuickFix*>(quickFix)->apply(entity(), controller);
            }
        };
        
        const IssueType MissingEntityDefinitionIssue::Type = Issue::freeType();
        
        IssueType MissingEntityDefinitionIssueGenerator::type() const {
            return MissingEntityDefinitionIssue::Type;
        }
        
        const String& MissingEntityDefinitionIssueGenerator::description() const {
            static const String description("Missing entity definition");
            return description;
        }
        
        Issue* MissingEntityDefinitionIssueGenerator::generate(Entity* entity) const {
            assert(entity != NULL);
            if (entity->definition() == NULL)
                return new MissingEntityDefinitionIssue(entity);
            return NULL;
        }
    }
}
