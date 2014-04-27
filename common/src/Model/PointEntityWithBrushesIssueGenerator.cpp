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

#include "PointEntityWithBrushesIssueGenerator.h"

#include "StringUtils.h"
#include "Assets/EntityDefinition.h"
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
        class PointEntityWithBrushesIssue : public EntityIssue {
        public:
            static const IssueType Type;
        public:
            PointEntityWithBrushesIssue(Entity* entity) :
            EntityIssue(Type, entity) {
                addSharedQuickFix(MoveBrushesToWorldspawnQuickFix::instance());
            }
            
            String description() const {
                return "Point entity contains brushes";
            }
            
            void applyQuickFix(const QuickFix* quickFix, View::ControllerSPtr controller) {
                if (quickFix->type() == MoveBrushesToWorldspawnQuickFix::Type)
                    static_cast<const MoveBrushesToWorldspawnQuickFix*>(quickFix)->apply(entity()->brushes(), controller);
            }
        };
        
        const IssueType PointEntityWithBrushesIssue::Type = Issue::freeType();
        
        IssueType PointEntityWithBrushesIssueGenerator::type() const {
            return PointEntityWithBrushesIssue::Type;
        }
        
        const String& PointEntityWithBrushesIssueGenerator::description() const {
            static const String description("Point entity with brushes");
            return description;
        }
        
        Issue* PointEntityWithBrushesIssueGenerator::generate(Entity* entity) const {
            assert(entity != NULL);
            const Assets::EntityDefinition* definition = entity->definition();
            if (definition != NULL && definition->type() == Assets::EntityDefinition::Type_PointEntity && !entity->brushes().empty())
                return new PointEntityWithBrushesIssue(entity);
            return NULL;
        }
    }
}
