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

#include "EmptyBrushEntityIssueGenerator.h"

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
        class EmptyBrushEntityIssueGenerator::EmptyBrushEntityIssue : public Issue {
        public:
            static const IssueType Type;
        public:
            EmptyBrushEntityIssue(Entity* entity) :
            Issue(entity) {}
        private:
            IssueType doGetType() const {
                return Type;
            }
            
            const String doGetDescription() const {
                const Entity* entity = static_cast<Entity*>(node());
                return "Entity '" + entity->classname() + "' does not contain any brushes";
            }
        };
        
        const IssueType EmptyBrushEntityIssueGenerator::EmptyBrushEntityIssue::Type = Issue::freeType();
        
        class EmptyBrushEntityIssueGenerator::EmptyBrushEntityIssueQuickFix : public IssueQuickFix {
        public:
            EmptyBrushEntityIssueQuickFix() :
            IssueQuickFix(EmptyBrushEntityIssue::Type, "Delete entities") {}
        private:
            void doApply(MapFacade* facade, const IssueList& issues) const {
                facade->deleteObjects();
            }
        };
        
        EmptyBrushEntityIssueGenerator::EmptyBrushEntityIssueGenerator() :
        IssueGenerator(EmptyBrushEntityIssue::Type, "Empty brush entity") {
            addQuickFix(new EmptyBrushEntityIssueQuickFix());
        }
        
        void EmptyBrushEntityIssueGenerator::doGenerate(Entity* entity, IssueList& issues) const {
            ensure(entity != NULL, "entity is null");
            const Assets::EntityDefinition* definition = entity->definition();
            if (definition != NULL && definition->type() == Assets::EntityDefinition::Type_BrushEntity && !entity->hasChildren())
                issues.push_back(new EmptyBrushEntityIssue(entity));
        }
    }
}
