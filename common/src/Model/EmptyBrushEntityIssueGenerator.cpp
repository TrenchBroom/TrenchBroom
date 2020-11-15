/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "Ensure.h"
#include "Assets/EntityDefinition.h"
#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/Issue.h"
#include "Model/IssueQuickFix.h"
#include "Model/MapFacade.h"

#include <string>

namespace TrenchBroom {
    namespace Model {
        class EmptyBrushEntityIssueGenerator::EmptyBrushEntityIssue : public Issue {
        public:
            static const IssueType Type;
        public:
            explicit EmptyBrushEntityIssue(EntityNode* entity) :
            Issue(entity) {}
        private:
            IssueType doGetType() const override {
                return Type;
            }

            std::string doGetDescription() const override {
                const EntityNode* entity = static_cast<EntityNode*>(node());
                return "Entity '" + entity->name() + "' does not contain any brushes";
            }
        };

        const IssueType EmptyBrushEntityIssueGenerator::EmptyBrushEntityIssue::Type = Issue::freeType();

        class EmptyBrushEntityIssueGenerator::EmptyBrushEntityIssueQuickFix : public IssueQuickFix {
        public:
            EmptyBrushEntityIssueQuickFix() :
            IssueQuickFix(EmptyBrushEntityIssue::Type, "Delete entities") {}
        private:
            void doApply(MapFacade* facade, const IssueList& /* issues */) const override {
                facade->deleteObjects();
            }
        };

        EmptyBrushEntityIssueGenerator::EmptyBrushEntityIssueGenerator() :
        IssueGenerator(EmptyBrushEntityIssue::Type, "Empty brush entity") {
            addQuickFix(new EmptyBrushEntityIssueQuickFix());
        }

        void EmptyBrushEntityIssueGenerator::doGenerate(EntityNode* entityNode, IssueList& issues) const {
            ensure(entityNode != nullptr, "entity is null");
            const Assets::EntityDefinition* definition = entityNode->entity().definition();
            if (definition != nullptr && definition->type() == Assets::EntityDefinitionType::BrushEntity && !entityNode->hasChildren())
                issues.push_back(new EmptyBrushEntityIssue(entityNode));
        }
    }
}
