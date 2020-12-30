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

#include "SoftMapBoundsIssueGenerator.h"

#include "Model/Game.h"
#include "Model/BrushNode.h"
#include "Model/EntityNode.h"
#include "Model/Issue.h"
#include "Model/IssueQuickFix.h"
#include "Model/MapFacade.h"
#include "Model/WorldNode.h"

#include <kdl/memory_utils.h>

#include <optional>
#include <string>

namespace TrenchBroom {
    namespace Model {
        class SoftMapBoundsIssueGenerator::SoftMapBoundsIssue : public Issue {
        public:
            friend class SoftMapBoundsIssueQuickFix;
        public:
            static const IssueType Type;
        public:
            explicit SoftMapBoundsIssue(Node* node) :
            Issue(node) {}

            IssueType doGetType() const override {
                return Type;
            }
            std::string doGetDescription() const override {
                return "Object is out of soft map bounds";
            }
        };

        class SoftMapBoundsIssueGenerator::SoftMapBoundsIssueQuickFix : public IssueQuickFix {
        public:
            SoftMapBoundsIssueQuickFix() :
            IssueQuickFix(SoftMapBoundsIssue::Type, "Delete objects") {}
        private:
            void doApply(MapFacade* facade, const IssueList& /* issues */) const override {
                facade->deleteObjects();
            }
        };

        const IssueType SoftMapBoundsIssueGenerator::SoftMapBoundsIssue::Type = Issue::freeType();

        SoftMapBoundsIssueGenerator::SoftMapBoundsIssueGenerator(std::weak_ptr<Game> game, const WorldNode* world) :
        IssueGenerator(SoftMapBoundsIssue::Type, "Objects out of soft map bounds"),
        m_game(game),
        m_world(world) {
            addQuickFix(new SoftMapBoundsIssueQuickFix());
        }

        void SoftMapBoundsIssueGenerator::generateInternal(Node* node, IssueList& issues) const {
            auto game = kdl::mem_lock(m_game);
            const Game::SoftMapBounds bounds = game->extractSoftMapBounds(m_world->entity());

            if (!bounds.bounds.has_value()) {
                return;
            }
            if (!bounds.bounds->contains(node->logicalBounds())) {
                issues.push_back(new SoftMapBoundsIssue(node));
            }
        }

        void SoftMapBoundsIssueGenerator::doGenerate(EntityNode* entity, IssueList& issues) const {
            generateInternal(entity, issues);
        }

        void SoftMapBoundsIssueGenerator::doGenerate(BrushNode* brush, IssueList& issues) const {
            generateInternal(brush, issues);
        }
    }
}
